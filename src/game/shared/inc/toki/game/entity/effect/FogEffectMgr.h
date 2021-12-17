#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_FOGEFFECTMGR_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_FOGEFFECTMGR_H

#include <tt/engine/renderer/ColorRGB.h>
#include <tt/math/interpolation.h>


#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

class FogEffectMgr
{
public:
	FogEffectMgr();
	
	void update(real p_deltaTime);
	
	inline bool hasFog() const { return m_defaultNearFarSet && m_defaultColorSet; }
	inline const tt::engine::renderer::ColorRGB& getDefaultColorEnd() { return m_colorEnd; }
	inline real                                  getDefaultNearEnd()  { return m_nearEnd;  }
	inline real                                  getDefaultFarEnd()   { return m_farEnd;   }
	void setDefaultColor(const tt::engine::renderer::ColorRGB& p_color, real p_duration,
	                     tt::math::interpolation::EasingType p_easingType);
	void setDefaultNearFar(real p_near, real p_far, real p_duration,
	                       tt::math::interpolation::EasingType p_easingType);
	
	void resetSettings();
	void applySettingsIfNotSet(const tt::engine::renderer::ColorRGB& p_color, real p_near, real p_far);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	const tt::engine::renderer::ColorRGB& getCurrentDefaultColor() const { return m_defaultColor; }
	real                                  getCurrentDefaultNear()  const { return m_defaultNear;  }
	real                                  getCurrentDefaultFar()   const { return m_defaultFar;   }
	
private:
	// Default Fog settings interpolation.
	tt::math::interpolation::EasingType m_colorEasingType;
	real                                m_colorTime;
	real                                m_colorDuration; // Set 0.0 if no interplation is being done.
	tt::engine::renderer::ColorRGB      m_colorBegin;
	tt::engine::renderer::ColorRGB      m_colorEnd;
	tt::engine::renderer::ColorRGB      m_defaultColor;
	// Used to see if script set fog color during init or spawn
	bool                                m_defaultColorSet;
	
	tt::math::interpolation::EasingType m_nearFarEasingType;
	real                                m_nearFarTime;
	real                                m_nearFarDuration; // Set 0.0 if no interplation is being done.
	real                                m_nearBegin;
	real                                m_nearEnd;
	real                                m_farBegin;
	real                                m_farEnd;
	real                                m_defaultNear;
	real                                m_defaultFar;
	// Used to see if script set fog near and far during init or spawn
	bool                                m_defaultNearFarSet;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_FOGEFFECTMGR_H)
