#if !defined(TT_BUILD_FINAL)

#include <unittestpp/unittestpp.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/unittest/unittest.h>

// Include all unittests here:
#include <toki/unittest/orientation_unittests.h>
#include <toki/unittest/serialization_unittests.h>
#include <toki/unittest/squirrel_compile_unittests.h>


int runUnitTests()
{
	TT_Printf("Starting unittest...\n");
	
	//overWriteDefaultPanicHandler();
	tt::platform::error::turnHeadlessModeOn();
	
	return UnitTest::RunAllTests();
}


#endif // !defined(TT_BUILD_FINAL)
