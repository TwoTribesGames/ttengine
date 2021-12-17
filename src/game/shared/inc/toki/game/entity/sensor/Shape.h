#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_SHAPE_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_SHAPE_H

#include <squirrel/squirrel.h>

#include <tt/code/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Rect.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/level/helpers.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class CircleShape;
class BoxShape;
class ConeShape;
class RayShape;

class Shape
{
public:
	// For serialization: identifier for the type of shape that was serialized
	// (these enum values need to be kept constant! Do not reorder them or remove values:
	//  this will cause saved serialization data to become invalid)
	enum ShapeType
	{
		ShapeType_Circle,
		ShapeType_Box,
		ShapeType_Cone,
		ShapeType_Ray
	};
	
	inline Shape()
	:
	m_position(tt::math::Vector2::zero),
	m_boundingRect()
	{ }
	
	virtual ~Shape() { }
	
	virtual void updateTransform(const Entity& p_parent, const tt::math::Vector2& p_position, const Sensor* p_sensor = 0);
	
	void getEntitiesWithCenterInRange   (const Sensor& p_sensor, EntityHandles& p_entities) const;
	void getEntitiesWithWorldRectInRange(const Sensor& p_sensor, EntityHandles& p_entities) const;
	
	bool isShapeInRange (const ShapePtr& p_shape) const;
	
	virtual void visualize(const tt::engine::renderer::ColorRGBA& p_color) const = 0;
	
	virtual void pushToVMStack(HSQUIRRELVM p_vm) = 0;
	
	void            serialize  (tt::code::BufferWriteContext* p_context) const;
	static ShapePtr unserialize(tt::code::BufferReadContext*  p_context);
	
	inline tt::math::PointRect getBoundingTileRect() const
	{
		return level::worldToTile(m_boundingRect);
	}
	
	inline const tt::math::VectorRect& getBoundingRect() const
	{
		return m_boundingRect;
	}
	
	inline const tt::math::Vector2& getPosition() const { return m_position; }
	inline void setPosition(const tt::math::Vector2& p_position) { m_position = p_position; }
	
	virtual ShapeType getType() const = 0;
	
	// Intersection methods
	virtual bool intersects(const tt::math::Vector2& p_position) const = 0;
	virtual bool intersects(const CircleShape&       p_circle  ) const = 0;
	virtual bool intersects(const BoxShape&          p_box     ) const = 0;
	virtual bool intersects(const ConeShape&         p_cone    ) const = 0;
	virtual bool intersects(const RayShape&          p_ray     ) const = 0;
	
protected:
	virtual void serializeImpl  (tt::code::BufferWriteContext* p_context) const = 0;
	virtual void unserializeImpl(tt::code::BufferReadContext*  p_context)       = 0;
	
	virtual void getEntitiesInRange(const Sensor& p_sensor, bool p_center, EntityHandles& p_entities) const = 0;
	virtual tt::math::VectorRect getRect(const Entity& p_parent) const = 0;
	
	bool isInRange(const Entity& p_target, bool p_center) const;
	
	void getEntitiesInRangeOnTilePosition(const Sensor&            p_sensor,
	                                      bool                     p_center,
	                                      const tt::math::Point2&  p_tilePosition,
	                                      const level::TileRegistrationMgr& p_tileMgr,
	                                      EntityHandles&           p_entities) const;
	
	tt::math::Vector2    m_position;
	tt::math::VectorRect m_boundingRect;
};


class BoundingRectShape : public Shape
{
public:
	virtual void getEntitiesInRange(const Sensor& p_sensor, bool p_center, EntityHandles& p_entities) const;
};


class CircleShape : public BoundingRectShape
{
public:
	CircleShape(real p_minRadius, real p_maxRadius);
	
	virtual void visualize(const tt::engine::renderer::ColorRGBA& p_color) const;
	
	virtual void pushToVMStack(HSQUIRRELVM p_vm);
	static ShapePtr createForUnserialize();
	
	inline real getMinRadius() const { return m_minRadius; }
	inline real getMaxRadius() const { return m_maxRadius; }
	inline real getMinRadiusSquared() const { return m_minRadiusSquared; }
	inline real getMaxRadiusSquared() const { return m_maxRadiusSquared; }
	inline void setMinRadius(real p_minRadius) { m_minRadius = p_minRadius; m_minRadiusSquared = (m_minRadius * m_minRadius); }
	inline void setMaxRadius(real p_maxRadius) { m_maxRadius = p_maxRadius; m_maxRadiusSquared = (m_maxRadius * m_maxRadius); }
	
	inline virtual ShapeType getType() const { return ShapeType_Circle; }
	
	// Intersection methods
	virtual bool intersects(const tt::math::Vector2& p_position) const;
	virtual bool intersects(const CircleShape&       p_circle  ) const;
	virtual bool intersects(const BoxShape&          p_box     ) const;
	virtual bool intersects(const ConeShape&         p_cone    ) const;
	virtual bool intersects(const RayShape&          p_ray     ) const;
	
protected:
	virtual void serializeImpl  (tt::code::BufferWriteContext* p_context) const;
	virtual void unserializeImpl(tt::code::BufferReadContext*  p_context);
	virtual tt::math::VectorRect getRect(const Entity& p_parent) const;
	
	real m_minRadius;
	real m_minRadiusSquared;
	real m_maxRadius;
	real m_maxRadiusSquared;
};



