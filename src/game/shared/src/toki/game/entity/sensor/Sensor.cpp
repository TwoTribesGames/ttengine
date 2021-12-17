#include <algorithm>
#include <iterator>

#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/thread/CriticalSection.h>

#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/sensor/Shape.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/wrappers/SensorWrapper.h>
#include <toki/game/Game.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>

// FIXME: Remove debug rendering
#include <toki/game/DebugView.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

tt::thread::Mutex Sensor::ms_sensorMutex;

//--------------------------------------------------------------------------------------------------
// Public member functions

Sensor::Sensor(const CreationParams& p_creationParams, const SensorHandle& p_ownHandle)
:
m_handle(p_ownHandle),
m_type(p_creationParams.type),
m_source(p_creationParams.source),
m_shape(p_creationParams.shape),
m_target(p_creationParams.target),
m_inLocalSpace(true),
m_translation(tt::math::Vector2::zero),
m_enabled(true),
m_suspended(false),
m_delayInSeconds(0.0f),
m_rayTracer(AppGlobal::getGame()->getAttributeLayer()),
m_rayTraceOffset(tt::math::Vector2::zero),
m_sensedEntities(),
m_filteredEntities(),
m_delayedEntities(),
m_currentySensedEntitiesCache(),
m_differenceCache(),
m_enterCallback(),
m_exitCallback(),
m_filterCallback(),
m_enabledInDarkness(p_creationParams.type == SensorType_Touch),
m_ignoreOwnCollision(false),
m_ignoreActiveCollision(false),
m_distanceSort(false),
m_isDirty(false)
{
	validate(m_type, m_shape, m_target);
	
	if (reserveCount > 0)
	{
		m_sensedEntities.reserve(reserveCount);
		m_filteredEntities.reserve(reserveCount);
		m_delayedEntities.reserve(reserveCount);
		m_currentySensedEntitiesCache.reserve(reserveCount);
		m_differenceCache.reserve(reserveCount);
	}
	
	if (m_shape != 0)
	{
		Entity* sourceEntity = m_source.getPtr();
		TT_NULL_ASSERT(sourceEntity);
		if (sourceEntity != 0)
		{
			m_shape->updateTransform(*sourceEntity, getWorldPosition(), this);
		}
	}
}


Sensor::~Sensor()
{
	removeAllSensedEntities(true, true);
}


void Sensor::update()
{
	Entity* sourceEntity = getSourceEntityForUpdate();
	if (sourceEntity == nullptr)
	{
		return;
	}
	
	m_isDirty = false;
	m_currentySensedEntitiesCache.clear();
	m_rayTracer.resetHitLocation();
	
	if (m_shape != 0)
	{
		m_shape->updateTransform(*sourceEntity, getWorldPosition(), this);
	}
	
	if (isEnabled() && isActive(sourceEntity))
	{
		// Check if target is set
		if (m_target.isEmpty() == false)
		{
			const FilterResult& filterResult = getFilterResult(m_target);
			if (filterResult.getResult())
			{
				Entity* targetEntity = m_target.getPtr();
				
				if (targetEntity != 0 && targetEntity->isSuspended() == false &&
				    
				    // Light checks
				    (isEnabledInDarkness() ||
				     (targetEntity->isDetectableByLight() == false || targetEntity->isInLight())) &&
				    
				    isTargetInRange(sourceEntity, targetEntity))
				{
					m_currentySensedEntitiesCache.push_back(m_target);
				}
			}
		}
		else if (m_shape != 0)
		{
			// no specific target, return all entities
			getSensedEntities(sourceEntity, &m_currentySensedEntitiesCache);
		}
	}
}


