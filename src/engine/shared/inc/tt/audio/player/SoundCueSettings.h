#if !defined(INC_TT_AUDIO_PLAYER_SOUNDCUESETTINGS_H)
#define INC_TT_AUDIO_PLAYER_SOUNDCUESETTINGS_H

#include <map>
#include <string>

#include <tt/audio/player/fwd.h>
#include <tt/audio/player/SoundCue.h>
#include <tt/code/BitMask.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace player {


class SoundCueSettings
{
public:
	enum UpdateFlag
	{
		UpdateFlag_ReverbVolume,
		//UpdateFlag_Variables, // Not required, as variables is a container
		UpdateFlag_Position,
		UpdateFlag_Radius,
		
		UpdateFlag_Count
	};
	
	
	inline void setReverbVolume(real p_normalizedVolume)
	{
		m_updateBitmask.setFlag(UpdateFlag_ReverbVolume);
		m_reverbVolume = p_normalizedVolume;
	}
	
	inline void setPosition(const math::Vector3& p_position)
	{
		m_updateBitmask.setFlag(UpdateFlag_Position);
		m_position = p_position;
	}
	
	inline void setRadius(real p_inner, real p_outer)
	{
		m_updateBitmask.setFlag(UpdateFlag_Radius);
		m_innerRadius = p_inner;
		m_outerRadius = p_outer;
	}
	
	inline void setVariable(const std::string& p_name, real p_value)
	{
		m_variables[p_name] = p_value;
	}
	
	inline bool getRadius(real* p_inner_OUT, real* p_outer_OUT) const
	{
		if (m_updateBitmask.checkFlag(UpdateFlag_Radius) == false)
		{
			return false;
		}
		
		TT_NULL_ASSERT(p_inner_OUT);
		TT_NULL_ASSERT(p_outer_OUT);
		
		if (p_inner_OUT != 0 && p_outer_OUT != 0)
		{
			*p_inner_OUT = m_innerRadius;
			*p_outer_OUT = m_outerRadius;
		}
		return true;
	}
	
	SoundCueSettings()
	:
	m_reverbVolume(0.0f),
	m_variables(),
	m_position(),
	m_innerRadius(0.0f),
	m_outerRadius(0.0f),
	m_updateBitmask(0)
	{ }
	
	void applyToSoundCue(SoundCue& p_soundCue);
	
private:
	typedef tt::code::BitMask<UpdateFlag, UpdateFlag_Count> UpdateBitMask;
	typedef std::map<std::string, real> Variables;
	
	
	real              m_reverbVolume;
	Variables         m_variables;
	tt::math::Vector3 m_position;
	real              m_innerRadius;
	real              m_outerRadius;
	
	UpdateBitMask     m_updateBitmask;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_SOUNDCUESETTINGS_H)
