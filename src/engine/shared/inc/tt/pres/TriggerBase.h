#if !defined(INC_TT_PRES_TRIGGERBASE_H)
#define INC_TT_PRES_TRIGGERBASE_H


#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>
#include <tt/pres/anim2d/Animation2D.h>
#include <tt/pres/PresentationValue.h>
#include <tt/pres/TriggerFactoryInterface.h>
#include <tt/pres/TriggerInfo.h>
#include <tt/pres/TriggerInterface.h>


namespace tt {
namespace pres {

class TriggerBase : public TriggerInterface
{
public:
	TriggerBase(const TriggerInfo& p_triggerInfo);
	virtual ~TriggerBase() { }
	
	virtual void update(real p_delta);
	
	inline virtual bool            isActive()  const { return m_active; }
	inline virtual bool            hasNameOrTagMatch()  const { return m_tagged; }
	inline virtual const DataTags& getTags()   const { return m_tags;   }
	inline virtual bool            isLooping() const { return m_looping;}
	
	virtual void start(const Tags& p_tags, PresentationObject* p_presObj, 
	                   const std::string& p_name, bool p_restart = false);
	virtual void stop();
	virtual void pause();
	virtual void resume();
	virtual void reset();
	
	virtual void trigger() = 0;
	inline virtual void reTrigger() { presentationEnded(); trigger(); }
	
	virtual void setPresentationObject(const PresentationObjectPtr& p_object)
	{ (void)p_object; }
	
	static void setTriggersDisabled(bool p_disabled) { ms_triggersDisabled = p_disabled; }
	
protected:
	TriggerBase(const TriggerBase& p_rhs);
	
	virtual void setRanges(PresentationObject* p_presObj);
	
	inline const PresentationValue& getDuration() const { return m_duration; }
	inline const PresentationValue& getDelay()    const { return m_delay;    }
	
private:
	// Disable assignment
	TriggerBase& operator=(const TriggerBase&);
	
	
	bool m_active;
	bool m_paused;
	bool m_triggered;
	bool m_looping;
	
	PresentationValue m_duration;
	PresentationValue m_delay;
	real              m_time;
	real              m_startTime;
	
	DataTags m_tags;
	bool     m_tagged;
	
	std::string m_syncId;
	bool m_endSync;
	
	static bool ms_triggersDisabled;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_PRES_TRIGGERBASE_H)
