#include <tt/profiler/TextureProfiler.h>

#ifdef TEXTURE_PROFILER_ENABLED

#include <tt/platform/tt_printf.h>

namespace tt {
namespace engine {
namespace renderer {

static bool         g_textureProfiler_isFirst = true;
static u64          g_textureProfiler_startTime = 0;
static std::size_t  g_textureProfiler_usedMemory = 0;

#ifdef TEXTURE_PROFILER_LOG_TO_FILE
static tt::fs::File g_textureProfiler_log;
#endif


u64 textureProfiler_getTime()
{
	return tt::system::Time::getInstance()->getMicroSeconds();
}

void textureProfiler_init()
{
	if (g_textureProfiler_isFirst)
	{
		g_textureProfiler_startTime = textureProfiler_getTime();

#ifdef TEXTURE_PROFILER_LOG_TO_FILE
		// Cannot close handle, but OS will do that ;-)
		tt::fs::FS::initFile(&g_textureProfiler_log);
		tt::fs::FS::openFile(&g_textureProfiler_log, 
			"texture_profiler.log", "w");
#endif

		g_textureProfiler_isFirst = false;
	}
}

void textureProfiler_writeInfo(Texture* p_texture, u64 p_loadTime, char p_mode)
{
	std::size_t textureSize = static_cast<std::size_t>
		((p_texture->getWidth() * p_texture->getHeight() * 4));

	// time in millisecs
	float curTime = 
		(textureProfiler_getTime() - g_textureProfiler_startTime) / 
		(1024.0f*1024.0f);

	if (p_mode == '+')
	{
		g_textureProfiler_usedMemory += textureSize;
	}
	else if (p_mode == '-')
	{
		g_textureProfiler_usedMemory -= textureSize;
	}

	// Enter log message
	char status[1024];

	std::sprintf(status, "%c %3.3f %8llu [%8d] [%2.3f MB] %s\n", 
		p_mode,
		curTime,
		p_loadTime,
		textureSize, 
		(float)g_textureProfiler_usedMemory / (1024 * 1024), 
		p_texture->getEngineID().toString().c_str());

#ifdef TEXTURE_PROFILER_LOG_TO_FILE
	if (tt::fs::FS::isAvailable())
	{
		tt::fs::FS::writeFile(&g_textureProfiler_log, 
			status, (s32)strlen(status));
		tt::fs::FS::flush(&g_textureProfiler_log);
	}
	else
	{
		TT_Printf("Failed to write: %s\n", status);
	}
#else
	TT_Printf("[TextureProfiler] %s", status);
#endif
}

// Namespace end
}
}
}

#endif // TEXTURE_PROFILER_ENABLED
