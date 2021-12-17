#ifndef INC_TT_AUDIO_XACT_CUE_H
#define INC_TT_AUDIO_XACT_CUE_H


#include <list>
#include <string>
#include <utility>
#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class Cue
{
public:
	enum ErrorStatus
	{
		ErrorStatus_OK,
		
		ErrorStatus_InvalidCue,
		ErrorStatus_InstanceLimited,
		ErrorStatus_NoPreviousSound
	};
	
	enum VariationType
	{
		Variation_Ordered,           // play sequentially
		Variation_OrderedFromRandom, // play sequentially from random start
		Variation_Random,            // play randomly
		Variation_RandomNoRepeat,    // play randomly without repeating the previous
		Variation_Shuffle,           // play randomly without repeating any, reset when all have been played
		Variation_Interactive        // not supported
	};
	
	enum LimitBehavior
	{
		LimitBehavior_None,
		
		LimitBehavior_FailToPlay,
		LimitBehavior_Queue,
		LimitBehavior_Replace
	};
	
	Cue();
	~Cue();
	
	void addSound(Sound* p_sound, int p_weight);
	
	void setVariation(VariationType p_type);
	inline VariationType getVariation() const { return m_variation; }
	
	void setLimitBehavior(LimitBehavior p_behavior);
	inline LimitBehavior getLimitBehavior() const { return m_limitBehavior; }
	
	void setInstanceLimit(int p_limit);
	inline int getInstanceLimit() const { return m_limit; }
	
	static Cue* createCue(const xml::XmlNode* p_node, SoundBank* p_soundBank);
	
	bool           play();
	CueInstancePtr playCue();
	bool           stop();
	bool           pause();
	bool           resume();
	
	bool pauseCategory (Category* p_category);
	bool resumeCategory(Category* p_category);
	bool stopCategory  (Category* p_category);
	
	/*! \brief Sets a new reverb mixing volume for all active instances in the specified category. */
	void setReverbVolumeForCategory(Category* p_category, real p_volumeInDB);
	
	void update(real p_delta);
	void updateVolume();
	
private:
	typedef std::pair<Sound*, int> PlayListEntry;
	typedef std::vector<PlayListEntry> PlayList;
	typedef std::vector<Sound*> OrderedList;
	typedef std::list<CueInstancePtr> Instances;
	
	Cue(const Cue&);
	Cue& operator=(const Cue&);
	
	ErrorStatus instantiate(CueInstancePtr& p_instance);
	
	inline bool instanceLimitReached() const
	{
		return m_instances.size() >= static_cast<Instances::size_type>(m_limit);
	}
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	void orderPlayList();
	void shufflePlayList();
	
	Sound* getNextSound();
	
	
	PlayList      m_playList;
	OrderedList   m_playOrder;
	Sound*        m_prevSound;
	Instances     m_instances;
	
	VariationType m_variation;
	LimitBehavior m_limitBehavior;
	int           m_limit;

#ifndef TT_BUILD_FINAL
	std::string m_name;
#endif
	
	friend class SoundBank;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_CUE_H
