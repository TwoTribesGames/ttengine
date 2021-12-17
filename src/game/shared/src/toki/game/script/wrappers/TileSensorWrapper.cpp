#include <tt/script/ScriptEngine.h>

#include <toki/game/entity/sensor/TileSensor.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/script/wrappers/TileSensorWrapper.h>
#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/script/EntityBase.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

TileSensorWrapper::TileSensorWrapper(const entity::sensor::TileSensorHandle& p_sensorHandle)
:
m_sensorHandle(p_sensorHandle)
{
}


bool TileSensorWrapper::isEnabled() const
{
	const entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->isEnabled() : false;
}


void TileSensorWrapper::setEnabled(bool p_enabled)
{
	entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setEnabled(p_enabled);
	}
}


bool TileSensorWrapper::isSuspended() const
{
	const entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->isSuspended() : false;
}


void TileSensorWrapper::setSuspended(bool p_enabled)
{
	entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setSuspended(p_enabled);
	}
}


void TileSensorWrapper::setShape(const ShapeWrapper* p_shape)
{
	entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		entity::sensor::ShapePtr shape = (p_shape == 0) ? entity::sensor::ShapePtr() : p_shape->getShape();
		sensor->setShape(shape);
	}
}


void TileSensorWrapper::setWorldPosition(const tt::math::Vector2& p_position)
{
	entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setWorldPosition(p_position);
	}
}


tt::math::Vector2 TileSensorWrapper::getWorldPosition() const
{
	const entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getWorldPosition() : tt::math::Vector2::zero;
}


void TileSensorWrapper::setOffset(const tt::math::Vector2& p_offset)
{
	entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setOffset(p_offset);
	}
}


bool TileSensorWrapper::hasIgnoreOwnCollision() const
{
	const entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->hasIgnoreOwnCollision() : false;
}


void TileSensorWrapper::setIgnoreOwnCollision(bool p_ignoreOwnCollision)
{
	entity::sensor::TileSensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setIgnoreOwnCollision(p_ignoreOwnCollision);
	}
}


void TileSensorWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace event;
	TT_SQBIND_INIT_NAME(TileSensorWrapper, "TileSensor");
	TT_SQBIND_METHOD(TileSensorWrapper, isEnabled);
	TT_SQBIND_METHOD(TileSensorWrapper, setEnabled);
	TT_SQBIND_METHOD(TileSensorWrapper, isSuspended);
	TT_SQBIND_METHOD(TileSensorWrapper, setSuspended);
	TT_SQBIND_METHOD(TileSensorWrapper, setShape);
	TT_SQBIND_METHOD(TileSensorWrapper, setWorldPosition);
	TT_SQBIND_METHOD(TileSensorWrapper, getWorldPosition);
	TT_SQBIND_METHOD(TileSensorWrapper, setOffset);
	TT_SQBIND_METHOD(TileSensorWrapper, hasIgnoreOwnCollision);
	TT_SQBIND_METHOD(TileSensorWrapper, setIgnoreOwnCollision);
	TT_SQBIND_METHOD(TileSensorWrapper, equals);
	TT_SQBIND_METHOD(TileSensorWrapper, getHandleValue);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
