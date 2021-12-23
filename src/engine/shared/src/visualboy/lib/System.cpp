#include <tt/engine/renderer/TexturePainter.h>
#if defined(TT_PLATFORM_SDL)
#include <tt/input/KeyboardController.h>
#include <tt/input/SDLJoypadController.h>
#elif defined(TT_PLATFORM_WIN)
#include <tt/input/KeyboardController.h>
#include <tt/input/Xbox360Controller.h>
#endif
#include <tt/code/bufferutils.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/thread/CriticalSection.h>

#include <visualboy/VisualBoy.h>

#include <tt/snd/StreamSource.h>
#include <tt/snd/Stream.h>
#include <tt/snd/types.h>

#include "gba/Globals.h"
#include "gba/Sound.h"
#include "gb/gbGlobals.h"
#include "gb/gbSound.h"
#include "common/SoundDriver.h"

#include "Util.h"
#include "System.h"

//--------------------------------------------------------------------------------------------------
// Globals

int emulating = 0;
int systemSpeed = 0;
int systemRedShift = 0;
int systemBlueShift = 0;
int systemGreenShift = 0;
int systemColorDepth = 0;
int systemVerbose = 0;
int systemFrameSkip = 0;
int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
uint32_t systemColorMap32[0x10000];
uint16_t systemColorMap16[0x10000];
uint16_t systemGbPalette[24];
int systemSizeX = 0;
int systemSizeY = 0;


//--------------------------------------------------------------------------------------------------
// VisualBoyPlayer helper class

class VisualBoyPlayer : public SoundDriver, public tt::snd::StreamSource
{
public:
	VisualBoyPlayer()
	:
	tt::snd::StreamSource(),
	m_buffer(),
	m_bufferReadContext(),
	m_bufferWriteContext(),
	m_playing(false),
	m_stream(nullptr),
	m_sampleRate(0),
	m_mutex(),
	m_isBuffering(true)
	{
	}
	
	~VisualBoyPlayer()
	{
		if (m_stream != nullptr)
		{
			if (m_stream->isPlaying())
			{
				m_stream->stop();
			}
			m_stream.reset();
			m_playing = false;
		}
	}
	
	inline tt::snd::size_type getSizePerFrame() const
	{
		return getBufferSize() * (getSampleSize() / 8) * getChannelCount();
	}
	
	static inline tt::code::BufferWriteContext::StatusCode loopBuffer(tt::code::BufferWriteContext* p_context)
	{
		TT_NULL_ASSERT(p_context);
		if (p_context == 0)
		{
			return 1;
		}
		TT_ASSERTMSG(p_context->cursor == p_context->end, "Should only refill if buffer is full.");
		p_context->cursor = p_context->start;
		return 0;
	}
	
	static inline tt::code::BufferReadContext::StatusCode loopBuffer(tt::code::BufferReadContext* p_context)
	{
		TT_NULL_ASSERT(p_context);
		if (p_context == 0)
		{
			return 1;
		}
		TT_ASSERTMSG(p_context->cursor == p_context->end, "Should only refill if buffer is full.");
		p_context->cursor = p_context->start;
		return 0;
	}
	
	virtual bool init(long p_sampleRate)
	{
		TT_ASSERT(m_stream == nullptr);
		if (tt::snd::hasSoundSystem(0) == false)
		{
			return false;
		}
		
		m_sampleRate = p_sampleRate;
		m_stream = tt::snd::openStream(this, 0);
		TT_ASSERT(m_stream != nullptr);
		
		m_buffer.reset(new tt::code::Buffer(getSizePerFrame() * FramesInBuffer) );
		m_bufferReadContext = m_buffer->getReadContext();
		m_bufferReadContext.refillFunc = VisualBoyPlayer::loopBuffer;
		
		m_bufferWriteContext = m_buffer->getWriteContext();
		m_bufferWriteContext.refillFunc = VisualBoyPlayer::loopBuffer;
		m_isBuffering = true;
		return true;
	}
	virtual void pause()
	{
		if (tt::snd::hasSoundSystem(0) == false)
		{
			return;
		}
		
		TT_NULL_ASSERT(m_stream);
		if (m_playing && m_stream != nullptr)
		{
			m_stream->pause();
		}
	}
	virtual void reset()
	{
		if (tt::snd::hasSoundSystem(0) == false)
		{
			return;
		}
		
		TT_NULL_ASSERT(m_stream);
		if (m_stream != nullptr)
		{
			if (m_stream->isPlaying())
			{
				m_stream->stop();
			}
			m_stream.reset();
			m_playing = false;
		}
		init(soundGetSampleRate());
	}
	virtual void resume()
	{
		if (tt::snd::hasSoundSystem(0) == false)
		{
			return;
		}
		
		TT_NULL_ASSERT(m_stream);
		if (m_stream != nullptr)
		{
			m_playing ? m_stream->resume() : m_stream->play();
			m_playing = true;
		}
	}
	virtual void write(uint16_t *p_finalWave, int p_length)
	{
		if (m_buffer == nullptr)
		{
			return;
		}
		
		TT_ASSERT(getSizePerFrame() == p_length);
		{
			if (getDistance() <= ReadBuffer)
			{
				namespace bu = tt::code::bufferutils;
				tt::thread::CriticalSection critSec(&m_mutex);
				bu::put(reinterpret_cast<const u8*>(p_finalWave), p_length, &m_bufferWriteContext);
			}
			// else drop write (emulator runs faster than 60 fps)
		}
	}
	
