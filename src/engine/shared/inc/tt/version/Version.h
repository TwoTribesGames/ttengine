#ifndef INC_TT_VERSION_VERSION_H
#define INC_TT_VERSION_VERSION_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace version {

void setClientVersionInfo(s32 p_clientRevisionNumber, 
                          const char* p_clientVersionName);

s32 getLibRevisionNumber();
const char* getLibVersionName();
s32 getClientRevisionNumber();
const char* getClientVersionName();

// Namespace end
}
}


#endif  // !defined(INC_TT_VERSION_VERSION_H)
