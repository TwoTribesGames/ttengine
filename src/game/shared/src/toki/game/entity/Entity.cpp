#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/code/NoDeleter.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/fs/utils/utils.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>

#include <toki/game/entity/graphics/PowerBeamGraphic.h>
#include <toki/game/entity/graphics/PowerBeamGraphicMgr.h>
#include <toki/game/entity/graphics/TextLabel.h>
#include <toki/game/entity/graphics/TextLabelMgr.h>
#include <toki/game/entity/movementcontroller/MovementControllerMgr.h>
#include <toki/game/entity/sensor/SensorMgr.h>
#include <toki/game/entity/sensor/Shape.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/event/Event.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/light/DarknessMgr.h>
#include <toki/game/movement/MoveBase.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/EntityState.h>
#include <toki/game/script/TimerMgr.h>
#include <toki/game/DebugView.h>
#include <toki/game/Game.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/pres/PresentationObjectMgr.h>
#include <toki/serialization/utils.h>
#include <toki/AppGlobal.h>


// Macro to make toggling "collision parent" (carry movement) debug output simpler
#define TOKI_CARRYMOVEMENT_DEBUGGING 0

#if TOKI_CARRYMOVEMENT_DEBUGGING
#define CM_Printf TT_Printf
#else
#define CM_Printf(...)
#endif


namespace toki {
namespace game {
namespace entity {


static const real              g_rectResizeSmallValue       = 0.0001f;
static const real              g_rectResizeDoubleSmallValue = 2.0f * g_rectResizeSmallValue;
static const tt::math::Vector2 g_rectResizeSmallVec(g_rectResizeSmallValue, 0.0f);


// Implementation of inner helper class to make particles follow an entity with an offset

tt::math::Vector3 Entity::ParticleEntityFollower::getPosition() const
{
	const entity::Entity* entityPtr = m_entityToFollow.getPtr();
	if (entityPtr == 0)
	{
		return tt::math::Vector3::zero;
	}
	
	return entityPtr->getPositionForParticles(m_followOffset);
}


bool Entity::ParticleEntityFollower::isCulled() const
{
	const entity::Entity* entityPtr = m_entityToFollow.getPtr();
	if (entityPtr == 0)
	{
		return true;
	}
	
	return entityPtr->isPositionCulled();
}


//--------------------------------------------------------------------------------------------------
// Public member functions

const std::string& Entity::getType() const
{
	TT_NULL_ASSERT(m_entityScript);
	return m_entityScript->getType();
}


void Entity::setEntityScriptFromSerialize(const script::EntityBasePtr& p_script)
{
	TT_NULL_ASSERT(p_script);
	TT_ASSERT(m_entityScript == 0);
	m_entityScript = p_script;
	
	// Now that we have a script (and thus an entity type string) available again,
	// restore info based on entity type
	m_pathAgentRadius = -1.0f;
	m_pathCrowdSeparation = false;
	if (m_entityScript != 0)
	{
		const level::entity::EntityInfo* entityInfo = AppGlobal::getEntityLibrary().getEntityInfo(getType());
		TT_ASSERTMSG(entityInfo != 0,
		             "No EntityInfo is available for entity type '%s'.", getType().c_str());
		if (entityInfo != 0)
		{
			m_pathAgentRadius     = entityInfo->getPathFindAgentRadius();
			m_pathCrowdSeparation = entityInfo->hasPathCrowdSeparation();
		}
	}
}


void Entity::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	// NOTE: Entity has no creation params (is an empty struct)
}


Entity::CreationParams Entity::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	// NOTE: Entity has no creation params (is an empty struct)
	return CreationParams();
}


void Entity::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8>(m_state,                p_context); // State
	bu::put(m_suspended,                    p_context); // bool
	bu::put(m_pos,                          p_context); // tt::math::Vector2
	bu::putEnum<u8>(m_orientationDown,      p_context); // movement::Direction
	bu::put(m_orientationForwardIsLeft,     p_context); // bool
	bu::put(m_inScreenspace,                p_context); // bool
	bu::put(m_submergeDepth,                p_context); // s32
	bu::put(m_flowToTheRight,               p_context); // bool
	bu::put(m_worldRect,                    p_context); // tt::math::VectorRect
	bu::put(m_localRect,                    p_context); // tt::math::VectorRect
	bu::put(m_localTileRect,                p_context); // tt::math::VectorRect
	bu::put(m_willEndMoveHereRect,          p_context); // tt::math::PointRect
	bu::put(m_tileRegistrationEnabled,      p_context); // bool
	bu::put(m_registeredTileRect,           p_context); // tt::math::PointRect
	bu::put(m_updateSurvey,                 p_context); // bool
	m_survey.serialize(    p_context); // movement::SurroundingsSurvey 
	m_prevSurvey.serialize(p_context); // movement::SurroundingsSurvey 
	bu::put(m_debugShowTagChanges,          p_context); // bool
	bu::put(m_debugPresentationObjectIdx,   p_context); // u32
	bu::putHandle(m_movementControllerHandle, p_context); // movementcontroller::MovementControllerHandle
	
	{
		const u32 count = static_cast<u32>(m_presentationObjects.size());
		bu::put(count, p_context); // u32
		for (PresentationObjects::const_iterator it = m_presentationObjects.begin();
			it != m_presentationObjects.end(); ++it)
		{
			bu::putHandle((*it), p_context); // pres::PresentationObjectHandle
		}
	}
	
	{
		const u32 beamGraphicCount = static_cast<u32>(m_powerBeamGraphics.size());
		bu::put(beamGraphicCount, p_context); // u32
		for (PowerBeamGraphicHandles::const_iterator it = m_powerBeamGraphics.begin();
		     it != m_powerBeamGraphics.end(); ++it)
		{
			bu::putHandle((*it), p_context); // graphics::PowerBeamGraphicHandle
		}
	}
	
	{
		const u32 count = static_cast<u32>(m_textLabels.size());
		bu::put(count, p_context); // u32
		for (TextLabelHandles::const_iterator it = m_textLabels.begin();
		     it != m_textLabels.end(); ++it)
		{
			bu::putHandle((*it), p_context); // graphics::TextLabelHandles
		}
	}
	
	{
		const u32 effectRectCount = static_cast<u32>(m_effectRects.size());
		bu::put(effectRectCount, p_context);
		for (EffectRectHandles::const_iterator it = m_effectRects.begin();
		     it != m_effectRects.end(); ++it)
		{
			bu::putHandle((*it), p_context);
		}
	}
	
	const u32 sensorCount = static_cast<u32>(m_sensors.size());
	bu::put(sensorCount,                    p_context); // u32
	for (SensorHandles::const_iterator it = m_sensors.begin(); it != m_sensors.end(); ++it)
	{
		bu::putHandle((*it), p_context); // Handle
	}
	
	const u32 filteredBySensorCount = static_cast<u32>(m_filteredBySensors.size());
	bu::put(filteredBySensorCount, p_context); // u32
	for (SensorHandles::const_iterator it = m_filteredBySensors.begin(); it != m_filteredBySensors.end(); ++it)
	{
		bu::putHandle((*it), p_context); // Handle
	}
	
	const u32 tileSensorCount = static_cast<u32>(m_tileSensors.size());
	bu::put(tileSensorCount, p_context); // u32
	for (TileSensorHandles::const_iterator it = m_tileSensors.begin(); it != m_tileSensors.end(); ++it)
	{
		bu::putHandle((*it), p_context); // Handle
	}
	
	const u32 lightCount = static_cast<u32>(m_lights.size());
	bu::put(lightCount,                     p_context); // u32
	for (LightHandles::const_iterator it = m_lights.begin(); it != m_lights.end(); ++it)
	{
		bu::putHandle((*it), p_context); // u32 (handle)
	}
	bu::put(m_isDetectableByLight, p_context); // bool
	bu::put(m_isInLight,           p_context); // bool
	bu::put(m_isLightBlocking,     p_context); // bool
	bu::put(m_scriptIsInLight,     p_context); // bool
	bu::put(m_isDetectableBySight, p_context); // bool
	bu::put(m_isDetectableByTouch, p_context); // bool
	
	const u32 vibrationDetectionPointsCount = static_cast<u32>(m_vibrationDetectionPoints.size());
	bu::put(vibrationDetectionPointsCount, p_context); // u32
	for (DetectionPoints::const_iterator it = m_vibrationDetectionPoints.begin(); it != m_vibrationDetectionPoints.end(); ++it)
	{
		bu::put((*it), p_context);
	}
	
	const u32 sightDetectionPointsCount = static_cast<u32>(m_sightDetectionPoints.size());
	bu::put(sightDetectionPointsCount, p_context); // u32
	for (DetectionPoints::const_iterator it = m_sightDetectionPoints.begin(); it != m_sightDetectionPoints.end(); ++it)
	{
		bu::put((*it), p_context);
	}
	
	const u32 lightDetectionPointsCount = static_cast<u32>(m_lightDetectionPoints.size());
	bu::put(lightDetectionPointsCount, p_context); // u32
	for (DetectionPoints::const_iterator it = m_lightDetectionPoints.begin(); it != m_lightDetectionPoints.end(); ++it)
	{
		bu::put((*it), p_context);
	}
	
	const bool validTouchShape(m_touchShape != 0);
	bu::put(validTouchShape, p_context);
	if (validTouchShape)
	{
		m_touchShape->serialize(p_context);
		bu::put(m_touchShapeOffset, p_context); // tt::math::Vector2
	}
	
	bu::put(m_positionCullingEnabled,              p_context); // bool
	bu::putHandle<Entity>(m_positionCullingParent, p_context); // EntityHandle
	bu::put(m_positionCullingInitialized,          p_context); // bool
	bu::put(m_isPositionCulled,                    p_context); // bool
	bu::put(m_isOnScreen,                          p_context); // bool
	bu::put(m_canBePushed,                         p_context); // bool
	bu::put(m_canBeCarried,                        p_context); // bool
	bu::put(m_canBePaused,                         p_context); // bool
	EntityTiles::serialize(m_collisionTiles,       p_context); // EntityTilesPtr
	bu::put(m_collisionTilesActive,                p_context); // bool
	bu::put(m_collisionTilesRegdRect,              p_context); // tt::math::PointRect
	bu::put(m_collisionTileOffset,                 p_context); // tt::math::Point2
	// m_entityScript is saved by ScriptMgr.
	bu::put(m_scriptDefinedDebugString,      p_context); // std::string
	
	for (s32 i = 0; i < fluid::FluidType_Count; ++i)
	{
		bool hasSettings = m_fluidSettings[i] != 0;
		bu::put(hasSettings, p_context);
		if (hasSettings)
		{
			m_fluidSettings[i]->serialize(p_context);
		}
	}
}


void Entity::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	m_state                    = bu::getEnum<u8, State       >(p_context);
	m_suspended                = bu::get<bool                >(p_context);
	m_pos                      = bu::get<tt::math::Vector2   >(p_context);
	m_orientationDown          = bu::getEnum<u8, movement::Direction>(p_context);
	m_orientationForwardIsLeft = bu::get<bool                >(p_context);
	m_inScreenspace            = bu::get<bool                >(p_context);
	m_submergeDepth            = bu::get<s32                 >(p_context);
	m_flowToTheRight           = bu::get<bool                >(p_context);
	m_worldRect                = bu::get<tt::math::VectorRect>(p_context);
	m_localRect                = bu::get<tt::math::VectorRect>(p_context);
	m_localTileRect            = bu::get<tt::math::VectorRect>(p_context);
	m_willEndMoveHereRect      = bu::get<tt::math::PointRect >(p_context);
	m_tileRegistrationEnabled  = bu::get<bool                >(p_context);
	m_registeredTileRect       = bu::get<tt::math::PointRect >(p_context);
	m_updateSurvey             = bu::get<bool                >(p_context);
	m_survey.unserialize(    p_context); // movement::SurroundingsSurvey 
	m_prevSurvey.unserialize(p_context); // movement::SurroundingsSurvey
	m_debugShowTagChanges          = bu::get<bool       >(p_context);
	m_debugPresentationObjectIdx   = bu::get<u32        >(p_context);
	m_movementControllerHandle = bu::getHandle<movementcontroller::DirectionalMovementController>(p_context);
	
	{
		const u32 count = bu::get<u32>(p_context);
		m_presentationObjects.clear();
		m_presentationObjects.reserve(count);
		for (u32 i = 0; i < count; ++i)
		{
			m_presentationObjects.push_back(bu::getHandle<pres::PresentationObject>(p_context));
		}
	}
	
	{
		const u32 beamGraphicCount = bu::get<u32>(p_context);
		m_powerBeamGraphics.clear();
		m_powerBeamGraphics.reserve(beamGraphicCount);
		for (u32 i = 0; i < beamGraphicCount; ++i)
		{
			m_powerBeamGraphics.push_back(bu::getHandle<graphics::PowerBeamGraphic>(p_context));
		}
	}
	
	{
		const u32 count = bu::get<u32>(p_context);
		m_textLabels.clear();
		m_textLabels.reserve(count);
		for (u32 i = 0; i < count; ++i)
		{
			m_textLabels.push_back(bu::getHandle<graphics::TextLabel>(p_context));
		}
	}
	
	{
		const u32 effectRectCount = bu::get<u32>(p_context);
		m_effectRects.clear();
		m_effectRects.reserve(effectRectCount);
		for (u32 i = 0; i < effectRectCount; ++i)
		{
			m_effectRects.push_back(bu::getHandle<effect::EffectRect>(p_context));
		}
	}
	
	const u32 sensorCount = bu::get<u32>(p_context);
	m_sensors.clear();              // Out with the old.
	m_sensors.reserve(sensorCount); // In with the new.
	for (u32 i = 0; i < sensorCount; ++i)
	{
		m_sensors.push_back(bu::getHandle<sensor::Sensor>(p_context));
	}
	
	const u32 filteredBySensorsCount = bu::get<u32>(p_context);
	m_filteredBySensors.clear();              // Out with the old.
	m_filteredBySensors.reserve(filteredBySensorsCount ); // In with the new.
	for (u32 i = 0; i < filteredBySensorsCount ; ++i)
	{
		m_filteredBySensors.push_back(bu::getHandle<sensor::Sensor>(p_context));
	}
	
	const u32 tileSensorCount = bu::get<u32>(p_context);
	m_tileSensors.clear();
	m_tileSensors.reserve(tileSensorCount);
	for (u32 i = 0; i < tileSensorCount; ++i)
	{
		m_tileSensors.push_back(bu::getHandle<sensor::TileSensor>(p_context));
	}
	
	const u32 lightCount = bu::get<u32>(p_context);
	m_lights.clear();
	m_lights.reserve(lightCount);
	for (u32 i = 0; i < lightCount; ++i)
	{
		m_lights.push_back(bu::getHandle<light::Light>(p_context));
	}
	m_isDetectableByLight = bu::get<bool>(p_context);
	m_isInLight           = bu::get<bool>(p_context);
	m_isLightBlocking     = bu::get<bool>(p_context);
	m_scriptIsInLight     = bu::get<bool>(p_context);
	m_isDetectableBySight = bu::get<bool>(p_context);
	m_isDetectableByTouch = bu::get<bool>(p_context);
	
	const u32 vibrationDetectionPointsCount = bu::get<u32>(p_context);
	m_vibrationDetectionPoints.clear();              // Out with the old.
	m_vibrationDetectionPoints.reserve(vibrationDetectionPointsCount); // In with the new.
	for (u32 i = 0; i < vibrationDetectionPointsCount; ++i)
	{
		m_vibrationDetectionPoints.push_back(bu::get<tt::math::Vector2>(p_context));
	}
	
	const u32 sightDetectionPointsCount = bu::get<u32>(p_context);
	m_sightDetectionPoints.clear();              // Out with the old.
	m_sightDetectionPoints.reserve(sightDetectionPointsCount); // In with the new.
	for (u32 i = 0; i < sightDetectionPointsCount; ++i)
	{
		m_sightDetectionPoints.push_back(bu::get<tt::math::Vector2>(p_context));
	}
	
	const u32 lightDetectionPointsCount = bu::get<u32>(p_context);
	m_lightDetectionPoints.clear();              // Out with the old.
	m_lightDetectionPoints.reserve(lightDetectionPointsCount); // In with the new.
	for (u32 i = 0; i < lightDetectionPointsCount; ++i)
	{
		m_lightDetectionPoints.push_back(bu::get<tt::math::Vector2>(p_context));
	}
	
	m_touchShape.reset();
	const bool validTouchShape = bu::get<bool>(p_context);
	if (validTouchShape)
	{
		m_touchShape       = sensor::Shape::unserialize(p_context);
		m_touchShapeOffset = bu::get<tt::math::Vector2>(p_context);
	}
	
	m_positionCullingEnabled     = bu::get<bool>(p_context);
	m_positionCullingParent      = bu::getHandle<Entity>(p_context);
	m_positionCullingInitialized = bu::get<bool>(p_context);
	m_isPositionCulled           = bu::get<bool>(p_context);
	m_isOnScreen                 = bu::get<bool>(p_context);
	m_canBePushed                = bu::get<bool>(p_context);
	m_canBeCarried               = bu::get<bool>(p_context);
	m_canBePaused                = bu::get<bool>(p_context);
	m_collisionTiles             = EntityTiles::unserialize(p_context); // EntityTilesPtr
	m_collisionTilesActive       = bu::get<bool               >(p_context);
	m_collisionTilesRegdRect     = bu::get<tt::math::PointRect>(p_context);
	m_collisionTileOffset        = bu::get<tt::math::Point2   >(p_context);
	m_entityScript.reset(); // m_entityScript is restored by ScriptMgr.
	m_scriptDefinedDebugString   = bu::get<std::string>(p_context);
	
	for (s32 i = 0; i < fluid::FluidType_Count; ++i)
	{
		bool hasSettings = bu::get<bool>(p_context);
		if (hasSettings)
		{
			m_fluidSettings[i] = fluid::FluidSettings::unserialize(p_context);
		}
	}
	
	// Restore TileRegistrationMgr registrations.
	if (AppGlobal::hasGame() && isInitialized())
	{
		level::TileRegistrationMgr& tileMgr(AppGlobal::getGame()->getTileRegistrationMgr());
		
		tileMgr.registerEntityHandle(getRegisteredTileRect(), getHandle());
		
		// However its collision tiles should always be registered
		if (m_collisionTiles != 0)
		{
			tileMgr.registerEntityTiles(m_collisionTilesRegdRect, m_collisionTiles, false);
		}
	}
}


