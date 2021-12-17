#include <tt/script/ScriptEngine.h>

#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/script/wrappers/SensorWrapper.h>
#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/script/EntityBase.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

SensorWrapper::SensorWrapper(const entity::sensor::SensorHandle& p_sensorHandle)
:
m_sensorHandle(p_sensorHandle)
{
}


bool SensorWrapper::isEnabled() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->isEnabled() : false;
}


void SensorWrapper::setEnabled(bool p_enabled)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setEnabled(p_enabled);
	}
}


bool SensorWrapper::isSuspended() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->isSuspended() : false;
}


void SensorWrapper::setSuspended(bool p_enabled)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setSuspended(p_enabled);
	}
}


real SensorWrapper::getDelay() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getDelay() : 0.0f;
}


void SensorWrapper::setDelay(real p_delayInSeconds)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setDelay(p_delayInSeconds);
	}
}


void SensorWrapper::setShape(const ShapeWrapper* p_shape)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		entity::sensor::ShapePtr shape = (p_shape == 0) ? entity::sensor::ShapePtr() : p_shape->getShape();
		sensor->setShape(shape);
	}
}


void SensorWrapper::setDefaultEnterAndExitCallback()
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		switch (sensor->getType())
		{
		case entity::sensor::SensorType_Sight:
			sensor->setEnterCallback("onSightEnter");
			sensor->setExitCallback("onSightExit");
			return;
		case entity::sensor::SensorType_Touch:
			sensor->setEnterCallback("onTouchEnter");
			sensor->setExitCallback("onTouchExit");
			return;
		default:
			TT_PANIC("Unknown Sensor Type: %d!", sensor->getType());
		}
	}
}


void SensorWrapper::setEnterCallback(const std::string& p_callbackName)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setEnterCallback(p_callbackName);
	}
}


const std::string& SensorWrapper::getEnterCallback() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor == 0)
	{
		static const std::string empty;
		return empty;
	}
	return sensor->getEnterCallback();
}


void SensorWrapper::setExitCallback(const std::string& p_callbackName)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setExitCallback(p_callbackName);
	}
}


const std::string& SensorWrapper::getExitCallback() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor == 0)
	{
		static const std::string empty;
		return empty;
	}
	return sensor->getExitCallback();
}



void SensorWrapper::setFilterCallback(const std::string& p_callbackName)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setFilterCallback(p_callbackName);
	}
}


const std::string& SensorWrapper::getFilterCallback() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor == 0)
	{
		static const std::string empty;
		return empty;
	}
	return sensor->getFilterCallback();
}


EntityBase* SensorWrapper::getTarget() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		entity::Entity* target = sensor->getTarget().getPtr();
		if (target != 0)
		{
			return target->getEntityScript().get();
		}
	}
	
	return 0;
}


void SensorWrapper::setTarget(const EntityBase* p_target)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		entity::EntityHandle target = (p_target == 0) ? entity::EntityHandle() : p_target->getHandle();
		sensor->setTarget(target);
	}
}


EntityBaseCollection SensorWrapper::getSensedEntities() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor == 0)
	{
		return EntityBaseCollection();
	}
	
	EntityBaseCollection result;
	const entity::EntityHandles& handles = sensor->getSensedEntities();
	result.reserve(handles.size());
	for (entity::EntityHandles::const_iterator it = handles.begin(); it != handles.end(); ++it)
	{
		entity::Entity* entity = it->getPtr();
		if (entity != 0)
		{
			const script::EntityBasePtr script = entity->getEntityScript();
			if (script != 0)
			{
				result.push_back(script.get());
			}
		}
	}
	return result;
}


void SensorWrapper::removeAllSensedEntities()
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->removeAllSensedEntities(false, false);
	}
}


void SensorWrapper::removeAllSensedEntitiesWithCallbacks()
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->removeAllSensedEntities(true, false);
	}
}


void SensorWrapper::setWorldPosition(const tt::math::Vector2& p_position)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setWorldPosition(p_position);
	}
}


tt::math::Vector2 SensorWrapper::getWorldPosition() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getWorldPosition() : tt::math::Vector2::zero;
}


void SensorWrapper::setOffset(const tt::math::Vector2& p_offset)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setOffset(p_offset);
	}
}


void SensorWrapper::setRayTraceOffset(const tt::math::Vector2& p_offset)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setRayTraceOffset(p_offset);
	}
}


bool SensorWrapper::hasIgnoreActiveCollision() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->hasIgnoreActiveCollision() : false;
}


void SensorWrapper::setIgnoreActiveCollision(bool p_ignoreActiveCollision)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setIgnoreActiveCollision(p_ignoreActiveCollision);
	}
}


bool SensorWrapper::hasIgnoreOwnCollision() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->hasIgnoreOwnCollision() : false;
}


void SensorWrapper::setIgnoreOwnCollision(bool p_ignoreOwnCollision)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setIgnoreOwnCollision(p_ignoreOwnCollision);
	}
}


bool SensorWrapper::isEnabledInDarkness() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->isEnabledInDarkness() : false;
}


