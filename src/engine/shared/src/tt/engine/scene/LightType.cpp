#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>

#include <tt/engine/scene/LightType.h>

namespace tt {
namespace engine {
namespace scene {


const char* getLightTypeName(LightType p_type)
{
	switch (p_type)
	{
	case LightType_Point:
		return "point";
	case LightType_Spot:
		return "spot";
	case LightType_Directional:
		return "directional";
	case LightType_Specular:
		return "specular";
	default:
		TT_PANIC("Unknown LightType: %d.", p_type);
		return "";
	}
}


LightType getLightType(const std::string& p_name)
{
	for (s32 i = 0; i < LightType_Count; ++i)
	{
		LightType type = static_cast<LightType>(i);
		if (p_name == getLightTypeName(type))
		{
			return type;
		}
	}
	
	return LightType_Invalid;
}


// Namespace end
}
}
}