effect::EffectRectHandle Entity::addEffectRect(effect::EffectRectTarget p_targetType)
{
	using namespace effect;
	Game* game = AppGlobal::getGame();
	EffectRectMgr& mgr(game->getEntityMgr().getEffectRectMgr());
	EffectRectHandle handle = mgr.createEffectRect(p_targetType, m_handle);
	
	EffectRect* effectRect = mgr.getEffectRect(handle);
	TT_NULL_ASSERT(effectRect);
	if (effectRect != 0)
	{
		m_effectRects.push_back(handle);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many effect rects used");
	}
	
	return handle;
}


void Entity::removeEffectRect(effect::EffectRectHandle& p_handle)
{
	if (p_handle.isEmpty())
	{
		return;
	}
	
	EffectRectHandles::iterator eraseIt =
			std::find(m_effectRects.begin(), m_effectRects.end(), p_handle);
	if (eraseIt == m_effectRects.end())
	{
		TT_PANIC("Effect Rect handle 0x%08X does not belong to this entity (0x%08X, '%s'). "
		         "Will not remove it.",
		         p_handle.getValue(), m_handle.getValue(), getType().c_str());
		return;
	}
	
	effect::EffectRectMgr& mgr(AppGlobal::getGame()->getEntityMgr().getEffectRectMgr());
	mgr.destroyEffectRect(p_handle);
	m_effectRects.erase(eraseIt);
}


void Entity::removeAllEffectRects()
{
	effect::EffectRectMgr& mgr(AppGlobal::getGame()->getEntityMgr().getEffectRectMgr());
	for (EffectRectHandles::iterator it = m_effectRects.begin();
	     it != m_effectRects.end(); ++it)
	{
		mgr.destroyEffectRect(*it);
	}
	m_effectRects.clear();
}


graphics::PowerBeamGraphicHandle Entity::addPowerBeamGraphic(
		graphics::PowerBeamType     p_type,
		const sensor::SensorHandle& p_source)
{
	using namespace graphics;
	Game* game = AppGlobal::getGame();
	PowerBeamGraphicMgr& mgr(game->getEntityMgr().getPowerBeamGraphicMgr());
	PowerBeamGraphicHandle graphicHandle = mgr.createPowerBeamGraphic(p_type, p_source);
	
	// Force a first update for the power beam graphic, to fix first frame glitches
	// (this function will be called from script functions,
	// which get executed after the PowerBeamGraphicMgr update has already run)
	PowerBeamGraphic* graphic = mgr.getPowerBeamGraphic(graphicHandle);
	TT_NULL_ASSERT(graphic);
	if (graphic != 0)
	{
		graphic->update(0.0f);
		
		// Only save the graphic handle if the graphic is actually valid
		m_powerBeamGraphics.push_back(graphicHandle);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many powerbeam graphics used");
	}
	
	return graphicHandle;
}


void Entity::removePowerBeamGraphic(graphics::PowerBeamGraphicHandle& p_graphicHandle)
{
	if (p_graphicHandle.isEmpty())
	{
		return;
	}
	
	PowerBeamGraphicHandles::iterator eraseIt =
			std::find(m_powerBeamGraphics.begin(), m_powerBeamGraphics.end(), p_graphicHandle);
	if (eraseIt == m_powerBeamGraphics.end())
	{
		TT_PANIC("Power Beam Graphic handle 0x%08X does not belong to this entity (0x%08X, '%s'). "
		         "Will not remove it.",
		         p_graphicHandle.getValue(), m_handle.getValue(), m_entityScript->getType().c_str());
		return;
	}
	
	graphics::PowerBeamGraphicMgr& mgr(AppGlobal::getGame()->getEntityMgr().getPowerBeamGraphicMgr());
	mgr.destroyPowerBeamGraphic(p_graphicHandle);
	m_powerBeamGraphics.erase(eraseIt);
}


void Entity::removeAllPowerBeamGraphics()
{
	graphics::PowerBeamGraphicMgr& mgr(AppGlobal::getGame()->getEntityMgr().getPowerBeamGraphicMgr());
	for (PowerBeamGraphicHandles::iterator it = m_powerBeamGraphics.begin();
	     it != m_powerBeamGraphics.end(); ++it)
	{
		mgr.destroyPowerBeamGraphic(*it);
	}
	m_powerBeamGraphics.clear();
}


graphics::TextLabelHandle Entity::addTextLabel(const std::string& p_localizationKey,
                                               real p_width, real p_height,
                                               utils::GlyphSetID p_glyphSetId)
{
	using namespace graphics;
	Game* game = AppGlobal::getGame();
	TextLabelMgr& mgr(game->getEntityMgr().getTextLabelMgr());
	TextLabelHandle graphicHandle = mgr.createTextLabel(m_handle, p_glyphSetId);
	
	TextLabel* graphic = mgr.getTextLabel(graphicHandle);
	TT_NULL_ASSERT(graphic);
	if (graphic != 0)
	{
		// Only save the graphic handle if the graphic is actually valid
		m_textLabels.push_back(graphicHandle);
		
		if (p_localizationKey.empty() == false)
		{
			graphic->setTextLocalized(p_localizationKey);
		}
		graphic->setSize(p_width, p_height);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many TextLabels used");
	}
	
	return graphicHandle;
}


void Entity::removeTextLabel(graphics::TextLabelHandle& p_labelHandle)
{
	if (p_labelHandle.isEmpty())
	{
		return;
	}
	
	TextLabelHandles::iterator eraseIt =
			std::find(m_textLabels.begin(), m_textLabels.end(), p_labelHandle);
	if (eraseIt == m_textLabels.end())
	{
		TT_PANIC("TextLabel handle 0x%08X does not belong to this entity (0x%08X, '%s'). "
		         "Will not remove it.",
		         p_labelHandle.getValue(), m_handle.getValue(), getType().c_str());
		return;
	}
	
	graphics::TextLabelMgr& mgr(AppGlobal::getGame()->getEntityMgr().getTextLabelMgr());
	mgr.destroyTextLabel(p_labelHandle);
	tt::code::helpers::unorderedErase(m_textLabels, eraseIt);
}


void Entity::removeAllTextLabels()
{
	graphics::TextLabelMgr& mgr(AppGlobal::getGame()->getEntityMgr().getTextLabelMgr());
	for (TextLabelHandles::iterator it = m_textLabels.begin();
	     it != m_textLabels.end(); ++it)
	{
		mgr.destroyTextLabel(*it);
	}
	m_textLabels.clear();
}


sensor::SensorHandle Entity::addSensor(sensor::SensorType p_type, const sensor::ShapePtr& p_shape,
                                       const EntityHandle& p_target)
{
	using namespace sensor;
	Game* game = AppGlobal::getGame();
	SensorMgr& mgr(game->getEntityMgr().getSensorMgr());
	SensorHandle sensor = mgr.createSensor(p_type, m_handle, p_shape, p_target);
	
	if (sensor.isEmpty() == false)
	{
		m_sensors.push_back(sensor);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many sensors used");
	}
	return sensor;
}


sensor::TileSensorHandle Entity::addTileSensor(const sensor::ShapePtr& p_shape)
{
	Game* game = AppGlobal::getGame();
	sensor::TileSensorMgr& mgr(game->getEntityMgr().getTileSensorMgr());
	sensor::TileSensorHandle tileSensor = mgr.createSensor(m_handle, p_shape);
	
	if (tileSensor.isEmpty() == false)
	{
		m_tileSensors.push_back(tileSensor);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many tile sensors used");
	}
	return tileSensor;
}


void Entity::removeSensor(sensor::SensorHandle& p_sensor)
{
	if (p_sensor.isEmpty())
	{
		return;
	}
	
	SensorHandles::iterator it = std::find(m_sensors.begin(), m_sensors.end(), p_sensor);
	if (it != m_sensors.end())
	{
		tt::code::helpers::unorderedErase(m_sensors, it);
	}
	
	using namespace sensor;
	SensorMgr& mgr(AppGlobal::getGame()->getEntityMgr().getSensorMgr());
	mgr.destroySensor(p_sensor);
	p_sensor.invalidate();
}


void Entity::removeTileSensor(sensor::TileSensorHandle& p_sensor)
{
	if (p_sensor.isEmpty())
	{
		return;
	}
	
	TileSensorHandles::iterator it = std::find(m_tileSensors.begin(), m_tileSensors.end(), p_sensor);
	if (it != m_tileSensors.end())
	{
		tt::code::helpers::unorderedErase(m_tileSensors, it);
	}
	
	sensor::TileSensorMgr& mgr(AppGlobal::getGame()->getEntityMgr().getTileSensorMgr());
	mgr.destroySensor(p_sensor);
	p_sensor.invalidate();
}


void Entity::removeAllSensors(sensor::SensorType p_type)
{
	using namespace sensor;
	SensorMgr& mgr(AppGlobal::getGame()->getEntityMgr().getSensorMgr());
	for (SensorHandles::iterator it = m_sensors.begin(); it != m_sensors.end();)
	{
		Sensor* sensor = mgr.getSensor(*it);
		if (sensor == 0)
		{
			it = tt::code::helpers::unorderedErase(m_sensors, it);
			continue;
		}
		else if (sensor->getType() == p_type)
		{
			mgr.destroySensor(*it);
			it = tt::code::helpers::unorderedErase(m_sensors, it);
		}
		else
		{
			++it;
		}
	}
}


void Entity::removeAllTileSensors()
{
	sensor::TileSensorMgr& mgr(AppGlobal::getGame()->getEntityMgr().getTileSensorMgr());
	for (TileSensorHandles::iterator it = m_tileSensors.begin(); it != m_tileSensors.end(); ++it)
	{
		mgr.destroySensor(*it);
	}
	tt::code::helpers::freeContainer(m_tileSensors);
}


void Entity::removeAllSensors()
{
	using namespace sensor;
	SensorMgr& mgr(AppGlobal::getGame()->getEntityMgr().getSensorMgr());
	for (SensorHandles::iterator it = m_sensors.begin(); it != m_sensors.end(); ++it)
	{
		mgr.destroySensor(*it);
	}
	tt::code::helpers::freeContainer(m_sensors);
	
	removeAllTileSensors();
}


void Entity::updateDirtySensors(real64 p_gameTime)
{
	sensor::SensorMgr& mgr(AppGlobal::getGame()->getEntityMgr().getSensorMgr());
	for (SensorHandles::iterator it = m_sensors.begin(); it != m_sensors.end(); ++it)
	{
		sensor::Sensor* sensor = mgr.getSensor(*it);
		if (sensor != 0 && sensor->hasDirtyFlag())
		{
			sensor->update();
			sensor->updateCallbacks(p_gameTime);
		}
	}
	
	sensor::TileSensorMgr& tileMgr(AppGlobal::getGame()->getEntityMgr().getTileSensorMgr());
	for (TileSensorHandles::iterator it = m_tileSensors.begin(); it != m_tileSensors.end(); ++it)
	{
		sensor::TileSensor* sensor = tileMgr.getSensor(*it);
		if (sensor != 0 && sensor->hasDirtyFlag())
		{
			sensor->update();
		}
	}
}


void Entity::registerSensorFilter(const sensor::SensorHandle& p_sensor)
{
	m_filteredBySensors.push_back(p_sensor);
}


void Entity::unregisterSensorFilter(const sensor::SensorHandle& p_sensor)
{
	for (SensorHandles::iterator it = m_filteredBySensors.begin(); it != m_filteredBySensors.end(); ++it)
	{
		if ((*it) == p_sensor)
		{
			tt::code::helpers::unorderedErase(m_filteredBySensors, it);
			return;
		}
	}
	TT_PANIC("sensor not found in m_filteredBySensors");
}


void Entity::removeAllSensorFilter()
{
	if (m_filteredBySensors.empty())
	{
		return;
	}
	
	SensorHandles copy;
	std::swap(copy, m_filteredBySensors);
	for (SensorHandles::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		const sensor::SensorHandle& sensorHandle = (*it);
		sensor::Sensor* sensor = sensorHandle.getPtr();
		if (sensor != 0)
		{
			sensor->removeEntityFilterWithoutUnregister(m_handle);
		}
		else
		{
			TT_PANIC("Invalid sensor handle found in Entity's m_filteredBySensors.");
		}
	}
}


light::LightHandle Entity::addLight(const tt::math::Vector2& p_offset, real p_radius, real p_strength)
{
	using namespace light;
	Game* game = AppGlobal::getGame();
	LightMgr& mgr(game->getLightMgr());
	LightHandle light = mgr.createLight(m_handle, p_offset, p_radius, p_strength);
	
	if (light.isEmpty() == false)
	{
		m_lights.push_back(light);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many lights used");
	}
	return light;
}


void Entity::removeLight(light::LightHandle& p_light)
{
	if (p_light.isEmpty())
	{
		return;
	}
	
	LightHandles::iterator it = std::find(m_lights.begin(), m_lights.end(), p_light);
	if (it != m_lights.end())
	{
		tt::code::helpers::unorderedErase(m_lights, it);
	}
	
	using namespace light;
	LightMgr& mgr(AppGlobal::getGame()->getLightMgr());
	mgr.destroyLight(p_light);
	p_light.invalidate();
}


void Entity::removeAllLights()
{
	using namespace light;
	LightMgr& mgr(AppGlobal::getGame()->getLightMgr());
	for (LightHandles::iterator it = m_lights.begin(); it != m_lights.end(); ++it)
	{
		mgr.destroyLight(*it);
	}
	tt::code::helpers::freeContainer(m_lights);
}


light::DarknessHandle Entity::addDarkness(real p_width, real p_height)
{
	using namespace light;
	Game* game = AppGlobal::getGame();
	DarknessMgr& mgr(game->getDarknessMgr());
	DarknessHandle darkness = mgr.createDarkness(m_handle, p_width, p_height);
	
	if (darkness.isEmpty() == false)
	{
		m_darknesses.push_back(darkness);
	}
	else
	{
		// Creation of failed.
		game->editorWarning(m_handle, "Too many darkness used");
	}
	return darkness;
}


void Entity::removeDarkness(light::DarknessHandle& p_darkness)
{
	if (p_darkness.isEmpty())
	{
		return;
	}
	
	DarknessHandles::iterator it = std::find(m_darknesses.begin(), m_darknesses.end(), p_darkness);
	if (it != m_darknesses.end())
	{
		tt::code::helpers::unorderedErase(m_darknesses, it);
	}
	
	using namespace light;
	DarknessMgr& mgr(AppGlobal::getGame()->getDarknessMgr());
	mgr.destroyDarkness(p_darkness);
	p_darkness.invalidate();
}


void Entity::removeAllDarknesses()
{
	using namespace light;
	DarknessMgr& mgr(AppGlobal::getGame()->getDarknessMgr());
	for (DarknessHandles::iterator it = m_darknesses.begin(); it != m_darknesses.end(); ++it)
	{
		mgr.destroyDarkness(*it);
	}
	tt::code::helpers::freeContainer(m_darknesses);
}


void Entity::onLightEnter()
{
	m_isInLight = true;
	if (isSuspended() == false && // No callbacks if suspended
	    m_scriptIsInLight == false)
	{
		m_scriptIsInLight = true;
		m_entityScript->onLightEnter();
	}
}


void Entity::onLightExit()
{
	m_isInLight = false;
	if (isSuspended() == false && // No callbacks if suspended
	    m_scriptIsInLight)
	{
		m_scriptIsInLight = false;
		m_entityScript->onLightExit();
	}
}


void Entity::setVibrationDetectionPoints(const DetectionPoints& p_points)
{
#ifndef TT_BUILD_FINAL
	// Verify whether these points fall inside the collision rect
	const tt::math::VectorRect& rect(getCollisionRect());
	const tt::math::Vector2     centerOffset(getCenterOffset());
	for (DetectionPoints::const_iterator it = p_points.begin(); it != p_points.end(); ++it)
	{
		if (rect.contains((*it) + centerOffset) == false)
		{
			TT_PANIC("setVibrationDetectionPoints sets a point (%f, %f) (as seen from center) "
			         "which lies outside the collison rect for entity type '%s'",
			         (*it).x, (*it).y, getType().c_str());
		}
	}
#endif
	
	m_vibrationDetectionPoints = p_points;
}


