#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEAUDIOPLAYER_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATEAUDIOPLAYER_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateAudioPlayer : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateAudioPlayer); }
	virtual ~LoadStateAudioPlayer() { }
	
	virtual std::string getName()               const { return "AudioPlayer"; }
	virtual s32         getEstimatedStepCount() const { return 1;             }
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateAudioPlayer()
	:
	m_initiatedPlayerCreation(false)
	{ }
	
	
	bool m_initiatedPlayerCreation;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEAUDIOPLAYER_H)
