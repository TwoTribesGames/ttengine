#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESCRIPTMGR_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATESCRIPTMGR_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateScriptMgr : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateScriptMgr); }
	virtual ~LoadStateScriptMgr() { }
	
	virtual std::string getName()               const { return "ScriptMgr"; }
	virtual s32         getEstimatedStepCount() const { return 1;           }
	
	// NOTE: Script manager initialization is another candidate for splitting up into several steps,
	//       as it currently takes around 1 second to perform its various steps.
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateScriptMgr() { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESCRIPTMGR_H)