void Sensor::updateCallbacks(real64 p_gameTime)
{
	Entity* sourceEntity = getSourceEntityForUpdate();
	if (sourceEntity == nullptr)
	{
		return;
	}
	
	if ((m_currentySensedEntitiesCache.empty() && m_sensedEntities.empty()) == false)
	{
		filterSensedEntities(sourceEntity, &m_currentySensedEntitiesCache);
		
		script::EntityBasePtr script(sourceEntity->getEntityScript());
		TT_NULL_ASSERT(script);
		
		// Entities entered sensor
		{
			// set_difference needs both containers to be sorted.
			std::sort(m_currentySensedEntitiesCache.begin(), m_currentySensedEntitiesCache.end());
			
			m_differenceCache.clear();
			std::set_difference(
				m_currentySensedEntitiesCache.begin(), m_currentySensedEntitiesCache.end(),
				m_sensedEntities.begin(), m_sensedEntities.end(), std::back_inserter(m_differenceCache));
			
			if (m_delayInSeconds > 0.0f)
			{
				doSensedEntityDelay(p_gameTime, &m_currentySensedEntitiesCache, &m_differenceCache);
			}
			
			if (m_distanceSort)
			{
				distanceSort(m_differenceCache);
			}
			for (EntityHandles::const_iterator it = m_differenceCache.begin();
				 it != m_differenceCache.end(); ++it)
			{
				handleOnEnter(*script, (*it));
			}
		}
		
		// Entities exited sensor
		{
			m_differenceCache.clear();
			std::set_difference(
				m_sensedEntities.begin(), m_sensedEntities.end(),
				m_currentySensedEntitiesCache.begin(), m_currentySensedEntitiesCache.end(), std::back_inserter(m_differenceCache));
		
			if (m_distanceSort)
			{
				distanceSort(m_differenceCache);
			}
			for (EntityHandles::const_iterator it = m_differenceCache.begin();
				 it != m_differenceCache.end(); ++it)
			{
				handleOnExit(*script, (*it));
			}
		}
		
		std::swap(m_sensedEntities, m_currentySensedEntitiesCache);
	}
	else
	{
		m_delayedEntities.clear();
	}
	
	removeUnusedFilters();
}


void Sensor::renderDebug() const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	{
		const Entity* sourceEntity = m_source.getPtr();
		if (sourceEntity == 0 || sourceEntity->isPositionCulled())
		{
			return;
		}
		
		Game* game = AppGlobal::getGame();
		toki::DebugRenderMask mask = AppGlobal::getDebugRenderMask();
		
		const bool visualizeSight = (m_type == SensorType_Sight) && mask.checkFlag(DebugRender_EntitySightSensorShapes);
		const bool visualizeTouch = (m_type == SensorType_Touch) && mask.checkFlag(DebugRender_EntityTouchSensorShapes);
		const bool visualizeText  = mask.checkFlag(DebugRender_EntitySensorShapesText);
		
		if (visualizeSight == false && visualizeTouch == false && visualizeText == false)
		{
			return;
		}
		
		const tt::math::Vector2 sensorPosition(getWorldPosition());
		
		DebugView& debugView = game->getDebugView();
		
		// Render debug texts
		// FIXME: This is called in the render() so don't use DebugView::registerSomething, but use
		// DebugView::renderSomething() instead!
		if (visualizeText)
		{
			using namespace tt::engine::renderer;
			Camera& cam = AppGlobal::getGame()->getCamera();
			tt::math::Vector2 scrPos(cam.worldToScreen(sensorPosition));
			
			s32 textCount = 0;
			if (m_enabledInDarkness)     ++textCount;
			if (m_ignoreOwnCollision)    ++textCount;
			if (m_ignoreActiveCollision) ++textCount;
			if (m_inLocalSpace == false) ++textCount;
			
			const s32 textHeight = 15;
			real yOffset = -(textCount * textHeight) / 2.0f - 5;
			
			if (m_inLocalSpace == false)
			{
				debugView.registerText(
					DebugView::TextInfo(scrPos + tt::math::Vector2(-25.0f, yOffset), "worldspace", true,
					                    tt::engine::renderer::ColorRGB::green, 0.0f));
			}
			yOffset += textHeight;
			
			if (m_enabledInDarkness)
			{
				debugView.registerText(
					DebugView::TextInfo(scrPos + tt::math::Vector2(-25.0f, yOffset), "enabled in darkness", true,
					                    tt::engine::renderer::ColorRGB::green, 0.0f));
			}
			yOffset += textHeight;
			
			if (m_ignoreOwnCollision)
			{
				debugView.registerText(
					DebugView::TextInfo(scrPos + tt::math::Vector2(-20, yOffset), "ign own col", true,
					                    tt::engine::renderer::ColorRGB::green, 0.0f));
			}
			yOffset += textHeight;
			
			if (m_ignoreActiveCollision)
			{
				debugView.registerText(
					DebugView::TextInfo(scrPos + tt::math::Vector2(-20.0f, yOffset), "ign act col", true,
					                    tt::engine::renderer::ColorRGB::green, 0.0f));
			}
			yOffset += textHeight;
		}
		
		// Visualize Line of Sight
		const tt::math::Vector2 rayTracePosition(getRayTracePosition());
		if (visualizeSight)
		{
			using namespace tt::engine::renderer;
			
			// Visualize starting point
			debugView.renderCircle(ColorRGBA(0, 255, 255, 255), rayTracePosition, 0.05f);
			
			for (EntityHandles::const_iterator it = m_sensedEntities.begin();
			     it != m_sensedEntities.end(); ++it)
			{
				const Entity* targetEntity = (*it).getPtr();
				if (targetEntity == 0 || targetEntity->isInitialized() == false)
				{
					continue;
				}
				
				// Raytrace again to determine which point is visible
				tt::math::Vector2 visiblePoint;
				if (isTargetVisible(targetEntity, &visiblePoint))
				{
					debugView.renderLine(ColorRGBA(255, 255, 255, 128), rayTracePosition, visiblePoint);
				}
			}
		}
		
		if (visualizeSight || visualizeTouch)
		{
			// Visualize target
			//if (m_target.isEmpty() == false)
			{
				const Entity* targetEntity = m_target.getPtr();
				if (targetEntity != 0)
				{
					using namespace tt::engine::renderer;
					const ColorRGBA targetArrowColor = getDebugColor();
					
					const tt::math::Vector2 targetPosition(targetEntity->getCenterPosition());
					debugView.renderLine(targetArrowColor, rayTracePosition, targetPosition);
					
					// Draw arrow
					// FIXME: Make DrawArrow() function in DebugView or libs?
					real angle = tt::math::atan2(targetPosition.y - rayTracePosition.y,
						targetPosition.x - rayTracePosition.x) + tt::math::pi;
					
					real length = 0.5f;
					real arrowDegrees = 0.5f;
					
					real x1 = targetPosition.x + length * tt::math::cos(angle - arrowDegrees);
					real y1 = targetPosition.y + length * tt::math::sin(angle - arrowDegrees);
					real x2 = targetPosition.x + length * tt::math::cos(angle + arrowDegrees);
					real y2 = targetPosition.y + length * tt::math::sin(angle + arrowDegrees);
					
					debugView.renderLine(targetArrowColor, targetPosition, tt::math::Vector2(x1, y1));
					debugView.renderLine(targetArrowColor, targetPosition, tt::math::Vector2(x2, y2));
				}
			}
			
			// Visualize sensor range
			if (m_shape != 0)
			{
				m_shape->visualize(getDebugColor());
			}
		}
	}
