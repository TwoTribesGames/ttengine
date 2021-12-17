#ifndef INC_TT_PROFILER_TEXTURE_PROFILER_H
#define INC_TT_PROFILER_TEXTURE_PROFILER_H

#include <tt/profiler/TextureProfilerConstants.h>

#ifdef TEXTURE_PROFILER_ENABLED
#include <tt/engine/renderer/Texture.h>

// Sanity Checks
#ifdef TT_BUILD_FINAL
#error TEXTURE_PROFILER_ENABLED should not be set in TT_BUILD_FINAL
#endif

#if defined(TEXTURE_PROFILER_LOG_TO_FILE) && !defined(TT_PLATFORM_WIN)
#error TEXTURE_PROFILER_LOG_TO_FILE should only be enabled on TT_PLATFORM_WIN
#endif

#include <tt/fs/FS.h>
#include <tt/system/Time.h>

namespace tt {
namespace engine {
namespace renderer {

u64 textureProfiler_getTime();
void textureProfiler_init();
void textureProfiler_writeInfo(Texture* p_texture, u64 p_loadTime, char p_mode);

// Namespace end
}
}
}

#endif // TEXTURE_PROFILER_ENABLED

#endif // INC_TT_PROFILER_TEXTURE_PROFILER_H
