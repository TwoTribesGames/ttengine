#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_SENSORMGR_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_SENSORMGR_H


#include <tt/code/HandleArrayMgr.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/sensor/Sensor.h>
#include <toki/game/entity/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class SensorMgr
{
public:
	explicit SensorMgr(s32 p_reserveCount);
	
	SensorHandle createSensor(SensorType          p_type,
	                          const EntityHandle& p_source,
	                          const ShapePtr&     p_shape,
	                          const EntityHandle& p_target);
	
	void destroySensor(SensorHandle& p_handle);
	
	inline Sensor* getSensor(const SensorHandle& p_handle) { return m_sensors.get(p_handle); }
	
	void update(real64 p_gameTime);
	void renderDebug() const;
	
	inline void reset() { m_sensors.reset(); }
	
	inline s32 getActiveSensorCount() const { return m_sensors.getActiveCount(); }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	void updateSensor(size_t p_index);

	typedef tt::code::HandleArrayMgr<Sensor> Sensors;
	
	Sensors m_sensors;
	s32 m_updateIndex;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_SENSORMGR_H)