#endif
}


void Sensor::setWorldPosition(const tt::math::Vector2& p_position)
{
	m_inLocalSpace = false;
	m_translation  = p_position;
}


void Sensor::setOffset(const tt::math::Vector2& p_offset)
{
	m_inLocalSpace = true;
	m_translation  = p_offset;
}


tt::math::Vector2 Sensor::getWorldPosition() const
{
	if (m_inLocalSpace == false)
	{
		return m_translation;
	}
	
	const entity::Entity* source = m_source.getPtr();
	if (source != 0)
	{
		return source->getCenterPosition() + source->applyOrientationToVector2(m_translation);
	}
	
	TT_PANIC("Sensor has invalid source");
	
	return tt::math::Vector2::zero;
}


tt::math::Vector2 Sensor::getRayTracePosition() const
{
	if (m_inLocalSpace == false)
	{
		return m_translation + m_rayTraceOffset;
	}
	
	const entity::Entity* source = m_source.getPtr();
	if (source != 0)
	{
		return source->getCenterPosition() + source->applyOrientationToVector2(m_translation + m_rayTraceOffset);
	}
	
	TT_PANIC("Sensor has invalid source");
	
	return tt::math::Vector2::zero;
}


void Sensor::setRayTraceOffset(const tt::math::Vector2& p_offset)
{
	TT_ASSERTMSG(m_type == SensorType_Sight, "setRayTraceOffset should only be used with sight sensors");
	
	m_rayTraceOffset = p_offset;
}


void Sensor::setIgnoreActiveCollision(bool p_ignoreActiveCollision)
{
	m_ignoreActiveCollision = p_ignoreActiveCollision;
	m_rayTracer.setIgnoreActiveCollision(p_ignoreActiveCollision);
}


void Sensor::setIgnoreOwnCollision(bool p_ignoreOwnCollision)
{
	m_ignoreOwnCollision = p_ignoreOwnCollision;
	
	if (m_ignoreOwnCollision)
	{
		m_rayTracer.setIgnoreEntityCollision(m_source);
	}
	else
	{
		m_rayTracer.setIgnoreEntityCollision(EntityHandle());
	}
}


