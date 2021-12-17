#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>
#include <tt/pres/PresentationObject.h>
#include <tt/str/parse.h>

#include <toki/pres/ResetTrigger.h>


namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

ResetTrigger::ResetTrigger(const tt::pres::TriggerInfo&           p_triggerInfo,
                           const tt::pres::PresentationObjectPtr& p_object)
:
tt::pres::TriggerBase(p_triggerInfo),
m_resetToTime(0.0f),
m_object(p_object)
{
	TT_ASSERT(p_triggerInfo.type == "reset");
	
	TT_ERR_CREATE("Parsing Real from Reset trigger data");
	m_resetToTime.resetValue(p_triggerInfo.data, &errStatus);
	
	if (errStatus.hasError())
	{
		m_resetToTime.setValue(0.0f);
	}
}


void ResetTrigger::trigger()
{
	tt::pres::PresentationObjectPtr object(m_object.lock());
	if (object == 0) return;
	
	object->scheduleReset(m_resetToTime);
}


ResetTrigger* ResetTrigger::clone() const
{
	return new ResetTrigger(*this);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ResetTrigger::ResetTrigger(const ResetTrigger& p_rhs)
:
tt::pres::TriggerBase(p_rhs),
m_resetToTime(p_rhs.m_resetToTime),
m_object() // NOTE: m_object is not copied. The copy needs the reference of the newly copied presentationObject
{
}

// Namespace end
}
}
