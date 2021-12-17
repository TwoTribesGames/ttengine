#if !defined(INC_TT_ENGINE_SCENE_LIGHT_H)
#define INC_TT_ENGINE_SCENE_LIGHT_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/LightProperties.h>
#include <tt/engine/scene/fwd.h>
#include <tt/engine/scene/LightType.h>


namespace tt {
namespace engine {
namespace scene {

class Light
{
public:
	Light();
	~Light();
	
	void update();
	void debugRender() const;
	
	inline void setColor(const renderer::ColorRGBA& p_color) { m_color = p_color; }
	inline const renderer::ColorRGBA& getColor() const { return m_color; }

	inline void setDirection(const math::Vector3& p_dir) { m_direction = p_dir; }
	inline const tt::math::Vector3& getDirection() const { return m_direction;  }
	
	inline void setPosition(const math::Vector3& p_position) { m_position = p_position; }
	inline const math::Vector3& getPosition() const          { return m_position;       }
	
	void setDistanceAttenuation(const math::Vector3& p_attenuation);
	void setDistanceAttenuation(real p_constant = 1.0f, real p_linear = 0.0f, real p_quadratic = 0.0f);
	
	void setType(LightType p_type);
	inline LightType getType() const { return m_type; }
	
	bool isEnabled() const { return m_enabled; }
	void setEnabled(bool p_enabled);
	
	void setIndex(s32 p_lightIndex);
	
private:
	renderer::ColorRGBA m_color;
	math::Vector3       m_direction;
	math::Vector3       m_position;
	math::Vector3       m_distanceAttenuation;
	LightType           m_type;
	bool                m_enabled;
	s32                 m_index;
	
	// Hardware
	renderer::LightProperties m_properties;
};

// Namespace end
}
}
}

#endif // INC_TT_ENGINE_SCENE_LIGHT_H
