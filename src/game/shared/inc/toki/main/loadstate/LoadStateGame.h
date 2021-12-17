#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEGAME_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATEGAME_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateGame : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateGame); }
	virtual ~LoadStateGame() { }
	
	virtual std::string getName()               const { return "Loading Game"; }
	virtual s32         getEstimatedStepCount() const { return 1;              }
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateGame() { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEGAME_H)
