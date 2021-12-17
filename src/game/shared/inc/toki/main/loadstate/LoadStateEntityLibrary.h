#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEENTITYLIBRARY_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATEENTITYLIBRARY_H


#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateEntityLibrary : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateEntityLibrary); }
	virtual ~LoadStateEntityLibrary() { }
	
	virtual std::string getName()               const { return "EntityLibrary"; }
	virtual s32         getEstimatedStepCount() const { return 1;               }
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	inline LoadStateEntityLibrary() { }
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEENTITYLIBRARY_H)
