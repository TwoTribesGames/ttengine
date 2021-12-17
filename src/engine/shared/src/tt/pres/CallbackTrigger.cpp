
#include <tt/pres/CallbackTrigger.h>
#include <tt/pres/CallbackTriggerInterface.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/TriggerInfo.h>


namespace tt {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

CallbackTrigger::CallbackTrigger(const tt::pres::TriggerInfo&           p_triggerInfo,
                                 const tt::pres::PresentationObjectPtr& p_object)
:
tt::pres::TriggerBase(p_triggerInfo),
m_object(p_object),
m_data(p_triggerInfo.data)
{
	TT_ASSERT(p_triggerInfo.type == "callback");
}


void CallbackTrigger::trigger()
{ 
	tt::pres::PresentationObjectPtr object(m_object.lock());
	if (object == 0) 
	{
		return;
	}
	
	CallbackTriggerInterfacePtr callbackPtr(object->getCallbackInterface());
	if (callbackPtr == 0)
	{
		TT_NULL_ASSERT(callbackPtr);
		return;
	}
	
	callbackPtr->callback(m_data, object);
}


CallbackTrigger* CallbackTrigger::clone() const
{
	return new CallbackTrigger(*this);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CallbackTrigger::CallbackTrigger(const CallbackTrigger& p_rhs)
:
tt::pres::TriggerBase(p_rhs),
m_object(), // NOTE: m_object is not copied. The copy needs the reference of the newly copied presentationObject
m_data(p_rhs.m_data)
{
}

//namespace end
}
}
