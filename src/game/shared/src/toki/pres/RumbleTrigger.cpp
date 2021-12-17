#include <tt/code/ErrorStatus.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/pres/RumbleTrigger.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

RumbleTrigger::RumbleTrigger(const tt::pres::TriggerInfo& p_triggerInfo)
:
TriggerBase(p_triggerInfo),
m_rumbleStrength(input::RumbleStrength_Low),
m_rumbleDuration(1.0f)
{
	TT_ASSERT(p_triggerInfo.type == "rumble");
	
	// NOTE: In case of a parsing error, keep the defaults that were set by the constructor
	
	std::string strengthStr;
	std::string durationStr;
	if (tt::str::split(p_triggerInfo.data, ',', strengthStr, durationStr) == false)
	{
		TT_PANIC("Malformed rumble trigger data: '%s'\nExpected format 'strength_name, durationInSeconds'. "
		         "Data does not contain a comma.", p_triggerInfo.data.c_str());
	}
	else
	{
		strengthStr = tt::str::trim(strengthStr);
		durationStr = tt::str::trim(durationStr);
		
		// - Parse strength
		const input::RumbleStrength strength = input::getRumbleStrengthFromName(strengthStr);
		TT_ASSERTMSG(input::isValidRumbleStrength(strength),
		             "Invalid rumble strength specified: '%s'", strengthStr.c_str());
		if (input::isValidRumbleStrength(strength))
		{
			m_rumbleStrength = strength;
		}
		
		// - Parse duration
		TT_ERR_CREATE("");
		const real duration = tt::str::parseReal(durationStr, &errStatus);
		TT_ASSERTMSG(errStatus.hasError() == false,
		             "Invalid rumble duration specified: '%s'. Must be a floating point number in range 0 - 1.",
		             durationStr.c_str());
		if (errStatus.hasError() == false)
		{
			m_rumbleDuration = duration;
			const bool wasClamped = tt::math::clamp(m_rumbleDuration, 0.0f, 1.0f);
			TT_ASSERTMSG(wasClamped == false,
			             "Invalid rumble duration specified: '%s'. Must be a floating point number in range 0 - 1.",
			             durationStr.c_str());
		}
	}
}


void RumbleTrigger::trigger()
{
	AppGlobal::getController(tt::input::ControllerIndex_One).rumble(m_rumbleStrength, m_rumbleDuration, 0.0f);
}


RumbleTrigger* RumbleTrigger::clone() const
{
	return new RumbleTrigger(*this);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

RumbleTrigger::RumbleTrigger(const RumbleTrigger& p_rhs)
:
tt::pres::TriggerBase(p_rhs),
m_rumbleStrength(p_rhs.m_rumbleStrength),
m_rumbleDuration(p_rhs.m_rumbleDuration)
{
}

// Namespace end
}
}
