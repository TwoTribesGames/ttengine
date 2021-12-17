#include <tt/audio/player/SoundPlayer.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace player {

//--------------------------------------------------------------------------------------------------
// Public member functions

SoundPlayer::SoundPlayer()
{
}


SoundPlayer::~SoundPlayer()
{
}


bool SoundPlayer::set3DAudioEnabled(bool)
{
	TT_PANIC("SoundPlayer::set3DAudioEnabled is not implemented for this player.");
	return false;
}


bool SoundPlayer::setListenerPosition(const math::Vector3&)
{
	TT_PANIC("SoundPlayer::setListenerPosition is not implemented for this player.");
	return false;
}


bool SoundPlayer::setGlobalVariable(const std::string& /*p_name*/, real /*p_value*/)
{
	return false;
}


bool SoundPlayer::getGlobalVariable(const std::string& /*p_name*/, real* /*p_value_OUT*/) const
{
	return false;
}


bool SoundPlayer::setReverbVolumeForCategory(const std::string& /*p_category*/, real /*p_normalizedVolume*/)
{
	return false;
}

// Namespace end
}
}
}
