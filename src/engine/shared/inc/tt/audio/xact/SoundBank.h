#ifndef INC_TT_AUDIO_XACT_SOUNDBANK_H
#define INC_TT_AUDIO_XACT_SOUNDBANK_H

#include <map>
#include <string>

#include <tt/audio/xact/Cue.h>
#include <tt/audio/xact/CueInstance.h>
#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class SoundBank
{
public:
	explicit SoundBank(int p_soundBankIndex);
	~SoundBank();
	
	Cue::ErrorStatus createCue(int p_cueIndex, CueInstancePtr& p_result_OUT);
	bool             play(int p_cueIndex);
	bool             play(const std::string& p_name);
	bool             stop(int p_cueIndex);
	bool             stop(const std::string& p_name);
	bool             stop();
	bool             pause(int p_cueIndex);
	bool             pause(const std::string& p_name);
	bool             resume(int p_cueIndex);
	bool             resume(const std::string& p_name);
	
	bool pauseCategory (Category* p_category);
	bool resumeCategory(Category* p_category);
	bool stopCategory  (Category* p_category);
	
	/*! \brief Sets a new reverb mixing volume for all already playing sounds in the specified category. */
	void setReverbVolumeForCategory(Category* p_category, real p_volumeInDB);
	
	void addCue(Cue* p_cue, const std::string& p_name);
	void addSound(Sound* p_sound);
	
	/*! \return Cue index for the specified cue name or -1 if cue name not found. */
	int getCueIndex(const std::string& p_name) const;
	
	inline int getSoundBankIndex() const { return m_soundBankIndex; }
	
	Sound* getSound(int p_index);
	
	static SoundBank* createSoundBank(int p_soundBankIndex, const xml::XmlNode* p_node);
	
	void update(real p_delta);
	void updateVolume();
	
private:
	typedef std::map<int, Sound*> Sounds;
	typedef std::map<std::string, int> CueIndices;
	typedef std::map<int, Cue*> Cues;
	
	
	SoundBank(const SoundBank&);
	SoundBank& operator=(const SoundBank&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	
	Sounds     m_sounds;
	CueIndices m_cueIndices;
	Cues       m_cues;
	
	int m_soundBankIndex;
	
	friend class AudioTT;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_SOUNDBANK_H
