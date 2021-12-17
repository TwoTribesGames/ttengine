#include <tt/platform/tt_error.h>

#include <unittest/unittest.h>


int runUnitTests()
{
#if defined(TT_PLATFORM_WIN)
	//overWriteDefaultPanicHandler();
	tt::platform::error::turnHeadlessModeOn();
#endif
	
	return UnitTest::RunAllTests();
}

