#ifndef UNITTEST_TIMEHELPERS_H
#define UNITTEST_TIMEHELPERS_H

#include "../../config.h"
#include "../HelperMacros.h"

#include <tt/platform/tt_types.h>

/*
#if !defined TT_PLATFORM_WIN
    #ifndef __int64
        #define __int64 u64
    #endif
#endif
	*/

namespace UnitTest {

class UNITTEST_LINKAGE Timer
{
public:
	Timer();
	void Start();
	double GetTimeInMs() const;
private:
	u64 m_startTime;
};


namespace TimeHelpers
{
	UNITTEST_LINKAGE void SleepMs(int ms);
}

}

#endif
