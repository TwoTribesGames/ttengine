#include <visualboy/VisualBoy.h>

#include <tt/app/Application.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/thread/CriticalSection.h>

#include "Util.h"
#include "common/ConfigManager.h"
#include "common/Patch.h"
#include "gb/gb.h"
#include "gb/gbGlobals.h"
#include "gb/gbSound.h"
#include "gba/GBA.h"
#include "gba/Sound.h"
#include "sdl/filters.h"


//--------------------------------------------------------------------------------------------------
// Globals

int RGB_LOW_BITS_MASK = 0x821;
IMAGE_TYPE emulatorType = IMAGE_UNKNOWN;
int filterEnlarge = 2;
int filter = kScanlinesTV;
int frameSkip = 0;
int borderSize = 8;

u32 g_outputWidth = 0;
u32 g_outputHeight = 0;
u32 g_outputBBP = 0;
u32 g_outputPitch = 0;



struct EmulatedSystem emulator = 
{
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	false,
	0
};


//--------------------------------------------------------------------------------------------------
// Members

tt::thread::handle VisualBoy::ms_thread;
tt::thread::Mutex VisualBoy::ms_mutex;
tt::thread::Mutex VisualBoy::ms_drawMutex;
tt::thread::Semaphore VisualBoy::ms_waitForUpdate;
tt::engine::renderer::TexturePtr VisualBoy::ms_outputTexture;
tt::code::BufferPtr VisualBoy::ms_buffer;
bool VisualBoy::ms_inputEnabled = true;
bool VisualBoy::ms_threadShouldExit = false;
VisualBoy::Queue VisualBoy::ms_queue;
std::string VisualBoy::ms_batteryFilename;


//--------------------------------------------------------------------------------------------------
// Helper functions

