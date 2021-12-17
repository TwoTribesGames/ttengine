#if !defined(INC_TT_APP_PLATFORMAPI_H)
#define INC_TT_APP_PLATFORMAPI_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace app {

class PlatformCallbackInterface;


class PlatformApi
{
public:
	/*! \param p_callbackInterface Pointer to instance that handles platform callbacks. Does not take ownership. */
	PlatformApi(PlatformCallbackInterface* p_callbackInterface);
	virtual ~PlatformApi();
	
	virtual bool init()     = 0;
	virtual void shutdown() = 0;
	virtual void update()   = 0;
	
protected:
	inline PlatformCallbackInterface* getCallbackInterface() { return m_callbackInterface; }
	
private:
	// No copying
	PlatformApi(const PlatformApi&);
	PlatformApi& operator=(const PlatformApi&);
	
	
	PlatformCallbackInterface* m_callbackInterface;
};

typedef tt_ptr<PlatformApi>::shared PlatformApiPtr;

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_PLATFORMAPI_H)
