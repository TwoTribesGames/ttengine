#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEGENERATEMETADATA_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATEGENERATEMETADATA_H

#include <toki/main/loadstate/LoadState.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStateGenerateMetaData : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateGenerateMetaData); }
	virtual ~LoadStateGenerateMetaData() { }
	
	virtual std::string getName()               const;
	virtual s32         getEstimatedStepCount() const { return 2; }
	
	virtual void doLoadStep();
	virtual bool isDone() const { return false; }	// LoadState will exit app, and is therefore never done
	
private:
	LoadStateGenerateMetaData();
	bool m_waitingForExit;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEMETADATA_H)
