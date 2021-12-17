#if !defined(INC_TOKI_GAME_LIGHT_JITTEREFFECT_H)
#define INC_TOKI_GAME_LIGHT_JITTEREFFECT_H

#include <tt/code/fwd.h>
#include <tt/platform/tt_types.h>

#include <toki/game/light/fwd.h>

namespace toki {
namespace game {
namespace light {

class JitterEffect
{
public:
	JitterEffect(real p_scaleFrequency,     real p_scaleAmplitude,
	             real p_positionXFrequency, real p_positionXAmplitude,
	             real p_positionYFrequency, real p_positionYAmplitude);
	
	void update(real p_deltaTime);
	
	inline real                     getScaleJitter()    const { return m_scaleJitterValue;    }
	inline const tt::math::Vector2& getPositionJitter() const { return m_positionJitterValue; }
	
	void                   serialize  (tt::code::BufferWriteContext* p_context) const;
	static JitterEffectPtr unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	struct Settings
	{
		Settings(real p_frequency, real p_amplitude);
		
		real frequency;
		real amplitude;
		real phase;
	};
	
	JitterEffect();
	
	real              m_scaleJitterValue;
	tt::math::Vector2 m_positionJitterValue;
	
	Settings          m_scaleSettings;
	Settings          m_positionXSettings;
	Settings          m_positionYSettings;
	
	real              m_time;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_JITTEREFFECT_H)
