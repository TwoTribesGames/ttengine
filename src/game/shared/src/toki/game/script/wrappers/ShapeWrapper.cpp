#include <tt/script/ScriptEngine.h>

#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

static CircleShapeWrapper* circleShapeWrapper_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 2)
	{
		TT_PANIC("CircleShape incorrect number of parameters. (Got: %d, expected: 2)", params);
		return 0;
	}
	
	SQFloat minRadius;
	if (SQ_FAILED(sq_getfloat(v, 2, &minRadius)))
	{
		TT_PANIC("minRadius is not a float value");
		return 0;
	}
	
	if (minRadius < 0)
	{
		TT_PANIC("Cannot create circle with minRadius < 0");
		return 0;
	}
	
	SQFloat maxRadius;
	if (SQ_FAILED(sq_getfloat(v, 3, &maxRadius)))
	{
		TT_PANIC("maxRadius is not a float value");
		return 0;
	}
	
	if (maxRadius <= 0)
	{
		TT_PANIC("Cannot create circle with maxRadius <= 0");
		return 0;
	}
	
	if (maxRadius <= minRadius)
	{
		TT_PANIC("Cannot create circle with maxRadius <= minRadius");
		return 0;
	}
	
	return new CircleShapeWrapper(minRadius, maxRadius);
}


static BoxShapeWrapper* boxShapeWrapper_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 2)
	{
		TT_PANIC("BoxShape incorrect number of parameters. (Got: %d, expected: 2)", params);
		return 0;
	}
	
	SQFloat width;
	if (SQ_FAILED(sq_getfloat(v, 2, &width)))
	{
		TT_PANIC("Width is not a float value");
		return 0;
	}
	
	SQFloat height;
	if (SQ_FAILED(sq_getfloat(v, 3, &height)))
	{
		TT_PANIC("Height is not a float value");
		return 0;
	}
	
	return new BoxShapeWrapper(width, height);
}


static ConeShapeWrapper* coneShapeWrapper_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 4)
	{
		TT_PANIC("ConeShape incorrect number of parameters. (Got: %d, expected: 4)", params);
		return 0;
	}
	
	SQFloat minRadius;
	if (SQ_FAILED(sq_getfloat(v, 2, &minRadius)))
	{
		TT_PANIC("minRadius is not a float value");
		return 0;
	}
	
	if (minRadius < 0)
	{
		TT_PANIC("Cannot create circle with minRadius < 0");
		return 0;
	}
	
	SQFloat maxRadius;
	if (SQ_FAILED(sq_getfloat(v, 3, &maxRadius)))
	{
		TT_PANIC("maxRadius is not a float value");
		return 0;
	}
	
	if (maxRadius <= 0)
	{
		TT_PANIC("Cannot create circle with maxRadius <= 0");
		return 0;
	}
	
	if (maxRadius <= minRadius)
	{
		TT_PANIC("Cannot create circle with maxRadius <= minRadius");
		return 0;
	}
	
	SQFloat orientation;
	if (SQ_FAILED(sq_getfloat(v, 4, &orientation)))
	{
		TT_PANIC("orientation is not a float value");
		return 0;
	}
	
	SQFloat spread;
	if (SQ_FAILED(sq_getfloat(v, 5, &spread)))
	{
		TT_PANIC("spread is not a float value");
		return 0;
	}
	
	if (spread <= 0.0f || spread > 360.0f)
	{
		TT_PANIC("spread '%f' should be in the range (0..360) (both exclusive)", spread);
		return 0;
	}
	
	return new ConeShapeWrapper(minRadius, maxRadius, orientation, spread);
}


static RayShapeWrapper* rayShapeWrapper_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 1 && params != 2)
	{
		TT_PANIC("RayShape incorrect number of parameters. (Got: %d, expected: 1 or 2)", params);
		return 0;
	}
	
	tt::math::Vector2    offsetEndPos;
	entity::EntityHandle target;
	
	if (params == 1)
	{
		offsetEndPos = SqBind<tt::math::Vector2>::get(v, -1);
	}
	else if (params == 2)
	{
		offsetEndPos = SqBind<tt::math::Vector2>::get(v, -2);
		EntityBase* targetEntity = SqBind<EntityBase>::GetterPtr().get(v, -1);
		if (targetEntity != 0)
		{
			target = targetEntity->getHandle();
		}
	}
	
	return new RayShapeWrapper(offsetEndPos, target);
}


//--------------------------------------------------------------------------------------------------
// Public member functions

ShapeWrapper::ShapeWrapper()
{
}


const tt::math::Vector2& ShapeWrapper::getPosition() const
{
	return m_shape != 0 ? m_shape->getPosition() : tt::math::Vector2::zero;
}


void ShapeWrapper::setPosition(const tt::math::Vector2& p_position)
{
	if (m_shape != 0)
	{
		m_shape->setPosition(p_position);
	}
}


