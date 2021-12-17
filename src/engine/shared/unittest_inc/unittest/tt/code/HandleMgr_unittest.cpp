#include <unittestpp/unittestpp.h>

#include <tt/code/HandleMgr.h>


SUITE(tt_code)
{

struct SetupHandleMgrFixture
{
	SetupHandleMgrFixture()
	:
	handleMgr(),
	emptyHandle()
	{
		for (s32 i = 0; i < arraySize; ++i)
		{
			testArray[i] = TestType(i);
		}
		pointer              = &testArray[0];
		pointerOtherLocation = &testArray[1];
		pointerThree         = &testArray[2];
	}
	
	enum { arraySize = 3 };
	typedef s32 TestType;
	tt::code::HandleMgr<TestType> handleMgr;
	TestType testArray[arraySize];
	TestType* pointer;
	TestType* pointerOtherLocation;
	TestType* pointerThree;
	typedef tt::code::Handle<TestType> HandleType;
	const HandleType emptyHandle;
private:
	const SetupHandleMgrFixture& operator=(const SetupHandleMgrFixture& p_rhs);
};


TEST_FIXTURE( SetupHandleMgrFixture, HandleMgr_addRemoveInvalid )
{
	CHECK_EQUAL(0, handleMgr.getCount());
	
	HandleType handle = handleMgr.add(pointer);
	
	CHECK_EQUAL(1, handleMgr.getCount());
	
	TestType* resultPtr     = 0;
	const TestType* nullPtr = 0;
	CHECK_EQUAL(false,   handleMgr.get(emptyHandle, resultPtr));
	CHECK_EQUAL(nullPtr, handleMgr.get(emptyHandle));
	
	CHECK_EQUAL(true,    handleMgr.get(handle, resultPtr));
	CHECK_EQUAL(pointer, resultPtr);
	CHECK_EQUAL(pointer, handleMgr.get(handle));
	
	handleMgr.update(handle, pointerOtherLocation);
	
	CHECK_EQUAL(true, handleMgr.get(handle, resultPtr));
	CHECK_EQUAL(pointerOtherLocation, resultPtr);
	CHECK_EQUAL(pointerOtherLocation, handleMgr.get(handle));
	
	handleMgr.removeHandle(handle);
	
	CHECK_EQUAL(0, handleMgr.getCount());
	
	CHECK_EQUAL(false,   handleMgr.get(handle, resultPtr));
	CHECK_EQUAL(nullPtr, handleMgr.get(handle));
}



TEST_FIXTURE( SetupHandleMgrFixture, HandleMgr_addWithSpecificHandle )
{
	HandleType handle          = handleMgr.add(pointer);
	HandleType handleInBetween = handleMgr.add(pointerThree);
	HandleType handleTwo       = handleMgr.add(pointerOtherLocation);
	CHECK_EQUAL(3, handleMgr.getCount());
	
	handleMgr.reset();
	CHECK_EQUAL(0, handleMgr.getCount());
	TestType* const nullPtr = 0;
	CHECK_EQUAL(nullPtr, handleMgr.get(handle));
	CHECK_EQUAL(nullPtr, handleMgr.get(handleInBetween));
	CHECK_EQUAL(nullPtr, handleMgr.get(handleTwo));
	
	// add the two pointers and handles again. (But swap handles as extra test.)
	handleMgr.addWithSpecificHandle(handleTwo, pointer);
	CHECK_EQUAL(pointer, handleMgr.get(handleTwo));
	
	CHECK_EQUAL(nullPtr, handleMgr.get(handleInBetween));
	CHECK_EQUAL(nullPtr, handleMgr.get(handle));
	CHECK_EQUAL(1,       handleMgr.getCount());
	
	handleMgr.addWithSpecificHandle(handle,    pointerOtherLocation);
	CHECK_EQUAL(pointerOtherLocation, handleMgr.get(handle));
	
	CHECK_EQUAL(nullPtr, handleMgr.get(handleInBetween));
	CHECK_EQUAL(pointer, handleMgr.get(handleTwo));
	CHECK_EQUAL(2,       handleMgr.getCount());
}


// End SUITE

// End SUITE
}
