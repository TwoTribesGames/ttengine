#include <algorithm>
#include <iterator>

#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/code/TileRayTracer.h>

#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/sensor/Shape.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/level/LevelData.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>

// FIXME: Remove debug rendering
#include <toki/game/DebugView.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {


//--------------------------------------------------------------------------------------------------
// Helper functions

inline bool isDetectedBySensor(const Sensor& p_sensor, const entity::Entity* p_target)
{
	return  (p_sensor.getType() == SensorType_Sight && p_target->isDetectableBySight()) ||
			(p_sensor.getType() == SensorType_Touch && p_target->isDetectableByTouch());
}


inline bool hasEffect(const Sensor& p_sensor, const entity::Entity* p_target)
{
	return p_sensor.isEnabledInDarkness() || p_target->isDetectableByLight() == false || p_target->isInLight();
}


//--------------------------------------------------------------------------------------------------
// Shape

void Shape::updateTransform(const Entity& p_parent, const tt::math::Vector2& p_position, const Sensor* /*p_sensor*/)
{
	m_position = p_position;
	m_boundingRect = getRect(p_parent);
}


void Shape::getEntitiesWithCenterInRange(const Sensor& p_sensor, EntityHandles& p_entities) const
{
	return getEntitiesInRange(p_sensor, true, p_entities);
}


void Shape::getEntitiesWithWorldRectInRange(const Sensor& p_sensor, EntityHandles& p_entities) const
{
	return getEntitiesInRange(p_sensor, false, p_entities);
}


bool Shape::isShapeInRange(const ShapePtr& p_shape) const
{
	if (p_shape == 0) return false;
	
	switch(p_shape->getType())
	{
	case ShapeType_Circle: return intersects(*reinterpret_cast<CircleShape *>(p_shape.get()));
	case ShapeType_Box   : return intersects(*reinterpret_cast<BoxShape *>   (p_shape.get()));
	case ShapeType_Cone  : return intersects(*reinterpret_cast<ConeShape *>  (p_shape.get()));
	case ShapeType_Ray   : return intersects(*reinterpret_cast<RayShape *>   (p_shape.get()));
	default:
		TT_PANIC("Type '%d' not implemented", p_shape->getType());
		break;
	}
	
	return false;
}


void Shape::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8>(getType(), p_context);
	bu::put(m_position, p_context);
	bu::put(m_boundingRect, p_context);
	
	serializeImpl(p_context);
}


ShapePtr Shape::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	const ShapeType shapeType = bu::getEnum<u8, ShapeType>(p_context);
	
	ShapePtr shape;
	switch (shapeType)
	{
	case ShapeType_Circle: shape = CircleShape::createForUnserialize(); break;
	case ShapeType_Box:    shape = BoxShape::createForUnserialize();    break;
	case ShapeType_Cone:   shape = ConeShape::createForUnserialize();   break;
	case ShapeType_Ray:    shape = RayShape::createForUnserialize();   break;
	
	default:
		TT_PANIC("Serialization data has unsupported sensor shape type: %d", shapeType);
		return ShapePtr();
	}
	
	shape->m_position     = bu::get<tt::math::Vector2>(p_context);
	shape->m_boundingRect = bu::get<tt::math::VectorRect>(p_context);
	
	shape->unserializeImpl(p_context);
	
	return shape;
}


//--------------------------------------------------------------------------------------------------
// Private Shape functions

bool Shape::isInRange(const Entity& p_target, bool p_center) const
{
	if (p_center)
	{
		return intersects(p_target.getCenterPosition());
	}
	else
	{
		return isShapeInRange(p_target.getTouchShape());
	}
}

