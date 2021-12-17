#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_SHAPEWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_SHAPEWRAPPER_H

#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/sensor/Shape.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/script/wrappers/EntityWrapper.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

#if defined(TT_BUILD_FINAL)
	#define TT_DYNAMIC_CAST static_cast
#else
	#define TT_DYNAMIC_CAST dynamic_cast
#endif


/*! \brief 'Shape' in Squirrel. Base class for a shape that a Sensor should use. */
class ShapeWrapper
{
public:
	ShapeWrapper();
	
	inline ~ShapeWrapper() {}
	
	inline const entity::sensor::ShapePtr& getShape() const { return m_shape; }
	
	// bindings
	/*! \brief Returns the world position of this shape */
	const tt::math::Vector2& getPosition() const;
	/*! \brief Sets the worldposition of this shape. Cannot be used when shape is tied to a sensor/entity */
	void setPosition(const tt::math::Vector2& p_position);
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
protected:
	typedef entity::sensor::Shape Shape;
	inline explicit ShapeWrapper(const entity::sensor::ShapePtr& p_shape)
	:
	m_shape(p_shape)
	{}
	
	entity::sensor::ShapePtr m_shape;
};


/*! \brief 'CircleShape' in Squirrel. */
class CircleShapeWrapper : public ShapeWrapper
{
public:
	inline CircleShapeWrapper() {}
	
	/*! \brief CircleShape() Creates a new circle shape to be used for sensors
	    \param p_minRadius the minimum radius of the circle. Can be 0
	    \param p_maxRadius the maximum radius of the circle. */
	explicit CircleShapeWrapper(real p_minRadius, real p_maxRadius);
	
	real getMinRadius() const; //!< Get minimum radius. In degrees. Can be 0
	real getMaxRadius() const; //!< Get maximum radius. In degrees
	void setMinRadius(real p_minRadius); //!< Set minimum radius. In degrees. Can be 0
	void setMaxRadius(real p_maxRadius); //!< Set maximum radius. In degrees
	
	/*! \brief Does this shape intersect with p_position */
	bool intersectsPosition(const tt::math::Vector2& p_position);
	
private:
	typedef entity::sensor::CircleShape CircleShape;
	inline CircleShape* getCircleShapePtr() const
	{
		if (m_shape == 0)
		{
			return 0;
		}
		else
		{
			TT_ASSERT(m_shape->getType() == Shape::ShapeType_Circle);
			return TT_DYNAMIC_CAST<CircleShape*>(m_shape.get());
		}
	}
};


/*! \brief 'BoxShape' in Squirrel. */
class BoxShapeWrapper : public ShapeWrapper
{
public:
	inline BoxShapeWrapper() {}
	
	/*! \brief Make BoxShape use intersection check. (The default) */
	void doIntersects();
	
	/*! \brief Make BoxShape use contains check. */
	void doContains();
	
	/*! \brief BoxShape() Creates a new box shape to be used for sensors
	    \param p_width width of the box
	    \param p_height height of the box */
	explicit BoxShapeWrapper(real p_width, real p_height);
	
	real getWidth( ) const; //!< Get width
	real getHeight() const; //!< Get height
	void setWidth( real p_width ); //!< Set width
	void setHeight(real p_height); //!< Set height
	
	/*! \brief Does this shape intersect with p_position */
	bool intersectsPosition(const tt::math::Vector2& p_position);
	
private:
	typedef entity::sensor::BoxShape BoxShape;
	inline BoxShape* getBoxShapePtr() const
	{
		if (m_shape == 0)
		{
			return 0;
		}
		else
		{
			TT_ASSERT(m_shape->getType() == Shape::ShapeType_Box);
			return TT_DYNAMIC_CAST<BoxShape*>(m_shape.get());
		}
	}
};


/*! \brief 'ConeShape' in Squirrel. */
class ConeShapeWrapper : public ShapeWrapper
{
public:
	inline ConeShapeWrapper() {}
	
	/*! \brief ConeShape() Creates a new cone shape to be used for sensors
	    \param p_minRadius the minimum radius of the cone. Can be 0
	    \param p_maxRadius the maximum radius of the cone
	    \param p_orientation the orientation of the cone in degrees (can also be negative)
	    \param p_spread the spread of the cone in degrees (0..360) (both not included) */
	explicit ConeShapeWrapper(real p_minRadius, real p_maxRadius, real p_orientation, real p_spread);
	
	explicit ConeShapeWrapper(const entity::sensor::ConeShape& p_shape);
	
	real getAngle()  const; //!< get the orientation of the cone in degrees (can also be negative)
	real getSpread() const; //!< get the spread of the cone in degrees (0..360) (both not included)
	void setAngle( real p_angle ); //!< Set the orientation of the cone in degrees (can also be negative)
	void setSpread(real p_spread); //!< Set the spread of the cone in degrees (0..360) (both not included)
	real getMinRadius() const; //!< Get minimum radius. In degrees. Can be 0
	real getMaxRadius() const; //!< Get maximum radius. In degrees
	void setMinRadius(real p_minRadius); //!< Set minimum radius. In degrees. Can be 0
	void setMaxRadius(real p_maxRadius); //!< Set maximum radius. In degrees
	
	/*! \brief Does this shape intersect with p_position */
	bool intersectsPosition(const tt::math::Vector2& p_position);
	
private:
	typedef entity::sensor::ConeShape ConeShape;
	inline ConeShape* getConeShapePtr() const
	{
		if (m_shape == 0)
		{
			return 0;
		}
		else
		{
			TT_ASSERT(m_shape->getType() == Shape::ShapeType_Cone);
			return TT_DYNAMIC_CAST<ConeShape*>(m_shape.get());
		}
	}
};


/*! \brief 'RayShape' in Squirrel. */
class RayShapeWrapper : public ShapeWrapper
{
public:
	inline RayShapeWrapper() {}
	
	/*! \brief RayShape() Creates a new ray shape to be used for sensors
	    \param p_offset offset of the ray relative to the target entity (if set). If no target is set, this will be the endposition of the ray (relative to the entity)
	    \param p_target (optional) target entity to which this ray points*/
	explicit RayShapeWrapper(const tt::math::Vector2& p_offset, const entity::EntityHandle& p_target);
	
	const tt::math::Vector2& getOffsetEndPos() const; //!< Get offset of the ray relative to the target entity (if set). If no target is set, this will be the endposition of the ray (relative to the entity)
	void setOffsetEndPos(const tt::math::Vector2&  p_offsetEndPos); //!< Set target entity to which this ray points.
	void setTarget(EntityWrapper* p_target); //!< Set offset of the ray relative to the target entity (if set). If no target is set, this will be the endposition of the ray (relative to the entity)
	
	/*! \brief Does this shape intersect with p_position */
	bool intersectsPosition(const tt::math::Vector2& p_position);
	
private:
	typedef entity::sensor::RayShape RayShape;
	inline RayShape* getRayShapePtr() const
	{
		if (m_shape == 0)
		{
			return 0;
		}
		else
		{
			TT_ASSERT(m_shape->getType() == Shape::ShapeType_Ray);
			return TT_DYNAMIC_CAST<RayShape*>(m_shape.get());
		}
	}
};


// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_SHAPEWRAPPER_H)
