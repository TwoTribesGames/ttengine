#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESCRIPTLISTS_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATESCRIPTLISTS_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateScriptLists : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateScriptLists); }
	virtual ~LoadStateScriptLists() { }
	
	virtual std::string getName()               const { return "ScriptLists"; }
	virtual s32         getEstimatedStepCount() const { return 1;             }
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateScriptLists() { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESCRIPTLISTS_H)