void Entity::removeAllVibrationDetectionPoints()
{
	tt::code::helpers::freeContainer(m_vibrationDetectionPoints);
}


void Entity::setSightDetectionPoints(const DetectionPoints& p_points)
{
#ifndef TT_BUILD_FINAL
	// Verify whether these points fall inside the collision rect
	const tt::math::VectorRect& rect(getCollisionRect());
	const tt::math::Vector2     centerOffset(getCenterOffset());
	for (DetectionPoints::const_iterator it = p_points.begin(); it != p_points.end(); ++it)
	{
		if (rect.contains((*it) + centerOffset) == false)
		{
			TT_PANIC("setSightDetectionPoints sets a point (%f, %f) (as seen from center) "
			         "which lies outside the collison rect for entity type '%s'",
			         (*it).x, (*it).y, getType().c_str());
		}
	}
#endif
	
	m_sightDetectionPoints = p_points;
}


void Entity::removeAllSightDetectionPoints()
{
	tt::code::helpers::freeContainer(m_sightDetectionPoints);
}


void Entity::setLightDetectionPoints(const DetectionPoints& p_points)
{
#ifndef TT_BUILD_FINAL
	// Verify whether these points fall inside the collision rect
	const tt::math::VectorRect& rect(getCollisionRect());
	const tt::math::Vector2     centerOffset(getCenterOffset());
	for (DetectionPoints::const_iterator it = p_points.begin(); it != p_points.end(); ++it)
	{
		if (rect.contains((*it) + centerOffset) == false)
		{
			TT_PANIC("setLightDetectionPoints sets a point (%f, %f) (as seen from center) "
			         "which lies outside the collison rect for entity type '%s'",
			         (*it).x, (*it).y, getType().c_str());
		}
	}
#endif
	
	m_lightDetectionPoints = p_points;
}


void Entity::removeAllLightDetectionPoints()
{
	tt::code::helpers::freeContainer(m_lightDetectionPoints);
}


void Entity::setTouchShape(const sensor::ShapePtr& p_shape, const tt::math::Vector2& p_offset)
{
	if (p_shape == 0)
	{
		removeTouchShape();
		return;
	}
	
	m_touchShape = p_shape;
	m_touchShapeOffset = p_offset;
	
	m_touchShape->updateTransform(*this, getPosition() + applyOrientationToVector2(m_touchShapeOffset), 0);
	
#if !defined(TT_BUILD_FINAL)
	// Verify whether the bounding rectangle of this shape is contained by the collision rect
	tt::math::VectorRect shapeRect(m_touchShape->getBoundingRect());
	shapeRect.setCenterPosition(applyOrientationToVector2(m_touchShapeOffset));
	
	const tt::math::VectorRect collisionRect(applyOrientationToVectorRect(getCollisionRect()));
	
	if (collisionRect.contains(shapeRect) == false)
	{
		TT_PANIC("setTouchShape sets a shape rect (x, y, w, h):\n(%.2f, %.2f, %.2f, %.2f)\n"
		         "that is not contained within the collison rect:\n(%.2f, %.2f, %.2f, %.2f)\nfor entity type '%s'",
		         shapeRect.getPosition().x, shapeRect.getPosition().y, shapeRect.getWidth(), shapeRect.getHeight(),
		         collisionRect.getPosition().x, collisionRect.getPosition().y, collisionRect.getWidth(), collisionRect.getHeight(),
		         getType().c_str());
	}
#endif
}


void Entity::removeTouchShape()
{
	m_touchShape.reset();
	m_touchShapeOffset = tt::math::Vector2::zero;
}


bool Entity::startNewMovement(movement::Direction p_dir, real p_endDistance)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller == 0)
	{
		// Create new dir controller for this entity.
		controller = createDirectionalMovementController();
	}
	TT_NULL_ASSERT(controller);
	return controller->startNewMovement(p_dir, p_endDistance);
}


void Entity::stopMovement()
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		controller = createDirectionalMovementController();
	}
	
	TT_NULL_ASSERT(controller);
	if (controller != 0)
	{
		controller->stopMovement();  // Stop current move
		//controller->scheduleReevalMove(); //resetMovement(); // Re-evaluate the current move. (Maybe fall of something.)
	}
}


void Entity::setMovementSet(const std::string& p_name)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller != 0)
	{
		controller->setMovementSet(p_name);
	}
	else
	{
		TT_PANIC("Can't call setMovementSet on an entity without a MovementController. (start movement first.)");
	}
}


bool Entity::changeDirection(movement::Direction p_dir)
{
	movementcontroller::DirectionalMovementController* controller = 
			getDirectionalMovementController();
	if (controller != 0)
	{
		return controller->changeDirection(p_dir);
	}
	else
	{
		// Create new dir controller for this entity.
		controller = createDirectionalMovementController();
		return controller->startNewMovement(p_dir, -1);
	}
}


movement::Direction Entity::getDirection() const
{
	const movementcontroller::DirectionalMovementController* controller =
		getDirectionalMovementController();
	return (controller != 0) ? controller->getDirection() : movement::Direction_Invalid;
}


movement::Direction Entity::getActualMovementDirection() const
{
	const movementcontroller::DirectionalMovementController* controller =
		getDirectionalMovementController();
	return (controller != 0) ? controller->getActualMovementDirection() : movement::Direction_Invalid;
}


bool Entity::setSpeed(const tt::math::Vector2& p_speed)
{
	movementcontroller::DirectionalMovementController* controller =
		getDirectionalMovementController();
	if (controller != 0)
	{
		controller->setSpeed(p_speed);
		return true;
	}
	
	return false;
}


const tt::math::Vector2& Entity::getSpeed() const
{
	if (isInitialized() == false)
	{
		return tt::math::Vector2::zero;
	}
	
	const movementcontroller::DirectionalMovementController* controller =
		getDirectionalMovementController();
	return (controller != 0) ? controller->getSpeed() : tt::math::Vector2::zero;
}


bool Entity::setExternalForce(const tt::math::Vector2& p_force)
{
	movementcontroller::DirectionalMovementController* controller =
	getDirectionalMovementController();
	if (controller != 0)
	{
		controller->setExternalForce(p_force);
		return true;
	}
	
	return false;
}


const tt::math::Vector2& Entity::getExternalForce() const
{
	if (isInitialized() == false)
	{
		return tt::math::Vector2::zero;
	}
	
	const movementcontroller::DirectionalMovementController* controller =
		getDirectionalMovementController();
	return (controller != 0) ? controller->getExternalForce() : tt::math::Vector2::zero;
}


bool Entity::setReferenceSpeedEntity(const EntityHandle& p_entity)
{
	movementcontroller::DirectionalMovementController* controller =
		getDirectionalMovementController();
	if (controller != 0)
	{
		controller->setReferenceSpeedEntity(p_entity);
		return true;
	}
	
	return false;
}


bool Entity::isDoingDirectionPhysicsMovement() const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller != 0)
	{
		return controller->isDoingDirectionPhysicsMovement();
	}
	return false;
}


void Entity::setMovementDirection(const tt::math::Vector2& p_direction)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0 || controller->isDoingDirectionPhysicsMovement() == false)
	{
		TT_PANIC("Can only set (Physics) Movement Direction when movement controller is in direction physics movement mode.");
		return;
	}
	
	controller->setMovementDirection(p_direction);
}


void Entity::createMovementController(bool p_snapDown)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	TT_ASSERTMSG(controller == 0, "Can't create new movement controller, already have a controller");
	
	if (controller == 0)
	{
		controller = createDirectionalMovementController();
	}
	TT_NULL_ASSERT(controller);
	
	if (p_snapDown)
	{
		snapToStandOnSolid();
		
		//TT_ASSERT(controller->getTileCollisionDirections(    *this).checkFlag(m_orientationDown) == false ||
		//          controller->getTouchingCollisionDirections(*this).checkFlag(m_orientationDown)          );
	}
}


void Entity::startMovementInDirection(const tt::math::Vector2& p_direction, const movementcontroller::PhysicsSettings& p_settings)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		controller = createDirectionalMovementController();
	}
	
	TT_NULL_ASSERT(controller);
	if (controller != 0)
	{
		controller->startMovementInDirection(p_direction, p_settings);
	}
}


void Entity::startPathMovement(const tt::math::Vector2&                   p_positionOrOffset,
                               const movementcontroller::PhysicsSettings& p_settings,
                               const EntityHandle&                        p_optinalTarget)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		controller = createDirectionalMovementController();
	}
	
	TT_NULL_ASSERT(controller);
	if (controller != 0)
	{
		controller->startPathMovement(p_positionOrOffset, p_settings, p_optinalTarget);
	}
}


void Entity::startMovementToPosition(const tt::math::Vector2& p_position, const movementcontroller::PhysicsSettings& p_settings)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		controller = createDirectionalMovementController();
	}
	
	TT_NULL_ASSERT(controller);
	if (controller != 0)
	{
		controller->startMovementToPosition(p_position, p_settings);
	}
}


void Entity::startMovementToEntity(const EntityHandle& p_target, const tt::math::Vector2& p_offset, const movementcontroller::PhysicsSettings& p_settings)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		controller = createDirectionalMovementController();
	}
	
	TT_NULL_ASSERT(controller);
	if (controller != 0)
	{
		controller->startMovementToEntity(p_target, p_offset, p_settings);
	}
}


void Entity::setPhysicsSettings(const movementcontroller::PhysicsSettings& p_settings)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		TT_PANIC("No MovementController. Can't set PhysicsSettings.\n");
		return;
	}
	controller->setPhysicsSettings(p_settings);
}


movementcontroller::PhysicsSettings Entity::getPhysicsSettings() const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		TT_PANIC("No MovementController. Can't get PhysicsSettings.\n");
		return movementcontroller::PhysicsSettings();
	}
	return controller->getPhysicsSettings();
}


void Entity::setParentEntity(const EntityHandle& p_parent, const tt::math::Vector2& p_offset)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	
	if (controller == 0)
	{
		TT_PANIC("Can't set parent entity on an entity with no movement controller.");
		return;
	}
	
	controller->setParentEntity(p_parent, p_offset);
}


EntityHandle Entity::getParentEntity() const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller == 0)
	{
		return EntityHandle();
	}
	
	return controller->getParentEntity();
}


void Entity::setUseParentForward(bool p_useForward)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller == 0)
	{
		TT_PANIC("Can't setUseParentForward without a movement controller!\n");
		return;
	}
	
	controller->setUseParentForward(p_useForward);
}


bool Entity::hasUseParentForward() const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller == 0)
	{
		TT_PANIC("Can't check hasUseParentForward without a movement controller!\n");
		return false;
	}
	
	return controller->hasUseParentForward();
}


void Entity::setUseParentDown(bool p_useDown)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller == 0)
	{
		TT_PANIC("Can't setUseParentDown without a movement controller!\n");
		return;
	}
	
	controller->setUseParentDown(p_useDown);
}


bool Entity::hasUseParentDown() const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller == 0)
	{
		TT_PANIC("Can't check hasUseParentDown without a movement controller!\n");
		return false;
	}
	
	return controller->hasUseParentDown();
}


std::string Entity::getCurrentMoveName() const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller != 0)
	{
		return controller->getCurrentMoveName();
	}
	return "";
}


bool Entity::canBeMovedInDirection(movement::Direction p_direction) const
{
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	return (controller != 0) ? controller->canBeMovedInDirection(p_direction) : false;
}


const fluid::FluidSettingsPtr Entity::getFluidSettings(fluid::FluidType p_type) const
{
	if (p_type >= 0 && p_type < fluid::FluidType_Count)
	{
		return m_fluidSettings[p_type];
	}
	
	TT_PANIC("FluidType '%d' out of bounds", p_type);
	return fluid::FluidSettingsPtr();
}


fluid::FluidSettingsPtr Entity::getFluidSettings(fluid::FluidType p_type)
{
	if (p_type >= 0 && p_type < fluid::FluidType_Count)
	{
		return m_fluidSettings[p_type];
	}
	
	TT_PANIC("FluidType '%d' out of bounds", p_type);
	return fluid::FluidSettingsPtr();
}


void Entity::setFluidSettings(fluid::FluidType p_type, const fluid::FluidSettingsPtr& p_fluidSettings)
{
	if (p_type >= 0 && p_type < fluid::FluidType_Count)
	{
		m_fluidSettings[p_type] = p_fluidSettings;
		return;
	}
	
	TT_PANIC("FluidType '%d' out of bounds", p_type);
}


bool Entity::hasFluidSettings(fluid::FluidType p_type) const
{
	if (p_type >= 0 && p_type < fluid::FluidType_Count)
	{
		return m_fluidSettings[p_type] != 0;
	}
	
	TT_PANIC("FluidType '%d' out of bounds", p_type);
	return false;
}


bool Entity::hasAnyFluidSettings() const
{
	for (s32 i = 0; i < fluid::FluidType_Count; ++i)
	{
		if (m_fluidSettings[i] != 0)
		{
			return true;
		}
	}
	
	return false;
}


void Entity::makeScreenSpaceEntity()
{
	if (m_inScreenspace)
	{
		// Already in screenspace.
		return;
	}
	
	m_inScreenspace = true;
	m_updateSurvey  = false;
	
	// Disable as many 'world' things as we can (e.g. Don't register tiles.)
	disableTileRegistration();
	m_isDetectableByLight = false;
	m_isDetectableBySight = false;
	m_isDetectableByTouch = false;
	m_canBePushed         = false;
	m_canBeCarried        = false;
	
	AppGlobal::getGame()->addScreenSpaceEntity(getHandle());
}


void Entity::setPositionForced(const tt::math::Vector2& p_pos, bool p_snapPos)
{
	if (tt::math::realEqual(p_pos.x, m_pos.x) && tt::math::realEqual(p_pos.y, m_pos.y))
	{
		return;
	}
	
	const tt::math::PointRect prevRegTileRect(m_registeredTileRect);
	m_pos   = p_pos;
	
	if (p_snapPos && m_inScreenspace == false)
	{
		// Snap to new tile position.
		m_pos = getSnappedToTilePos();
	}
	
	// Update everything for new pos.
	updateRects();
	
	if (prevRegTileRect == m_registeredTileRect)
	{
		// Early exit
		return;
	}
	
	level::TileRegistrationMgr& tileMgr(AppGlobal::getGame()->getTileRegistrationMgr());
	
	tileMgr.moveRegisterEntityHandle(prevRegTileRect, m_registeredTileRect, m_handle);
	
	// Let the controller know the position was changed.
	if (m_movementControllerHandle.isEmpty() == false)
	{
		EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
		movementcontroller::DirectionalMovementController* controller =
				entityMgr.getMovementControllerMgr().getDirectionalController(m_movementControllerHandle);
		if (controller != 0)
		{
			controller->scheduleSurveyUpdate();
			controller->updateLocalCollision(m_registeredTileRect, *this);
		}
	}
	
	updateSurvey(true);
	
}


tt::math::Vector2 Entity::getSnappedToTilePos() const
{
	tt::math::Vector2 parentOffset(tt::math::Vector2::zero);
	
	Entity* collisionParent = (isInitialized()) ? getCollisionParentEntity().getPtr() : 0;
	if (collisionParent != 0)
	{
		parentOffset   = collisionParent->calcEntityTileRect().getPosition();
		parentOffset.x = tt::math::fmod(parentOffset.x, level::tileToWorld(1));
		parentOffset.y = tt::math::fmod(parentOffset.y, level::tileToWorld(1));
	}
	
	return level::snapToTilePos(m_pos - parentOffset, applyOrientationToVector2(m_localTileRect.getPosition())) +
	       parentOffset;
}


tt::math::Vector2 Entity::getSnappedToTilePosLevelOnly() const
{
	return level::snapToTilePos(m_pos, applyOrientationToVector2(m_localTileRect.getPosition()));
}


