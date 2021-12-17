#if !defined(INC_TT_PRES_CALLBACKTRIGGER_H)
#define INC_TT_PRES_CALLBACKTRIGGER_H


#include <tt/pres/TriggerBase.h>


namespace tt {
namespace pres {


class CallbackTrigger : public TriggerBase
{
public:
	CallbackTrigger(const tt::pres::TriggerInfo&           p_triggerInfo,
	                const tt::pres::PresentationObjectPtr& p_object);
	virtual ~CallbackTrigger() { }
	
	
	virtual void trigger();
	virtual void presentationEnded(){}
	
	virtual CallbackTrigger* clone() const;
	
	inline virtual void setPresentationObject(const tt::pres::PresentationObjectPtr& p_object)
	{ m_object = p_object; }
	
private:
	
	CallbackTrigger(const CallbackTrigger& p_rhs);
	
	// disable assignment
	CallbackTrigger& operator=(const CallbackTrigger&);
	
	
	tt::pres::PresentationObjectWeakPtr m_object;
	std::string m_data;
	
};

// Namespace end
}
}


#endif  // !defined(INC_TT_PRES_PARTICLESPAWNER_H)
