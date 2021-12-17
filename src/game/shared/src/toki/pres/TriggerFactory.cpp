#include <toki/pres/AudioTrigger.h>
#include <toki/pres/OverallVolumeTrigger.h>
#include <toki/pres/ResetTrigger.h>
#include <toki/pres/RumbleTrigger.h>
#include <toki/pres/TriggerFactory.h>


namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

tt::pres::TriggerInterfacePtr TriggerFactory::createTrigger(
		const tt::pres::TriggerInfo&           p_creationInfo,
		const tt::pres::PresentationObjectPtr& p_object)
{
	if (p_creationInfo.type == "audio" ||
	    p_creationInfo.type == "global_audio")
	{
		return tt::pres::TriggerInterfacePtr(new AudioTrigger(p_creationInfo, p_object));
	}
	else if (p_creationInfo.type == "overall_volume"    ||
	         p_creationInfo.type == "overall_volume_tv" ||
	         p_creationInfo.type == "overall_volume_drc")
	{
		return tt::pres::TriggerInterfacePtr(new OverallVolumeTrigger(p_creationInfo));
	}
	else if (p_creationInfo.type == "reset")
	{
		return tt::pres::TriggerInterfacePtr(new ResetTrigger(p_creationInfo, p_object));
	}
	else if (p_creationInfo.type == "rumble")
	{
		return tt::pres::TriggerInterfacePtr(new RumbleTrigger(p_creationInfo));
	}
	else
	{
		TT_PANIC("Unsupported presentation trigger type: '%s'.\n"
		         "Supported:\n"
		         "- audio\n"
		         "- global_audio\n"
		         "- overall_volume\n"
		         "- overall_volume_tv\n"
		         "- overall_volume_drc\n"
		         "- reset\n"
		         "- rumble",
		         p_creationInfo.type.c_str());
		return tt::pres::TriggerInterfacePtr();
	}
}

// Namespace end
}
}
