#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATE_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATE_H


#include <string>

#include <tt/platform/tt_types.h>


namespace toki {
namespace main {
namespace loadstate {

/*! \brief Base class for steps in the application loading process. */
class LoadState
{
public:
	virtual ~LoadState() { }
	
	virtual std::string getName()               const = 0;
	virtual s32         getEstimatedStepCount() const = 0;
	
	virtual void doLoadStep()   = 0;
	virtual bool isDone() const = 0;
	
protected:
	inline LoadState() { }
	
private:
	// No copying
	LoadState(const LoadState&);
	LoadState& operator=(const LoadState&);
};

typedef tt_ptr<LoadState>::shared LoadStatePtr;

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATE_H)