void Entity::snapToStandOnSolid()
{
	// Snap to the bottom edge if we have a specific stand-on direction. (To make it easier to grab walls and ceilings.)
	static const real epsilon = 0.00001f;
	static const real epsilonParent = 0.00001f;
	const tt::math::VectorRect worldRect(getWorldVectorRectFromLocal(m_localRect));
	
	{
		movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
		if (controller != 0 &&
		    controller->getCollisionParentEntity().isEmpty() == false)
		{
			Entity* collisionParent = controller->getCollisionParentEntity().getPtr();
			if (collisionParent != 0)
			{
				const tt::math::VectorRect parentTileRect(collisionParent->calcEntityTileRect());
				
				switch (m_orientationDown)
				{
				case movement::Direction_Down:
					modifyPosition().y -= (worldRect.getMin().y     - parentTileRect.getMaxEdge().y ) - epsilonParent;
					TT_ASSERT(controller->getTouchingCollisionDirections(*this).checkFlag(m_orientationDown));
					return;
					
				case movement::Direction_Right:
					modifyPosition().x -= (worldRect.getMaxEdge().x - parentTileRect.getMin().x     ) + epsilonParent;
					TT_ASSERT(controller->getTouchingCollisionDirections(*this).checkFlag(m_orientationDown));
					return;
					
				case movement::Direction_Up:
					modifyPosition().y -= (worldRect.getMaxEdge().y - parentTileRect.getMin().y     ) + epsilonParent;
					TT_ASSERT(controller->getTouchingCollisionDirections(*this).checkFlag(m_orientationDown));
					return;
					
				case movement::Direction_Left:
					modifyPosition().x -= (worldRect.getMin().x     - parentTileRect.getMaxEdge().x ) - epsilonParent;
					TT_ASSERT(controller->getTouchingCollisionDirections(*this).checkFlag(m_orientationDown));
					return;
					
				case movement::Direction_None:
					break;
				default:
					TT_PANIC("Unknown entity down orientation: %d", m_orientationDown);
					break;
				}
			}
		}
	}
	
	const tt::math::PointRect  tileRect( level::worldToTile(worldRect));
	
	switch (m_orientationDown)
	{
	case movement::Direction_Down:
		modifyPosition().y -= (worldRect.getMin().y     - tileRect.getMin().y    ) - epsilon;
		TT_ASSERT(tt::math::VectorRect(getWorldVectorRectFromLocal(m_localRect)).getMin().y >= tileRect.getMin().y);
		break;
		
	case movement::Direction_Right:
		modifyPosition().x -= (worldRect.getMaxEdge().x - tileRect.getMaxEdge().x) + epsilon;
		TT_ASSERT(tt::math::VectorRect(getWorldVectorRectFromLocal(m_localRect)).getMaxEdge().x <= tileRect.getMaxEdge().x);
		break;
		
	case movement::Direction_Up:
		modifyPosition().y -= (worldRect.getMaxEdge().y - tileRect.getMaxEdge().y) + epsilon;
		TT_ASSERT(tt::math::VectorRect(getWorldVectorRectFromLocal(m_localRect)).getMaxEdge().y <= tileRect.getMaxEdge().y);
		break;
		
	case movement::Direction_Left:
		modifyPosition().x -= (worldRect.getMin().x     - tileRect.getMin().x    ) - epsilon;
		TT_ASSERT(tt::math::VectorRect(getWorldVectorRectFromLocal(m_localRect)).getMin().x >= tileRect.getMin().x);
		break;
		
	case movement::Direction_None:
		break; // Do nothing.
		
	default:
		TT_PANIC("Unknown down orientation: %d\n", m_orientationDown);
		break;
	}
}


void Entity::setOrientationDown(movement::Direction p_orientationDown)
{
	if (movement::isValidDirection(p_orientationDown) == false)
	{
		TT_PANIC("Trying to set invalid down orientation / floor direction: %d", p_orientationDown);
		return;
	}
	
	if (p_orientationDown == m_orientationDown)
	{
		// Nothing changed.
		return;
	}
	
	const tt::math::VectorRect prevRect(getWorldVectorRectFromLocal(m_localRect));
	
	m_orientationDown = p_orientationDown; 
	handleOrientationChange();
	
	const tt::math::VectorRect newRect(getWorldVectorRectFromLocal(m_localRect));
	const tt::math::Vector2 difference(prevRect.getPosition() - newRect.getPosition());
	modifyPosition() += difference;
	
	if (AppGlobal::hasGame())
	{
		// Let the controller know the orientation was changed.
		EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
		movementcontroller::DirectionalMovementController* controller =
				entityMgr.getMovementControllerMgr().getDirectionalController(m_movementControllerHandle);
		if (controller != 0)
		{
			controller->scheduleSurveyUpdate();
			controller->scheduleReselectMove();
			
			if (canBeCarried())
			{
				// An orientation change will change parent.
				reevaluateCollisionParent(EntityHandle());
				
				// Need to flush current parent so snapping isn't done relative to the (now invalid) parent.
				controller->makeScheduledCollisionParentCurrent();
			}
		}
		
		snapToStandOnSolid();
		
		if (controller != 0)
		{
			controller->checkCollision_HACK(*this, tt::math::Vector2::zero);
			controller->updateLocalCollision(calcRegisteredTileRect(), *this);
		}
		
		// Update graphic with new position. (Can't use updateRect because tile registration might get broken!)
		updatePresentationObjectsPosition();
		
		// Update survey and rects because this entity might not have a controller to do that later.
		updateSurvey(false);
		
		level::TileRegistrationMgr& mgr(AppGlobal::getGame()->getTileRegistrationMgr());
		const tt::math::PointRect prevRegTileRect(getRegisteredTileRect());
		updateRects();
		mgr.moveRegisterEntityHandle(prevRegTileRect, getRegisteredTileRect(), getHandle());
	}
}


tt::math::VectorRect Entity::calcEntityTileRect() const
{
	if (m_collisionTiles == 0)
	{
		return calcWorldTileRect();
	}
	return tt::math::VectorRect(m_pos + m_localTileRect.getPosition(),
	                            level::tileToWorld(m_collisionTiles->getWidth() ),
	                            level::tileToWorld(m_collisionTiles->getHeight()));
}


void Entity::updateRects(const tt::math::PointRect* p_moveToTileRect)
{
	updatePresentationObjectsPosition();
	
	const tt::math::PointRect prevTileRect = m_registeredTileRect;
	
	// Which tiles are we overlapping? (Need to register them)
	m_registeredTileRect = calcRegisteredTileRect();
	
	m_worldRect = calcWorldRect();
	
	if (prevTileRect != m_registeredTileRect || p_moveToTileRect != 0)
	{
		if (p_moveToTileRect != 0)
		{
			m_willEndMoveHereRect = *p_moveToTileRect;
		}
		else
		{
			m_willEndMoveHereRect = tt::math::PointRect(
					m_registeredTileRect.getPosition(),
					level::worldToTile(tt::math::ceil(m_worldRect.getWidth())),
					level::worldToTile(tt::math::ceil(m_worldRect.getHeight())));
		}
	}
	
	if (m_collisionTiles != 0)
	{
		const tt::math::PointRect prevCollisionTileRect(m_collisionTilesRegdRect);
		m_collisionTilesRegdRect = calcCollisionTilesRegdRect();
		
		const tt::math::Point2& newCollisionTilePos = m_willEndMoveHereRect.getPosition();
		
		// Changed registered tiles or have new a tile rect
		if (prevCollisionTileRect != m_collisionTilesRegdRect ||
		    (p_moveToTileRect != 0 && newCollisionTilePos   != m_collisionTiles->getPos()))
		{
			const tt::math::Point2 newCollisionTilePosWithOffset = newCollisionTilePos + m_collisionTileOffset;
			
			// Entity has collision tiles and the registered tile rect changed: update our registration
			level::TileRegistrationMgr& tileRegMgr(AppGlobal::getGame()->getTileRegistrationMgr());
			tileRegMgr.unregisterEntityTiles(prevCollisionTileRect,    m_collisionTiles);
			if (newCollisionTilePosWithOffset != m_collisionTiles->getPos() || p_moveToTileRect != 0)
			{
				// Entity with collision has moved tiles
				m_collisionTiles->onNewTilePosition(newCollisionTilePosWithOffset);
				
				{
					// Make sure the new position of the tiles is inside the registration rect.
					const tt::math::PointRect colTileRect = m_collisionTiles->getRect();
					m_collisionTilesRegdRect = tt::math::merge(colTileRect, m_collisionTilesRegdRect);
				}
				
				/*
				TT_Printf("Entity::updateRects H:0X%X on tiles: (%d, %d) (%d, %d) w: %d, h: %d, newCollisionTilePosWithOffset: (%d, %d)\n",
				          getHandle().getValue(), 
				          m_collisionTilesRegdRect.getMin().x,       m_collisionTilesRegdRect.getMin().y, 
				          m_collisionTilesRegdRect.getMaxInside().x, m_collisionTilesRegdRect.getMaxInside().y,
				          m_collisionTilesRegdRect.getWidth(),       m_collisionTilesRegdRect.getHeight(),
				          newCollisionTilePosWithOffset.x,           newCollisionTilePosWithOffset.y);
				// */
				
				tileRegMgr.registerEntityTiles  (m_collisionTilesRegdRect, m_collisionTiles);
				
				
				makeSurroundingEntitiesScheduleReevaluateCollisionParent();
				
				// FIXME: Code similar to that below used to be inside EntityTiles::onNewTilePosition,
				//        when it still kept track of the collision children. Might need to move this elsewhere.
				movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
				if (ctrl != 0)
				{
					EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
					const EntityHandleSet& children(ctrl->getCollisionChildren());
					for (EntityHandleSet::const_iterator it = children.begin(); it != children.end(); ++it)
					{
						Entity* child = entityMgr.getEntity(*it);
						if (child != 0 && child->isInitialized())
						{
							child->onTileChange();
						}
					}
				}
			}
			else
			{
				tileRegMgr.registerEntityTiles  (m_collisionTilesRegdRect, m_collisionTiles);
			}
		}
	}
	
	if (m_touchShape != 0)
	{
		tt::math::Vector2 pos(getPosition() + applyOrientationToVector2(m_touchShapeOffset));
		m_touchShape->updateTransform(*this, pos, 0);
	}
}


void Entity::setCollisionRect(const tt::math::VectorRect& p_rect)
{
	// Unregister the current tiles
	level::TileRegistrationMgr& tileMgr(AppGlobal::getGame()->getTileRegistrationMgr());
	
	const tt::math::PointRect prevRegTileRect(m_registeredTileRect);
	
	setLocalRects(p_rect);
	
	// Ensure the other rects are recalculated as well
	updateRects();
	
	// Register tiles for the new rect
	tileMgr.moveRegisterEntityHandle(prevRegTileRect, m_registeredTileRect, m_handle);
	
	updateSurvey(true);
	
	movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
	if (ctrl != 0)
	{
		ctrl->scheduleSurveyUpdate(); // Update local collision and select new move.
	}
}


void Entity::updateSurvey(bool p_doScriptCallbacks)
{
	if (m_updateSurvey == false)
	{
		return;
	}
	m_survey = movement::SurroundingsSurvey(*this);
	
	if (p_doScriptCallbacks == false || isSuspended()) // No script callbacks when suspended.
	{
		return;
	}
	doSurveyCallbacks();
}


void Entity::doSurveyCallbacks()
{
	level::CollisionType           enclosed = m_survey.getInsideCollisionTypeForAllTiles();
	level::CollisionType           prevEnclosed = m_prevSurvey.getInsideCollisionTypeForAllTiles();
	
	//-------------------------------------
	// Fluid checks
	
	fluid::FluidType         fluidEnclosed     = m_survey.getInsideFluidTypeForAllTiles();
	const fluid::FluidTypes& fluidTouching     = m_survey.getTouchFluidTypes();
	fluid::FluidType         waterfallEnclosed = m_survey.getInsideWaterfallTypeForAllTiles();
	const fluid::FluidTypes& waterfallTouching = m_survey.getTouchWaterfallTypes();
	
	fluid::FluidType         prevFluidEnclosed     = m_prevSurvey.getInsideFluidTypeForAllTiles();
	const fluid::FluidTypes& prevFluidTouching     = m_prevSurvey.getTouchFluidTypes();
	fluid::FluidType         prevWaterfallEnclosed = m_prevSurvey.getInsideWaterfallTypeForAllTiles();
	const fluid::FluidTypes& prevWaterfallTouching = m_prevSurvey.getTouchWaterfallTypes();
	
	fluid::FluidMgr& fluidMgr = AppGlobal::getGame()->getFluidMgr();
	
	// HACK: Hack to make sure fluidMgr.onEntityEnters/ExitsFluid follows different logic
	// than script.onWaterTouchEnter/Exit. The script equivalent doesn't work well with splashing Hermits
	// and this new code does a somewhat better job.
	// FIXME: Visual behavior should match script callback behavior
	{
		fluid::FluidType type = fluid::FluidType_Invalid;
		if (fluidTouching.checkFlag(fluid::FluidType_Lava))
		{
			type = fluid::FluidType_Lava;
		}
		else if (fluidTouching.checkFlag(fluid::FluidType_Water))
		{
			type = fluid::FluidType_Water;
		}
		else if (enclosed < level::CollisionType_Count && level::isFluidType(enclosed))
		{
			type = fluid::getFluidType(enclosed);
		}
		
		fluid::FluidType prevType = fluid::FluidType_Invalid;
		if (prevFluidTouching.checkFlag(fluid::FluidType_Lava))
		{
			prevType = fluid::FluidType_Lava;
		}
		else if (prevFluidTouching.checkFlag(fluid::FluidType_Water))
		{
			prevType = fluid::FluidType_Water;
		}
		else if (prevEnclosed < level::CollisionType_Count && level::isFluidType(prevEnclosed))
		{
			prevType = fluid::getFluidType(prevEnclosed);
		}
		
		bool isTouchingFluid  = type     == fluid::FluidType_Water || type     == fluid::FluidType_Lava;
		bool wasTouchingFluid = prevType == fluid::FluidType_Water || prevType == fluid::FluidType_Lava;
		
		if (isTouchingFluid && wasTouchingFluid == false)
		{
			fluidMgr.onEntityEntersFluid(*this, type);
		}
		else if (isTouchingFluid == false && wasTouchingFluid)
		{
			fluidMgr.onEntityExitsFluid(*this, prevType);
		}
	}
	// END HACK
	
	// Water
	if (    fluidTouching.checkFlag(fluid::FluidType_Water) &&
	    prevFluidTouching.checkFlag(fluid::FluidType_Water) == false)
	{
		getEntityScript()->onWaterTouchEnter();
	}
	else if (    fluidTouching.checkFlag(fluid::FluidType_Water) == false &&
	         prevFluidTouching.checkFlag(fluid::FluidType_Water))
	{
		getEntityScript()->onWaterTouchExit();
	}
	
	if (    fluidEnclosed == fluid::FluidType_Water &&
	    prevFluidEnclosed != fluid::FluidType_Water)
	{
		getEntityScript()->onWaterEnclosedEnter();
	}
	else if (    fluidEnclosed != fluid::FluidType_Water &&
	         prevFluidEnclosed == fluid::FluidType_Water)
	{
		getEntityScript()->onWaterEnclosedExit();
	}
	
	if (    waterfallTouching.checkFlag(fluid::FluidType_Water) &&
	    prevWaterfallTouching.checkFlag(fluid::FluidType_Water) == false)
	{
		fluidMgr.onEntityEntersFall(*this, fluid::FluidType_Water);
		getEntityScript()->onWaterfallTouchEnter();
	}
	else if (    waterfallTouching.checkFlag(fluid::FluidType_Water) == false &&
	         prevWaterfallTouching.checkFlag(fluid::FluidType_Water))
	{
		fluidMgr.onEntityExitsFall(*this, fluid::FluidType_Water);
		getEntityScript()->onWaterfallTouchExit();
	}
	
	if (    waterfallEnclosed == fluid::FluidType_Water &&
	    prevWaterfallEnclosed != fluid::FluidType_Water)
	{
		getEntityScript()->onWaterfallEnclosedEnter();
	}
	else if (    waterfallEnclosed != fluid::FluidType_Water &&
	         prevWaterfallEnclosed == fluid::FluidType_Water)
	{
		getEntityScript()->onWaterfallEnclosedExit();
	}
	
	// Lava
	if (    fluidTouching.checkFlag(fluid::FluidType_Lava) &&
	    prevFluidTouching.checkFlag(fluid::FluidType_Lava) == false)
	{
		getEntityScript()->onLavaTouchEnter();
	}
	else if (    fluidTouching.checkFlag(fluid::FluidType_Lava) == false &&
	         prevFluidTouching.checkFlag(fluid::FluidType_Lava))
	{
		getEntityScript()->onLavaTouchExit();
	}
	
	if (    fluidEnclosed == fluid::FluidType_Lava &&
	    prevFluidEnclosed != fluid::FluidType_Lava)
	{
		getEntityScript()->onLavaEnclosedEnter();
	}
	else if (    fluidEnclosed != fluid::FluidType_Lava &&
	         prevFluidEnclosed == fluid::FluidType_Lava)
	{
		getEntityScript()->onLavaEnclosedExit();
	}
	
	if (    waterfallTouching.checkFlag(fluid::FluidType_Lava) &&
	    prevWaterfallTouching.checkFlag(fluid::FluidType_Lava) == false)
	{
		fluidMgr.onEntityEntersFall(*this, fluid::FluidType_Lava);
		getEntityScript()->onLavafallTouchEnter();
	}
	else if (    waterfallTouching.checkFlag(fluid::FluidType_Lava) == false &&
	         prevWaterfallTouching.checkFlag(fluid::FluidType_Lava))
	{
		fluidMgr.onEntityExitsFall(*this, fluid::FluidType_Lava);
		getEntityScript()->onLavafallTouchExit();
	}
	
	if (    waterfallEnclosed == fluid::FluidType_Lava &&
	    prevWaterfallEnclosed != fluid::FluidType_Lava)
	{
		getEntityScript()->onLavafallEnclosedEnter();
	}
	else if (    waterfallEnclosed != fluid::FluidType_Lava &&
	         prevWaterfallEnclosed == fluid::FluidType_Lava)
	{
		getEntityScript()->onLavafallEnclosedExit();
	}
	
	if (fluidTouching.isEmpty() == false || waterfallTouching.isEmpty() == false)
	{
		for (s32 i = 0; i < fluid::FluidType_Count; ++i)
		{
			fluid::FluidType fluidType = static_cast<fluid::FluidType>(i);
			if (fluidTouching.checkFlag(fluidType))
			{
				fluidMgr.scheduleEntityEffectsRectsUpdate(*this, fluid::EntityFluidEffectType_Surface, fluidType);
			}
			if (waterfallTouching.checkFlag(fluidType))
			{
				fluidMgr.scheduleEntityEffectsRectsUpdate(*this, fluid::EntityFluidEffectType_Fall,    fluidType);
			}
		}
	}
	
	m_prevSurvey = m_survey;
}