tt::engine::renderer::ColorRGBA Sensor::getDebugColor() const
{
	const entity::Entity* source = m_source.getPtr();
	TT_NULL_ASSERT(source);
	
	using tt::engine::renderer::ColorRGBA;
	
	const bool entityInRange = (m_sensedEntities.empty() == false);
	
	switch (m_type)
	{
	case SensorType_Sight:
		if (isEnabled() && isActive(source))
		{
			return entityInRange ? ColorRGBA(200, 200, 255, 255): ColorRGBA(100, 100, 155, 255);
		}
		return ColorRGBA(150, 150, 150, 255);
		
	case SensorType_Touch:
		if (isEnabled() && isActive(source))
		{
			return entityInRange ? ColorRGBA(255, 200,  50, 255): ColorRGBA(155, 100,  50, 255);
		}
		return ColorRGBA(100, 100, 100, 255);
		
	default:
		TT_PANIC("Unhandled sensor type %d", m_type);
		break;
	}
	
	return ColorRGBA(255, 0, 255, 255);
}


void Sensor::setShape(const ShapePtr& p_shape)
{
	if (validate(m_type, p_shape, m_target))
	{
		m_shape = p_shape;
	}
}


void Sensor::setTarget(const EntityHandle& p_target)
{
	if (validate(m_type, m_shape, p_target))
	{
		m_target = p_target;
	}
}


void Sensor::setEnabled(bool p_enabled)
{
	if (p_enabled)
	{
		m_isDirty = true;
	}
	else
	{
		removeAllSensedEntities(true, false);
	}
	m_enabled = p_enabled;
}


bool Sensor::validate(SensorType p_type, const ShapePtr& p_shape, const EntityHandle& /*p_target*/)
{
	// Sanity checking
	switch (p_type)
	{
	case SensorType_Sight:
		return true;
		
	case SensorType_Touch:
		if (p_shape == 0)
		{
			TT_PANIC("Touchsensor should have a shape");
			return false;
		}
		break;
		
	default:
		TT_PANIC("Unhandled sensor type %d", p_type);
		return false;
	}
	
	return true;
}


void Sensor::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8>(m_type, p_context);
	bu::putHandle(m_source, p_context);
	bu::putHandle(m_target, p_context);
	
	const bool haveShape = (m_shape != 0);
	bu::put(haveShape, p_context);
	if (haveShape)
	{
		m_shape->serialize(p_context);
	}
}


Sensor::CreationParams Sensor::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const SensorType   sensorType = bu::getEnum<u8, SensorType>(p_context);
	const EntityHandle source     = bu::getHandle<Entity>(p_context);
	const EntityHandle target     = bu::getHandle<Entity>(p_context);
	
	const bool haveShape = bu::get<bool>(p_context);
	ShapePtr shape;
	if (haveShape)
	{
		shape = Shape::unserialize(p_context);
	}
	
	return CreationParams(sensorType, source, shape, target);
}


void Sensor::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_inLocalSpace,   p_context);
	bu::put(m_translation,    p_context);
	bu::put(m_enabled,        p_context);
	bu::put(m_suspended,      p_context);
	bu::put(m_delayInSeconds, p_context);
	m_rayTracer.serialize(    p_context);
	bu::put(m_rayTraceOffset, p_context);
	
	bu::put(static_cast<u32>(m_sensedEntities.size()), p_context);
	for (EntityHandles::const_iterator it = m_sensedEntities.begin(); it != m_sensedEntities.end(); ++it)
	{
		bu::putHandle(*it, p_context);
	}
	
	bu::put(static_cast<u32>(m_filteredEntities.size()), p_context);
	for (FilterResults::const_iterator it = m_filteredEntities.begin(); it != m_filteredEntities.end(); ++it)
	{
		it->serialize(p_context);
	}
	
	bu::put(static_cast<u32>(m_delayedEntities.size()), p_context);
	for (DelayedEntities::const_iterator it = m_delayedEntities.begin(); it != m_delayedEntities.end(); ++it)
	{
		it->serialize(p_context);
	}
	
	bu::put(m_enterCallback,         p_context);
	bu::put(m_exitCallback,          p_context);
	bu::put(m_filterCallback,        p_context);
	bu::put(m_enabledInDarkness,     p_context);
	bu::put(m_ignoreOwnCollision,    p_context);
	bu::put(m_ignoreActiveCollision, p_context);
	bu::put(m_distanceSort,          p_context);
	bu::put(m_isDirty,               p_context);
}


