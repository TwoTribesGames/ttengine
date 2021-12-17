#include <tt/profiler/SceneObjectProfiler.h>

#ifdef SCENEOBJECT_PROFILER_ENABLED

#include <tt/platform/tt_printf.h>

namespace tt {
namespace engine {
namespace scene {

static bool         g_sceneObjectProfiler_isFirst = true;
static u64          g_sceneObjectProfiler_startTime = 0;
static int          g_sceneObjectProfiler_totalModels = 0;

#ifdef SCENEOBJECT_PROFILER_LOG_TO_FILE
static tt::fs::File g_sceneObjectProfiler_log;
#endif


u64 sceneObjectProfiler_getTime()
{
	return tt::system::Time::getInstance()->getMicroSeconds();
}

void sceneObjectProfiler_init()
{
	if (g_sceneObjectProfiler_isFirst)
	{
		g_sceneObjectProfiler_startTime = sceneObjectProfiler_getTime();

#ifdef SCENEOBJECT_PROFILER_LOG_TO_FILE
		// Cannot close handle, but OS will do that ;-)
		tt::fs::FS::initFile(&g_sceneObjectProfiler_log);
		tt::fs::FS::openFile(&g_sceneObjectProfiler_log, 
			"sceneobject_profiler.log", "w");
#endif

		g_sceneObjectProfiler_isFirst = false;
	}
}

void sceneObjectProfiler_writeInfo(SceneObject* p_object, u64 p_loadTime, char p_mode)
{
	// time in millisecs
	float curTime = 
		(sceneObjectProfiler_getTime() - g_sceneObjectProfiler_startTime) / 
		(1024.0f*1024.0f);

	// Enter log message
	char status[1024];

	if (p_mode == '+')
	{
		g_sceneObjectProfiler_totalModels++;
	}
	else if (p_mode == '-')
	{
		g_sceneObjectProfiler_totalModels--;
	}

	std::sprintf(status, "%c %3.3f %8llu [%4d] %s\n", 
		p_mode,
		curTime,
		p_loadTime,
		g_sceneObjectProfiler_totalModels,
		p_object->getName().c_str());

#ifdef SCENEOBJECT_PROFILER_LOG_TO_FILE
	if (tt::fs::FS::isAvailable())
	{
		tt::fs::FS::writeFile(&g_sceneObjectProfiler_log, 
			status, (s32)strlen(status));
		tt::fs::FS::flush(&g_sceneObjectProfiler_log);
	}
	else
	{
		TT_Printf("Failed to write: %s\n", status);
	}
#else
	TT_Printf("[SceneObjectProfiler] %s", status);
#endif
}

// Namespace end
}
}
}

#endif // SCENEOBJECT_PROFILER_ENABLED