static int sdlCalculateShift(uint32_t mask)
{
    int m = 0;

    while (mask) {
        m++;
        mask >>= 1;
    }

    return m - 5;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

void VisualBoy::initialize()
{
	TT_ASSERT(ms_outputTexture == nullptr);
	TT_ASSERT(ms_buffer == nullptr);
	TT_ASSERT(ms_thread == nullptr); 
	ms_outputTexture = tt::engine::renderer::TextureCache::get(tt::engine::EngineID(getVisualBoyTextureCRC()), true);
	// Let OS schedule this thread properly
	ms_thread = tt::thread::create(updateImpl, nullptr, false, 0L,
		tt::thread::priority_normal, tt::thread::Affinity_None, "VisualBoy");
}


void VisualBoy::deinitialize()
{
	ms_threadShouldExit = true;
	ms_waitForUpdate.signal();
	tt::thread::wait(ms_thread);
	ms_thread.reset();
	TT_ASSERT(ms_outputTexture == nullptr);
	TT_ASSERT(ms_buffer == nullptr);
}


void VisualBoy::load(const std::string& p_file)
{
	tt::thread::CriticalSection critSec(&ms_mutex);
	ms_queue.emplace_back(Action_Load, p_file);
}


bool VisualBoy::isLoaded()
{
	return emulatorType != IMAGE_UNKNOWN;
}


void VisualBoy::setSoundVolume(real p_volume)
{
	soundSetVolume(p_volume);
}


void VisualBoy::setFilter(const std::string& p_filter)
{
	tt::thread::CriticalSection critSec(&ms_mutex);
	ms_queue.emplace_back(Action_SetFilter, p_filter);
}


void VisualBoy::pause()
{
	if (isLoaded() && emulating == 1)
	{
		emulating = 0;
		soundPause();
	}
}


bool VisualBoy::isPaused()
{
	return isLoaded() && emulating == 0;
}


void VisualBoy::resume()
{
	if (isLoaded() && emulating == 0)
	{
		emulating = 1;
		soundResume();
	}
}


void VisualBoy::update()
{
	ms_waitForUpdate.signal();
}


void VisualBoy::updateForRender()
{
	if (ms_outputTexture == nullptr || ms_buffer == nullptr)
	{
		return;
	}
	{
		tt::thread::CriticalSection critSec(&ms_drawMutex);
		tt::engine::renderer::TexturePainter outputPainter(ms_outputTexture->lock());
		// FIXME: MR For some reason the texture has an offset of a few pixels, causing the lower right pixels to be cropped.
		// Account for this by offsetting it with these magic numbers
		outputPainter.setRegion(g_outputWidth, g_outputHeight, borderSize - 4, borderSize - 3, (u8*)ms_buffer->getData());
	}
}


void VisualBoy::unload()
{
	tt::thread::CriticalSection critSec(&ms_mutex);
	ms_queue.emplace_back(Action_Unload, std::string());
}


void VisualBoy::systemDrawScreen()
{
	tt::thread::CriticalSection critSec(&ms_drawMutex);
	
	TT_NULL_ASSERT(ms_buffer);
	TT_NULL_ASSERT(pix);
	TT_NULL_ASSERT(filterFunction);
	if (ms_buffer == nullptr)
	{
		return;
	}
	
	const int srcPitch = (systemSizeX+1) * g_outputBBP;
	
	filterFunction(pix + srcPitch, srcPitch, 0, (uint8_t*)ms_buffer->getData(),
		g_outputPitch, systemSizeX, systemSizeY);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void VisualBoy::loadImpl(const std::string& p_file)
{
	TT_NULL_ASSERT(ms_outputTexture);
	TT_ASSERT(ms_buffer == nullptr);
	
	unloadImpl();
	
	initSound();
	
	TT_ASSERT(emulating == 0);
	emulatorType = utilFindType(p_file.c_str());
	if (emulatorType == IMAGE_UNKNOWN)
	{
		return;
	}
	else if (emulatorType == IMAGE_GB)
	{
		const bool success = gbLoadRom(p_file.c_str());
		if (success == false)
		{
			return;
		}
		
		gbGetHardwareType();
		
		// used for the handling of the gb Boot Rom
		if (gbHardware & 7)
		{
			gbCPUInit(biosFileNameGB, useBios);
		}
		
		emulator = GBSystem;
		gbReset();
	}
	else if (emulatorType == IMAGE_GBA)
	{
		int size = CPULoadRom(p_file.c_str());
		if (size == 0)
		{
			return;
		}
		
		if (cpuSaveType == 0)
		{
			utilGBAFindSave(size);
		}
		else
		{
			saveType = cpuSaveType;
		}
		
		doMirroring(mirroringEnable);
		
		emulator = GBASystem;
		
		CPUInit(biosFileNameGBA, useBios);
		CPUReset();
	}
	
	// Setup video and audio and then start emulating
	initVideo();
	
	TT_ASSERT(ms_batteryFilename.empty());
	{
		const tt::fs::identifier fsid(tt::app::getApplication()->getSaveFsID());
		TT_ASSERT(fsid == 0); // load/write of savedata uses oldschool FILE io; so this MUST be id 0
		
		if (tt::fs::mountSaveVolume(tt::fs::OpenMode_Read, fsid))
		{
			const std::string& pathToFile(tt::fs::getSaveRootDir(fsid));
			const std::string  saveFilename(tt::fs::utils::getFileTitle(p_file) + ".sav");
			ms_batteryFilename = pathToFile + saveFilename;
			const bool success = emulator.emuReadBattery(ms_batteryFilename.c_str());
			tt::fs::unmountSaveVolume(fsid);
			
			if (success == false)
			{
				// Try loading 'shipped' save file
				const std::string path(tt::fs::utils::getDirectory(p_file) + saveFilename);
				emulator.emuReadBattery(path.c_str());
			}
		}
		else
		{
			TT_PANIC("Save volume cannot be mounted");
			ms_batteryFilename.clear();
		}
	}
	
	emulating = 1;
}


bool VisualBoy::setFilterImpl(const std::string& p_filter)
{
	for (int i = 0; i < kInvalidFilter; ++i)
	{
		if (std::string(getFilterName(i)) == p_filter)
		{
			filterEnlarge  = getFilterEnlargeFactor(i);
			g_outputWidth  = filterEnlarge * systemSizeX;
			g_outputHeight = filterEnlarge * systemSizeY;
			g_outputBBP    = systemColorDepth / 8;
			g_outputPitch  = g_outputWidth * g_outputBBP;
	
			filterFunction = initFilter(i, systemColorDepth, systemSizeX);
			TT_NULL_ASSERT(filterFunction);
			{
				tt::thread::CriticalSection critSec(&ms_mutex);
				const tt::math::Point2 outputSize(g_outputWidth + borderSize*2, g_outputHeight + borderSize*2);
				ms_buffer = tt::code::BufferPtr(new tt::code::Buffer(g_outputWidth * g_outputHeight * g_outputBBP));
				ms_outputTexture = tt::engine::renderer::Texture::createForText(outputSize);
				{
					tt::engine::renderer::TexturePainter painter(ms_outputTexture->lock());
					painter.clear();
				}
				memset((void*)ms_buffer->getData(), 0, ms_buffer->getSize());
			}
			return true;
		}
	}
	TT_PANIC("Cannot find filter '%s'", p_filter.c_str());
	return false;
}


void VisualBoy::initVideo()
{
	TT_ASSERT(emulating == 0);
	TT_ASSERT(emulatorType != IMAGE_UNKNOWN);
	systemSizeX = 320;
	systemSizeY = 240;
	if (emulatorType == IMAGE_GBA)
	{
		systemSizeX = 240;
		systemSizeY = 160;
		systemFrameSkip = frameSkip;
	}
	else if (emulatorType == IMAGE_GB)
	{
		if (gbBorderOn)
		{
			systemSizeX = 256;
			systemSizeY = 224;
			gbBorderLineSkip = 256;
			gbBorderColumnSkip = 48;
			gbBorderRowSkip = 40;
		}
		else
		{
			systemSizeX = 160;
			systemSizeY = 144;
			gbBorderLineSkip = 160;
			gbBorderColumnSkip = 0;
			gbBorderRowSkip = 0;
		}
		systemFrameSkip = gbFrameSkip;
	}
	
	systemColorDepth = 32;
	
	uint32_t rmask, gmask, bmask;
	
	rmask = 0x00FF0000;
	gmask = 0x0000FF00;
	bmask = 0x000000FF;

	systemRedShift = sdlCalculateShift(rmask);
	systemGreenShift = sdlCalculateShift(gmask);
	systemBlueShift = sdlCalculateShift(bmask);
	
	utilUpdateSystemColorMaps();
	
	setFilterImpl("TV Mode");
}


void VisualBoy::initSound()
{
	soundInit();
	gbSoundSetDeclicking(true);
}


void VisualBoy::deinitSound()
{
	gbSoundShutdown(); // for GB
	soundShutdown();   // for GBA (also calls System callback)
}


int VisualBoy::updateImpl(void* /*p_arg*/)
{
	while (ms_threadShouldExit == false)
	{
		ms_waitForUpdate.wait();
		
		Queue toProcess;
		{
			tt::thread::CriticalSection critSec(&ms_mutex);
			ms_queue.swap(toProcess);
		}
		
		for (auto& it : toProcess)
		{
			switch (it.first)
			{
			case Action_Load: loadImpl(it.second); break;
			case Action_SetFilter: setFilterImpl(it.second); break;
			case Action_Unload: unloadImpl(); break;
			default:
				TT_PANIC("Unhandled action '%d'", it.first);
				break;
			}
		}
		
		if (emulating)
		{
			// FIXME: MR: This * 2 hack wasn't present in the original code, but I have no idea how
			// it could emulate double speed properly then, since emuCount didn't change
			emulator.emuMain((gbSpeed && emulatorType == IMAGE_GB) ? emulator.emuCount * 2: emulator.emuCount);
			updateForRender();
		}
	}
	
	unloadImpl();
	ms_outputTexture.reset();
	ms_buffer.reset();
	
	return 0;
}


void VisualBoy::unloadImpl()
{
	emulating = 0;
	{
		tt::thread::CriticalSection critSec(&ms_mutex);
		ms_outputTexture = tt::engine::renderer::TextureCache::get(tt::engine::EngineID(getVisualBoyTextureCRC()), true);
		ms_buffer.reset();
	}
	emulatorType = IMAGE_UNKNOWN;
	deinitSound();
	
	if ((gbRom != nullptr || rom != nullptr) && ms_batteryFilename.empty() == false)
	{
		const tt::fs::identifier fsid(tt::app::getApplication()->getSaveFsID());
		TT_ASSERT(fsid == 0); // load/write of savedata uses oldschool FILE io; so this MUST be id 0
		if (tt::fs::mountSaveVolume(tt::fs::OpenMode_Write, fsid))
		{
			emulator.emuWriteBattery(ms_batteryFilename.c_str());
			if (tt::fs::canCommit(fsid))
			{
				tt::fs::commit(fsid);
			}
			tt::fs::unmountSaveVolume(fsid);
		}
		else
		{
			TT_PANIC("Save volume cannot be mounted");
		}
		
		emulator.emuCleanUp();
	}
	ms_batteryFilename.clear();
}