	tt::snd::size_type getDistance()
	{
		const tt::snd::size_type size(getSizePerFrame());
		tt::thread::CriticalSection critSec(&m_mutex);
		auto writeAt((m_bufferWriteContext.cursor - m_bufferWriteContext.start) / size);
		auto readAt((m_bufferReadContext.cursor - m_bufferReadContext.start) / size);
		return readAt <= writeAt ? writeAt - readAt :
			(FramesInBuffer - readAt) + writeAt;
	}
	
	virtual tt::snd::size_type fillBuffer(tt::snd::size_type  p_frames,
	                                      tt::snd::size_type  p_channels,
	                                      void**              p_buffer,
	                                      tt::snd::size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT)
	{
		TT_PANIC("Not implemented");
		return 0;
	}
	
	virtual tt::snd::size_type fillBufferInterleaved(tt::snd::size_type  p_frames,
	                                                 tt::snd::size_type  p_channels,
	                                                 void*               p_buffer,
	                                                 tt::snd::size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT)
	{
		if (m_buffer == nullptr)
		{
			return 0;
		}
		
		const tt::snd::size_type size(getSizePerFrame());
		TT_ASSERT((p_frames * p_channels * (getSampleSize() / 8)) == size);

		tt::snd::size_type distance(getDistance());
		if (distance == 0)
		{
			m_isBuffering = true;
		}
		else if (distance >= ReadBuffer)
		{
			m_isBuffering = false;
		}
		
		if (m_isBuffering)
		{
			// write silence
			tt::mem::fill8(p_buffer, 0, size);
		}
		else
		{
			namespace bu = tt::code::bufferutils;
			tt::thread::CriticalSection critSec(&m_mutex);
			bu::get((u8*)p_buffer, size, &m_bufferReadContext);
		}
		
		return p_frames;
	}
	
	virtual tt::snd::size_type getSampleSize() const
	{
		return 16;
	}
	
	virtual tt::snd::size_type getBufferSize() const
	{
		return m_sampleRate / 60; // 1 frame buffer
	}
	
	virtual tt::snd::size_type getFramerate() const
	{
		return m_sampleRate;
	}
	
	virtual tt::snd::size_type getChannelCount() const
	{
		return 2;
	}
	
private:
	enum { ReadBuffer = 3, FramesInBuffer = 10 };
	
	tt::code::BufferPtr m_buffer;
	tt::code::BufferReadContext m_bufferReadContext;
	tt::code::BufferWriteContext m_bufferWriteContext;
	bool m_playing;
	tt::snd::StreamPtr m_stream;
	tt::snd::size_type m_sampleRate;
	tt::thread::Mutex  m_mutex;
	bool m_isBuffering;
};


//--------------------------------------------------------------------------------------------------
// Functions

bool systemPauseOnFrame()
{
	return false;
}

void systemGbPrint(uint8_t *, int, int, int, int, int)
{
}


void systemScreenCapture(int)
{
}


void systemDrawScreen()
{
	VisualBoy::systemDrawScreen();
}


bool systemReadJoypads()
{
	return true;
	// FIXME: MR
	//return VisualBoy::isInputEnabled();
}


enum GBKeys
{
	GBKeys_A      = 1,
	GBKeys_B      = 2,
	GBKeys_Select = 4,
	GBKeys_Start  = 8,
	GBKeys_Right  = 16,
	GBKeys_Left   = 32,
	GBKeys_Up     = 64,
	GBKeys_Down   = 128,
	GBKeys_RB     = 256,
	GBKeys_LB     = 512
};


