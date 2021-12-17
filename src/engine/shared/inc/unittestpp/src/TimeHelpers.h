#include "../config.h"

#if defined UNITTEST_POSIX
    #include "Posix/TimeHelpers.h"
#elif defined UNITTEST_WIN32
    #include "Win32/TimeHelpers.h"
#else
    #include "shared/TimeHelpers.h"
#endif
