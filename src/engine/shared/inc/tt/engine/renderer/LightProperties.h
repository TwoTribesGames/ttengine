#if !defined(INC_TT_ENGINE_RENDERER_LIGHTPROPERTIES_H)
#define INC_TT_ENGINE_RENDERER_LIGHTPROPERTIES_H


#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector4.h>


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Light properties needed by FF emulation shader */
struct LightProperties
{
	math::Vector4 color;
	math::Vector4 positionDirection;
	math::Vector4 attenuation;
	
	inline void setColor(const ColorRGBA& p_color) { color = p_color.normalized(); }
	
	inline void setPosition(const math::Vector3& p_position)
	{
		positionDirection.x = p_position.x;
		positionDirection.y = p_position.y;
		positionDirection.z = p_position.z;
		positionDirection.w = 1.0f;
	}
	
	inline void setDirection(const math::Vector3& p_direction)
	{
		// NOTE: Flipping the sign here, because lighting computations use the direction towards
		//       the light source, not from the light source
		math::Vector3 direction = p_direction;
		direction.normalize();
		positionDirection.x = -direction.x;
		positionDirection.y = -direction.y;
		positionDirection.z = -direction.z;
		positionDirection.w = 0.0f;
	}
	
	inline void setAttenuation(const math::Vector3& p_attenuation)
	{
		attenuation.x = p_attenuation.x;
		attenuation.y = p_attenuation.y;
		attenuation.z = p_attenuation.z;
	}
	
	inline void setRange(real p_range) { attenuation.w = p_range; }
};


}
}
}

#endif //INC_TT_ENGINE_RENDERER_LIGHTPROPERTIES_H
