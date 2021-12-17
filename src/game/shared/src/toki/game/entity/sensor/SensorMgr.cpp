#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>

#include <toki/game/entity/sensor/SensorMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/serialization/SerializationMgr.h>
#include <tt/thread/ThreadedWorkload.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

#define USE_THREADING 1

//--------------------------------------------------------------------------------------------------
// Public member functions

SensorMgr::SensorMgr(s32 p_reserveCount)
:
m_sensors(p_reserveCount),
m_updateIndex(0)
{
}


SensorHandle SensorMgr::createSensor(SensorType p_type, const EntityHandle& p_source,
                                     const ShapePtr& p_shape, const EntityHandle& p_target)
{
	return m_sensors.create(Sensor::CreationParams(p_type, p_source, p_shape, p_target));
}


void SensorMgr::destroySensor(SensorHandle& p_handle)
{
	m_sensors.destroy(p_handle);
	
	p_handle.invalidate();
}


void SensorMgr::update(real64 p_gameTime)
{
#if USE_THREADING
	tt::thread::ThreadedWorkload work(m_sensors.getActiveCount(),
		std::bind(&SensorMgr::updateSensor, this, std::placeholders::_1));
	work.startAndWaitForCompletion();
#else
	{
		Sensor* sensor = m_sensors.getFirst();
		// Note JL: this loop construction assumes no sensors are added or destroyed during sensor->update
		for (s32 i = 0; i < m_sensors.getActiveCount(); ++i, ++sensor)
		{
			sensor->update();
		}
	}
#endif
	
	// Fire all the callbacks (cannot be threaded easily)
	Sensor* sensor = m_sensors.getFirst();
	// Note JL: this loop construction assumes no sensors are added or destroyed during sensor->update
	for (s32 i = 0; i < m_sensors.getActiveCount(); ++i, ++sensor)
	{
		sensor->updateCallbacks(p_gameTime);
	}
}


void SensorMgr::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	const Sensor* sensor = m_sensors.getFirst();
	for (s32 i = 0; i < m_sensors.getActiveCount(); ++i, ++sensor)
	{
		sensor->renderDebug();
	}
#endif
}


void SensorMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_SensorMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the SensorMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::serializeHandleArrayMgr(m_sensors, &context);
	
	bu::put(m_updateIndex, &context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void SensorMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_SensorMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the SensorMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::unserializeHandleArrayMgr(&m_sensors, &context);
	
	m_updateIndex = bu::get<s32>(&context);
	
	m_sensors.doInternalCleanup();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SensorMgr::updateSensor(size_t p_index)
{
	Sensor* sensors = m_sensors.getFirst();
	sensors[p_index].update();
}

// Namespace end
}
}
}
}
