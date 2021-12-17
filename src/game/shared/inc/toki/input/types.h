#if !defined(INC_TOKI_INPUT_TYPES_H)
#define INC_TOKI_INPUT_TYPES_H


#include <string>

#include <tt/platform/tt_error.h>


namespace toki  /*! */ {
namespace input /*! */ {

/*! \brief Different control schemes */
enum GamepadControlScheme
{
	GamepadControlScheme_A1, //!< TwinStick 1
	GamepadControlScheme_A2, //!< TwinStick 2
	GamepadControlScheme_B1, //!< 'Normal' 1
	GamepadControlScheme_B2, //!< 'Normal' 2
	
	GamepadControlScheme_Count,
	GamepadControlScheme_Invalid
};


/*! \brief Preset strengths of controller vibration/rumble. */
enum RumbleStrength
{
	RumbleStrength_Low,        //!< Not a lot of vibration.
	RumbleStrength_Medium,     //!< Average vibration.
	RumbleStrength_High,       //!< Lots of vibration.
	
	RumbleStrength_Count,
	RumbleStrength_Invalid
};


inline bool isValidRumbleStrength(RumbleStrength p_strength)
{ return p_strength >= 0 && p_strength < RumbleStrength_Count; }


inline const char* getRumbleStrengthName(RumbleStrength p_strength)
{
	switch (p_strength)
	{
	case RumbleStrength_Low:        return "low";
	case RumbleStrength_Medium:     return "medium";
	case RumbleStrength_High:       return "high";
	
	default:
		TT_PANIC("Invalid rumble strength: %d", p_strength);
		return "";
	}
}


inline RumbleStrength getRumbleStrengthFromName(const std::string& p_name)
{
	for (s32 i = 0; i < RumbleStrength_Count; ++i)
	{
		const RumbleStrength strength = static_cast<RumbleStrength>(i);
		if (getRumbleStrengthName(strength) == p_name)
		{
			return strength;
		}
	}
	
	return RumbleStrength_Invalid;
}

// Namespace end
}
}


#endif  // !defined(INC_TOKI_INPUT_TYPES_H)