CircleShapeWrapper::CircleShapeWrapper(real p_minRadius, real p_maxRadius)
:
ShapeWrapper(entity::sensor::ShapePtr(new entity::sensor::CircleShape(p_minRadius, p_maxRadius)))
{
}


real CircleShapeWrapper::getMinRadius() const
{
	CircleShape* ptr = getCircleShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getMinRadius();
}


real CircleShapeWrapper::getMaxRadius() const
{
	CircleShape* ptr = getCircleShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getMaxRadius();
}


void CircleShapeWrapper::setMinRadius(real p_minRadius)
{
	CircleShape* ptr = getCircleShapePtr();
	if (ptr != 0)
	{
		ptr->setMinRadius(p_minRadius);
	}
}


void CircleShapeWrapper::setMaxRadius(real p_maxRadius)
{
	CircleShape* ptr = getCircleShapePtr();
	if (ptr != 0)
	{
		ptr->setMaxRadius(p_maxRadius);
	}
}


bool CircleShapeWrapper::intersectsPosition(const tt::math::Vector2& p_position)
{
	CircleShape* ptr = getCircleShapePtr();
	if (ptr != 0)
	{
		return ptr->intersects(p_position);
	}
	return false;
}


BoxShapeWrapper::BoxShapeWrapper(real p_width, real p_height)
:
ShapeWrapper(entity::sensor::ShapePtr(new entity::sensor::BoxShape(p_width, p_height, true)))
{
}


real BoxShapeWrapper::getWidth( ) const
{
	BoxShape* ptr = getBoxShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getWidth();
}


real BoxShapeWrapper::getHeight() const
{
	BoxShape* ptr = getBoxShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getHeight();
}


void BoxShapeWrapper::setWidth( real p_width )
{
	BoxShape* ptr = getBoxShapePtr();
	if (ptr != 0)
	{
		ptr->setWidth(p_width);
	}
}


void BoxShapeWrapper::setHeight(real p_height)
{
	BoxShape* ptr = getBoxShapePtr();
	if (ptr != 0)
	{
		ptr->setHeight(p_height);
	}
}


bool BoxShapeWrapper::intersectsPosition(const tt::math::Vector2& p_position)
{
	BoxShape* ptr = getBoxShapePtr();
	if (ptr != 0)
	{
		return ptr->intersects(p_position);
	}
	return false;
}


ConeShapeWrapper::ConeShapeWrapper(real p_minRadius, real p_maxRadius, real p_orientation, real p_spread)
:
ShapeWrapper(entity::sensor::ShapePtr(new entity::sensor::ConeShape(
	p_minRadius, p_maxRadius, p_orientation, p_spread)))
{
}


ConeShapeWrapper::ConeShapeWrapper(const entity::sensor::ConeShape& p_shape)
:
ShapeWrapper(entity::sensor::ShapePtr(new entity::sensor::ConeShape(p_shape)))
{
}


real ConeShapeWrapper::getAngle()  const
{
	ConeShape* ptr = getConeShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getAngle();
}


real ConeShapeWrapper::getSpread() const
{
	ConeShape* ptr = getConeShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getSpread();
}


void ConeShapeWrapper::setAngle( real p_angle )
{
	ConeShape* ptr = getConeShapePtr();
	if (ptr != 0)
	{
		ptr->setAngle(p_angle);
	}
}


void ConeShapeWrapper::setSpread(real p_spread)
{
	ConeShape* ptr = getConeShapePtr();
	if (ptr != 0)
	{
		ptr->setSpread(p_spread);
	}
}


real ConeShapeWrapper::getMinRadius() const
{
	ConeShape* ptr = getConeShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getMinRadius();
}


real ConeShapeWrapper::getMaxRadius() const
{
	ConeShape* ptr = getConeShapePtr();
	return (ptr == 0) ? 0.0f : ptr->getMaxRadius();
}


void ConeShapeWrapper::setMinRadius(real p_minRadius)
{
	ConeShape* ptr = getConeShapePtr();
	if (ptr != 0)
	{
		ptr->setMinRadius(p_minRadius);
	}
}


void ConeShapeWrapper::setMaxRadius(real p_maxRadius)
{
	ConeShape* ptr = getConeShapePtr();
	if (ptr != 0)
	{
		ptr->setMaxRadius(p_maxRadius);
	}
}


bool ConeShapeWrapper::intersectsPosition(const tt::math::Vector2& p_position)
{
	ConeShape* ptr = getConeShapePtr();
	if (ptr != 0)
	{
		return ptr->intersects(p_position);
	}
	return false;
}


RayShapeWrapper::RayShapeWrapper(const tt::math::Vector2& p_offsetEndPos, const entity::EntityHandle& p_target)
:
ShapeWrapper(entity::sensor::ShapePtr(new entity::sensor::RayShape(p_offsetEndPos, p_target)))
{
}


const tt::math::Vector2& RayShapeWrapper::getOffsetEndPos() const
{
	RayShape* ptr = getRayShapePtr();
	return (ptr == 0) ? tt::math::Vector2::zero : ptr->getOffsetEndPos();
}