void SensorWrapper::setEnabledInDarkness(bool p_enable)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setEnabledInDarkness(p_enable);
	}
}


bool SensorWrapper::hasDistanceSort() const
{
	const entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->hasDistanceSort() : false;
}


void SensorWrapper::setDistanceSort(bool p_distanceSort)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->setDistanceSort(p_distanceSort);
	}
}


void SensorWrapper::setStopOnEmpty(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnEmpty(p_stop);
	}
}


void SensorWrapper::setStopOnSolid(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnSolid(p_stop);
	}
}


void SensorWrapper::setStopOnCrystal(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnCrystal(p_stop);
	}
}


void SensorWrapper::setStopOnWaterPool(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnWaterPool(p_stop);
	}
}


void SensorWrapper::setStopOnWaterFall(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnWaterFall(p_stop);
	}
}


void SensorWrapper::setStopOnLavaPool(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnLavaPool(p_stop);
	}
}


void SensorWrapper::setStopOnLavaFall(bool p_stop)
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	if (sensor != 0)
	{
		sensor->getRayTracer().setStopOnLavaFall(p_stop);
	}
}


bool SensorWrapper::getStopOnEmpty() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnEmpty() : false;
}


bool SensorWrapper::getStopOnSolid() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnSolid() : false;
}


bool SensorWrapper::getStopOnCrystal() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnCrystal() : false;
}


bool SensorWrapper::getStopOnWaterPool() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnWaterPool() : false;
}


bool SensorWrapper::getStopOnWaterFall() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnWaterFall() : false;
}


bool SensorWrapper::getStopOnLavaPool() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnLavaPool() : false;
}


bool SensorWrapper::getStopOnLavaFall() const
{
	entity::sensor::Sensor* sensor = m_sensorHandle.getPtr();
	return (sensor != 0) ? sensor->getRayTracer().getStopOnLavaFall() : false;
}


void SensorWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace event;
	TT_SQBIND_INIT_NAME(SensorWrapper, "Sensor");
	TT_SQBIND_METHOD(SensorWrapper, isEnabled);
	TT_SQBIND_METHOD(SensorWrapper, setEnabled);
	TT_SQBIND_METHOD(SensorWrapper, isSuspended);
	TT_SQBIND_METHOD(SensorWrapper, setSuspended);
	TT_SQBIND_METHOD(SensorWrapper, getDelay);
	TT_SQBIND_METHOD(SensorWrapper, setDelay);
	TT_SQBIND_METHOD(SensorWrapper, setShape);
	TT_SQBIND_METHOD(SensorWrapper, setDefaultEnterAndExitCallback);
	TT_SQBIND_METHOD(SensorWrapper, setEnterCallback);
	TT_SQBIND_METHOD(SensorWrapper, getEnterCallback);
	TT_SQBIND_METHOD(SensorWrapper, setExitCallback);
	TT_SQBIND_METHOD(SensorWrapper, getExitCallback);
	TT_SQBIND_METHOD(SensorWrapper, setFilterCallback);
	TT_SQBIND_METHOD(SensorWrapper, getFilterCallback);
	TT_SQBIND_METHOD(SensorWrapper, getTarget);
	TT_SQBIND_METHOD(SensorWrapper, setTarget);
	TT_SQBIND_METHOD(SensorWrapper, getSensedEntities);
	TT_SQBIND_METHOD(SensorWrapper, removeAllSensedEntities);
	TT_SQBIND_METHOD(SensorWrapper, removeAllSensedEntitiesWithCallbacks);
	TT_SQBIND_METHOD(SensorWrapper, setWorldPosition);
	TT_SQBIND_METHOD(SensorWrapper, getWorldPosition);
	TT_SQBIND_METHOD(SensorWrapper, setOffset);
	TT_SQBIND_METHOD(SensorWrapper, setRayTraceOffset);
	TT_SQBIND_METHOD(SensorWrapper, hasIgnoreActiveCollision);
	TT_SQBIND_METHOD(SensorWrapper, setIgnoreActiveCollision);
	TT_SQBIND_METHOD(SensorWrapper, hasIgnoreOwnCollision);
	TT_SQBIND_METHOD(SensorWrapper, setIgnoreOwnCollision);
	TT_SQBIND_METHOD(SensorWrapper, isEnabledInDarkness);
	TT_SQBIND_METHOD(SensorWrapper, setEnabledInDarkness);
	TT_SQBIND_METHOD(SensorWrapper, hasDistanceSort);
	TT_SQBIND_METHOD(SensorWrapper, setDistanceSort);
	TT_SQBIND_METHOD(SensorWrapper, equals);
	TT_SQBIND_METHOD(SensorWrapper, getHandleValue);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnEmpty);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnSolid);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnCrystal);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnWaterPool);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnWaterFall);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnLavaPool);
	TT_SQBIND_METHOD(SensorWrapper, setStopOnLavaFall);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnEmpty);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnSolid);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnCrystal);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnWaterPool);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnWaterFall);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnLavaPool);
	TT_SQBIND_METHOD(SensorWrapper, getStopOnLavaFall);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
