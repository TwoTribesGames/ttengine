#include <tt/app/AppSystems.h>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

PlatformApi::PlatformApi(PlatformCallbackInterface* p_callbackInterface)
:
m_callbackInterface(p_callbackInterface)
{
}


PlatformApi::~PlatformApi()
{
}

// Namespace end
}
}
