#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_MUSICTRACKWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_MUSICTRACKWRAPPER_H


#include <map>
#include <string>

#include <tt/code/fwd.h>
#include <tt/script/helpers.h>

#include <toki/audio/fwd.h>
#include <toki/game/script/wrappers/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'MusicTrack' in Squirrel. Used to control music. */
class MusicTrackWrapper
{
public:
	inline explicit MusicTrackWrapper(const audio::MusicTrackHandle& p_track = audio::MusicTrackHandle())
	:
	m_track(p_track)
	{ }
	
	/*! \brief Starts playing the music track. */
	void play();
	
	/*! \brief Stop the music track. */
	void stop();
	
	/*! \brief Pause the music track. */
	void pause();
	
	/*! \brief Resume a paused music track. Cannot use the word "resume" as that is a reserved keyword in Squirrel. */
	void unpause();
	
	/*! \brief Indicates whether the music track is currently playing. */
	bool isPlaying() const;
	
	/*! \brief Returns volume in range 0-100 of this track */
	real getVolume() const;
	
	/*! \brief Sets the volume of the music track.
	    \param p_volumePercentage Volume in range 0 - 100.
	    \param p_fadeDuration If greater than 0: fade to the new volume in this many seconds. Otherwise, the new volume is set instantly.
	    \param p_stopTrackAfterwards Whether to stop the music track after fading is complete / the new volume has been set. */
	void setVolume(real p_volumePercentage, real p_fadeDuration, bool p_stopTrackAfterFade);
	
	/*! \brief Sets the entity that is interested in callbacks from this music track (e.g. playback looped notifications).
	    \param p_entity The entity that will receive callbacks. */
	void setCallbackEntity(const EntityWrapper* p_entity);
	
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline const audio::MusicTrackHandle& getHandle() const { return m_track; }
	inline void invalidate() { m_track.invalidate(); }
	
private:
	audio::MusicTrackHandle m_track;
};

// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_MUSICTRACKWRAPPER_H)