void Sensor::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	m_inLocalSpace          = bu::get<bool             >(p_context);
	m_translation           = bu::get<tt::math::Vector2>(p_context);
	m_enabled               = bu::get<bool             >(p_context);
	m_suspended             = bu::get<bool             >(p_context);
	m_delayInSeconds        = bu::get<real             >(p_context);
	m_rayTracer.unserialize(                             p_context);
	m_rayTraceOffset = bu::get<tt::math::Vector2>(       p_context);
	
	m_sensedEntities.clear();
	const u32 sensedEntityCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < sensedEntityCount; ++i)
	{
		m_sensedEntities.push_back(bu::getHandle<Entity>(p_context));
	}
	
	m_filteredEntities.clear();
	const u32 filteredEntityCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < filteredEntityCount; ++i)
	{
		m_filteredEntities.push_back(FilterResult::createByUnserialize(p_context));
	}
	
	m_delayedEntities.clear();
	const u32 delayedEntityCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < delayedEntityCount; ++i)
	{
		m_delayedEntities.push_back(DelayedEntity::createByUnserialize(p_context));
	}
	
	m_enterCallback         = bu::get<std::string>(p_context);
	m_exitCallback          = bu::get<std::string>(p_context);
	m_filterCallback        = bu::get<std::string>(p_context);
	m_enabledInDarkness     = bu::get<bool       >(p_context);
	m_ignoreOwnCollision    = bu::get<bool       >(p_context);
	m_ignoreActiveCollision = bu::get<bool       >(p_context);
	m_distanceSort          = bu::get<bool       >(p_context);
	m_isDirty               = bu::get<bool       >(p_context);
}


void Sensor::removeEntityFilterWithoutUnregister(const entity::EntityHandle& p_entityHandle)
{
	for (FilterResults::iterator it = m_filteredEntities.begin(); it != m_filteredEntities.end();)
	{
		const FilterResult& filterResult = (*it);
		if (filterResult.handle == p_entityHandle)
		{
			it = tt::code::helpers::unorderedErase(m_filteredEntities, it);
			return;
		}
		else
		{
			++it;
		}
	}
	
	TT_PANIC("EntityHandle %d not found in this sensors m_filteredEntities.",
	         p_entityHandle.getValue());
}


Sensor* Sensor::getPointerFromHandle(const SensorHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getSensorMgr().getSensor(p_handle);
}


/* NOTE: invalidate is called on the temp copy used by swap.
         It's used to suppress any double work that would be done in this copy when the destructor is called.*/
void Sensor::invalidateTempCopy()
{
	m_sensedEntities.clear();
	m_filteredEntities.clear();
	m_delayedEntities.clear();
}


void Sensor::removeAllSensedEntities(bool p_handleOnExitCallbacks, bool p_queueCallbacks)
{
	EntityHandles sensed;
	std::swap(m_sensedEntities, sensed);
	
	if (p_handleOnExitCallbacks && sensed.empty() == false && isEnabled())
	{
		Entity* sourceEntity = m_source.getPtr();
		if (sourceEntity != 0 && sourceEntity->isInitialized() && isActive(sourceEntity))
		{
			// Handle onexit callbacks
			script::EntityBasePtr script(sourceEntity->getEntityScript());
			TT_NULL_ASSERT(script);
			for (EntityHandles::const_iterator it = sensed.begin(); it != sensed.end(); ++it)
			{
				handleOnExit(*script, (*it), p_queueCallbacks);
			}
		}
	}
	
	if (AppGlobal::hasGameAndEntityMgr())
	{
		removeAllFilters();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void Sensor::FilterResult::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	bu::putHandle( handle, p_context);
	bu::putBitMask(flags,  p_context);
}


void Sensor::FilterResult::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	handle = bu::getHandle<Entity>(p_context);
	flags  = bu::getBitMask<FilterResult::Flag, FilterResult::Flag_Count>(p_context);
}


Sensor::FilterResult Sensor::FilterResult::createByUnserialize(tt::code::BufferReadContext* p_context)
{
	FilterResult result(EntityHandle(), false);
	result.unserialize(p_context);
	return result;
}


