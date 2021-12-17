#if !defined(INC_TOKI_CONSTANTS_H)
#define INC_TOKI_CONSTANTS_H


#include <tt/code/BitMask.h>
#include <tt/platform/tt_error.h>


namespace toki /*! */{


enum DebugRender
{
	DebugRender_EntityCollisionRect,
	DebugRender_EntityTileRect,
	DebugRender_EntityRegisteredRect,
	DebugRender_EntityMoveToRect,
	DebugRender_DisableDebugRenderForEntityWithParent,
	DebugRender_EntityRegisteredTiles,
	DebugRender_EntitySightSensorShapes,
	DebugRender_EntityTouchSensorShapes,
	DebugRender_EntitySensorShapesText,
	DebugRender_EntityTileSensorShapes,
	DebugRender_EntityVibrationDetectionPoints,
	DebugRender_EntitySightDetectionPoints,
	DebugRender_EntityLightDetectionPoints,
	DebugRender_EntityTouchShape,
	DebugRender_EntityMissingFrame,
	DebugRender_EntityPosition,
	DebugRender_EntityCollisionParent,
	DebugRender_Event,
	DebugRender_Light,
	DebugRender_LevelBorder,
	DebugRender_SectionProfiler,
	DebugRender_SectionProfilerTwo,
	
	DebugRender_Disable_PresentationInFrontOfEntities,
	DebugRender_Disable_Fog,
	
	DebugRender_PathMgrAgents,
	DebugRender_PathMgrNavMesh,
	DebugRender_PathMgrContours,
	DebugRender_PathMgrCompactHeightfield,
	DebugRender_PathMgrHeightfield,
	DebugRender_PathMgrPolyMesh,
	DebugRender_PathMgrObstacles,
	
	DebugRender_RenderScreenspace,
	DebugRender_RenderEffectRects,
	DebugRender_RenderCullingRects,
	DebugRender_RenderParticleRects,
	
	DebugRender_Count
};
typedef tt::code::BitMask<DebugRender, DebugRender_Count> DebugRenderMask;


enum Screen
{
	Screen_TV,
	Screen_DRC,
	
	Screen_Count
};
inline bool isValidScreen(Screen p_screen) { return p_screen >= 0 && p_screen < Screen_Count; }


/*! \brief The registry and checkpoints stored per ProgressType. */
enum ProgressType
{
	ProgressType_Main,      //!< Progress in the main game. (normal progress)
	ProgressType_UserLevel, //!< Progress when playing a userlevel.
	
	ProgressType_Count,
	ProgressType_Invalid
};


inline bool isValidProgressType(ProgressType p_progressType)
{
	return p_progressType >= 0 && p_progressType < ProgressType_Count;
}


// Controls whether loading should happen in a separate loading thread
#define USE_THREADED_LOADING 1


// A panic for situations that aren't fatal (sanity checks etc):
#define TT_NONFATAL_PANIC TT_PANIC


#if !defined(TT_DEMO_BUILD)
#	error Expected a TT_DEMO_BUILD define. Please make sure the build settings for this configuration includes TT_DEMO_BUILD=0 or TT_DEMO_BUILD=1 as part of the preprocessor definitions.
#endif

// Namespace end
}


#endif  // !defined(INC_TOKI_CONSTANTS_H)
