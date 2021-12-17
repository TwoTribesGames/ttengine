#if !defined(INC_TOKI_PRES_RUMBLETRIGGER_H)
#define INC_TOKI_PRES_RUMBLETRIGGER_H


#include <tt/pres/TriggerBase.h>

#include <toki/input/types.h>


namespace toki {
namespace pres {

class RumbleTrigger : public tt::pres::TriggerBase
{
public:
	explicit RumbleTrigger(const tt::pres::TriggerInfo& p_triggerInfo);
	virtual ~RumbleTrigger() { }
	
	virtual void trigger();
	
	virtual void presentationEnded() { }
	
	virtual RumbleTrigger* clone() const;
	
private:
	RumbleTrigger(const RumbleTrigger& p_rhs);
	
	// Disable assignment
	RumbleTrigger& operator=(const RumbleTrigger&);
	
	
	input::RumbleStrength m_rumbleStrength;
	real                  m_rumbleDuration;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_PRES_RUMBLETRIGGER_H)