void Sensor::DelayedEntity::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	bu::putHandle(handle, p_context);
	bu::put(      time  , p_context);
}


void Sensor::DelayedEntity::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	handle = bu::getHandle<Entity>(p_context);
	time   = bu::get<      real64>(p_context);
}


Sensor::DelayedEntity Sensor::DelayedEntity::createByUnserialize(tt::code::BufferReadContext* p_context)
{
	DelayedEntity result(EntityHandle(), 0.0);
	result.unserialize(p_context);
	return result;
}


void Sensor::distanceSort(EntityHandles& p_handles) const
{
	if (p_handles.size() <= 1)
	{
		return;
	}
	
	typedef std::map<real, EntityHandle> EntityDistances;
	EntityDistances entityDistances;
	
	const entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	const tt::math::Vector2 position(getWorldPosition());
	for (EntityHandles::const_iterator it = p_handles.begin(); it != p_handles.end(); ++it)
	{
		const Entity* entity = entityMgr.getEntity(*it);
		if (entity != 0)
		{
			const real distance = tt::math::distanceSquared(entity->getCenterPosition(), position);
			entityDistances.insert(std::make_pair(distance, (*it)));
		}
	}
	
	p_handles.clear();
	for (EntityDistances::const_iterator it = entityDistances.begin(); it != entityDistances.end(); ++it)
	{
		p_handles.push_back((*it).second);
	}
}


void Sensor::getSensedEntities(Entity* p_source, EntityHandles* p_result_OUT)
{
	TT_NULL_ASSERT(p_source);
	TT_NULL_ASSERT(p_result_OUT);
	
	if (m_shape == 0)
	{
		TT_PANIC("getSensedEntities called without shape");
		return;
	}
	
	// Get sensed entities
	switch (m_type)
	{
	case SensorType_Sight:
		m_shape->getEntitiesWithCenterInRange(*this, *p_result_OUT);
		break;
		
	case SensorType_Touch:
		m_shape->getEntitiesWithWorldRectInRange(*this, *p_result_OUT);
		break;
		
	default:
		TT_PANIC("Unhandled sensor type %d", m_type);
		break;
	}
}


void Sensor::filterSensedEntities(Entity* p_source, EntityHandles* p_result_OUT)
{
	// Now remove the targets that do not meet the criteria of this sensor
	const entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	for (EntityHandles::iterator it = p_result_OUT->begin(); it != p_result_OUT->end();)
	{
		const entity::EntityHandle entityHandle = (*it);
		const entity::Entity* targetEntity = entityMgr.getEntity(entityHandle);
		
		// Entity should be valid and should omit itself
		if (targetEntity == 0 || targetEntity->isInitialized() == false ||
		    (m_inLocalSpace && targetEntity == p_source) ) // Only omit itself for local space sensors.
		{
			it = tt::code::helpers::unorderedErase(*p_result_OUT, it);
			continue;
		}
		
		// Filter check.
		{
			const FilterResult& filter = getFilterResult(entityHandle);
			if (filter.getResult() == false)
			{
				it = tt::code::helpers::unorderedErase(*p_result_OUT, it);
				continue;
			}
		}
		
		// Finally do raycheck (in case of sight)
		switch (m_type)
		{
		case SensorType_Sight:
			{
				if (isTargetVisible(targetEntity) == false)
				{
					it = tt::code::helpers::unorderedErase(*p_result_OUT, it);
					continue;
				}
			}
			break;
			
		case SensorType_Touch:
			break;
		
		default:
			TT_PANIC("Unhandled sensor type %d", m_type);
			break;
		}
		
		// else
		++it;
	}
}


bool Sensor::isTargetInRange(Entity* p_source, Entity* p_target) const
{
	TT_NULL_ASSERT(p_source);
	TT_NULL_ASSERT(p_target);
	
	// FIXME: Not ideal to explicitly check for a state to determine whether entity is alive or not
	if (p_target->isInitialized() == false)
	{
		return false;
	}
	
	switch (m_type)
	{
	case SensorType_Sight:
		if (m_shape != 0 && m_shape->intersects((*p_target).getCenterPosition()) == false)
		{
			// Not in shape area
			return false;
		}
		else
		{
			// Target within shape area (or there is no shape)
			// make final selection based on raychecks with visibility points
			return isTargetVisible(p_target);
		}
		//break;  (unreachable: will always return above)
		
	case SensorType_Touch:
		if (m_shape != 0 && m_shape->isShapeInRange(p_target->getTouchShape()) == false)
		{
			// Not in shape area
			return false;
		}
		return true;
		
	default:
		TT_PANIC("Unhandled sensor type %d", m_type);
		break;
	}
	
	return false;
}


