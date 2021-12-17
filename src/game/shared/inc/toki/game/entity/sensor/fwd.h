#if !defined(INC_TOKI_GAME_ENTITY_SENSOR_FWD_H)
#define INC_TOKI_GAME_ENTITY_SENSOR_FWD_H

#include <tt/code/Handle.h>

namespace toki {
namespace game {
namespace entity {
namespace sensor {

enum SensorType
{
	SensorType_Sight,
	SensorType_Touch
};


inline const char* const getSensorTypeName(SensorType p_type)
{
	switch (p_type)
	{
	case SensorType_Sight: return "sight";
	case SensorType_Touch: return "touch";
	default:
		TT_PANIC("Unknown SensorType: %d", p_type);
		return "";
	}
}


class RayTracer;

class Sensor;
typedef tt::code::Handle<Sensor> SensorHandle;

class SensorMgr;
class Shape;
typedef tt_ptr<Shape>::shared ShapePtr;

class TileSensor;
typedef tt::code::Handle<TileSensor> TileSensorHandle;

class TileSensorMgr;


// Namespace end
}
}
}
}

#endif  // !defined(INC_TOKI_GAME_ENTITY_SENSOR_FWD_H)
