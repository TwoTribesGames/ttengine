#ifndef INC_TT_AUDIO_XACT_WAVE_H
#define INC_TT_AUDIO_XACT_WAVE_H


#include <string>
#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/code/fwd.h>
#include <tt/fs/types.h>
#include <tt/snd/types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class Wave
{
public:
	Wave();
	
	int         getSampleRate() const;
	inline real getDuration()   const { return m_duration; }
	
	inline const std::string& getFileName() const { return m_fileName;       }
	
	inline void setWaveBankIndex(int p_index) { m_waveBankIndex = p_index; }
	inline int  getWaveBankIndex() const      { return m_waveBankIndex;    }
	
	inline void setWaveIndex(int p_index) { m_waveIndex = p_index; }
	inline int  getWaveIndex() const      { return m_waveIndex;    }
	
	inline bool isSilent() const { return m_silent; }
	
	static Wave* createWave(tt::code::BufferReadContext* p_context);
	
	WaveInstance* instantiate();
	
	s32 getMemorySize() const;
	
	inline const snd::BufferPtr& getBuffer() const { return m_buffer; }
	
private:
	Wave(const Wave&);
	Wave& operator=(const Wave&);
	
	std::string    m_fileName;
	bool           m_silent;
	snd::BufferPtr m_buffer;
	real           m_duration; // Duration of wave in seconds
	
	int m_waveIndex;      // the index in the wavebank, used for saving/loading,  -1 if not set
	int m_waveBankIndex;  // the wavebank index this wave is in, used for saving/loading,  -1 if not set
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_WAVE_H