void Entity::setCollisionTiles(const std::string& p_tiles)
{
	level::TileRegistrationMgr& tileRegMgr(AppGlobal::getGame()->getTileRegistrationMgr());
	
	if (m_collisionTiles != 0)
	{
		tileRegMgr.unregisterEntityTiles(m_collisionTilesRegdRect, m_collisionTiles);
	}
	
	m_collisionTiles = EntityTiles::create(
			p_tiles,
			m_registeredTileRect.getMin() + m_collisionTileOffset,
			m_collisionTilesActive,
			m_handle);
	
	if (m_collisionTiles != 0)
	{
		m_collisionTilesRegdRect = calcCollisionTilesRegdRect();
		tileRegMgr.registerEntityTiles(m_collisionTilesRegdRect, m_collisionTiles);
	}
	
	movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
	if (ctrl != 0 && ctrl->isCollisionParentEnabled())
	{
		ctrl->makeCollisionChildrenReevaluateParent(getHandle());
	}
}


void Entity::setCollisionTilesActive(bool p_active)
{
	m_collisionTilesActive = p_active;
	if (m_collisionTiles != 0)
	{
		m_collisionTiles->setActive(p_active);
	}
}


void Entity::removeCollisionTiles()
{
	if (m_collisionTiles != 0)
	{
		AppGlobal::getGame()->getTileRegistrationMgr().unregisterEntityTiles(
			m_collisionTilesRegdRect, m_collisionTiles);
		
		m_collisionTiles.reset();
		
		movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
		if (ctrl != 0 && ctrl->isCollisionParentEnabled())
		{
			ctrl->makeCollisionChildrenReevaluateParent(getHandle());
		}
	}
}


void Entity::setEntityCollisionRectAndTiles(s32 p_width, s32 p_height, level::CollisionType p_collisionType)
{
	// Do something like this:
	// -----------------------------
	// local rect = getCollisionRect();
	// rect.setHeight(max(0.5, p_height));
	// setOnlyCollisionRectWithVectorRect(rect);
	// setEntityCollisionTiles(createCollisionTileString(1, p_height, "X"));
	// -----------------------------
	
	level::TileRegistrationMgr& tileMgr(AppGlobal::getGame()->getTileRegistrationMgr());
	
	const tt::math::PointRect prevRegTileRect(m_registeredTileRect);
	
	// ------------------  Resize the rects  ------------------------
	// This is kind of a duplication of setLocalRects(rect), but without the small value applied to pos.
	// When repeatedly calling this function the m_localRect pos would change a little bit each time.
	const real width  = level::tileToWorld(p_width);
	const real height = level::tileToWorld(p_height);
	m_localRect.setWidth( width  - g_rectResizeDoubleSmallValue);
	m_localRect.setHeight(height - g_rectResizeDoubleSmallValue);
	m_localTileRect.setWidth( width);
	m_localTileRect.setHeight(height);
	TT_ASSERT(m_localTileRect.getPosition() == (tt::math::floor(m_localRect.getPosition() * 2.0f)) * 0.5f);
	
	// Ensure the other rects are recalculated as well
	updateRects();
	
	// Register tiles for the new rect
	tileMgr.moveRegisterEntityHandle(prevRegTileRect, m_registeredTileRect, m_handle);
	
	movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
	if (ctrl != 0)
	{
		ctrl->scheduleSurveyUpdate(); // Update local collision and select new move.
	}
	else
	{
		updateSurvey(true);
	}
	
	// ------------------  Resize the collsion tiles ------------------------
	bool wasSolid = false;
	if (m_collisionTiles != 0)
	{
		tileMgr.unregisterEntityTiles(m_collisionTilesRegdRect, m_collisionTiles);
		wasSolid = m_collisionTiles->isSolid();
	}
	
	const tt::math::PointRect tileRect(m_registeredTileRect.getMin() + m_collisionTileOffset,
	                                   p_width, p_height);
	
	m_collisionTiles = EntityTiles::create(
			tileRect,
			p_collisionType,
			m_collisionTilesActive,
			m_handle);
	
	if (m_collisionTiles != 0)
	{
		m_collisionTilesRegdRect = calcCollisionTilesRegdRect();
		tileMgr.registerEntityTiles(m_collisionTilesRegdRect, m_collisionTiles);
	}
	
	if (ctrl != 0 && wasSolid)
	{
		ctrl->makeCollisionChildrenReevaluateParent(getHandle());
	}
}


void Entity::enableTileRegistration()
{
	if (m_tileRegistrationEnabled == false)
	{
		// Make sure m_tileRegistrationEnabled is true before calling registerEntityHandle() 
		m_tileRegistrationEnabled = true;
		
		level::TileRegistrationMgr& mgr(AppGlobal::getGame()->getTileRegistrationMgr());
		mgr.registerEntityHandle(m_registeredTileRect, m_handle);
		if (m_collisionTiles != 0)
		{
			mgr.registerEntityTiles(m_collisionTilesRegdRect, m_collisionTiles);
		}
	}
}


void Entity::disableTileRegistration()
{
	if (m_tileRegistrationEnabled)
	{
		// Make sure m_tileRegistrationEnabled is true before calling unregisterEntityHandle() 
		level::TileRegistrationMgr& mgr(AppGlobal::getGame()->getTileRegistrationMgr());
		mgr.unregisterEntityHandle(m_registeredTileRect, m_handle);
		if (m_collisionTiles != 0)
		{
			mgr.unregisterEntityTiles(m_collisionTilesRegdRect, m_collisionTiles);
		}
		
		m_tileRegistrationEnabled = false;
	}
}


pres::PresentationObjectHandle Entity::createPresentationObject(const std::string& p_filename,
                                                                const tt::pres::Tags& p_requiredTags,
                                                                ParticleLayer p_layer)
{
	if (AppGlobal::hasGame() == false)
	{
		return pres::PresentationObjectHandle();
	}
	
	Game* game = AppGlobal::getGame();
	TT_NULL_ASSERT(game);
	
	pres::PresentationObjectHandle handle = 
		game->getPresentationObjectMgr().createPresentationObject(m_handle, p_filename, p_requiredTags, p_layer);
	
	pres::PresentationObject* pres = handle.getPtr();
	if (pres == 0)
	{
		return pres::PresentationObjectHandle();
	}
	
	m_presentationObjects.push_back(handle);
	
	return handle;
}


void Entity::destroyPresentationObject(pres::PresentationObjectHandle& p_handle)
{
	PresentationObjects::iterator it = 
		std::find(m_presentationObjects.begin(), m_presentationObjects.end(), p_handle);
	
	if (it == m_presentationObjects.end())
	{
		TT_PANIC("Trying to destroy presentation object handle '%p', but this handle is not present in this entity",
			p_handle.getValue());
		return;
	}

	Game* game = AppGlobal::getGame();
	TT_NULL_ASSERT(game);
	
	game->getPresentationObjectMgr().destroyPresentationObject(p_handle);
	tt::code::helpers::unorderedErase(m_presentationObjects, it);
}


void Entity::destroyAllPresentationObjects()
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	Game* game = AppGlobal::getGame();
	TT_NULL_ASSERT(game);
	pres::PresentationObjectMgr& mgr = game->getPresentationObjectMgr();
	
	for (PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end(); ++it)
	{
		mgr.destroyPresentationObject(*it);
	}
	tt::code::helpers::freeContainer(m_presentationObjects);
}


bool Entity::hasPresentationObject(const pres::PresentationObjectHandle& p_handle) const
{
	return std::find(m_presentationObjects.begin(), m_presentationObjects.end(), p_handle) !=
	       m_presentationObjects.end();
}


void Entity::startAllPresentationObjects(const std::string& p_name,
                                         const tt::pres::Tags& p_tagsToStart,
                                         pres::StartType p_startType)
{
	for (PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end(); ++it)
	{
		pres::PresentationObject* pres = (*it).getPtr();
		if (pres != 0)
		{
			pres->start(p_name, p_tagsToStart, false, p_startType);
		}
	}
}


void Entity::stopAllPresentationObjects()
{
	for (PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end(); ++it)
	{
		pres::PresentationObject* pres = (*it).getPtr();
		if (pres != 0)
		{
			pres->stop();
		}
	}
}


void Entity::startAllPresentationObjectsForMovement(const std::string& p_name,
                                                    const tt::pres::Tags& p_tagsToStart,
                                                    pres::StartType p_startType)
{
	for (PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end(); ++it)
	{
		pres::PresentationObject* pres = (*it).getPtr();
		if (pres != 0 && pres->isAffectedByMovement())
		{
			pres->start(p_name, p_tagsToStart, false, p_startType);
		}
	}
}


void Entity::stopAllPresentationObjectsForMovement()
{
	for (PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end(); ++it)
	{
		pres::PresentationObject* pres = (*it).getPtr();
		if (pres != 0 && pres->isAffectedByMovement())
		{
			pres->stop();
		}
	}
}


tt::pres::Tags Entity::getStandOnTags() const
{
	tt::pres::Tags tags;
	
	const std::string standOnStr(getStandOnStr());
	if (standOnStr.empty() == false)
	{
		tags.insert(tt::pres::Tag(std::string("on_") + standOnStr));
	}
	
	return tags;
}


const char* const Entity::getStandOnStr() const
{
	if (getSurvey().getStandOnEntityTile())
	{
		return "entity";
	}
	else if (level::skin::isValidMaterialTheme(getSurvey().getStandOnTheme()))
	{
		return level::skin::getMaterialThemeName(getSurvey().getStandOnTheme());
	}
	return "";
}


tt::engine::particles::ParticleEffectPtr Entity::spawnParticle(SpawnType                p_spawnType,
                                                               const std::string&       p_filename,
                                                               const tt::math::Vector2& p_position,
                                                               bool                     p_followEntity,
                                                               real                     p_spawnDelay,
                                                               bool                     p_positionIsInWorldSpace,
                                                               ParticleLayer            p_particleLayer,
                                                               real                     p_scale) const
{
	TT_ASSERTMSG(p_scale > 0.0f, "Particle spawn: invalid scale %.2f used for particle with filename '%s",
	             p_scale, p_filename.c_str());
	TT_ASSERTMSG(p_followEntity == false || p_positionIsInWorldSpace == false,
	             "Particle spawn: Cannot follow an entity AND specify the position in world space. "
	             "These options are mutually exclusive (use one or the other, not both).");
	
	using namespace tt::engine;
	particles::ParticleMgr* particleMgr = particles::ParticleMgr::getInstance();
	
	const u32  category    = static_cast<u32>(particles::Category_All);
	const s32  renderGroup = getParticleLayerRenderGroup(p_particleLayer);
	
	particles::ParticleEffectPtr effect;
	if (p_followEntity)
	{
		ParticleEntityFollower* followHelper = new ParticleEntityFollower(getHandle(), p_position);
		effect = particleMgr->createEffect(p_filename + ".trigger", followHelper, category, p_scale, renderGroup);
	}
	else
	{
		tt::math::Vector3 pos(p_position.x, p_position.y, 0.0f);
		if (p_positionIsInWorldSpace == false)
		{
			pos = getPositionForParticles(p_position);
		}
		
		effect = particleMgr->createEffect(p_filename + ".trigger", pos, category, p_scale, renderGroup);
	}
	
	if (effect == 0)
	{
		// NOTE: Panic about being unable to load particle effect is already triggered by createEffect
		return particles::ParticleEffectPtr();
	}
	
	if (p_followEntity)
	{
		effect->getTrigger()->setTriggerHasFollowObjectOwnership(true);
	}
	
	effect->getTrigger()->addDelay(p_spawnDelay);
	
	if (p_spawnType == SpawnType_OneShot)
	{
		particleMgr->spawnEffect(particles::SpawnType_OneShot, effect);
	}
	else if (p_spawnType == SpawnType_Continuous)
	{
		effect->spawn();
	}
	else
	{
		TT_ASSERT(p_spawnType == SpawnType_NoSpawn);
	}
	
	return effect;
}


tt::math::Vector3 Entity::getPositionForParticles(const tt::math::Vector2& p_offset) const
{
	// FIXME: This code is no longer valid, since we have multiple presentation objects per entity
	/*
	// If this entity has a PresentationObject, use that to base the position on
	if (ttPres != 0)
	{
		return tt::math::Vector3(p_offset.x, p_offset.y, 0.0f) * ttPres->getCombinedMatrix();
	}
	*/
	
	// Otherwise, use the center point of the Entity as the base for the position
	const tt::math::Vector2 pos(getCenterPosition() + applyOrientationToVector2(p_offset));
	return tt::math::Vector3(pos.x, pos.y, 0.0f);
}


