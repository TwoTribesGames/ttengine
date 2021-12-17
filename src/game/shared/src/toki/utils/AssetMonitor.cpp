#include <tt/code/BitMask.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/platform/tt_error.h>
#include <tt/pres/PresentationCache.h>
#include <tt/thread/CriticalSection.h>

#include <toki/game/Game.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/utils/AssetMonitor.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace utils {


//--------------------------------------------------------------------------------------------------
// Helper functions

static int staticAssetMonitorThread(void* p_arg)
{
	AssetMonitor* ptr = reinterpret_cast<AssetMonitor*>(p_arg);
	ptr->run();
	return 0;
}

//--------------------------------------------------------------------------------------------------
// Public member functions

AssetMonitor::AssetMonitor()
:
m_thread(),
m_mutex(),
m_changedBitMask(),
m_threadShouldExit(false),
m_isLockedForReload(false)
{
}


AssetMonitor::~AssetMonitor()
{
	stop();
}


void AssetMonitor::lockForReload()
{
	TT_ASSERT(m_thread != 0);
	TT_ASSERT(m_isLockedForReload == false);
	
	m_mutex.lock();
	m_isLockedForReload = true;
}


void AssetMonitor::unlockFromReload()
{
	TT_ASSERT(m_thread != 0);
	TT_ASSERT(m_isLockedForReload);
	
	m_isLockedForReload = false;
	m_mutex.unlock();
}


void AssetMonitor::signalAssetReloadCompleted(AssetType p_type)
{
	TT_ASSERT(m_thread != 0);
	TT_ASSERT(m_isLockedForReload);
	
	m_changedBitMask.resetFlag(p_type);
}


bool AssetMonitor::shouldReloadAssets(AssetType p_type) const
{
	TT_ASSERT(m_isLockedForReload);
	
	return m_changedBitMask.checkFlag(p_type);
}


void AssetMonitor::start()
{
	TT_ASSERT(m_thread == 0);
	m_threadShouldExit = false;
	m_thread =
			tt::thread::create(staticAssetMonitorThread, this, false, 0, 
			tt::thread::priority_below_normal, tt::thread::Affinity_None, "Asset Monitor Thread");
}


void AssetMonitor::stop()
{
	if (m_thread != 0)
	{
		// Tell asset monitor thread to exit and wait for the thread to die
		{
			tt::thread::CriticalSection criticalSection(&m_mutex);
			m_threadShouldExit = true;
		}
		tt::thread::wait(m_thread); // join
		m_thread.reset();
	}
}


void AssetMonitor::run()
{
	if (m_thread == 0)
	{
		TT_PANIC("Thread not started");
		return;
	}
	
	for (;;)
	{
		const s32 sleepTime = 500;
		
		if (m_threadShouldExit)
		{
			break;
		}
		
		if (AppGlobal::hasGame() == false)
		{
			// Don't do anthing if there is no game
			tt::thread::sleep(sleepTime);
			continue;
		}
		
		// Check for changed textures
		{
			tt::engine::EngineIDs engineIDs = tt::engine::renderer::TextureCache::getEngineIDs();
			if (tt::engine::renderer::TextureCache::checkForChanges(engineIDs))
			{
				tt::thread::CriticalSection criticalSection(&m_mutex);
				m_changedBitMask.setFlag(AssetType_Texture);
			}
		}
		
		// Check for changed presentations
		{
			tt::str::Strings filenames = tt::pres::PresentationCache::getFilenames();
			if (tt::pres::PresentationCache::checkForChanges(filenames))
			{
				tt::thread::CriticalSection criticalSection(&m_mutex);
				m_changedBitMask.setFlag(AssetType_Presentation);
			}
		}
		
		tt::thread::sleep(sleepTime);
	}
}


// Namespace end
}
}
