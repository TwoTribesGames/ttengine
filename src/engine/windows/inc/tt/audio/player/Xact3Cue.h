#if !defined(INC_TT_AUDIO_PLAYER_XACT3CUE_H)
#define INC_TT_AUDIO_PLAYER_XACT3CUE_H


#define NOMINMAX
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <XAudio2.h>
#else
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\XAudio2.h>
#endif
#include <xact3.h>
#include <xact3d3.h>

#include <tt/audio/player/SoundCue.h>


namespace tt {
namespace audio {
namespace player {

class Xact3Player;

class Xact3Cue : public SoundCue
{
public:
	virtual ~Xact3Cue();
	
	virtual bool play();
	virtual bool stop(bool p_immediately = false);
	virtual bool pause();
	virtual bool resume();
	virtual State getState() const;
	
	virtual bool setVariable(const std::string& p_name, real  p_value);
	virtual bool getVariable(const std::string& p_name, real* p_value_OUT) const;
	
	virtual bool setPosition(const math::Vector3& p_position);
	virtual bool setRadius  (real p_inner, real p_outer);
	
private:
	Xact3Cue(Xact3Player* p_player, IXACT3SoundBank* p_soundBank, XACTINDEX p_cueIndex,
	         bool p_positional, const X3DAUDIO_EMITTER& p_defaultEmitter, const std::string& p_cueName);
	
	
	Xact3Player*     m_player;
	IXACT3SoundBank* m_soundBank;
	XACTINDEX        m_cueIndex;
	X3DAUDIO_EMITTER m_emitter;      // for positional audio only
	
	// Created at play()
	IXACT3Cue*       m_cue;
	
#if !defined(TT_BUILD_FINAL)
	std::string      m_cueName;
#endif
	
	friend class Xact3Player;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_XACT3CUE_H)
