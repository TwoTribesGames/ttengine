#ifndef INC_TT_PROFILER_SCENEOBJECT_PROFILER_H
#define INC_TT_PROFILER_SCENEOBJECT_PROFILER_H

#include <tt/profiler/SceneObjectProfilerConstants.h>

#ifdef SCENEOBJECT_PROFILER_ENABLED
#include <tt/engine/scene/SceneObject.h>

// Sanity Checks
#ifdef TT_BUILD_FINAL
#error SCENEOBJECT_PROFILER_ENABLED should not be set in TT_BUILD_FINAL
#endif

#if defined(SCENEOBJECT_PROFILER_LOG_TO_FILE) && !defined(TT_PLATFORM_WIN)
#error SCENEOBJECT_PROFILER_LOG_TO_FILE should only be enabled on TT_PLATFORM_WIN
#endif

#include <tt/fs/FS.h>
#include <tt/system/Time.h>

namespace tt {
namespace engine {
namespace scene {

u64 sceneObjectProfiler_getTime();
void sceneObjectProfiler_init();
void sceneObjectProfiler_writeInfo(SceneObject* p_texture, u64 p_loadTime, char p_mode);


// Namespace end
}
}
}

#endif // SCENEOBJECT_PROFILER_ENABLED

#endif // INC_TT_PROFILER_SCENEOBJECT_PROFILER_H
