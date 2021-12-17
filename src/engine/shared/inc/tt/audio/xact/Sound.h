#ifndef INC_TT_AUDIO_XACT_SOUND_H
#define INC_TT_AUDIO_XACT_SOUND_H


#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

typedef std::vector<RuntimeParameterControl*> RPCs;


class Sound
{
public:
	Sound();
	~Sound();
	
	void addTrack(Track* p_track);
	
	inline void      setCategory(Category* p_category) { m_category = p_category; }
	inline Category* getCategory() const               { return m_category;       }
	
	void setVolume(real p_volumeInDB);
	
	/*! \return Sound volume in dB. */
	inline real getVolume() const { return m_volumeInDB; }
	
	void setPitch(real p_pitch);
	inline real getPitch() const { return m_pitch; }
	
	void setPriority(int p_priority);
	inline int getPriority() const { return m_priority; }
	
	inline void setSoundBankIndex(int p_index) { m_soundBankIndex = p_index; }
	inline int  getSoundBankIndex() const      { return m_soundBankIndex;    }
	
	inline void setSoundIndex(int p_index) { m_soundIndex = p_index; }
	inline int  getSoundIndex() const      { return m_soundIndex;    }
	
	static Sound* createSound(const xml::XmlNode* p_node);
	
	SoundInstance* instantiate(CueInstance* p_cue);
	
	inline const RPCs& getRPCs() { return m_RPCs; }
	
private:
	typedef std::vector<Track*> Tracks;
	
	Sound(const Sound&);
	Sound& operator=(const Sound&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	static void addRPC(Sound* p_sound, const std::string& p_RPCName);
	
	// mixing variables
	real m_volumeInDB;  // -96.0 - 6.0
	real m_pitch;       // -12.0 - 12.0
	int  m_priority;    // 0 - 255
	
	Category* m_category;
	Tracks    m_tracks;
	
	RPCs m_RPCs;
	
	int m_soundIndex;      // the index in the soundbank, used for saving/loading,  -1 if not set
	int m_soundBankIndex;  // the soundbank index this sound is in, used for saving/loading,  -1 if not set
	
	friend class SoundBank;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_SOUND_H
