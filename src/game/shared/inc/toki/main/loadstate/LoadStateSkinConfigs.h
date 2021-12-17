#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESKINCONFIGS_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATESKINCONFIGS_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateSkinConfigs : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateSkinConfigs); }
	virtual ~LoadStateSkinConfigs() { }
	
	virtual std::string getName()               const { return "Skin Configs"; }
	virtual s32         getEstimatedStepCount() const { return 1;              }
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateSkinConfigs() { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESKINCONFIGS_H)