bool Sensor::isActive(const Entity* p_source) const
{
	if (p_source->isSuspended() || m_suspended)
	{
		return false;
	}
	
	if (m_target.isEmpty() && m_shape == 0)
	{
		return false;
	}
	
	return true;
}


bool Sensor::isTargetVisible(const Entity* p_target, tt::math::Vector2* p_visiblePoint) const
{
	const Entity::DetectionPoints& points(p_target->getSightDetectionPoints());
	
	// Sanity check; should not ever occur
	TT_ASSERTMSG(points.empty() == false,
		"the visibility points of an entity should not be empty at this point");
	
	// Check if detectable by any of the sight detection points
	for (Entity::DetectionPoints::const_iterator pointIt = points.begin();
			pointIt != points.end(); ++pointIt)
	{
		const tt::math::Vector2 targetPosition(p_target->getCenterPosition() +
			p_target->applyOrientationToVector2(*pointIt));
		
		// One of the visibility points is visible, exit early
		if (m_rayTracer.trace(getRayTracePosition(), targetPosition))
		{
			if (p_visiblePoint != 0)
			{
				*p_visiblePoint = targetPosition;
			}
			return true;
		}
	}
	
	return false;
}


void Sensor::doSensedEntityDelay(real64         p_gameTime,
                                 EntityHandles* p_currentySensedEntities,
                                 EntityHandles* p_difference)
{
	TT_NULL_ASSERT(p_currentySensedEntities);
	TT_NULL_ASSERT(p_difference);
	
	// Remove delayed entities which are no longer seen.
	for (DelayedEntities::iterator delayedIt = m_delayedEntities.begin();
	     delayedIt != m_delayedEntities.end();)
	{
		EntityHandles::iterator sensedIt = std::find(p_currentySensedEntities->begin(),
		                                             p_currentySensedEntities->end(),
		                                             (*delayedIt).handle);
		if (sensedIt == p_currentySensedEntities->end())
		{
			delayedIt = tt::code::helpers::unorderedErase(m_delayedEntities, delayedIt);
		}
		else
		{
			++delayedIt;
		}
	}
	
	for (EntityHandles::iterator diffIt = p_difference->begin(); diffIt != p_difference->end(); )
	{
		// Find the seen entity in delayed
		EntityHandle seenEntity = (*diffIt);
		
		enum Result
		{
			Result_NotFound,
			Result_Delayed,
			Result_Ready
		};
		
		Result result = Result_NotFound;
		for (DelayedEntities::const_iterator delayIt = m_delayedEntities.begin();
		     delayIt != m_delayedEntities.end(); ++delayIt)
		{
			const DelayedEntity& delayed = (*delayIt);
			
			// Found in delayed entries.
			if (seenEntity == delayed.handle)
			{
				result = (p_gameTime < delayed.time) ? Result_Delayed : Result_Ready;
				break;
			}
		}
		
		if (result != Result_Ready)
		{
			if (result == Result_NotFound)
			{
				m_delayedEntities.push_back(DelayedEntity(seenEntity, p_gameTime + m_delayInSeconds));
			}
			EntityHandles::iterator sensedIt = std::find(p_currentySensedEntities->begin(),
			                                             p_currentySensedEntities->end(),
			                                             seenEntity);
			if (sensedIt != p_currentySensedEntities->end())
			{
				tt::code::helpers::unorderedErase(*p_currentySensedEntities, sensedIt);
			}
			else
			{
				TT_PANIC("Expected to found seenEntity in p_currentySensedEntities");
			}
			diffIt = tt::code::helpers::unorderedErase(*p_difference, diffIt);
		}
		else
		{
			++diffIt;
		}
	}
}


void Sensor::handleOnEnter(const script::EntityBase& p_source, const EntityHandle& p_target)
{
	if (m_enterCallback.empty())
	{
		return;
	}
	
	const script::EntityBase* targetBase = script::EntityBase::getEntityBase(p_target);
	if (targetBase != 0)
	{
		p_source.queueSqFun(m_enterCallback, targetBase, script::wrappers::SensorWrapper(m_handle));
	}
}


