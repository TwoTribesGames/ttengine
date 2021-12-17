#if !defined(INC_TOKI_PRES_AUDIOTRIGGER_H)
#define INC_TOKI_PRES_AUDIOTRIGGER_H


#include <string>

#include <tt/audio/player/SoundCue.h>
#include <tt/pres/TriggerBase.h>


namespace toki {
namespace pres {

class AudioTrigger : public tt::pres::TriggerBase
{
public:
	AudioTrigger(const tt::pres::TriggerInfo&           p_triggerInfo,
	             const tt::pres::PresentationObjectPtr& p_object);
	virtual ~AudioTrigger() { }
	
	virtual void trigger();
	
	virtual AudioTrigger* clone() const;
	
	virtual void presentationEnded();
	
	inline virtual void setPresentationObject(const tt::pres::PresentationObjectPtr& p_object)
	{ m_presentationObject = p_object; }
	
private:
	AudioTrigger(const AudioTrigger& p_rhs);
	
	// Disable assignment
	AudioTrigger& operator=(const AudioTrigger&);
	
	
	const std::string                   m_soundName;
	const bool                          m_positional;
	tt::audio::player::SoundCuePtr      m_soundCue;
	tt::pres::PresentationObjectWeakPtr m_presentationObject;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_PRES_AUDIOTRIGGER_H)