void Entity::renderDebug(bool p_hudRenderPass)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	TT_ASSERT(isInitialized());
	
	if (p_hudRenderPass != m_inScreenspace)
	{
		return;
	}
	
	// Debug renders
	const DebugRenderMask& mask = AppGlobal::getDebugRenderMask();
	using namespace tt::engine;
	renderer::Renderer* renderer = renderer::Renderer::getInstance();
	const debug::DebugRendererPtr debug(renderer->getDebug());
	
	if (mask.checkFlag(DebugRender_RenderCullingRects) )
	{
		if (isPositionCulled())
		{
			debug->renderSolidRect(renderer::ColorRGBA(100, 255, 255, 128), m_worldRect);
			return;
		}
		if (isPositionCullingEnabled() == false)
		{
			debug->renderSolidRect(renderer::ColorRGBA(255, 160, 0, 128), m_worldRect);
		}
	}
	
	if (mask.checkFlag(DebugRender_DisableDebugRenderForEntityWithParent))
	{
		// Don't render debug rects for entity with a parent.
		movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
		if (controller != 0)
		{
			if (controller->getParentEntity().isEmpty() == false)
			{
				return;
			}
		}
	}
	
	if (mask.checkFlag(DebugRender_EntityVibrationDetectionPoints))
	{
		// Visualize sight detection points
		for (DetectionPoints::const_iterator it = m_vibrationDetectionPoints.begin();
				it != m_vibrationDetectionPoints.end(); ++it)
		{
			const tt::math::Vector2 pos(getCenterPosition() + applyOrientationToVector2(*it));
			
			debug->renderSolidRect(tt::engine::renderer::ColorRGB::orange,
			tt::math::VectorRect(
				pos - tt::math::Vector2(0.075f, 0.075f),
				pos + tt::math::Vector2(0.075f, 0.075f)));
		}
	}
	
	if (mask.checkFlag(DebugRender_EntitySightDetectionPoints))
	{
		// Visualize sight detection points
		for (DetectionPoints::const_iterator it = m_sightDetectionPoints.begin();
				it != m_sightDetectionPoints.end(); ++it)
		{
			const tt::math::Vector2 pos(getCenterPosition() + applyOrientationToVector2(*it));
			
			debug->renderSolidRect(tt::engine::renderer::ColorRGB::yellow,
			tt::math::VectorRect(
				pos - tt::math::Vector2(0.075f, 0.075f),
				pos + tt::math::Vector2(0.075f, 0.075f)));
		}
	}
	
	if (mask.checkFlag(DebugRender_EntityLightDetectionPoints))
	{
		// Visualize light detection points
		for (DetectionPoints::const_iterator it = m_lightDetectionPoints.begin();
				it != m_lightDetectionPoints.end(); ++it)
		{
			const tt::math::Vector2 pos(getCenterPosition() + applyOrientationToVector2(*it));
			
			debug->renderSolidRect(tt::engine::renderer::ColorRGB::blue,
			tt::math::VectorRect(
				pos - tt::math::Vector2(0.075f, 0.075f),
				pos + tt::math::Vector2(0.075f, 0.075f)));
		}
	}
	
	if (mask.checkFlag(DebugRender_EntityTouchShape))
	{
		// Visualize touch shape
		if (m_touchShape != 0)
		{
			m_touchShape->visualize(tt::engine::renderer::ColorRGB::green);
		}
	}
	
	if (mask.checkFlag(DebugRender_EntityCollisionRect))
	{
		const u8 opacity = m_tileRegistrationEnabled ? 255 : 100;
		
		using tt::engine::renderer::ColorRGBA;
		if (isSuspended() == false)
		{
			debug->renderRect(ColorRGBA(255, 0,  0, opacity), m_worldRect);
		}
		else
		{
			debug->renderRect(ColorRGBA(128, 128, 128, opacity), m_worldRect);
		}
	}
	if (mask.checkFlag(DebugRender_EntityRegisteredRect))
	{
		debug->renderRect(tt::engine::renderer::ColorRGB::yellow, m_registeredTileRect);
	}
	if (mask.checkFlag(DebugRender_EntityMoveToRect))
	{
		debug->renderRect(tt::engine::renderer::ColorRGB::blue, m_willEndMoveHereRect);
	}
	if (mask.checkFlag(DebugRender_EntityTileRect))
	{
		debug->renderRect(tt::engine::renderer::ColorRGB::orange, calcWorldTileRect());
	}
	if (mask.checkFlag(DebugRender_EntityPosition))
	{
		debug->renderSolidRect(tt::engine::renderer::ColorRGB::red,
			tt::math::VectorRect(
				getPosition() - tt::math::Vector2(0.075f, 0.075f),
				getPosition() + tt::math::Vector2(0.075f, 0.075f)));
		
		debug->renderSolidRect(tt::engine::renderer::ColorRGB::green,
			tt::math::VectorRect(
				getCenterPosition() - tt::math::Vector2(0.075f, 0.075f),
				getCenterPosition() + tt::math::Vector2(0.075f, 0.075f)));
	}
	if (mask.checkFlag(DebugRender_EntityCollisionParent))
	{
		const movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
		if (ctrl != 0)
		{
			const Entity* collisionAncestor = ctrl->getCollisionAncestor().getPtr();
			const Entity* collisionParent   = ctrl->getCollisionParentEntity().getPtr();
			
			if (collisionAncestor != 0               &&
			    collisionAncestor != collisionParent && // no need to draw the same line twice
			    collisionAncestor != this)
			{
				const tt::math::Vector2 ourCenter   (getCenterPosition());
				const tt::math::Vector2 ancestorCenter(collisionAncestor->getCenterPosition());
				
				debug->renderLine(
						tt::engine::renderer::ColorRGB(200, 200, 200),
						tt::math::Vector3(ourCenter.x,      ourCenter.y,      0.0f),
						tt::math::Vector3(ancestorCenter.x, ancestorCenter.y, 0.0f));
			}
			
			if (collisionParent != 0)
			{
				const tt::math::Vector2 ourCenter   (getCenterPosition());
				const tt::math::Vector2 parentCenter(collisionParent->getCenterPosition());
				
				debug->renderLine(
						tt::engine::renderer::ColorRGB::magenta,
						tt::math::Vector3(ourCenter.x,    ourCenter.y,    0.0f),
						tt::math::Vector3(parentCenter.x, parentCenter.y, 0.0f));
			}
			
			char buf[64] = { 0 };
			sprintf(buf, "0x%08X", static_cast<unsigned int>(m_handle.getValue()));
			
			const tt::math::Point2 textPos(
					AppGlobal::getGame()->getCamera().worldToScreen(m_pos) + tt::math::Point2(10, -15));
			debug->renderText(buf, textPos.x, textPos.y, tt::engine::renderer::ColorRGB::green);
		}
	}
	
	{
		bool frameAnimMissing = false;
		
		toki::pres::PresentationObject* pres = 0;
		tt::pres::ConstPresentationObjectPtr ttPres;
		if (m_presentationObjects.empty() == false)
		{
			if (m_debugPresentationObjectIdx >= m_presentationObjects.size())
			{
				m_debugPresentationObjectIdx = 0;
			}
			
			pres = m_presentationObjects.at(m_debugPresentationObjectIdx).getPtr();
			if (pres != 0)
			{
				ttPres = pres->getPresentationObject();
			}
		}
		
		// Debug render a cross with a solid circle if the frameanimation of this entity is not visible
		DebugView& debugView = AppGlobal::getGame()->getDebugView();
		if (ttPres != 0)
		{
			if (ttPres->isMissingFrame())
			{
				if (mask.checkFlag(DebugRender_EntityMissingFrame))
				{
					debugView.registerLine(DebugView::LineInfo(
						getPosition() + tt::math::Vector2(-1.0f, -1.0f),
						getPosition() + tt::math::Vector2( 1.0f,  1.0f),
						tt::engine::renderer::ColorRGB::yellow, -1.0f));
					debugView.registerLine(DebugView::LineInfo(
						getPosition() + tt::math::Vector2(-1.0f,  1.0f),
						getPosition() + tt::math::Vector2( 1.0f, -1.0f),
						tt::engine::renderer::ColorRGB::yellow, -1.0f));
					
					debugView.registerCircle(DebugView::CircleInfo(getPosition(), 1.0f, true,
						tt::engine::renderer::ColorRGBA(255, 150, 150, 255), -1.0f));
					
					frameAnimMissing = true;
				}
				
				if (m_debugShowTagChanges)
				{
					TT_Printf("[%u] Entity::renderDebug: [Update Frame: %u] [0x%08X, '%s'] missing frame! (ttPres->isVisible() == false).\n",
					          AppGlobal::getUpdateFrameCount(), AppGlobal::getUpdateFrameCount(), this, getType().c_str());
				}
			}
		}
		
		if (m_debugShowTagChanges || frameAnimMissing)
		{
			tt::math::Point2 entityScreenPos(AppGlobal::getGame()->getCamera().worldToScreen(getPosition()));
			if (m_inScreenspace)
			{
				tt::math::Vector2 screenPos(getPosition());
				screenPos   *= renderer->getScreenHeight();
				screenPos.x += (0.5f * renderer->getScreenWidth( ));
				screenPos.y -= (0.5f * renderer->getScreenHeight());
				entityScreenPos.setValues(static_cast<s32>(screenPos.x), static_cast<s32>(-screenPos.y));
			}
			const s32 x = entityScreenPos.x;
			s32 y       = entityScreenPos.y + 5;
			const s32 lineHeight = 17;
			
			static const tt::engine::renderer::ColorRGBA shadowCol(tt::engine::renderer::ColorRGB::black);
			static const s32                             shadowOffset = 1;
			
			std::string text;
			
			if (ttPres != 0)
			{
				text = "[" + tt::str::toStr(m_debugPresentationObjectIdx+1) + "/" + 
					tt::str::toStr(m_presentationObjects.size()) + "]";
				debug->renderText(text, x + shadowOffset, y + shadowOffset, shadowCol);
				debug->renderText(text, x, y, tt::engine::renderer::ColorRGB::green);
				y += lineHeight;
				
				const std::string& animationName = ttPres->getCurrentActiveName();
				text = "Current Name: '" + animationName + "'";
				debug->renderText(text, x + shadowOffset, y + shadowOffset, shadowCol);
				debug->renderText(text, x, y, tt::engine::renderer::ColorRGB::green);
				y += lineHeight;
				
				// The original strings in Hash are only available in non-final (tt::pres::Tag is a Hash)
				const tt::pres::Tags& activeTags(ttPres->getCurrentActiveTags());
				
				text = "Current Tags:";
				debug->renderText(text, x + shadowOffset, y + shadowOffset, shadowCol);
				debug->renderText(text, x, y, tt::engine::renderer::ColorRGB::green);
				y += lineHeight;
				
				for (tt::pres::Tags::const_iterator it = activeTags.begin(); it != activeTags.end(); ++it)
				{
					debug->renderText((*it).getName(), x + shadowOffset, y + shadowOffset, shadowCol);
					debug->renderText((*it).getName(), x, y, tt::engine::renderer::ColorRGB::green);
					y += lineHeight;
				}
				
				if (activeTags.empty())
				{
					text = "<No Tags>";
					debug->renderText(text, x + shadowOffset, y + shadowOffset, shadowCol);
					debug->renderText(text, x, y, tt::engine::renderer::ColorRGB::green);
					y += lineHeight;
				}
			}
			
			if (pres != 0 && pres->isAffectedByMovement())
			{
				movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
				if (controller != 0)
				{
					std::string moveName = "<no current move>";
					std::string moveAnimationName;
					
					movement::MoveBasePtr currentMove = controller->getCurrentMove();
					
					if (currentMove != 0)
					{
						moveName = currentMove->getName();
						if (moveName.empty())
						{
							moveName = "<unnamed_move>";
						}
						
						moveAnimationName = currentMove->getAnimationName();
						if (moveAnimationName.empty())
						{
							moveAnimationName = "<unnamed move animation>";
						}
					}
					moveName = "Move Name: '" + moveName + "'";
					
					debug->renderText(moveName, x + shadowOffset, y + shadowOffset, shadowCol);
					debug->renderText(moveName, x, y, tt::engine::renderer::ColorRGB::magenta);
					y += lineHeight;
					
					if (moveAnimationName.empty() == false)
					{
						moveAnimationName = "Move Animation Name: '" + moveAnimationName + "'";
						
						debug->renderText(moveAnimationName, x + shadowOffset, y + shadowOffset, shadowCol);
						debug->renderText(moveAnimationName, x, y, tt::engine::renderer::ColorRGB::magenta);
						y += lineHeight;
					}
				}
			}
			
			// Show debug string
			if (m_scriptDefinedDebugString.empty() == false)
			{
				y += lineHeight;
				debug->renderText(m_scriptDefinedDebugString, x + shadowOffset, y + shadowOffset, shadowCol);
				debug->renderText(m_scriptDefinedDebugString, x, y, tt::engine::renderer::ColorRGB::yellow);
			}
		}
	}
#else
	(void)p_hudRenderPass;
#endif
}


void Entity::kill()
{
	TT_NULL_ASSERT(m_entityScript);
	TT_ASSERT(isInitialized());
	if (m_state == State_Initialized)
	{
		// Set state to Dying to prevent multiple kill calls to this entity because of
		// circular calls in onDie().
		m_state = State_Dying;
		m_entityScript->onDie();
		AppGlobal::getGame()->getEntityMgr().deinitEntity(m_handle);
	}
}


void Entity::setSuspended(bool p_suspended)
{
	m_suspended = p_suspended;
	if (m_suspended == false)
	{
		script::TimerMgr::resumeAllTimers(m_handle);
		
		// We could be on a whole new position. reevaluate current position.
		movementcontroller::DirectionalMovementController* controller = m_movementControllerHandle.getPtr();
		if (controller != 0)
		{
			controller->updateLocalCollision(getRegisteredTileRect(), *this);
		}
		
		updateSurvey(true);
		
		/*
		// Rewind doesn't use onLightEnter/Exit callbacks
		if (AppGlobal::getGame()->getLightMgr().isEntityInLight(*this))
		{
			onLightEnter();
		}
		else
		{
			onLightExit();
		}
		*/
		
		// We could be on a whole new position. reevaluate current position.
		if (controller != 0)
		{
			controller->scheduleReselectMove();
		}
	}
	else
	{
		script::TimerMgr::suspendAllTimers(m_handle);
	}
}


void Entity::setCanBeCarried(bool p_canBeCarried)
{
	movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
	if (ctrl != 0)
	{
		ctrl->setCollisionParentEnabled(p_canBeCarried);
	}
	
	m_canBeCarried = p_canBeCarried;
}


void Entity::reevaluateCollisionParent(const EntityHandle& p_caller)
{
	TT_ASSERT(canBeCarried());
	TT_ASSERT(isInitialized());
#if TOKI_CARRYMOVEMENT_DEBUGGING
	const EntityHandle prevParent = getCollisionParentEntityScheduled();
#endif
	
	EntityHandle standOnParentHandle = determineStandOnParent(p_caller);
	Entity*      standOnParent       = standOnParentHandle.getPtr();
	if (standOnParent != 0)
	{
		setCollisionParentEntity(standOnParentHandle);
	}
	else if (getCollisionParentEntityScheduled().isEmpty() == false)
	{
		setCollisionParentEntity(EntityHandle());
	}
	
#if TOKI_CARRYMOVEMENT_DEBUGGING
	if (getCollisionParentEntityScheduled() != prevParent)
	{
		TT_Printf("[%06u] E::reCP: [0x%08X] Parent changed from 0x%08X to 0x%08X\n",
		          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), prevParent.getValue(), getCollisionParentEntityScheduled().getValue());
	}
#endif
}


void Entity::makeSurroundingEntitiesScheduleReevaluateCollisionParent()
{
	if (m_collisionTiles == 0)
	{
		return;
	}
	
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	
	const tt::math::PointRect ownTilesRect(m_collisionTiles->getPos(),
	                                       m_collisionTiles->getWidth(),
	                                       m_collisionTiles->getHeight());
	const tt::math::PointRect tilesToCheck(ownTilesRect.getMin()       - tt::math::Point2(1, 1),
	                                       ownTilesRect.getMaxInside() + tt::math::Point2(1, 1));
	
	EntityHandleSet entities;
	AppGlobal::getGame()->getTileRegistrationMgr().findRegisteredEntityHandles(tilesToCheck, entities);
	
	for (EntityHandleSet::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if (*it == getHandle())
		{
			continue;
		}
		
		Entity* otherEntity = entityMgr.getEntity(*it);
		if (otherEntity != 0 && otherEntity->isInitialized())
		{
			otherEntity->scheduleReevaluateCollisionParent();
		}
	}
}


void Entity::scheduleReevaluateCollisionParent()
{
	TT_ASSERT(isInitialized());
	
	movementcontroller::DirectionalMovementController* ctrl = getDirectionalMovementController();
	if (ctrl != 0)
	{
		ctrl->scheduleReevaluateCollisionParent();
	}
}


void Entity::onPotentialCollisionParentStartsMove(const EntityHandle& p_caller)
{
	TT_ASSERT(isInitialized());
	if (p_caller == getHandle())
	{
		return;
	}
	
	EntityHandle standOnParentHandle = determineStandOnParent(p_caller);
	if (standOnParentHandle != getCollisionParentEntityScheduled())
	{
		CM_Printf("[%06u] Entity::onPotentialCollisionParentStartsMove: [0x%08X] Got parent 0x%08X; notifying all surrounding entities.\n",
		          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), standOnParentHandle.getValue());
		
		if (setCollisionParentEntity(standOnParentHandle) == false)
		{
			// Failed so early exit, otherwise things down can cause an endless loop
			return;
		}
		
		if (m_collisionTiles != 0)
		{
			EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
			
			const tt::math::PointRect ownTilesRect(m_collisionTiles->getPos(),
			                                       m_collisionTiles->getWidth(),
			                                       m_collisionTiles->getHeight());
			const tt::math::PointRect tilesToCheck(ownTilesRect.getMin()       - tt::math::Point2(1, 1),
			                                       ownTilesRect.getMaxInside() + tt::math::Point2(1, 1));
			
			EntityHandleSet entities;
			AppGlobal::getGame()->getTileRegistrationMgr().findRegisteredEntityHandles(tilesToCheck, entities);
			
			for (EntityHandleSet::iterator it = entities.begin(); it != entities.end(); ++it)
			{
				if (*it == getHandle() || *it == p_caller)
				{
					continue;
				}
				
				Entity* otherEntity = entityMgr.getEntity(*it);
				if (otherEntity != 0)
				{
					otherEntity->onPotentialCollisionParentStartsMove(getHandle());
				}
			}
		}
	}
}


EntityHandle Entity::getCollisionParentEntity() const
{
	if (isInitialized() == false)
	{
		return EntityHandle();
	}
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	return (controller == 0) ? EntityHandle() : controller->getCollisionParentEntity();
}


EntityHandle Entity::getCachedCollisionAncestor() const
{
	if (isInitialized() == false)
	{
		return EntityHandle();
	}
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	return (controller == 0) ? EntityHandle() : controller->getCollisionAncestor();
}


EntityHandle Entity::getCollisionAncestor(bool p_useScheduledParents) const
{
	return getCollisionAncestor(0, EntityHandle(), p_useScheduledParents);
}


s32 Entity::getCollisionAncestryDepth(bool p_useScheduledParents) const
{
	s32 ancestryDepth = 0;
	getCollisionAncestor(&ancestryDepth, EntityHandle(), p_useScheduledParents);
	return ancestryDepth;
}


bool Entity::isInCollisionAncestry(const EntityHandle& p_entity, bool p_useScheduledParents) const
{
	return getCollisionAncestor(0, p_entity, p_useScheduledParents) == p_entity;
}


bool Entity::hasCollisionParent(bool p_useScheduledParents) const
{
	EntityHandle collisionParent = p_useScheduledParents ?
			getCollisionParentEntityScheduled() : getCollisionParentEntity();
	return collisionParent.getPtr() != 0;
}


void Entity::onMovementEnded(movement::Direction p_direction)
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onMovementEnded(p_direction);
}


void Entity::onMovementFailed(movement::Direction p_direction, const std::string& p_moveName)
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onMovementFailed(p_direction, p_moveName);
}


void Entity::onPathMovementFailed(const tt::math::Vector2& p_closestPoint)
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onPathMovementFailed(p_closestPoint);
}


void Entity::onSolidCollision(const tt::math::Vector2& p_collisionNormal, const tt::math::Vector2& p_speed)
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onSolidCollision(p_collisionNormal, p_speed);
}


void Entity::onPhysicsTurn()
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onPhysicsTurn();
}


void Entity::onTileChange()
{
	TT_ASSERT(isInitialized());
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	if (controller != 0)
	{
		controller->scheduleSurveyUpdate();
		controller->updateLocalCollision(getRegisteredTileRect(), *this);
	}
	updateSurvey(true);
}