uint32_t systemReadJoypad(int p_index)
{
	using namespace tt::input;
	const ControllerIndex idx(p_index < ControllerIndex_One || p_index > ControllerIndex_Four ?
		ControllerIndex_One :
		static_cast<ControllerIndex>(p_index));
	
	bool isUpDown    = false;
	bool isDownDown  = false;
	bool isLeftDown  = false;
	bool isRightDown = false;
	bool isXDown     = false;
	bool isYDown     = false;
	bool isADown     = false;
	bool isBDown     = false;
	bool isRDown     = false;
	bool isLDown     = false;
	tt::input::Stick stick;

#if defined(TT_PLATFORM_SDL)
    const SDLJoypadController& ctrl(SDLJoypadController::getState(idx));
	const KeyboardController& kbd(KeyboardController::getState(ControllerIndex_One));

	isUpDown    = ctrl.up.down;
	isDownDown  = ctrl.down.down;
	isLeftDown  = ctrl.left.down;
	isRightDown = ctrl.right.down;
	isXDown     = ctrl.y.down;
	isBDown     = ctrl.a.down;
	isYDown     = ctrl.b.down;
	isADown     = ctrl.x.down;
	isRDown     = ctrl.r.down || ctrl.rtrig.value > 0.5f;
	isLDown     = ctrl.l.down || ctrl.ltrig.value > 0.5f;
	stick       = ctrl.lstick;
#elif defined(TT_PLATFORM_WIN)
	const Xbox360Controller& ctrl(Xbox360Controller::getState(idx));
	const KeyboardController& kbd(KeyboardController::getState(ControllerIndex_One));
	
	isUpDown    = ctrl.up.down;
	isDownDown  = ctrl.down.down;
	isLeftDown  = ctrl.left.down;
	isRightDown = ctrl.right.down;
	isXDown     = ctrl.y.down;
	isBDown     = ctrl.a.down;
	isYDown     = ctrl.b.down;
	isADown     = ctrl.x.down;
	isRDown     = ctrl.r.down || ctrl.rtrig.value > 0.5f;
	isLDown     = ctrl.l.down || ctrl.ltrig.value > 0.5f;
	stick       = ctrl.lstick;
#endif
	uint32_t res = 0;

	// Shared controller lstick
	{
		const Stick::Direction8& direction(ctrl.lstick.getNormalizedStick().getDirection8());
		switch (direction)
		{
		case Stick::Direction8_Up:        res |= GBKeys_Up; break;
		case Stick::Direction8_UpRight:   res |= GBKeys_Up | GBKeys_Right; break;
		case Stick::Direction8_Right:     res |= GBKeys_Right; break;
		case Stick::Direction8_DownRight: res |= GBKeys_Down | GBKeys_Right; break;
		case Stick::Direction8_Down:      res |= GBKeys_Down; break;
		case Stick::Direction8_DownLeft:  res |= GBKeys_Down | GBKeys_Left; break;
		case Stick::Direction8_Left:      res |= GBKeys_Left; break;
		case Stick::Direction8_UpLeft:    res |= GBKeys_Up | GBKeys_Left; break;
		case Stick::Direction8_None:
		default:
			// Do nothing
			break;
		}
	}
	
	// Shared controller buttons
	if (isADown)     res |= GBKeys_A;
	if (isBDown)     res |= GBKeys_B;
	if (isXDown)     res |= GBKeys_Start;
	if (isYDown)     res |= GBKeys_Select;
	if (isRightDown) res |= GBKeys_Right;
	if (isLeftDown)  res |= GBKeys_Left;
	if (isUpDown)    res |= GBKeys_Up;
	if (isDownDown)  res |= GBKeys_Down;
	if (isRDown)     res |= GBKeys_RB;
	if (isLDown)     res |= GBKeys_LB;
	
	// Platform specific
#if defined(TT_PLATFORM_WIN)
	// Keyboard
	if (kbd.keys[tt::input::Key_Enter].down)     res |= GBKeys_A;
	if (kbd.keys[tt::input::Key_Backspace].down) res |= GBKeys_B;
	if (kbd.keys[tt::input::Key_Right].down)     res |= GBKeys_Right;
	if (kbd.keys[tt::input::Key_Left].down)      res |= GBKeys_Left;
	if (kbd.keys[tt::input::Key_Up].down)        res |= GBKeys_Up;
	if (kbd.keys[tt::input::Key_Down].down)      res |= GBKeys_Down;
	if (kbd.keys[tt::input::Key_Home].down)      res |= GBKeys_Select;
	if (kbd.keys[tt::input::Key_End].down)       res |= GBKeys_Start;
#endif

	return res;
}


uint32_t systemGetClock()
{
	return 0;
}


void systemMessage(int, const char *, ...)
{
}


void systemSetTitle(const char *)
{
}


SoundDriver *systemSoundInit()
{
	gbSoundShutdown(); // for GB
	soundShutdown();   // for GBA (also calls System callback)
	
	return new VisualBoyPlayer();
}


void systemOnWriteDataToSoundBuffer(const uint16_t *finalWave, int length)
{
}


void systemOnSoundShutdown()
{
}


void systemScreenMessage(const char *)
{
}


void systemUpdateMotionSensor()
{
}


int systemGetSensorX()
{
	return 0;
}


int systemGetSensorY()
{
	return 0;
}


int systemGetSensorZ()
{
	return 0;
}


uint8_t systemGetSensorDarkness()
{
	return 0xE8;
}


void systemCartridgeRumble(bool)
{
}


void systemPossibleCartridgeRumble(bool)
{
}


void updateRumbleFrame()
{
}


bool systemCanChangeSoundQuality()
{
	return false;
}


void systemShowSpeed(int)
{
}


void system10Frames(int)
{
}


void systemFrame()
{
}


void systemGbBorderOn()
{
}


void Sm60FPS_Init()
{
}


bool Sm60FPS_CanSkipFrame()
{
	return true;
}


void Sm60FPS_Sleep()
{
}


void DbgMsg(const char *msg, ...)
{
}
