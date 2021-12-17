#include <algorithm>

#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/engine/renderer/MatrixStack.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/light/Glow.h>
#include <toki/game/light/JitterEffect.h>
#include <toki/game/light/Light.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace light {

//--------------------------------------------------------------------------------------------------
// Public member functions

Light::Light(const CreationParams& p_creationParams, const LightHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_source(  p_creationParams.source),
m_worldPos(tt::math::Vector2::zero),
m_offset(  p_creationParams.offset),
m_strength(p_creationParams.strength),
m_radius(  p_creationParams.radius),
m_radiusSquared(p_creationParams.radius * p_creationParams.radius),
m_spread(tt::math::twoPi),
m_enabled(true),
m_blockedAtSource(false),
m_affectsEntities(true),
m_lightShape(tt::math::Vector2::zero, p_creationParams.radius, tt::engine::renderer::ColorRGB::white),
m_colorAlpha(255),
m_textureRotationSpeed(0.0f),
m_glow(),
m_jitterEffect()
{
	update(0.0f);
}


void Light::update(real p_deltaTime)
{
	if (isEnabled() == false)
	{
		return;
	}
	
	updateWorldPosition();
	
	m_offset.update(p_deltaTime);
	m_strength.update(p_deltaTime);
	m_radius.update(p_deltaTime);
	m_spread.update(p_deltaTime);
	
	if (tt::math::realEqual(m_lightShape.getRadius(), m_radius.getValue()) == false)
	{
		setRadiusImpl(m_radius.getValue());
	}
	
	if (tt::math::realEqual(m_lightShape.getSpread(), m_spread.getValue()) == false)
	{
		m_lightShape.setSpread(m_spread.getValue());
	}
	
	if (m_jitterEffect != 0)
	{
		m_jitterEffect->update(p_deltaTime);
		
		m_lightShape.setRadius(   m_radius.getValue() + m_jitterEffect->getScaleJitter()   );
		m_lightShape.setCenterPos( getWorldPosition() + m_jitterEffect->getPositionJitter());
	}
	else
	{
		m_lightShape.setCenterPos(getWorldPosition());
	}
	
	if (m_glow != 0)
	{
		m_glow->update(*this);
	}
}


void Light::updateLightShape(const Polygons& p_occluders)
{
	if (isActive() == false)
	{
		return;
	}
	TT_ASSERT(m_strength.getValue() >= 0.0f && m_strength.getValue() <= 1.0f);
	
	//TT_MINMAX_ASSERT(m_strength.getValue(), 0.0f, 1.0f);
	
	m_lightShape.update(0.0f, p_occluders, m_colorAlpha);
}


void Light::render(real p_currentTime) const
{
	if (isActive() == false)
	{
		return;
	}
	
	using tt::engine::renderer::MatrixStack;
	MatrixStack*                    stack    = MatrixStack::getInstance();
	
	const bool doRotation = m_textureRotationSpeed != 0 || m_lightShape.getDirection() != 0;
	
	if (doRotation)
	{
		stack->setMode(MatrixStack::Mode_Texture);
		stack->rotateZ((p_currentTime * m_textureRotationSpeed) - m_lightShape.getDirection());
		stack->updateTextureMatrix();
		stack->setMode(MatrixStack::Mode_Position);
	}
	
	m_lightShape.render();
	
	if (doRotation)
	{
		stack->resetTextureMatrix();
	}
}


void Light::renderGlow() const
{
	if (m_glow == 0 || isActive() == false)
	{
		return;
	}
	m_glow->render();
}


void Light::debugRender()
{
	m_lightShape.renderDebug();
}


void Light::getAffectedEntities(entity::EntityHandleSet&        p_entities,
                                const level::AttributeLayerPtr& /*p_attribs*/)
{
	const entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	const level::TileRegistrationMgr& tileMgr = AppGlobal::getGame()->getTileRegistrationMgr();
	
	using namespace tt::math;
	const Vector2 sourcePos = getWorldPosition();
	
	tt::math::Point2 sourceTilePos = level::worldToTile(sourcePos);
	m_blockedAtSource = lightHitTest(sourceTilePos);
	
	if (m_blockedAtSource)
	{
		// Do extra checking because we want to ignore the dynamic collision tiles of a carry parent.
		// Instead the rect should be used.
		
		// new code which checks all entities tile rects, not just parent.
		// FIXME: Why get attrib layer from Game when it is passed to this function?
		level::CollisionTypes types = tileMgr.getCollisionTypesFromRegisteredTiles(sourceTilePos,
				AppGlobal::getGame()->getAttributeLayer(),
				0, game::entity::EntityHandle(), false, true);
		if (level::g_collisionTypesLightBlocking.checkAnyFlags(types) == false)
		{
			m_blockedAtSource = false;
		}
	}
	
	// The isActive check is after the blocked check because is uses m_blockedAtSource.
	if (isActive() == false || affectsEntities() == false)
	{
		return;
	}
	
	const real radius   = std::max(getRadius(), 0.0f);
	const real diameter = radius * 2.0f;
	VectorRect lightRect(Vector2(sourcePos.x - radius, sourcePos.y - radius), diameter, diameter);
	
	if(lightRect.getWidth() <= 0.0f || lightRect.getHeight() <= 0.0f) return;
	
	tt::math::PointRect tileRect = level::worldToTile(lightRect);
	
	const tt::math::Point2 minPos = tt::math::pointMax(tileRect.getMin(), tt::math::Point2::zero);
	tt::math::Point2 maxPos = tileRect.getMaxInside();
	
	// FIXME: Why get attrib layer from Game when it is passed to this function?
	const level::AttributeLayerPtr& level = AppGlobal::getGame()->getAttributeLayer();
	maxPos.x = std::min(maxPos.x, level->getWidth()  - 1);
	maxPos.y = std::min(maxPos.y, level->getHeight() - 1);
	
	tt::math::Point2 pos;
	
	const s32 cellSize(level::TileRegistrationMgr::cellSize);
	
	const s32 minx = (minPos.x / cellSize) * cellSize;
	const s32 miny = (minPos.y / cellSize) * cellSize;
	const s32 maxx = static_cast<s32>(tt::math::ceil(maxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 maxy = static_cast<s32>(tt::math::ceil(maxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	for (pos.y = miny; pos.y <= maxy; pos.y += cellSize)
	{
		for (pos.x = minx; pos.x <= maxx; pos.x += cellSize)
		{
			if(tileMgr.hasEntityAtPosition(pos.x, pos.y))
			{
				// Handle entities at this position
				const entity::EntityHandleSet& entities = tileMgr.getRegisteredEntityHandles(pos);
				
				for (entity::EntityHandleSet::const_iterator it = entities.begin();
					 it != entities.end(); ++it)
				{
					const entity::Entity* targetEntity = entityMgr.getEntity(*it);
					
					if (targetEntity != 0                        && // Check if target is alive
						targetEntity->isSuspended() == false     && // Target should not be suspended
						targetEntity->isDetectableByLight()      && // Target should be affected by light
						p_entities.find(*it) == p_entities.end() && // Not already part of the set of affected entities
						isInLight(*targetEntity))                   // In light.
					{
						p_entities.insert(*it);
					}
				}
			}
		}
	}
}


bool Light::isInLight(const tt::math::Vector2& p_position) const
{
	// Check if within radius, not out of bounds and finally do raytrace
	return isActive() && 
	       affectsEntities() &&
	       distanceSquared(       getWorldPosition(), p_position) <= m_radiusSquared &&
	       m_lightShape.inSpread( getWorldPosition(), p_position) &&
	       tt::code::tileRayTrace(getWorldPosition(), p_position, lightHitTest, 0);
}


bool Light::isInLight(const entity::Entity& p_targetEntity) const
{
	if (isActive() == false || affectsEntities() == false)
	{
		return false;
	}
	
	const entity::Entity::DetectionPoints& points(p_targetEntity.getLightDetectionPoints());
	const tt::math::Vector2 position(p_targetEntity.getCenterPosition());
	for (entity::Entity::DetectionPoints::const_iterator it = points.begin(); it != points.end(); ++it)
	{
		if (isInLight(position + (*it)))
		{
			return true;
		}
	}
	
	return false;
}


void Light::createGlow(const std::string& p_imageName, real p_scale,
                       real p_minRadius, real p_maxRadius, real p_fadeRadius)
{
	m_glow = light::Glow::create(
		light::Glow::CreationParams(p_imageName, p_scale, p_minRadius, p_maxRadius, p_fadeRadius));
	if (m_glow != 0)
	{
		m_glow->update(*this);
	}
}


void Light::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_ownHandle,       p_context);
	bu::putHandle(m_source,          p_context);
	bu::put      (m_worldPos,        p_context);
	bu::putTLI   (m_offset,          p_context);
	bu::putTLI   (m_radius,          p_context);
	bu::putTLI   (m_spread,          p_context);
	bu::putTLI   (m_strength,        p_context);
	bu::put      (m_enabled,         p_context);
	bu::put      (m_affectsEntities, p_context);
	m_lightShape.serialize(p_context);
	bu::put( m_colorAlpha, p_context);
	bu::put( m_textureRotationSpeed, p_context);
	
	bool hasGlow = m_glow != 0;
	bu::put      (hasGlow,           p_context);
	if (hasGlow)
	{
		m_glow->serialize(p_context);
	}
	
	bool hasJittterEffect = m_jitterEffect != 0;
	bu::put      (hasJittterEffect,  p_context);
	if (hasJittterEffect)
	{
		m_jitterEffect->serialize(p_context);
	}
}


Light Light::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	LightHandle          lightHandle = bu::getHandle<Light         >(p_context);
	entity::EntityHandle source      = bu::getHandle<entity::Entity>(p_context);
	const tt::math::Vector2 worldPos = bu::get<tt::math::Vector2   >(p_context);
	tt::math::TimedLinearInterpolation<tt::math::Vector2> offset   = bu::getTLI<tt::math::Vector2>(p_context);
	tt::math::TimedLinearInterpolation<real             > radius   = bu::getTLI<real             >(p_context);
	tt::math::TimedLinearInterpolation<real             > spread   = bu::getTLI<real             >(p_context);
	tt::math::TimedLinearInterpolation<real             > strength = bu::getTLI<real             >(p_context);
	bool enabled = bu::get<bool>(p_context);
	
	Light lightObject(CreationParams(source, offset.getValue(), radius.getValue(), strength.getValue()),
	                  lightHandle);
	
	lightObject.m_affectsEntities = bu::get<bool>(p_context);
	
	// Overwrite newly created TLIs with correct TLIs
	lightObject.m_offset   = offset;
	lightObject.m_radius   = radius;
	lightObject.m_spread   = spread;
	lightObject.m_strength = strength;
	
	lightObject.setEnabled(enabled);
	
	lightObject.m_worldPos = worldPos;
	
	lightObject.m_lightShape.unserialize(p_context);
	lightObject.m_colorAlpha = bu::get<u8>(p_context);
	
	lightObject.m_textureRotationSpeed = bu::get<real>(p_context);
	
	bool hasGlow = bu::get<bool>(p_context);
	if (hasGlow)
	{
		lightObject.m_glow = Glow::unserialize(p_context);
	}
	
	bool hasJittterEffect = bu::get<bool>(p_context);
	if (hasJittterEffect)
	{
		lightObject.m_jitterEffect = JitterEffect::unserialize(p_context);
	}
	
	return lightObject;
}


Light* Light::getPointerFromHandle(const LightHandle& p_handle)
{
	if (AppGlobal::hasGame() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getLightMgr().getLight(p_handle);
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

void Light::updateWorldPosition()
{
	const entity::Entity* source = m_source.getPtr();
	if (source == 0)
	{
		TT_PANIC("Light has invalid source");
		m_worldPos = tt::math::Vector2::zero;
		return;
	}
	
	m_worldPos = source->getCenterPosition() + source->applyOrientationToVector2(m_offset.getValue());
}



// Namespace end
}
}
}
