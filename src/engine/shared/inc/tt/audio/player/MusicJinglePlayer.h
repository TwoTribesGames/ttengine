#if !defined(INC_TT_AUDIO_PLAYER_MUSICJINGLEPLAYER_H)
#define INC_TT_AUDIO_PLAYER_MUSICJINGLEPLAYER_H

#include <tt/audio/player/MusicPlayer.h>


namespace tt {
namespace audio {
namespace player {

class MusicJinglePlayer : public MusicPlayer
{
public:
	virtual ~MusicJinglePlayer() { }
	
	virtual bool playJingle(const std::string& p_jingle) = 0;
	
protected:
	MusicJinglePlayer() : MusicPlayer() { }
};


// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_PLAYER_MUSICJINGLEPLAYER_H)
