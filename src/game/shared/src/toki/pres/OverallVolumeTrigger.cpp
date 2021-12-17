#include <tt/pres/PresentationObject.h>
#include <tt/str/str.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/pres/OverallVolumeTrigger.h>


namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

OverallVolumeTrigger::OverallVolumeTrigger(const tt::pres::TriggerInfo& p_triggerInfo)
:
TriggerBase(p_triggerInfo),
m_device((p_triggerInfo.type == "overall_volume_tv") ? audio::Device_TV : audio::Device_DRC),
m_affectAllDevices(p_triggerInfo.type == "overall_volume"),
m_normalizedBeginVolume(-1.0f),
m_normalizedEndVolume(1.0f),
m_fadeDuration(1.0f)
{
	TT_ASSERT(p_triggerInfo.type == "overall_volume"    ||
	          p_triggerInfo.type == "overall_volume_tv" ||
	          p_triggerInfo.type == "overall_volume_drc");
	
	// NOTE: In case of a parsing error, keep the defaults that were set by the constructor
	
	std::string beginVolumeStr;
	std::string endVolumeStr;
	if (tt::str::split(p_triggerInfo.data, ',', beginVolumeStr, endVolumeStr) == false)
	{
		// Trigger specifies only an end volume: use current volume as begin
		m_normalizedBeginVolume = -1.0f;
		parseVolume(p_triggerInfo.data, &m_normalizedEndVolume);
	}
	else
	{
		// Trigger specifies both a begin and end volume
		parseVolume(beginVolumeStr, &m_normalizedBeginVolume);
		parseVolume(endVolumeStr,   &m_normalizedEndVolume);
	}
}


void OverallVolumeTrigger::trigger()
{
	if (audio::AudioPlayer::hasInstance())
	{
		audio::AudioPlayer* player = audio::AudioPlayer::getInstance();
		if (m_affectAllDevices)
		{
			for (s32 i = 0; i < audio::Device_Count; ++i)
			{
				player->setOverallVolume(static_cast<audio::Device>(i), m_normalizedEndVolume, m_fadeDuration, m_normalizedBeginVolume);
			}
		}
		else
		{
			player->setOverallVolume(m_device, m_normalizedEndVolume, m_fadeDuration, m_normalizedBeginVolume);
		}
	}
}


OverallVolumeTrigger* OverallVolumeTrigger::clone() const
{
	return new OverallVolumeTrigger(*this);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

OverallVolumeTrigger::OverallVolumeTrigger(const OverallVolumeTrigger& p_rhs)
:
tt::pres::TriggerBase(p_rhs),
m_device               (p_rhs.m_device),
m_affectAllDevices     (p_rhs.m_affectAllDevices),
m_normalizedBeginVolume(p_rhs.m_normalizedBeginVolume),
m_normalizedEndVolume  (p_rhs.m_normalizedEndVolume),
m_fadeDuration         (p_rhs.m_fadeDuration)
{
}


void OverallVolumeTrigger::parseVolume(std::string p_volumePercentageStr,
                                       real*       p_normalizedVolume_OUT)
{
	TT_NULL_ASSERT(p_normalizedVolume_OUT);
	
	p_volumePercentageStr = tt::str::trim(p_volumePercentageStr);
	
	TT_ERR_CREATE("");
	
	real volume = tt::str::parseReal(p_volumePercentageStr, &errStatus);
	TT_ASSERTMSG(errStatus.hasError() == false,
	             "Invalid overall volume percentage specified: '%s'. "
	             "Must be a floating point number in range 0 - 100.",
	             p_volumePercentageStr.c_str());
	if (errStatus.hasError() == false)
	{
		const bool wasClamped = tt::math::clamp(volume, 0.0f, 100.0f);
		TT_ASSERTMSG(wasClamped == false,
		             "Invalid overall volume percentage specified: '%s'. "
		             "Must be a floating point number in range 0 - 100. Volume was clamped to %.2f",
		             p_volumePercentageStr.c_str(), volume);
		volume /= 100.0f;  // translate from percentage to normalized volume
		*p_normalizedVolume_OUT = volume;
	}
}


void OverallVolumeTrigger::setRanges(tt::pres::PresentationObject* p_presObj)
{
	TriggerBase::setRanges(p_presObj);
	m_fadeDuration = getDuration().get();
}

// Namespace end
}
}
