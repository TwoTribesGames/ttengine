#if !defined(INC_TT_AUDIO_PLAYER_MUSICPLAYERCALLBACKINTERFACE_H)
#define INC_TT_AUDIO_PLAYER_MUSICPLAYERCALLBACKINTERFACE_H


namespace tt {
namespace audio {
namespace player {

class MusicPlayerCallbackInterface
{
public:
	virtual ~MusicPlayerCallbackInterface() { }
	
	virtual void onMusicPlayerLooped() = 0;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_MUSICPLAYERCALLBACKINTERFACE_H)
