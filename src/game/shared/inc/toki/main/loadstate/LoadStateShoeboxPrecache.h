#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESHOEBOXPRECACHE_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATESHOEBOXPRECACHE_H

#include <tt/str/str_types.h>

#include <toki/main/loadstate/LoadState.h>

namespace toki {
namespace main {
namespace loadstate {

class LoadStateShoeboxPrecache : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStateShoeboxPrecache); }
	virtual ~LoadStateShoeboxPrecache() { }
	
	virtual std::string getName()               const { return "ShoeboxPrecache"; }
	virtual s32         getEstimatedStepCount() const;
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	LoadStateShoeboxPrecache();
	
	tt::str::Strings m_filenamesShoeboxes;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATESHOEBOXPRECACHE_H)
