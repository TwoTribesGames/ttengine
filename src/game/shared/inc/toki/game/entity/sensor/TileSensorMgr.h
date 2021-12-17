#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_TILESENSORMGR_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_TILESENSORMGR_H


#include <tt/code/HandleArrayMgr.h>

#include <toki/game/entity/sensor/fwd.h>
#include <toki/game/entity/sensor/TileSensor.h>
#include <toki/game/entity/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace sensor {

class TileSensorMgr
{
public:
	explicit TileSensorMgr(s32 p_reserveCount);
	
	TileSensorHandle createSensor(const EntityHandle& p_source,
	                              const ShapePtr&     p_shape);
	
	void destroySensor(TileSensorHandle& p_handle);
	
	inline TileSensor* getSensor(const TileSensorHandle& p_handle) { return m_sensors.get(p_handle); }
	
	void update();
	void renderDebug() const;
	
	inline void reset() { m_sensors.reset(); }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<TileSensor> Sensors;
	
	Sensors m_sensors;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_TILESENSORMGR_H)