class BoxShape : public BoundingRectShape
{
public:
	inline BoxShape(real p_width, real p_height, bool p_useIntersects)
	:
	BoundingRectShape(),
	m_width(p_width),
	m_height(p_height),
	m_useIntersects(p_useIntersects)
	{}
	
	inline void useIntersects(bool p_useIntersects) { m_useIntersects = p_useIntersects; }
	
	virtual void visualize(const tt::engine::renderer::ColorRGBA& p_color) const;
	
	virtual void pushToVMStack(HSQUIRRELVM p_vm);
	static ShapePtr createForUnserialize();
	
	inline real getWidth( ) const { return m_width;  }
	inline real getHeight() const { return m_height; }
	inline void setWidth( real p_width ) { m_width  = p_width;  }
	inline void setHeight(real p_height) { m_height = p_height; }
	
	inline virtual ShapeType getType() const { return ShapeType_Box; }
	
	// Intersection methods
	virtual bool intersects(const tt::math::Vector2& p_position) const;
	virtual bool intersects(const CircleShape&       p_circle  ) const;
	virtual bool intersects(const BoxShape&          p_box     ) const;
	virtual bool intersects(const ConeShape&         p_cone    ) const;
	virtual bool intersects(const RayShape&          p_ray     ) const;
	
protected:
	virtual void serializeImpl  (tt::code::BufferWriteContext* p_context) const;
	virtual void unserializeImpl(tt::code::BufferReadContext*  p_context);
	virtual tt::math::VectorRect getRect(const Entity& p_parent) const;
	
private:
	real m_width;
	real m_height;
	bool m_useIntersects; // Or use contains.
};


class ConeShape : public CircleShape
{
public:
	ConeShape(real p_minRadius, real p_maxRadius, real p_angle, real p_spread);
	
	virtual void updateTransform(const Entity& p_parent, const tt::math::Vector2& p_position,
	                             const Sensor* p_sensor = 0);
	
	virtual void visualize(const tt::engine::renderer::ColorRGBA& p_color) const;
	
	virtual void pushToVMStack(HSQUIRRELVM p_vm);
	static ShapePtr createForUnserialize();
	
	inline real getAngle()  const { return tt::math::radToDeg(m_angle ); }
	inline real getSpread() const { return tt::math::radToDeg(m_spread); }
	inline void setAngle( real p_angle ) { m_angle  = tt::math::degToRad(p_angle ); }
	inline void setSpread(real p_spread) { m_spread = tt::math::degToRad(p_spread); }
	
	inline virtual ShapeType getType() const { return ShapeType_Cone; }
	
	// Intersection methods
	virtual bool intersects(const tt::math::Vector2& p_position) const;
	virtual bool intersects(const CircleShape&       p_circle  ) const;
	virtual bool intersects(const BoxShape&          p_box     ) const;
	virtual bool intersects(const ConeShape&         p_cone    ) const;
	virtual bool intersects(const RayShape&          p_ray     ) const;
	
protected:
	virtual void serializeImpl  (tt::code::BufferWriteContext* p_context) const;
	virtual void unserializeImpl(tt::code::BufferReadContext*  p_context);
	
private:
	bool inSpread(const tt::math::Vector2& p_targetPos) const;
	
	real m_angle;
	real m_spread;
	
	real m_startAngle;
	real m_endAngle;
};


class RayShape : public BoundingRectShape
{
public:
	RayShape(const tt::math::Vector2& p_offsetEndPos, const entity::EntityHandle& p_target);
	
	virtual void updateTransform(const Entity& p_parent, const tt::math::Vector2& p_position,
	                             const Sensor* p_sensor = 0);
	
	virtual void getEntitiesInRange(const Sensor& p_sensor, bool p_center, EntityHandles& p_entities) const;
	
	virtual void visualize(const tt::engine::renderer::ColorRGBA& p_color) const;
	
	virtual void pushToVMStack(HSQUIRRELVM p_vm);
	static ShapePtr createForUnserialize();
	
	inline const tt::math::Vector2& getOffsetEndPos() const { return m_offsetEndPos; }
	inline entity::EntityHandle     getTarget()       const { return m_target;       }
	inline void setOffsetEndPos(const tt::math::Vector2&  p_offsetEndPos) { m_offsetEndPos = p_offsetEndPos; }
	inline void setTarget(entity::EntityHandle p_target)                  { m_target       = p_target;       }
	
	virtual ShapeType getType() const { return ShapeType_Ray; }
	
	// Intersection methods
	virtual bool intersects(const tt::math::Vector2& p_position) const;
	virtual bool intersects(const CircleShape&       p_circle  ) const;
	virtual bool intersects(const BoxShape&          p_box     ) const;
	virtual bool intersects(const ConeShape&         p_cone    ) const;
	virtual bool intersects(const RayShape&          p_ray     ) const;
	
protected:
	virtual void serializeImpl  (tt::code::BufferWriteContext* p_context) const;
	virtual void unserializeImpl(tt::code::BufferReadContext*  p_context);
	virtual tt::math::VectorRect getRect(const Entity& p_parent) const;
	
private:
	tt::math::Vector2    m_offsetEndPos;
	entity::EntityHandle m_target;
	
	tt::math::Vector2    m_startPosition;
	tt::math::Vector2    m_endPosition;
	tt::math::Vector2    m_hitPosition;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_SHAPE_H)