void RayShapeWrapper::setOffsetEndPos(const tt::math::Vector2&  p_offsetEndPos)
{
	RayShape* ptr = getRayShapePtr();
	if (ptr != 0)
	{
		ptr->setOffsetEndPos(p_offsetEndPos);
	}
}


void RayShapeWrapper::setTarget(EntityWrapper* p_target)
{
	RayShape* ptr = getRayShapePtr();
	if (ptr != 0)
	{
		ptr->setTarget( (p_target == 0) ? entity::EntityHandle() : p_target->getHandle());
	}
}


bool RayShapeWrapper::intersectsPosition(const tt::math::Vector2& p_position)
{
	RayShape* ptr = getRayShapePtr();
	if (ptr != 0)
	{
		return ptr->intersects(p_position);
	}
	return false;
}


void ShapeWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace event;
	TT_SQBIND_INIT_NO_INSTANCING_NAME(ShapeWrapper, "Shape");
	TT_SQBIND_METHOD(ShapeWrapper, getPosition);
	TT_SQBIND_METHOD(ShapeWrapper, setPosition);
	
	TT_SQBIND_INIT_DERIVED_NAME(CircleShapeWrapper, "Shape", "CircleShape");
	TT_SQBIND_INIT_DERIVED_NAME(BoxShapeWrapper,    "Shape", "BoxShape");
	TT_SQBIND_INIT_DERIVED_NAME(ConeShapeWrapper,   "Shape", "ConeShape");
	TT_SQBIND_INIT_DERIVED_NAME(RayShapeWrapper,    "Shape", "RayShape");
	
	TT_SQBIND_SET_CONSTRUCTOR(CircleShapeWrapper, circleShapeWrapper_constructor);
	TT_SQBIND_METHOD(CircleShapeWrapper, getMinRadius);
	TT_SQBIND_METHOD(CircleShapeWrapper, getMaxRadius);
	TT_SQBIND_METHOD(CircleShapeWrapper, setMinRadius);
	TT_SQBIND_METHOD(CircleShapeWrapper, setMaxRadius);
	TT_SQBIND_METHOD(CircleShapeWrapper, setMaxRadius);
	TT_SQBIND_METHOD(CircleShapeWrapper, intersectsPosition);
	
	TT_SQBIND_SET_CONSTRUCTOR(BoxShapeWrapper,    boxShapeWrapper_constructor);
	TT_SQBIND_METHOD(BoxShapeWrapper, doIntersects);
	TT_SQBIND_METHOD(BoxShapeWrapper, doContains);
	TT_SQBIND_METHOD(BoxShapeWrapper, getWidth);
	TT_SQBIND_METHOD(BoxShapeWrapper, getHeight);
	TT_SQBIND_METHOD(BoxShapeWrapper, setWidth);
	TT_SQBIND_METHOD(BoxShapeWrapper, setHeight);
	TT_SQBIND_METHOD(BoxShapeWrapper, intersectsPosition);
	
	TT_SQBIND_SET_CONSTRUCTOR(ConeShapeWrapper,   coneShapeWrapper_constructor);
	TT_SQBIND_METHOD(ConeShapeWrapper, getAngle);
	TT_SQBIND_METHOD(ConeShapeWrapper, getSpread);
	TT_SQBIND_METHOD(ConeShapeWrapper, setAngle);
	TT_SQBIND_METHOD(ConeShapeWrapper, setSpread);
	TT_SQBIND_METHOD(ConeShapeWrapper, getMinRadius);
	TT_SQBIND_METHOD(ConeShapeWrapper, getMaxRadius);
	TT_SQBIND_METHOD(ConeShapeWrapper, setMinRadius);
	TT_SQBIND_METHOD(ConeShapeWrapper, setMaxRadius);
	TT_SQBIND_METHOD(ConeShapeWrapper, setMaxRadius);
	TT_SQBIND_METHOD(ConeShapeWrapper, intersectsPosition);
	
	TT_SQBIND_SET_CONSTRUCTOR(RayShapeWrapper,    rayShapeWrapper_constructor);
	TT_SQBIND_METHOD(RayShapeWrapper, getOffsetEndPos);
	TT_SQBIND_METHOD(RayShapeWrapper, setOffsetEndPos);
	TT_SQBIND_METHOD(RayShapeWrapper, setTarget);
	TT_SQBIND_METHOD(RayShapeWrapper, intersectsPosition);
}


void BoxShapeWrapper::doIntersects()
{
	if (m_shape != 0)
	{
		using entity::sensor::BoxShape;
		BoxShape* box = reinterpret_cast<BoxShape*>(m_shape.get());
		box->useIntersects(true);
	}
}


void BoxShapeWrapper::doContains()
{
	if (m_shape != 0)
	{
		using entity::sensor::BoxShape;
		BoxShape* box = reinterpret_cast<BoxShape*>(m_shape.get());
		box->useIntersects(false);
	}
}


// Namespace end
}
}
}
}
