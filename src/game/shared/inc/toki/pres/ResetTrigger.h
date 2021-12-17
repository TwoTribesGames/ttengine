#if !defined(INC_TOKI_PRES_RESETTRIGGER_H)
#define INC_TOKI_PRES_RESETTRIGGER_H


#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>
#include <tt/pres/PresentationValue.h>
#include <tt/pres/TriggerBase.h>


namespace toki {
namespace pres {

class ResetTrigger : public tt::pres::TriggerBase
{
public:
	ResetTrigger(const tt::pres::TriggerInfo&           p_triggerInfo,
	             const tt::pres::PresentationObjectPtr& p_object);
	virtual ~ResetTrigger() { }
	
	virtual void trigger();
	
	virtual ResetTrigger* clone() const;
	
	virtual void presentationEnded() { }
	
	inline virtual void setPresentationObject(const tt::pres::PresentationObjectPtr& p_object)
	{ m_object = p_object; }
	
private:
	ResetTrigger(const ResetTrigger& p_rhs);
	
	// Disable assignment
	ResetTrigger& operator=(const ResetTrigger&);
	
	
	tt::pres::PresentationValue         m_resetToTime;
	tt::pres::PresentationObjectWeakPtr m_object;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_PRES_RESETTRIGGER_H)
