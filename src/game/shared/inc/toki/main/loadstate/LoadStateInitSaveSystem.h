#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEINITSAVESYSTEM_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATEINITSAVESYSTEM_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateInitSaveSystem : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateInitSaveSystem); }
	virtual ~LoadStateInitSaveSystem() { }
	
	virtual std::string getName()               const { return "Init Save System"; }
	virtual s32         getEstimatedStepCount() const { return 1;                  }
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateInitSaveSystem() { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEINITSAVESYSTEM_H)
