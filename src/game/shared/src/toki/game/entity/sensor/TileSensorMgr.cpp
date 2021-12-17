#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>

#include <toki/game/entity/sensor/TileSensorMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

//--------------------------------------------------------------------------------------------------
// Public member functions

TileSensorMgr::TileSensorMgr(s32 p_reserveCount)
:
m_sensors(p_reserveCount)
{
}


TileSensorHandle TileSensorMgr::createSensor(const EntityHandle& p_source, const ShapePtr& p_shape)
{
	return m_sensors.create(TileSensor::CreationParams(p_source, p_shape));
}


void TileSensorMgr::destroySensor(TileSensorHandle& p_handle)
{
	m_sensors.destroy(p_handle);
	
	p_handle.invalidate();
}


void TileSensorMgr::update()
{
	TileSensor* sensor = m_sensors.getFirst();
	for (s32 i = 0; i <  m_sensors.getActiveCount(); ++i, ++sensor)
	{
		sensor->update();
	}
}


void TileSensorMgr::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	const TileSensor* sensor = m_sensors.getFirst();
	for (s32 i = 0; i < m_sensors.getActiveCount(); ++i, ++sensor)
	{
		sensor->renderDebug();
	}
#endif
}


void TileSensorMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_TileSensorMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the TileSensorMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	tt::code::serializeHandleArrayMgr(m_sensors, &context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void TileSensorMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_TileSensorMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the TileSensorMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	tt::code::unserializeHandleArrayMgr(&m_sensors, &context);
}

// Namespace end
}
}
}
}