void Entity::handleEvent(const toki::game::event::Event& p_event)
{
	// Ignore all events if entity is suspended 
	// FIXME: Perhaps optimize this, since events are still generated for this entity
	if (m_suspended)
	{
		return;
	}
	
	TT_NULL_ASSERT(m_entityScript);
	using namespace event;
	
	switch (p_event.getType())
	{
	case EventType_Sound:
		m_entityScript->onSound(p_event);
		break;
		
	case EventType_Vibration:
		m_entityScript->onVibration(p_event);
		break;
		
	default:
		TT_PANIC("Unhandled event '%d'", p_event.getType());
		break;
	}
}


void Entity::onCarryBegin(const EntityHandle& p_carryingEntity)
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onCarryBegin(p_carryingEntity);
}


void Entity::onCarryEnd()
{
	TT_NULL_ASSERT(m_entityScript);
	m_entityScript->onCarryEnd();
}


Entity::Entity(const CreationParams& /*p_creationParams*/, const EntityHandle& p_ownHandle)
:
m_orientationDown(movement::Direction_Down),
m_state(State_Created),
m_suspended(false),
m_orientationForwardIsLeft(false),
m_inScreenspace(false),
m_flowToTheRight(true),
m_submergeDepth(1),
m_debugPresentationObjectIdx(0),
m_tileRegistrationEnabled(true),
m_updateSurvey(false),
m_debugShowTagChanges(false),
m_isDetectableByLight(false),
m_isInLight(false),
m_isLightBlocking(false),
m_scriptIsInLight(false),
m_isDetectableBySight(true),
m_isDetectableByTouch(true),
m_positionCullingEnabled(false),
m_positionCullingInitialized(false),
m_isPositionCulled(false),
m_isOnScreen(false),
m_canBePushed(false),
m_canBeCarried(false),
m_canBePaused(true),
m_pathAgentRadius(-1.0f),
m_pathCrowdSeparation(false),
m_collisionTilesActive(false),
m_handle(p_ownHandle),
m_pos(tt::math::Vector2::zero),
m_worldRect(),
m_localRect(),
m_localTileRect(),
m_willEndMoveHereRect(),
m_registeredTileRect(),
m_survey(),
m_prevSurvey(),
m_presentationObjects(),
m_movementControllerHandle(),
m_powerBeamGraphics(),
m_textLabels(),
m_effectRects(),
m_sensors(),
m_filteredBySensors(),
m_tileSensors(),
m_lights(),
m_darknesses(),
m_vibrationDetectionPoints(),
m_sightDetectionPoints(),
m_lightDetectionPoints(),
m_touchShape(),
m_touchShapeOffset(tt::math::Vector2::zero),
m_positionCullingParent(EntityHandle()),
m_collisionTiles(),
m_collisionTilesRegdRect(),
m_collisionTileOffset(tt::math::Point2::zero),
m_entityScript(),
m_scriptDefinedDebugString()
{
}


Entity* Entity::getPointerFromHandle(const EntityHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getEntity(p_handle);
}


void Entity::invalidateTempCopy()
{
	if (isInitialized())
	{
		deinit();
	}
}


const movementcontroller::DirectionalMovementController* Entity::getDirectionalMovementController() const
{
	TT_ASSERT(isInitialized());
	return getMovementControllerHandle().getPtr();
}


void Entity::initPositionCulling(const tt::math::VectorRect& p_cullingRect)
{
	// Default is no culling
	m_isPositionCulled           = false;
	m_positionCullingInitialized = false;
	
	if (m_inScreenspace == false)
	{
		// Parent logic (parents should be initialized here!)
		if (m_positionCullingParent.isEmpty() == false)
		{
			Entity* parent = m_positionCullingParent.getPtr();
			// Parent can be null here (just been killed)
			if (parent != 0)
			{
				TT_ASSERTMSG(parent->hasPositionCullingParent() == false, 
					"Position culling parents cannot have position culling parents themselves.");
				
				if (parent->m_positionCullingEnabled)
				{
					TT_ASSERTMSG(parent->m_positionCullingInitialized, 
						"Parent position culling should be initialized before initializing its children.");
					
					// Copy culling flag from parent
					m_isPositionCulled = parent->m_isPositionCulled;
				}
			}
		}
		else if (m_positionCullingEnabled && m_worldRect.intersects(p_cullingRect) == false)
		{
			m_isPositionCulled = true;
		}
	}
	
	setPositionCulled(m_isPositionCulled);
	
	m_positionCullingInitialized = true;
}


void Entity::updatePositionCulling(const tt::math::VectorRect& p_cullingRect,
                                   const tt::math::VectorRect& p_uncullingRect)
{
	if (m_positionCullingInitialized == false)
	{
		initPositionCulling(p_cullingRect);
		return;
	}
	
	// FIXME: Cull screenspace entities
	if (m_inScreenspace)
	{
		return;
	}
	
	// Parent logic first
	if (m_positionCullingParent.isEmpty() == false)
	{
		Entity* parent = m_positionCullingParent.getPtr();
		if (parent != 0)
		{
			if (parent->m_positionCullingEnabled == false)
			{
				return;
			}
			
			if (parent->m_isPositionCulled != m_isPositionCulled)
			{
				// Copy culling flag from parent
				setPositionCulled(parent->m_isPositionCulled);
			}
			return;
		}
	}
	
	// No parent; handle normal logic
	if (m_positionCullingEnabled == false)
	{
		return;
	}
	
	if (m_isPositionCulled)
	{
		// Don't rewrite these if statements into a single check with &&
		if (m_worldRect.intersects(p_uncullingRect))
		{
			setPositionCulled(false);
		}
	}
	else
	{
		// Don't rewrite these if statements into a single check with &&
		if (m_worldRect.intersects(p_cullingRect) == false)
		{
			setPositionCulled(true);
		}
	}
}

static const std::string g_onScreenEnter("onScreenEnter");
static const std::string g_onScreenExit("onScreenExit");

void Entity::updateIsOnScreen(const tt::math::VectorRect& p_screenRect)
{
	const bool isOnScreen = m_worldRect.intersects(p_screenRect);
	if (m_isOnScreen != isOnScreen)
	{
		m_entityScript->queueSqFun(isOnScreen ? g_onScreenEnter : g_onScreenExit);
		m_isOnScreen = isOnScreen;
	}
}


#if !defined(TT_BUILD_FINAL)
std::string Entity::getDebugInfo() const
{
	const tt::math::Vector2& pos(getPosition());
	const s32 id = AppGlobal::getGame()->getEntityMgr().getEntityIDByHandle(m_handle);
	return "[" + getType() + ":" + tt::str::toStr(id) + " (" + tt::str::toStr(pos.x) + ", " + tt::str::toStr(pos.y) + ")]";
}
#endif


//--------------------------------------------------------------------------------------------------
// Private member functions

bool Entity::load(const std::string& p_type, s32 p_id)
{
	TT_ASSERT(m_state == State_Created);
	TT_ASSERT(m_entityScript == 0);
	m_entityScript = script::EntityBase::create(m_handle, p_type, p_id);
	
	if (m_entityScript == 0)
	{
		// Do not trigger a panic here, as the creation code should already have triggered those
		return false;
	}
	
	const level::entity::EntityInfo* entityInfo = AppGlobal::getEntityLibrary().getEntityInfo(p_type);
	if (entityInfo == 0)
	{
		TT_PANIC("No EntityInfo is available for entity type '%s'.", p_type.c_str());
		return false;
	}
	
	m_pathAgentRadius     = entityInfo->getPathFindAgentRadius();
	m_pathCrowdSeparation = entityInfo->hasPathCrowdSeparation();
	
	{
		// Set the initial collision rect (defined by the entity type)
		setLocalRects(entityInfo->getCollisionRect());
	}
	
	m_state = State_Loaded;
	return true;
}


void Entity::init(const tt::math::Vector2& p_position)
{
	// Make sure we're in the right state.
	TT_ASSERT(m_state == State_Loaded);
	
	m_pos   = p_position;
	//snapToStandOnSolid();
	
	m_state = State_Initialized;
	
	updateRects();
	
	updateSurvey(false);
	
	level::TileRegistrationMgr& mgr(AppGlobal::getGame()->getTileRegistrationMgr());
	mgr.registerEntityHandle(m_registeredTileRect, m_handle);
	
	//update(0.0f);
	
	TT_NULL_ASSERT(m_entityScript);
}


void Entity::init(const tt::math::Vector2& p_position, EntityProperties& p_properties, bool p_gameReloaded)
{
	init(p_position);
	m_entityScript->init(p_properties, p_gameReloaded);
	doSurveyCallbacks();
}


void Entity::init(const tt::math::Vector2& p_position, const HSQOBJECT& p_properties)
{
	init(p_position);
	m_entityScript->init(p_properties);
	doSurveyCallbacks();
}


void Entity::deinit()
{
	TT_ASSERT(isInitialized());
	
	// Cleanup before changing state.
	removeCollisionTiles();
	
	// Change state.
	m_state = State_Loaded;
	m_entityScript->deinit();
	
	EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	entityMgr.getMovementControllerMgr().destroyController(m_movementControllerHandle);
	m_movementControllerHandle.invalidate();
	
	removeAllSensorFilter();
	TT_ASSERT(m_filteredBySensors.empty());
	removeAllEffectRects();
	removeAllSensors();
	removeAllLights();
	removeAllDarknesses();
	removeAllPowerBeamGraphics();
	removeAllTextLabels();
	removeAllSightDetectionPoints();
	removeAllLightDetectionPoints();
	
	if (m_inScreenspace)
	{
		m_inScreenspace = false;
		AppGlobal::getGame()->removeScreenSpaceEntity(getHandle());
	}
	
	destroyAllPresentationObjects();
	
	script::TimerMgr::stopAllTimers(m_handle);
	
	level::TileRegistrationMgr& tileMgr(AppGlobal::getGame()->getTileRegistrationMgr());
	tileMgr.unregisterEntityHandle(m_registeredTileRect, m_handle);
}


tt::math::PointRect Entity::calcCollisionTilesRegdRect() const
{
	if (m_collisionTiles == 0)
	{
		TT_PANIC("Cannot calculate collision tiles registered tile rect if entity has no collision tiles.");
		return tt::math::PointRect(tt::math::Point2(0, 0), 0, 0);
	}
	
	TT_ASSERTMSG(m_orientationDown == movement::Direction_Down ||
	             m_orientationDown == movement::Direction_None,
	             "Entities with their own collision tiles are only supported in 'normal' orientation.");
	
	return level::worldToTile(tt::math::VectorRect(
			m_pos + m_localTileRect.getPosition() + level::tileToWorld(m_collisionTileOffset),
			level::tileToWorld(m_collisionTiles->getWidth() ) - 0.0001f,
			level::tileToWorld(m_collisionTiles->getHeight()) - 0.0001f));
}


void Entity::setLocalRects(const tt::math::VectorRect& p_rect)
{
	// Update the local rects, but make it a little bit smaller
	m_localRect.setValues(p_rect.getPosition() + g_rectResizeSmallVec,
	                      p_rect.getWidth()    - g_rectResizeDoubleSmallValue,
	                      p_rect.getHeight()   - g_rectResizeDoubleSmallValue);
	
	const tt::math::Vector2 size(m_localRect.getWidth(), m_localRect.getHeight());
	const tt::math::Vector2 sizeInTiles(level::worldToTile(tt::math::ceil(size)));
	
	// We want a whole number or halves as tile position
	const tt::math::Vector2 position((tt::math::floor(m_localRect.getPosition() * 2.0f)) * 0.5f);
	
	m_localTileRect = tt::math::VectorRect(position, sizeInTiles.x, sizeInTiles.y);
}


