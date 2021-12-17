#if !defined(INC_TOKI_PRES_OVERALLVOLUMETRIGGER_H)
#define INC_TOKI_PRES_OVERALLVOLUMETRIGGER_H


#include <tt/pres/TriggerBase.h>

#include <toki/audio/constants.h>


namespace toki {
namespace pres {

class OverallVolumeTrigger : public tt::pres::TriggerBase
{
public:
	explicit OverallVolumeTrigger(const tt::pres::TriggerInfo& p_triggerInfo);
	virtual ~OverallVolumeTrigger() { }
	
	virtual void trigger();
	
	virtual void presentationEnded() { }
	
	virtual OverallVolumeTrigger* clone() const;
	
private:
	OverallVolumeTrigger(const OverallVolumeTrigger& p_rhs);
	
	void parseVolume(std::string p_volumePercentageStr, real* p_normalizedVolume_OUT);
	
	virtual void setRanges(tt::pres::PresentationObject* p_presObj);
	
	// Disable assignment
	OverallVolumeTrigger& operator=(const OverallVolumeTrigger&);
	
	
	const audio::Device m_device;
	const bool          m_affectAllDevices;
	real                m_normalizedBeginVolume;  // if negative number, fade starts from current volume
	real                m_normalizedEndVolume;
	real                m_fadeDuration;  // NOTE: The duration is retrieved from TriggerBase
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_PRES_OVERALLVOLUMETRIGGER_H)
