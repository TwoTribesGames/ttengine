#if !defined(INC_TT_ENGINE_SCENE_LIGHTTYPE_H)
#define INC_TT_ENGINE_SCENE_LIGHTTYPE_H

#include <string>

namespace tt {
namespace engine {
namespace scene {


enum LightType
{
	// Diffuse light types
	LightType_Point,
	LightType_Spot,
	LightType_Directional,
	
	// Specular type
	LightType_Specular, // Is directional
	
	LightType_Count,
	LightType_Invalid
};


inline bool isValidLightType(LightType p_type) { return p_type >= 0 && p_type < LightType_Count; }
const char* getLightTypeName(LightType p_type);
LightType getLightType(const std::string& p_name);


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_SCENE_LIGHT_H