void Sensor::handleOnExit(const script::EntityBase& p_source, const EntityHandle& p_target,
                          bool p_queueCallback)
{
	if (m_exitCallback.empty())
	{
		return;
	}
	
	const Entity* entity = p_target.getPtr();
	// script::EntityBase::getEntityBase() isn't used. So no isInitialized is done. (So killed entities still get an exit.)
	const script::EntityBase* targetBase = (entity != 0) ? entity->getEntityScript().get() : 0;
	
	if (targetBase != 0)
	{
		if (p_queueCallback)
		{
			p_source.queueSqFun(m_exitCallback, targetBase, script::wrappers::SensorWrapper(m_handle));
		}
		else
		{
			p_source.callSqFun( m_exitCallback, targetBase, script::wrappers::SensorWrapper(m_handle));
		}
	}
}


bool Sensor::doFilterCallback(const script::EntityBase& p_script, const EntityHandle& p_targetEntity) const
{
	if (m_filterCallback.empty()) // When no filter function is set, accept everything.
	{
		return true;
	}
	
	const script::EntityBase* targetBase = script::EntityBase::getEntityBase(p_targetEntity);
	if (targetBase == 0)
	{
		// Target doesn't exit (anymore), so reject.
		return false;
	}
	bool returnValue = false;
	tt::thread::CriticalSection critSec(&ms_sensorMutex);
	p_script.callSqFunWithReturn(&returnValue, m_filterCallback, targetBase);
	return returnValue;
}


const Sensor::FilterResult& Sensor::getFilterResult(const EntityHandle& p_entityHandle)
{
	for (FilterResults::iterator it = m_filteredEntities.begin(); it != m_filteredEntities.end(); ++it)
	{
		FilterResult& result = (*it);
		if (result.handle == p_entityHandle)
		{
			result.makeUsed();
			return result;
		}
	}
	
	// Not found.
	Entity* sourceEntity = m_source.getPtr();
	if (sourceEntity != 0 && sourceEntity->isInitialized() && isActive(sourceEntity))
	{
		script::EntityBasePtr script(sourceEntity->getEntityScript());
		TT_NULL_ASSERT(script);
		bool filterResult = doFilterCallback(*script, p_entityHandle);
		if (registerFilterWithEntity(p_entityHandle))
		{
			m_filteredEntities.push_back(FilterResult(p_entityHandle, filterResult));
			return m_filteredEntities.back();
		}
	}
	static const FilterResult empty(EntityHandle(), false);
	return empty;
}


void Sensor::removeUnusedFilters()
{
	for (FilterResults::iterator it = m_filteredEntities.begin(); it != m_filteredEntities.end();)
	{
		FilterResult& filterResult = (*it);
		if (filterResult.wasUsed() == false)
		{
			unregisterFilterWithEntity(filterResult.handle);
			it = tt::code::helpers::unorderedErase(m_filteredEntities, it);
		}
		else
		{
			filterResult.resetUsed();
			++it;
		}
	}
}


void Sensor::removeAllFilters()
{
	FilterResults copy;
	std::swap(copy, m_filteredEntities);
	
	for (FilterResults::iterator it = copy.begin(); it != copy.end(); ++it)
	{
		FilterResult& filterResult = (*it);
		unregisterFilterWithEntity(filterResult.handle);
	}
}


bool Sensor::registerFilterWithEntity(const EntityHandle& p_entityHandle) const
{
	if (p_entityHandle.isEmpty() == false)
	{
		entity::Entity* entity = p_entityHandle.getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			tt::thread::CriticalSection critSec(&ms_sensorMutex);
			entity->registerSensorFilter(m_handle);
			return true;
		}
	}
	return false;
}


void Sensor::unregisterFilterWithEntity(const EntityHandle& p_entityHandle) const
{
	if (p_entityHandle.isEmpty() == false)
	{
		entity::Entity* entity = p_entityHandle.getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			tt::thread::CriticalSection critSec(&ms_sensorMutex);
			entity->unregisterSensorFilter(m_handle);
		}
		else
		{
			TT_PANIC("Can't unregister filter with invalid entity");
		}
	}
}


Entity* Sensor::getSourceEntityForUpdate() const
{
	Entity* sourceEntity = m_source.getPtr();
	if (sourceEntity == 0 || sourceEntity->isSuspended() || m_suspended || sourceEntity->isPositionCulled())
	{
		return nullptr;
	}
	
	return sourceEntity;
}

// Namespace end
}
}
}
}