void Shape::getEntitiesInRangeOnTilePosition(const Sensor&           p_sensor,
                                             bool                    p_center,
                                             const tt::math::Point2& p_tilePosition,
                                             const level::TileRegistrationMgr& p_tileMgr,
                                             EntityHandles&          p_entities) const
{
	const entity::EntityHandleSet& unfilteredHandles = p_tileMgr.getRegisteredEntityHandles(p_tilePosition);
	
	p_entities.reserve(p_entities.size() + unfilteredHandles.size());
	
	const entity::Entity* source = p_sensor.getSource().getPtr();
	TT_NULL_ASSERT(source);
	
	for (entity::EntityHandleSet::const_iterator it = unfilteredHandles.begin();
		it != unfilteredHandles.end(); ++it)
	{
		const entity::EntityHandle& targetHandle(*it);
		
		if((p_sensor.isInLocalSpace() == false || targetHandle != p_sensor.getSource()) &&    // Only self check for local space sensors
		   std::find(p_entities.begin(), p_entities.end(), targetHandle) == p_entities.end()) // No duplicates.
		{
			const entity::Entity* target = targetHandle.getPtr();
			if(target != 0 && target->isSuspended() == false && target->isPositionCulled() == false)
			{
				if( isDetectedBySensor(p_sensor, target) && 
					hasEffect         (p_sensor, target) &&
					isInRange         (*target, p_center))
				{
					p_entities.push_back(targetHandle);
				}
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------
// BoundingRectShape

void BoundingRectShape::getEntitiesInRange(const Sensor& p_sensor, bool p_center, EntityHandles& p_entities) const
{
	const level::TileRegistrationMgr& tileMgr = AppGlobal::getGame()->getTileRegistrationMgr();
	
	tt::math::PointRect tileRect(getBoundingTileRect());
	const tt::math::Point2 minPos = tt::math::pointMax(tileRect.getMin(), tt::math::Point2::zero);
	tt::math::Point2 maxPos = tileRect.getMaxInside();
	
	const level::AttributeLayerPtr& level = AppGlobal::getGame()->getAttributeLayer();
	maxPos.x = std::min(maxPos.x, level->getWidth()  - 1);
	maxPos.y = std::min(maxPos.y, level->getHeight() - 1);
	
	const s32 cellSize(level::TileRegistrationMgr::cellSize);
	const s32 minx = (minPos.x / cellSize) * cellSize;
	const s32 miny = (minPos.y / cellSize) * cellSize;
	const s32 maxx = static_cast<s32>(tt::math::ceil(maxPos.x / static_cast<real>(cellSize))) * cellSize;
	const s32 maxy = static_cast<s32>(tt::math::ceil(maxPos.y / static_cast<real>(cellSize))) * cellSize;
	
	for (s32 y = miny; y <= maxy; y += cellSize)
	{
		for (s32 x = minx; x <= maxx; x += cellSize)
		{
			if (tileMgr.hasEntityAtPosition(x,y))
			{
				getEntitiesInRangeOnTilePosition(p_sensor, p_center, tt::math::Point2(x,y), tileMgr, p_entities);
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------
// CircleShape

CircleShape::CircleShape(real p_minRadius, real p_maxRadius)
:
BoundingRectShape(),
m_minRadius(p_minRadius),
m_minRadiusSquared(m_minRadius * m_minRadius),
m_maxRadius(p_maxRadius),
m_maxRadiusSquared(m_maxRadius * m_maxRadius)
{
	// Do some sanity checking
	TT_ASSERT(m_minRadius >= 0);
	TT_ASSERT(m_maxRadius > 0);
	TT_ASSERT(m_minRadius < m_maxRadius);
}

void CircleShape::visualize(const tt::engine::renderer::ColorRGBA& p_color) const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView& debugView = AppGlobal::getGame()->getDebugView();
	
	if (m_minRadius > 0)
	{
		tt::engine::renderer::ColorRGBA color = p_color;
		
		// make inner circle a bit darker
		color.r -= 50;
		color.g -= 50;
		color.b -= 50;
		
		debugView.renderCircle(color, m_position, m_minRadius);
	}
	
	debugView.renderCircle(p_color, m_position, m_maxRadius);
#else
	(void)p_color;
#endif
}


void CircleShape::pushToVMStack(HSQUIRRELVM p_vm)
{
	using script::wrappers::CircleShapeWrapper;
	CircleShapeWrapper wrapper(m_minRadius, m_maxRadius);
	SqBind<CircleShapeWrapper>::push(p_vm, wrapper);
}


ShapePtr CircleShape::createForUnserialize()
{
	return ShapePtr(new CircleShape(0.5f, 1.0f));
}


void CircleShape::serializeImpl(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_minRadius, p_context);
	bu::put(m_maxRadius, p_context);
}


void CircleShape::unserializeImpl(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	m_minRadius        = bu::get<real>(p_context);
	m_maxRadius        = bu::get<real>(p_context);
	m_minRadiusSquared = m_minRadius * m_minRadius;
	m_maxRadiusSquared = m_maxRadius * m_maxRadius;
}


tt::math::VectorRect CircleShape::getRect(const Entity& /*p_source*/) const
{
	tt::math::Vector2 pos(m_position);
	pos.x -= m_maxRadius;
	pos.y -= m_maxRadius;
	real size = m_maxRadius * 2.0f;
	
	return tt::math::VectorRect(pos, size, size);
}


bool CircleShape::intersects(const tt::math::Vector2& p_position) const
{
	const real distance = distanceSquared(m_position, p_position);
	return distance <= m_maxRadiusSquared && distance >= m_minRadiusSquared;
}


bool CircleShape::intersects(const CircleShape& p_circle) const
{
	// FIXME: Account for inner radius
	const real distance = tt::math::distance(m_position, p_circle.m_position);
	return distance <= (m_maxRadius + p_circle.m_maxRadius);
}


bool CircleShape::intersects(const BoxShape& p_box) const
{
	using namespace tt::math;
	Vector2 closestPoint(m_position);
	VectorRect aabb(p_box.getBoundingRect());
	
	// FIXME: Move Circle->AABB intersect test to ttdev
	if (m_minRadius == 0.0f)
	{
		if (m_position.x < aabb.getLeft())
		{
			closestPoint.x = aabb.getLeft();
		}
		else if(m_position.x > aabb.getRight())
		{
			closestPoint.x = aabb.getRight();
		}
		
		if (m_position.y < aabb.getTop())
		{
			closestPoint.y = aabb.getTop();
		}
		else if (m_position.y > aabb.getBottom())
		{
			closestPoint.y = aabb.getBottom();
		}
		
		real distance = (closestPoint - m_position).lengthSquared();
		return distance <= m_maxRadiusSquared;
	}
	
	Vector2 farthestPoint(m_position);
	if (m_position.x < aabb.getLeft())
	{
		closestPoint.x  = aabb.getLeft();
		farthestPoint.x = aabb.getRight();
	}
	else if(m_position.x > aabb.getRight())
	{
		closestPoint.x  = aabb.getRight();
		farthestPoint.x = aabb.getLeft();
	}
	
	if (m_position.y < aabb.getTop())
	{
		closestPoint.y  = aabb.getTop();
		farthestPoint.y = aabb.getBottom();
	}
	else if (m_position.y > aabb.getBottom())
	{
		closestPoint.y  = aabb.getBottom();
		farthestPoint.y = aabb.getTop();
	}
	
	real distanceNear = (closestPoint - m_position).lengthSquared();
	real distanceFar  = (farthestPoint - m_position).lengthSquared();
	
	return distanceNear <= m_maxRadiusSquared && distanceFar >= m_minRadiusSquared;
}


bool CircleShape::intersects(const ConeShape& p_cone) const
{
	return p_cone.intersects(*this);
}


bool CircleShape::intersects(const RayShape& p_ray) const
{
	return p_ray.intersects(*this);
}


//--------------------------------------------------------------------------------------------------
// BoxShape

void BoxShape::visualize(const tt::engine::renderer::ColorRGBA& p_color) const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	using namespace tt::math;
	
	Vector2 vtx1(m_boundingRect.getPosition());
	Vector2 vtx2(m_boundingRect.getPosition() + Vector2(m_boundingRect.getWidth(), 0.0f));
	Vector2 vtx3(m_boundingRect.getPosition() + Vector2(m_boundingRect.getWidth(), m_boundingRect.getHeight()));
	Vector2 vtx4(m_boundingRect.getPosition() + Vector2(0.0f,                      m_boundingRect.getHeight()));
	
	DebugView& debugView = AppGlobal::getGame()->getDebugView();
	debugView.renderLine(p_color, vtx1, vtx2);
	debugView.renderLine(p_color, vtx2, vtx3);
	debugView.renderLine(p_color, vtx3, vtx4);
	debugView.renderLine(p_color, vtx4, vtx1);
#else
	(void)p_color;
#endif
}


void BoxShape::pushToVMStack(HSQUIRRELVM p_vm)
{
	using script::wrappers::BoxShapeWrapper;
	BoxShapeWrapper wrapper(m_width, m_height);
	SqBind<BoxShapeWrapper>::push(p_vm, wrapper);
}


ShapePtr BoxShape::createForUnserialize()
{
	return ShapePtr(new BoxShape(1.0f, 1.0f, true));
}


void BoxShape::serializeImpl(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_width,         p_context);
	bu::put(m_height,        p_context);
	bu::put(m_useIntersects, p_context);
}


void BoxShape::unserializeImpl(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	m_width         = bu::get<real>(p_context);
	m_height        = bu::get<real>(p_context);
	m_useIntersects = bu::get<bool>(p_context);
}


tt::math::VectorRect BoxShape::getRect(const Entity& p_parent) const
{
	tt::math::Vector2 pos(m_position);
	
	tt::math::Vector2 size(m_width, m_height);
	size = p_parent.applyOrientationToVector2(size);
	size.x = std::abs(size.x);
	size.y = std::abs(size.y);
	
	pos.x -= size.x / 2.0f;
	pos.y -= size.y / 2.0f;
	
	return tt::math::VectorRect(pos, size.x, size.y);
}


bool BoxShape::intersects(const tt::math::Vector2& p_position) const
{
	return m_boundingRect.intersects(p_position);
}


bool BoxShape::intersects(const CircleShape& p_circle) const
{
	return (m_useIntersects) ? p_circle.intersects(*this) :
	                           m_boundingRect.contains(p_circle.getBoundingRect());
}


bool BoxShape::intersects(const BoxShape& p_box) const
{
	return (m_useIntersects) ? m_boundingRect.intersects(p_box.getBoundingRect())
	                         : m_boundingRect.contains(  p_box.getBoundingRect());
}


bool BoxShape::intersects(const ConeShape& p_cone) const
{
	return p_cone.intersects(*this);
}


bool BoxShape::intersects(const RayShape& p_ray) const
{
	return p_ray.intersects(*this);
}


//--------------------------------------------------------------------------------------------------
// ConeShape

ConeShape::ConeShape(real p_minRadius, real p_maxRadius, real p_angle, real p_spread)
:
CircleShape(p_minRadius, p_maxRadius),
m_angle(tt::math::degToRad(p_angle)),
m_spread(tt::math::degToRad(p_spread)),
m_startAngle(0.0f),
m_endAngle(0.0f)
{
	TT_ASSERT(p_spread > 0.0f);
}


void ConeShape::updateTransform(const Entity& p_parent, const tt::math::Vector2& p_position, const Sensor* /*p_sensor*/)
{
	BoundingRectShape::updateTransform(p_parent, p_position);
	
	const real coneAngle = p_parent.applyOrientationToAngle(m_angle);
	TT_ASSERT(coneAngle >= 0.0f && coneAngle <  tt::math::twoPi);
	
	TT_ASSERT(m_spread  >  0.0f && m_spread  <= tt::math::twoPi);
	m_startAngle = coneAngle - (0.5f * m_spread);
	if (m_startAngle < 0.0f)
	{
		m_startAngle += tt::math::twoPi;
	}
	
	if (m_startAngle == tt::math::twoPi)
	{
		m_startAngle = 0.0f;
	}
	
	TT_ASSERT(m_startAngle >= 0.0f && m_startAngle < tt::math::twoPi);
	
	m_endAngle   = m_startAngle + m_spread;
	// Don't clamp end so we know which range to check.
}


void ConeShape::visualize(const tt::engine::renderer::ColorRGBA& p_color) const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView& debugView = AppGlobal::getGame()->getDebugView();
	
	if (m_minRadius > 0)
	{
		tt::engine::renderer::ColorRGBA color = p_color;
		
		// make inner circle a bit darker
		color.r -= 50;
		color.g -= 50;
		color.b -= 50;
		
		debugView.renderCirclePart(color, m_position, m_minRadius, m_startAngle, m_endAngle);
	}
	
	tt::math::Vector2 startSpreadStart = tt::math::Vector2(tt::math::sin(m_startAngle) * m_minRadius,
	                                                       tt::math::cos(m_startAngle) * m_minRadius);
	
	tt::math::Vector2 startSpreadEnd = tt::math::Vector2(tt::math::sin(m_endAngle) * m_minRadius,
	                                                     tt::math::cos(m_endAngle) * m_minRadius);
	
	tt::math::Vector2 endSpreadStart = tt::math::Vector2(tt::math::sin(m_startAngle) * m_maxRadius,
	                                                     tt::math::cos(m_startAngle) * m_maxRadius);
	
	tt::math::Vector2 endSpreadEnd  = tt::math::Vector2(tt::math::sin(m_endAngle) * m_maxRadius,
	                                                    tt::math::cos(m_endAngle) * m_maxRadius);
	
	debugView.renderLine      (p_color, m_position + startSpreadStart, m_position + endSpreadStart);
	debugView.renderLine      (p_color, m_position + startSpreadEnd,   m_position + endSpreadEnd  );
	debugView.renderCirclePart(p_color, m_position, m_maxRadius, m_startAngle, m_endAngle);
#else
	(void)p_color;
#endif
}


void ConeShape::pushToVMStack(HSQUIRRELVM p_vm)
{
	using script::wrappers::ConeShapeWrapper;
	
	ConeShapeWrapper wrapper(*this);
	SqBind<ConeShapeWrapper>::push(p_vm, wrapper);
}


ShapePtr ConeShape::createForUnserialize()
{
	return ShapePtr(new ConeShape(0.5f, 1.0f, 0.0f, 0.5f));
}


void ConeShape::serializeImpl(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	// Serialize base class
	CircleShape::serializeImpl(p_context);
	
	bu::put(m_angle,      p_context);
	bu::put(m_spread,     p_context);
	bu::put(m_startAngle, p_context);
	bu::put(m_endAngle,   p_context);
}


void ConeShape::unserializeImpl(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	// Unserialize base class
	CircleShape::unserializeImpl(p_context);
	
	m_angle      = bu::get<real>(p_context);
	m_spread     = bu::get<real>(p_context);
	m_startAngle = bu::get<real>(p_context);
	m_endAngle   = bu::get<real>(p_context);
}


bool ConeShape::intersects(const tt::math::Vector2& p_position) const
{
	if (CircleShape::intersects(p_position) == false)
	{
		return false;
	}
	// Early exit when cone is perfect circle
	else if (tt::math::realEqual(m_spread, tt::math::twoPi))
	{
		return true;
	}
	
	return inSpread(p_position);
}


bool ConeShape::intersects(const CircleShape& p_circle) const
{
	// FIXME: Needs correct implementation!
	const real distance = tt::math::distance(m_position, p_circle.getPosition());
	
	// Early exit
	if (distance > (m_maxRadius + p_circle.getMaxRadius()) ||
		distance < (m_minRadius - p_circle.getMaxRadius()))
	{
		return false;
	}
	// Early exit when cone is perfect circle
	else if (tt::math::realEqual(m_spread, tt::math::twoPi))
	{
		return true;
	}
	
	// Circles intersect; now check if other circle is within spread
	return inSpread(p_circle.getPosition());
}


bool ConeShape::intersects(const BoxShape& p_box) const
{
	if (CircleShape::intersects(p_box) == false)
	{
		return false;
	}
	// Early exit when cone is perfect circle
	else if (tt::math::realEqual(m_spread, tt::math::twoPi))
	{
		return true;
	}
	
	const tt::math::VectorRect aabb(p_box.getBoundingRect());
	
	// now check if any of the four edges of the aabb are within the spread
	if (inSpread(tt::math::Vector2(aabb.getLeft(), aabb.getTop())))
	{
		return true;
	}
	
	if (inSpread(tt::math::Vector2(aabb.getRight(), aabb.getTop())))
	{
		return true;
	}
	
	if (inSpread(tt::math::Vector2(aabb.getLeft(), aabb.getBottom())))
	{
		return true;
	}
	
	if (inSpread(tt::math::Vector2(aabb.getRight(), aabb.getBottom())))
	{
		return true;
	}
	
	return false;
}


bool ConeShape::intersects(const ConeShape& p_cone) const
{
	(void)p_cone;
	TT_PANIC("ConeShape::testCone not implemented");
	return false;
}


bool ConeShape::intersects(const RayShape& p_ray) const
{
	(void)p_ray;
	TT_PANIC("ConeShape::testRay not implemented");
	return false;
}


bool ConeShape::inSpread(const tt::math::Vector2& p_targetPos) const
{
	tt::math::Vector2 localPos(p_targetPos - m_position);
	real angle = tt::math::atan2(localPos.x, localPos.y);
	if (localPos.x < 0.0f)
	{
		angle += tt::math::twoPi;
	}
	TT_ASSERT(angle >= 0.0f && angle < tt::math::twoPi);
	TT_ASSERT(m_startAngle <= m_endAngle);
	
	return (angle >= m_startAngle && angle <= m_endAngle) ||
	       (m_endAngle > tt::math::twoPi && (angle + tt::math::twoPi) <= m_endAngle); // The cone goes over 0.
}


//--------------------------------------------------------------------------------------------------
// RayShape

typedef std::vector<tt::math::Point2> VisitedPositions;

class RayHitTester
{
public:
	inline RayHitTester(VisitedPositions& p_positions)
	: m_positions(p_positions) {}
	
	inline bool operator()(const tt::math::Point2& p_location)
	{
		m_positions.push_back(p_location);
		return false;
	}
	
private:
	// No assignment
	RayHitTester& operator=(const RayHitTester&);
	
	VisitedPositions& m_positions;
};


RayShape::RayShape(const tt::math::Vector2& p_offsetEndPos, const entity::EntityHandle& p_target)
:
BoundingRectShape(),
m_offsetEndPos(p_offsetEndPos),
m_target(p_target),
m_startPosition(tt::math::Vector2::zero),
m_endPosition(tt::math::Vector2::zero),
m_hitPosition(tt::math::Vector2::zero)
{
}


void RayShape::updateTransform(const Entity& p_parent, const tt::math::Vector2& p_position, const Sensor* p_sensor)
{
	BoundingRectShape::updateTransform(p_parent, p_position);
	
	m_startPosition = m_position;
	
	const entity::Entity* entity = m_target.getPtr();
	if (entity != 0)
	{
		m_endPosition = entity->applyOrientationToVector2(m_offsetEndPos) + entity->getCenterPosition();
	}
	else
	{
		m_endPosition = m_startPosition + p_parent.applyOrientationToVector2(m_offsetEndPos);
	}
	
	// No sensor (yet?) so we're not able to determine if ray hits collision before reaching endpoint
	if (p_sensor == 0)
	{
		m_hitPosition = m_endPosition;
		return;
	}
	
	const RayTracer& rayTracer(p_sensor->getRayTracer());
	rayTracer.trace(m_startPosition, m_endPosition);
	m_hitPosition = rayTracer.getHitLocation();
}


void RayShape::getEntitiesInRange(const Sensor& p_sensor, bool p_center, EntityHandles& p_entities) const
{
	const level::TileRegistrationMgr& tileMgr = AppGlobal::getGame()->getTileRegistrationMgr();

	VisitedPositions positions;
	positions.reserve(50);
	
	RayHitTester hitTester(positions);
	tt::code::tileRayTrace(m_startPosition, m_hitPosition, hitTester);
	
	for (VisitedPositions::const_iterator it = positions.begin(); it != positions.end(); ++it)
	{
		if(AppGlobal::getGame()->getAttributeLayer()->contains(*it))
		{
			getEntitiesInRangeOnTilePosition(p_sensor, p_center, (*it), tileMgr, p_entities);
		}
	}
}


void RayShape::visualize(const tt::engine::renderer::ColorRGBA& p_color) const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView& debugView = AppGlobal::getGame()->getDebugView();
	
	debugView.renderLine(p_color, m_startPosition, m_endPosition);
#else
	(void)p_color;
#endif
}


void RayShape::pushToVMStack(HSQUIRRELVM p_vm)
{
	using script::wrappers::RayShapeWrapper;
	RayShapeWrapper wrapper(m_offsetEndPos, m_target);
	SqBind<RayShapeWrapper>::push(p_vm, wrapper);
}


ShapePtr RayShape::createForUnserialize()
{
	return ShapePtr(new RayShape(tt::math::Vector2::zero, entity::EntityHandle()));
}


void RayShape::serializeImpl(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_offsetEndPos,  p_context);
	bu::putHandle(m_target,  p_context);
	bu::put(m_startPosition, p_context);
	bu::put(m_endPosition,   p_context);
	bu::put(m_hitPosition,   p_context);
}


void RayShape::unserializeImpl(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	m_offsetEndPos  = bu::get<tt::math::Vector2>(p_context);
	m_target        = bu::getHandle<entity::Entity>(p_context);
	m_startPosition = bu::get<tt::math::Vector2>(p_context);
	m_endPosition   = bu::get<tt::math::Vector2>(p_context);
	m_hitPosition   = bu::get<tt::math::Vector2>(p_context);
}


tt::math::VectorRect RayShape::getRect(const Entity& /*p_parent*/) const
{
	real left   = std::min(m_startPosition.x, m_hitPosition.x);
	real top    = std::max(m_startPosition.y, m_hitPosition.y);
	real right  = std::max(m_startPosition.x, m_hitPosition.x);
	real bottom = std::min(m_startPosition.y, m_hitPosition.y);
	
	right++;
	top++;
	
	return tt::math::VectorRect(tt::math::Vector2(left, bottom), tt::math::Vector2(right, top));
}


bool RayShape::intersects(const tt::math::Vector2& /*p_position*/) const
{
	TT_PANIC("RayShape::intersects(Vector2) is not implemented, and should not be used");
	return false;
}


bool RayShape::intersects(const CircleShape& p_circle) const
{
	// See http://www.melloland.com/scripts-and-tutos/collision-detection-between-circles-and-lines
	using namespace tt::math;
	
	Vector2 circlePos = p_circle.getPosition();
	const real minRadius = p_circle.getMinRadius();
	const real maxRadius = p_circle.getMaxRadius();
	
	// Before all of this, check if the line isn't inside the radii
	real beginDist = distance(m_startPosition, circlePos);
	real endDist =   distance(m_endPosition, circlePos);
	
	if (beginDist < minRadius && endDist < minRadius)
	{
		return false;
	}
	
	if (beginDist <= maxRadius || endDist <= maxRadius)
	{
		return true;
	}
	
	// Check if the line touches the maxRadius
	real vx = m_endPosition.x - m_startPosition.x;
	real vy = m_endPosition.y - m_startPosition.y;
	real xdiff = m_startPosition.x - circlePos.x;
	real ydiff = m_startPosition.y - circlePos.y;
	
	real a = (vx * vx) + (vy * vy);
	real b = 2.0f * ((vx * xdiff) + (vy * ydiff));
	real c = (xdiff * xdiff) + (ydiff * ydiff) - (maxRadius * maxRadius);
	real quad = (b * b) - (4.0f * a * c);
	
	if (quad >= 0.0f)
	{
		// An infinite collision is happening, but let's not stop here
		real quadsqrt = tt::math::sqrt(quad);
		for (int i = -1; i <= 1; i += 2)
		{
			// Returns the two coordinates of the intersection points
			real t = (i * -b + quadsqrt) / (2.0f * a);
			real x = m_startPosition.x + (i * vx * t);
			real y = m_startPosition.y + (i * vy * t);
			
			// If one of them is in the boundaries of the segment, it collides
			if (x >= std::min(m_startPosition.x, m_endPosition.x) &&
                x <= std::max(m_startPosition.x, m_endPosition.x) &&
                y >= std::min(m_startPosition.y, m_endPosition.y) &&
                y <= std::max(m_startPosition.y, m_endPosition.y))
			{
				return true;
			}
		}
	}
	return false;
}


bool RayShape::intersects(const BoxShape& p_box) const
{
	// See http://www.gamedev.net/topic/338987-aabb---line-segment-intersection-test/
	const real epsilon = 10e-7f;
	
	using namespace tt::math;
	VectorRect aabb(p_box.getBoundingRect());
	
	Vector2 min(aabb.getMin());
	Vector2 max(aabb.getMaxInside());
	
	Vector2 d = (m_hitPosition - m_startPosition) * 0.5f;
	Vector2 e = (max - min) * 0.5f;
	Vector2 c = m_startPosition + d - (min + max) * 0.5f;
	Vector2 ad;
	ad.x = tt::math::fabs(d.x);
	ad.y = tt::math::fabs(d.y);
	
	if (tt::math::fabs(c.x) > e.x + ad.x)
		return false;
	if (tt::math::fabs(c.y) > e.y + ad.y)
		return false;
	
	if (tt::math::fabs(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + epsilon)
		return false;
	
	return true;
}


bool RayShape::intersects(const ConeShape& p_cone) const
{
	(void)p_cone;
	TT_PANIC("Not implemented");
	return false;
}


bool RayShape::intersects(const RayShape& p_ray) const
{
	(void)p_ray;
	TT_PANIC("Not implemented");
	return false;
}


// Namespace end
}
}
}
}
