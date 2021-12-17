#ifndef INC_TT_AUDIO_XACT_WAVEBANK_H
#define INC_TT_AUDIO_XACT_WAVEBANK_H


#include <map>

#include <tt/audio/xact/fwd.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class WaveBank
{
public:
	explicit WaveBank(int p_waveBankIndex, const std::string& p_name);
	~WaveBank();
	
	void addWave(Wave* p_wave);
	
	Wave* getWave(int p_index) const;
	
	inline int getWaveBankIndex() const { return m_waveBankIndex; }
	inline const std::string& getName() const { return m_name; }
	
	bool loadWaves();
	void unloadWaves();
	
	/*! \brief Returns the size (in bytes) of this wave bank in memory. */
	s32 getMemorySize() const;
	
	static WaveBank* createWaveBank(int p_waveBankIndex, const xml::XmlNode* p_node, bool p_fromConverter = false);
	
private:
	typedef std::vector<Wave*> Waves;
	
	WaveBank(const WaveBank&);
	WaveBank& operator=(const WaveBank&);
	
	bool load(const fs::FilePtr& p_file);
	bool save(const fs::FilePtr& p_file) const;
	
	std::string m_name;
	Waves       m_waves;
	int         m_waveBankIndex;
	
	friend class AudioTT;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_WAVEBANK_H