EntityHandle Entity::determineStandOnParent(const EntityHandle& p_caller) const
{
	if (m_orientationDown == movement::Direction_None)
	{
		// This entity doesn't stand on anything: cannot stand on any parent
		return EntityHandle();
	}
	
	if (getDirectionalMovementController() == 0)
	{
		// This entity does not have a movement controller, and as such it cannot move,
		// not even when carried by another entity (so cannot have a "stand on" parent either)
		return EntityHandle();
	}
	
	static const real edgeEpsilon = 0.05f;  // wiggle room for "on edge"
	
	// Check whether this entity is aligned to a level tile boundary
	const tt::math::Vector2 ourSnappedPos(getSnappedToTilePosLevelOnly());
	const bool isTileAligned =
			hasCollisionTiles() == false ||  // no alignment requirement if we do not have our own collision tiles
			(tt::math::fabs(ourSnappedPos.x - getPosition().x) < edgeEpsilon &&
			 tt::math::fabs(ourSnappedPos.y - getPosition().y) < edgeEpsilon);
	
	EntityMgr&         entityMgr             = AppGlobal::getGame()->getEntityMgr();
	const EntityHandle collisionParentHandle = getCollisionParentEntityScheduled();
	const Entity*      collisionParent       = entityMgr.getEntity(collisionParentHandle);
	
	/*
	if (isTileAligned == false)
	{
		CM_Printf("[%06u] E::dSOP: [0x%08X] [C 0x%08X] [P 0x%08X] Entity is not tile aligned!\n",
		          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), p_caller.getValue(),
		          collisionParentHandle.getValue());
	}
	// */
	
	// If this entity is not horizontally aligned with a tile (and does not already have a parent),
	// it cannot stand on anything
	if (isTileAligned == false && collisionParent == 0)
	{
		CM_Printf("[%06u] E::dSOP: [0x%08X] [C 0x%08X] [P 0x%08X] Not aligned and no existing parent: not picking new one.\n",
		          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), p_caller.getValue(),
		          collisionParentHandle.getValue());
		return EntityHandle();
	}
	
	s32 staticCount = -1;  // bias against static tiles (prefer a left/right tile)
	s32 leftCount   = 0;
	s32 rightCount  = 0;
	
	// Determine what the tile row "below" the entity is
	tt::math::Point2 startPos; // where the row starts
	tt::math::Point2 endPos;   // one-past where the row ends
	tt::math::Point2 right;    // what to add to move one tile to the "right" in the row
	tt::math::Vector2 startPosVec; // The start pos as vector (in world space.)
	tt::math::Vector2 endPosVec;   // The start pos as vector (in world space.)
	
	switch (m_orientationDown)
	{
	case movement::Direction_Down:
		// "Below" is just that: one tile lower in Y
		startPos.setValues(m_registeredTileRect.getMin().x, m_registeredTileRect.getMin().y - 1   );
		startPosVec.setValues(      m_worldRect.getMin().x,          m_worldRect.getMin().y - 1.0f);
		endPos.setValues  (m_registeredTileRect.getMaxEdge().x, startPos.y   );
		endPosVec.setValues(        m_worldRect.getMaxEdge().x, startPosVec.y);
		right.setValues(1, 0);
		break;
		
	case movement::Direction_Up:
		// "Below" is actually above the entity
		startPos.setValues(m_registeredTileRect.getMaxInside().x     , m_registeredTileRect.getMaxEdge().y);
		startPosVec.setValues(      m_worldRect.getMaxEdge().x - 1.0f,          m_worldRect.getMaxEdge().y); // Note vector doesn't modify getMaxInside, do edge - 1.0 to get the same value.
		endPos.setValues  (m_registeredTileRect.getMin().x - 1   ,   startPos.y   );
		endPosVec.setValues(        m_worldRect.getMin().x - 1.0f,   startPosVec.y);
		right.setValues(-1, 0);
		break;
		
	case movement::Direction_Left:
		// "Below" is to the left of the entity
		startPos.setValues(m_registeredTileRect.getMin().x - 1   , m_registeredTileRect.getMaxInside().y);
		startPosVec.setValues(      m_worldRect.getMin().x - 1.0f,          m_worldRect.getMaxEdge().y - 1.0f);
		endPos.setValues   (startPos.x   , m_registeredTileRect.getMin().y - 1   );
		endPosVec.setValues(startPosVec.x,          m_worldRect.getMin().y - 1.0f);
		right.setValues(0, -1);
		break;
		
	case movement::Direction_Right:
		startPos.setValues(m_registeredTileRect.getMaxEdge().x, m_registeredTileRect.getMin().y);
		startPosVec.setValues(      m_worldRect.getMaxEdge().x, m_worldRect.getMin().y         );
		endPos.setValues  ( startPos.x   , m_registeredTileRect.getMaxEdge().y);
		endPosVec.setValues(startPosVec.x,          m_worldRect.getMaxEdge().y);
		right.setValues(0, 1);
		break;
		
	default:
		TT_PANIC("Unsupported 'down' orientation: %d", m_orientationDown);
		return EntityHandle();
	}
	
	level::AttributeLayerPtr    attribLayer(AppGlobal::getGame()->getAttributeLayer());
	level::TileRegistrationMgr& tileMgr    (AppGlobal::getGame()->getTileRegistrationMgr());
	
	const EntityTiles* ownCollisionTiles = getCollisionTiles();
	
	EntityTilesPtr parentTiles;
	if (collisionParent != 0)
	{
		parentTiles = collisionParent->m_collisionTiles;
	}
	const tt::math::Vector2 parentWorldTilePos((collisionParent != 0) ?
			collisionParent->calcEntityTileRect().getMin() :
			tt::math::Vector2::zero);
	
	EntityHandle leftMostEntity;
	EntityHandle rightMostEntity;
	EntityHandle upEntity;
	EntityHandle downEntity;
	
	bool foundCallerInTiles     = false;
	bool parentIsStillCandidate = false;  // whether existing collision parent is still one of the possible choices
	
	tt::math::Vector2 posVec(startPosVec);
	for (tt::math::Point2 pos(startPos); pos != endPos; pos += right, posVec += tt::math::Vector2(right))
	{
		// Find out if this tile is solid, ignoring our own tiles, but taking parent-relative tiles into account
		
		bool           tileIsSolid                = false;
		bool           foundTileInCollisionParent = false;
		EntityTilesPtr entityTilesReportingSolid;
		
		if (collisionParent != 0 && parentTiles != 0)
		{
			const tt::math::Point2 parentLocalCheckPos(level::worldToTile(
			     (posVec + tt::math::Vector2(0.5f, 0.5f) - parentWorldTilePos)));
			
			if (parentTiles->containsLocal(parentLocalCheckPos))
			{
				foundTileInCollisionParent = true;
				
				if (level::isSolid(parentTiles->getCollisionTypeLocal(parentLocalCheckPos)))
				{
					tileIsSolid               = true;
					entityTilesReportingSolid = parentTiles;
				}
			}
		}
		
		if (foundTileInCollisionParent == false)
		{
			tileIsSolid = tileMgr.isSolid(pos, attribLayer, ownCollisionTiles, EntityHandle(),
			                              false, &entityTilesReportingSolid);
			if (parentTiles != 0 && entityTilesReportingSolid == parentTiles)
			{
				// The tile was not found in the collision parent, yet this is still the
				// entity causing this location to be reported as solid: treat the tile as
				// not solid to prevent our parent taking up more tiles than it is actually wide
				// NOTE: Perhaps more correct would be to perform a more extensive check:
				//       check if solid, ignoring both our own tiles and our parent's tiles
				tileIsSolid = false;
			}
		}
		
		if (tileIsSolid == false)
		{
			// This tile is not solid: ignore it
			// HACK: To keep the same behavior as before, check whether the caller is registered
			//       at this position. If so, we encountered it in the tiles below us.
			if (tileMgr.isEntityAtPosition(pos, p_caller))
			{
				foundCallerInTiles = true;
			}
			continue;
		}
		
		// Tile is solid: continue checking
		
		const tt::math::VectorRect ownTileRect = calcWorldRect();
		const tt::math::Vector2& ownTileRectMin = ownTileRect.getMin();
		const tt::math::Vector2  ownTileRectMax = ownTileRect.getMaxEdge();
		
		if (entityTilesReportingSolid == 0)
		{
			// The level layer was responsible for this solid tile: count it as static
			++staticCount;
		}
		else
		{
			EntityHandle  entityHandleReportingSolid = entityTilesReportingSolid->getOwner();
			const Entity* entityReportingSolid       = entityMgr.getEntity(entityHandleReportingSolid);
			
			if (entityReportingSolid == 0)
			{
				TT_PANIC("Internal error: Entity that owns EntityTiles instance no longer exists!");
				++staticCount;
				continue;
			}
			
			// The entity must be horizontally aligned to a tile ("horizontal" for us)
			// in order to be a candidate. However, do not enforce this requirement for
			// our current parent (so that we stay attached to our parent when it is moving)
			
			// MARTIJN: Disabled this alignment check for RIVE since entities are hardly ever aligned
			/*
			const tt::math::Vector2 otherEntitySnappedPos(entityReportingSolid->getSnappedToTilePosLevelOnly());
			const bool otherEntityOnTileX =
					tt::math::fabs(otherEntitySnappedPos.x - entityReportingSolid->getPosition().x) < edgeEpsilon;
			const bool otherEntityOnTileY =
					tt::math::fabs(otherEntitySnappedPos.y - entityReportingSolid->getPosition().y) < edgeEpsilon;
			
			if (entityReportingSolid != collisionParent &&             // only apply this rule to "not our parent"
			    (((m_orientationDown == movement::Direction_Down ||    //--> our horizontal axis is the world X axis
			       m_orientationDown == movement::Direction_Up) &&     // /
			      otherEntityOnTileX == false) ||                      // so, must be horizontally aligned
			     ((m_orientationDown == movement::Direction_Left ||    //--> our horizontal axis is the world Y axis
			       m_orientationDown == movement::Direction_Right) &&  // /
			      otherEntityOnTileY == false)))                       // so, must be vertically aligned
			{
				continue;
			}
			*/
			
			// Check if we're actually standing on (the edge of) solid entity.
			tt::math::VectorRect entityReportingSolidTileRect = entityReportingSolid->calcEntityTileRect();
			real distanceToSolid = 0.0f;
			switch (m_orientationDown)
			{
			case movement::Direction_Down:
				//was: if (entityReportingSolidTileRect.getMaxEdge().y + edgeEpsilon < ownTileRectMin.y)
				distanceToSolid = ownTileRectMin.y - entityReportingSolidTileRect.getMaxEdge().y;
				break;
				
			case movement::Direction_Up:
				//was: if (entityReportingSolidTileRect.getMin().y - edgeEpsilon > ownTileRectMax.y)
				distanceToSolid = entityReportingSolidTileRect.getMin().y - ownTileRectMax.y;
				break;
				
			case movement::Direction_Left:
				//was: if (entityReportingSolidTileRect.getMaxEdge().x + edgeEpsilon < ownTileRectMin.x)
				distanceToSolid = ownTileRectMin.x - entityReportingSolidTileRect.getMaxEdge().x;
				break;
				
			case movement::Direction_Right:
				//was: if (entityReportingSolidTileRect.getMin().x - edgeEpsilon > ownTileRectMax.x)
				distanceToSolid = entityReportingSolidTileRect.getMin().x - ownTileRectMax.x;
				break;
				
			default:
				TT_PANIC("Unsupported 'down' orientation: %d", m_orientationDown);
				return EntityHandle();
			}
			if (distanceToSolid > edgeEpsilon || distanceToSolid < -0.3f)
			{
				// entityReportingSolid is too close or far.
				// When inside the solid rect we allow more distance.
				continue;
			}
			
			if (entityHandleReportingSolid == p_caller)
			{
				foundCallerInTiles = true;
			}
			
			// Find the ancestor of this collision parent tree
			const Entity* directionCheckEntity = entityReportingSolid;
			{
				const Entity* ancestor = entityReportingSolid->getCollisionAncestor(true).getPtr();
				if (ancestor != 0)
				{
					directionCheckEntity = ancestor;
				}
			}
			
			LocalDir checkDir = LocalDir_None;
			const movementcontroller::DirectionalMovementController* ctrl =
					directionCheckEntity->getDirectionalMovementController();
			if (ctrl != 0)
			{
				movement::Direction worldDir = ctrl->getActualMovementDirection();
				if (movement::isValidDirection(worldDir))
				{
					checkDir = entity::getLocalDirFromDirection(worldDir, m_orientationDown);
				}
			}
			
			switch (checkDir)
			{
				// "Up" takes precedence: if we encounter an upward moving entity, use that as parent
			case LocalDir_Up:
				// FIXME: Should use the fastest moving entity here, but for now just use the last one
				upEntity = entityHandleReportingSolid;
				if (entityHandleReportingSolid == collisionParentHandle)
				{
					parentIsStillCandidate = true;
				}
				break;
				
			case LocalDir_Down:
				// FIXME: Should use the slowest moving entity here
				downEntity = entityHandleReportingSolid;
				if (entityHandleReportingSolid == collisionParentHandle)
				{
					parentIsStillCandidate = true;
				}
				break;
				
			case LocalDir_Back:  // Because we ignore flipping, "back" is always "left"
				if (leftMostEntity.isEmpty())
				{
					leftMostEntity = entityHandleReportingSolid;
				}
				++leftCount;
				if (entityHandleReportingSolid == collisionParentHandle)
				{
					parentIsStillCandidate = true;
				}
				break;
				
			case LocalDir_Forward:  // Because we ignore flipping, "forward" is always "right"
				rightMostEntity = entityHandleReportingSolid;
				++rightCount;
				if (entityHandleReportingSolid == collisionParentHandle)
				{
					parentIsStillCandidate = true;
				}
				break;
				
			default:
				// Entity isn't moving: it is a static tile
				++staticCount;
				break;
			}
		}
	}
	
	if (foundCallerInTiles == false && // Guard against recursion
	    p_caller.isEmpty() == false && // Empty caller can happen when this function is called by this entity itself and it had no parent yet
	    collisionParent    != 0     &&
	    p_caller != collisionParentHandle) // Except when called by my parent (in this case, if the parent is no longer there, detach)
	{
		CM_Printf("[%06u] E::dSOP: [0x%08X] [C 0x%08X] [P 0x%08X] Keeping existing parent.\n",
		          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), p_caller.getValue(),
		          collisionParentHandle.getValue());
		return collisionParentHandle;
	}
	
	// Start out with the entity that is moving down (if any).
	// This will be overridden if another parent takes precedence.
	EntityHandle newParent = downEntity;
	bool         pickedSomething = downEntity.isEmpty() == false;
#if TOKI_CARRYMOVEMENT_DEBUGGING
	const char* pickReason = downEntity.isEmpty() ? "tiles are static" : "it is moving down";
#endif
	
	if (upEntity.isEmpty() == false)
	{
#if TOKI_CARRYMOVEMENT_DEBUGGING
		pickReason = "it is moving up";
#endif
		newParent = upEntity;
		pickedSomething = true;
	}
	else if (isTileAligned && (leftCount >= staticCount || rightCount >= staticCount))
	{
		if (leftCount > rightCount)
		{
			// More entity tiles moving left than right: parent is leftmost entity
#if TOKI_CARRYMOVEMENT_DEBUGGING
			pickReason = "it is moving left";
#endif
			newParent = leftMostEntity;
			pickedSomething = true;
		}
		else if (rightCount > leftCount)
		{
			// More entity tiles moving right than left: parent is rightmost entity
#if TOKI_CARRYMOVEMENT_DEBUGGING
			pickReason = "it is moving right";
#endif
			newParent = rightMostEntity;
			pickedSomething = true;
		}
	}
	
	// If this entity is not aligned to a tile and the current parent
	// is still one of the valid options, keep the existing parent
	if (isTileAligned == false && pickedSomething == false && parentIsStillCandidate)
	{
#if TOKI_CARRYMOVEMENT_DEBUGGING
		pickReason = "not aligned and existing parent still valid";
#endif
		newParent = collisionParentHandle;
	}
	
	CM_Printf("[%06u] E::dSOP: [0x%08X] [C 0x%08X] [P 0x%08X] Picked 0x%08X, because %s. L: %d R: %d S: %d\n",
	          AppGlobal::getUpdateFrameCount(), getHandle().getValue(), p_caller.getValue(),
	          collisionParentHandle.getValue(), newParent.getValue(), pickReason,
	          leftCount, rightCount, staticCount);
	
	return newParent;
}


bool Entity::setCollisionParentEntity(const EntityHandle& p_parent)
{
	movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	TT_ASSERTMSG(controller != 0, "Can't set collision parent entity on an entity with no movement controller.");
	if (controller != 0)
	{
		return controller->setCollisionParentEntity(p_parent);
	}
	return false;
}


EntityHandle Entity::getCollisionParentEntityScheduled() const
{
	TT_ASSERT(isInitialized());
	const movementcontroller::DirectionalMovementController* controller = getDirectionalMovementController();
	return (controller == 0) ? EntityHandle() : controller->getCollisionParentEntityScheduled();
}


EntityHandle Entity::getCollisionAncestor(s32*                p_ancestryDepth,
                                          const EntityHandle& p_stopOnThisEntity,
                                          bool                p_useScheduledParents) const
{
	EntityHandle ancestor = p_useScheduledParents ? getCollisionParentEntityScheduled() : getCollisionParentEntity();
	
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	const Entity* directParent = entityMgr.getEntity(ancestor);
	
	s32 ancestryDepth = 0;
	
	if (directParent != 0 && ancestor != p_stopOnThisEntity)
	{
		std::vector<const Entity*> processedEntities;
		processedEntities.reserve(10);
		for (const Entity* parent = directParent; parent != 0;
		     parent = entityMgr.getEntity(p_useScheduledParents ? parent->getCollisionParentEntityScheduled() : parent->getCollisionParentEntity()))
		{
			if (std::find(processedEntities.begin(),
			              processedEntities.end(), parent) != processedEntities.end())
			{
				TT_PANIC("Detected a cyclic collision parent relationship! "
				         "This should not happen (indicates a bug in the carry movement code).");
				break;
			}
			processedEntities.push_back(parent);
			
			ancestor = parent->getHandle();
			++ancestryDepth;
			
			if (ancestor == p_stopOnThisEntity)
			{
				break;
			}
		}
	}
	
	if (p_ancestryDepth != 0)
	{
		*p_ancestryDepth = ancestryDepth;
	}
	
	return ancestor;
}


void Entity::setMovementController(const movementcontroller::MovementControllerHandle& p_movementControllerHandle, 
                                         movementcontroller::MovementControllerMgr&    p_controllerMgr)
{
	if (m_movementControllerHandle.isEmpty() == false)
	{
		p_controllerMgr.destroyController(m_movementControllerHandle);
	}
	m_movementControllerHandle = p_movementControllerHandle;
}


movementcontroller::DirectionalMovementController* Entity::getDirectionalMovementController()
{
	TT_ASSERT(isInitialized());
	return getMovementControllerHandle().getPtr();
}


movementcontroller::DirectionalMovementController* Entity::createDirectionalMovementController()
{
	TT_ASSERT(isInitialized());
	
	EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	movementcontroller::MovementControllerMgr& ctrlMgr = entityMgr.getMovementControllerMgr();
	
	// Create new controller and ensure the new controller is known to the entity
	movementcontroller::MovementControllerHandle ctrl = ctrlMgr.createDirectionalController(getHandle());
	setMovementController(ctrl, ctrlMgr);
	
	// Start an initial move for the new controller
	movementcontroller::DirectionalMovementController* ptr = ctrlMgr.getDirectionalController(ctrl);
	
	ptr->updateLocalCollision(m_registeredTileRect, *this);
	updateSurvey(false); // FIXME: Move local collision to entity so we don't need controller for stand_on_solid check.
	
	ptr->stopMovement(); 
	
	return ptr;
}


void Entity::updatePresentationObjectsPosition()
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	const tt::math::Vector2 pos(getCenterPosition());
	
	for(PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end();)
	{
		pres::PresentationObject* pres = (*it).getPtr();
		
		if (pres != 0 && pres->hasPresentationObject())
		{
			if (pres->isFollowingParent())
			{
				pres->updatePosition(pos);
			}
			++it;
		}
		else
		{
			it = tt::code::helpers::unorderedErase(m_presentationObjects, it);
		}
	}
}


void Entity::handleOrientationChange()
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	const tt::pres::PresentationObject::FlipMask flipMask = m_orientationForwardIsLeft ?
		tt::pres::PresentationObject::FlipMask_Horizontal :
		tt::pres::PresentationObject::FlipMask_None;
	
	const tt::math::Quaternion quat(getOrientationQuaternion(m_orientationDown));
	
	// Update presentation objects
	for (PresentationObjects::iterator it = m_presentationObjects.begin();
		it != m_presentationObjects.end(); ++it)
	{
		pres::PresentationObject* pres = (*it).getPtr();
		pres->updateOrientation(flipMask, quat);
	}
}


#if !defined(TT_BUILD_FINAL)

tt::str::Strings Entity::getSortedTagNames(const tt::pres::Tags& p_tags)
{
	tt::str::StringSet tagNames;
	for (tt::pres::Tags::const_iterator it = p_tags.begin(); it != p_tags.end(); ++it)
	{
		tagNames.insert((*it).getName());
	}
	
	return tt::str::Strings(tagNames.begin(), tagNames.end());
}

#endif


void Entity::setPositionCulled(bool p_isCulled)
{
	// Don't set if flag hasn't been changed and culling has been initialized
	if (m_isPositionCulled == p_isCulled && m_positionCullingInitialized)
	{
		return;
	}
	
	if (p_isCulled)
	{
		m_entityScript->queueSqFun("onCulled");
		
		// Cull timers
		script::TimerMgr::suspendAllTimers(m_handle);
		
		// TODO: Cull 'pause' path movement recast part
		
		// Cull presentations
		for (PresentationObjects::iterator it = m_presentationObjects.begin();
			it != m_presentationObjects.end(); ++it)
		{
			pres::PresentationObject* pres((*it).getPtr());
			if (pres != 0)
			{
				pres->setIsCulled(true);
			}
		}
	}
	else
	{
		m_entityScript->queueSqFun("onUnculled");
		
		// Uncull timers
		script::TimerMgr::resumeAllTimers(m_handle);
		
		// Uncull presentations
		for (PresentationObjects::iterator it = m_presentationObjects.begin();
			it != m_presentationObjects.end(); ++it)
		{
			pres::PresentationObject* pres((*it).getPtr());
			if (pres != 0)
			{
				pres->setIsCulled(false);
			}
		}
	}
	
	m_isPositionCulled = p_isCulled;
}

// Namespace end
}
}
}
