#if !defined(INC_TOKI_AUDIO_MUSICTRACKMGR_H)
#define INC_TOKI_AUDIO_MUSICTRACKMGR_H


#include <string>

#include <tt/code/fwd.h>
#include <tt/code/HandleArrayMgr.h>
#include <tt/str/str_types.h>

#include <toki/audio/fwd.h>
#include <toki/audio/MusicTrack.h>


namespace toki {
namespace audio {

class MusicTrackMgr : private tt::code::HandleArrayMgr<MusicTrack>
{
public:
	explicit MusicTrackMgr(s32 p_reserveCount);
	~MusicTrackMgr();
	
	MusicTrackHandle createTrack(const std::string& p_musicName);
	void destroyTrack(const MusicTrackHandle& p_handle);
	
	inline MusicTrack* getTrack(const MusicTrackHandle& p_handle) { return get(p_handle); }
	
	inline s32 getActiveTrackCount() const { return getActiveCount(); }
	
	void update(real p_deltaTime);
	
	void pauseAllTracks();
	void resumeAllTracks();
	
	void handleMusicVolumeSettingChanged();  // user changed the music volume
	
	void resetAll();
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	inline const tt::str::StringSet& getAvailableMusicNames() const { return m_availableMusicNames; }
	
private:
	void gatherAvailableMusicNames();
	
	
	tt::str::StringSet m_availableMusicNames;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_MUSICTRACKMGR_H)
