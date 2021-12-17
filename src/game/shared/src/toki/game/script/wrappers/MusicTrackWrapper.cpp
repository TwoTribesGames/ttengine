#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>

#include <toki/audio/MusicTrack.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/MusicTrackWrapper.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

void MusicTrackWrapper::play()
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		track->play();
	}
}


void MusicTrackWrapper::stop()
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		track->stop();
	}
}


void MusicTrackWrapper::pause()
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		track->pause();
	}
}


void MusicTrackWrapper::unpause()
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		track->resume();
	}
}


bool MusicTrackWrapper::isPlaying() const
{
	audio::MusicTrack* track = m_track.getPtr();
	return (track != 0) ? track->isPlaying() : false;
}


real MusicTrackWrapper::getVolume() const
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		return track->getVolume() * 100.0f;
	}
	return 0.0f;
}


void MusicTrackWrapper::setVolume(real p_volumePercentage, real p_fadeDuration, bool p_stopTrackAfterFade)
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		real clampedVolume = p_volumePercentage;
		if (tt::math::clamp(clampedVolume, 0.0f, 100.0f))
		{
			TT_NONFATAL_PANIC("Invalid track volume specified: %f . Must be a percentage (range 0 - 100). "
			                  "Volume has been clamped to %f.", p_volumePercentage, clampedVolume);
		}
		track->setVolume(clampedVolume / 100.0f, p_fadeDuration, p_stopTrackAfterFade);
	}
}


void MusicTrackWrapper::setCallbackEntity(const EntityWrapper* p_entity)
{
	audio::MusicTrack* track = m_track.getPtr();
	if (track != 0)
	{
		track->setCallbackEntity((p_entity == 0) ? entity::EntityHandle() : p_entity->getHandle());
	}
}


void MusicTrackWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	tt::code::bufferutils::putHandle(m_track, p_context);
}


void MusicTrackWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	m_track = tt::code::bufferutils::getHandle<audio::MusicTrack>(p_context);
}


void MusicTrackWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(MusicTrackWrapper, "MusicTrack");
	TT_SQBIND_METHOD(MusicTrackWrapper, play);
	TT_SQBIND_METHOD(MusicTrackWrapper, stop);
	TT_SQBIND_METHOD(MusicTrackWrapper, pause);
	TT_SQBIND_METHOD(MusicTrackWrapper, unpause);
	TT_SQBIND_METHOD(MusicTrackWrapper, isPlaying);
	TT_SQBIND_METHOD(MusicTrackWrapper, getVolume);
	TT_SQBIND_METHOD(MusicTrackWrapper, setVolume);
	TT_SQBIND_METHOD(MusicTrackWrapper, setCallbackEntity);
}

// Namespace end
}
}
}
}
