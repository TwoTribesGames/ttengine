#include <unittestpp/unittestpp.h>

#include <tt/code/BitMask.h>


SUITE(tt_code)
{

struct SetupBitMaskFixture
{
	SetupBitMaskFixture()
	{
	
	}
	
protected:
	enum EnumValue
	{
		EnumValue_Zero,
		EnumValue_One,
		EnumValue_Two,
		EnumValue_Three,
		
		EnumValue_Count
	};
	typedef tt::code::BitMask<EnumValue, EnumValue_Count> TestBitMask;
};


TEST_FIXTURE( SetupBitMaskFixture, BitMask_Test )
{
	// Create with default constructor. (Empty)
	TestBitMask test;
	CHECK_EQUAL(0u,   test.getFlags());
	CHECK_EQUAL(true, test.isEmpty());
	
	// Set Two, check flags, no longer empty.
	test.setFlag(EnumValue_Two);
	CHECK_EQUAL((1u << EnumValue_Two), test.getFlags());
	CHECK_EQUAL(true,                  test.checkFlag(EnumValue_Two));
	CHECK_EQUAL(false,                 test.isEmpty());
	
	// Create second BitMask with different flag (Zero)
	TestBitMask testZero(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Zero), testZero.getFlags());
	CHECK_EQUAL(true,                   testZero.checkFlag(EnumValue_Zero));
	
	// Combine the two mask into a new result.
	TestBitMask result = test | testZero;
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Two));
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Zero));
	
	// Set Zero again, nothing should change.
	result.setFlag(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Two));
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Zero));
	
	// Reset Zero, bit flag should be gone
	result.resetFlag(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                  result.checkFlag(EnumValue_Two));
	
	// Resetting zero again, nothing should change.
	result.resetFlag(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                  result.checkFlag(EnumValue_Two));
	
	// Create two masks with multiple bits.
	const TestBitMask zeroThree(TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_Three));
	const TestBitMask zeroOne(  TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_One)  );
	
	// Test OR
	TestBitMask orResult(zeroThree | zeroOne);
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, orResult.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_Three));
	
	// Test AND
	TestBitMask andResult(zeroThree & zeroOne);
	CHECK_EQUAL(true,  andResult.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_Three));
	
	// Test the bitmask functions for multiple flags
	TestBitMask flags(zeroThree);
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree));
	CHECK_EQUAL(true,  flags == zeroThree);
	CHECK_EQUAL(false, flags.checkFlags(zeroOne));
	CHECK_EQUAL(false, flags == zeroOne);
	CHECK_EQUAL(true,  flags.checkAnyFlags(zeroOne));
	
	flags.setFlags(zeroOne);
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree | zeroOne));
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree));
	CHECK_EQUAL(true,  flags.checkFlags(zeroOne));
	CHECK_EQUAL(false, flags == zeroThree);
	CHECK_EQUAL(false, flags == zeroOne);
	CHECK_EQUAL(true,  flags == (zeroThree | zeroOne));
	
	// Test bit toggle
	TestBitMask mask(TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_Two));
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlag(EnumValue_Two); // Turn it off.
	CHECK_EQUAL((1u << EnumValue_Zero), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlag(EnumValue_Two); // Turn it on again.
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	// Test mask toggle.
	const TestBitMask oneTwo(TestBitMask(EnumValue_One) | TestBitMask(EnumValue_Two));
	mask.toggleFlags(oneTwo);
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_One), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlags(oneTwo);
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	// Check empty and all flags set.
	const TestBitMask emptyFlags;
	mask.resetAllFlags();
	CHECK_EQUAL(true, mask.isEmpty());
	CHECK_EQUAL(true, mask.checkFlags(emptyFlags));
	CHECK_EQUAL(true, emptyFlags == mask);
	
	const TestBitMask allFlags(TestBitMask(EnumValue_Zero) |
	                           TestBitMask(EnumValue_One)  |
	                           TestBitMask(EnumValue_Two)  |
	                           TestBitMask(EnumValue_Three));
	
	mask.setAllFlags();
	CHECK_EQUAL(false, mask.isEmpty());
	CHECK_EQUAL(true,  mask.checkFlags(allFlags));
	CHECK_EQUAL(true, allFlags == mask);
}



struct SetupBitMask64Fixture
{
	SetupBitMask64Fixture()
	{
	
	}
	
protected:
	enum EnumValue
	{
		EnumValue_0,
		EnumValue_1,
		EnumValue_2,
		EnumValue_3,
		EnumValue_4,
		EnumValue_5,
		EnumValue_6,
		EnumValue_7,
		EnumValue_8,
		EnumValue_9,
		EnumValue_10,
		EnumValue_11,
		EnumValue_12,
		EnumValue_13,
		EnumValue_14,
		EnumValue_15,
		EnumValue_16,
		EnumValue_17,
		EnumValue_18,
		EnumValue_19,
		EnumValue_20,
		EnumValue_21,
		EnumValue_22,
		EnumValue_23,
		EnumValue_24,
		EnumValue_25,
		EnumValue_26,
		EnumValue_27,
		EnumValue_28,
		EnumValue_29,
		EnumValue_30,
		EnumValue_31,
		EnumValue_32,
		EnumValue_33,
		EnumValue_34,
		EnumValue_35,
		EnumValue_36,
		EnumValue_37,
		EnumValue_38,
		EnumValue_39,
		
		EnumValue_Count
	};
	typedef tt::code::BitMask<EnumValue, EnumValue_Count> TestBitMask;
};


TEST_FIXTURE( SetupBitMask64Fixture, BitMask_Test_64_lower )
{
	const EnumValue EnumValue_Zero  = EnumValue_0;
	const EnumValue EnumValue_One   = EnumValue_1;
	const EnumValue EnumValue_Two   = EnumValue_2;
	const EnumValue EnumValue_Three = EnumValue_3;
	
	// Create with default constructor. (Empty)
	TestBitMask test;
	CHECK_EQUAL(0u,   test.getFlags());
	CHECK_EQUAL(true, test.isEmpty());
	
	// Set Two, check flags, no longer empty.
	test.setFlag(EnumValue_Two);
	CHECK_EQUAL((1u << EnumValue_Two), test.getFlags());
	CHECK_EQUAL(true,                  test.checkFlag(EnumValue_Two));
	CHECK_EQUAL(false,                 test.isEmpty());
	
	// Create second BitMask with different flag (Zero)
	TestBitMask testZero(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Zero), testZero.getFlags());
	CHECK_EQUAL(true,                   testZero.checkFlag(EnumValue_Zero));
	
	// Combine the two mask into a new result.
	TestBitMask result = test | testZero;
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Two));
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Zero));
	
	// Set Zero again, nothing should change.
	result.setFlag(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Two));
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Zero));
	
	// Reset Zero, bit flag should be gone
	result.resetFlag(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                  result.checkFlag(EnumValue_Two));
	
	// Resetting zero again, nothing should change.
	result.resetFlag(EnumValue_Zero);
	CHECK_EQUAL((1u << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                  result.checkFlag(EnumValue_Two));
	
	// Create two masks with multiple bits.
	const TestBitMask zeroThree(TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_Three));
	const TestBitMask zeroOne(  TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_One)  );
	
	// Test OR
	TestBitMask orResult(zeroThree | zeroOne);
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, orResult.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_Three));
	
	// Test AND
	TestBitMask andResult(zeroThree & zeroOne);
	CHECK_EQUAL(true,  andResult.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_Three));
	
	// Test the bitmask functions for multiple flags
	TestBitMask flags(zeroThree);
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree));
	CHECK_EQUAL(true,  flags == zeroThree);
	CHECK_EQUAL(false, flags.checkFlags(zeroOne));
	CHECK_EQUAL(false, flags == zeroOne);
	CHECK_EQUAL(true,  flags.checkAnyFlags(zeroOne));
	
	flags.setFlags(zeroOne);
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree | zeroOne));
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree));
	CHECK_EQUAL(true,  flags.checkFlags(zeroOne));
	CHECK_EQUAL(false, flags == zeroThree);
	CHECK_EQUAL(false, flags == zeroOne);
	CHECK_EQUAL(true,  flags == (zeroThree | zeroOne));
	
	// Test bit toggle
	TestBitMask mask(TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_Two));
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlag(EnumValue_Two); // Turn it off.
	CHECK_EQUAL((1u << EnumValue_Zero), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlag(EnumValue_Two); // Turn it on again.
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	// Test mask toggle.
	const TestBitMask oneTwo(TestBitMask(EnumValue_One) | TestBitMask(EnumValue_Two));
	mask.toggleFlags(oneTwo);
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_One), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlags(oneTwo);
	CHECK_EQUAL((1u << EnumValue_Zero) | (1u << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));

	// Check empty and all flags set.
	const TestBitMask emptyFlags;
	mask.resetAllFlags();
	CHECK_EQUAL(true, mask.isEmpty());
	CHECK_EQUAL(true, mask.checkFlags(emptyFlags));
	CHECK_EQUAL(true, emptyFlags == mask);
	
	const TestBitMask allFlags(TestBitMask(EnumValue_0)  |
	                           TestBitMask(EnumValue_1)  |
	                           TestBitMask(EnumValue_2)  |
	                           TestBitMask(EnumValue_3)  |
	                           TestBitMask(EnumValue_4)  |
	                           TestBitMask(EnumValue_5)  |
	                           TestBitMask(EnumValue_6)  |
	                           TestBitMask(EnumValue_7)  |
	                           TestBitMask(EnumValue_8)  |
	                           TestBitMask(EnumValue_9)  |
	                           TestBitMask(EnumValue_10) |
	                           TestBitMask(EnumValue_11) |
	                           TestBitMask(EnumValue_12) |
	                           TestBitMask(EnumValue_13) |
	                           TestBitMask(EnumValue_14) |
	                           TestBitMask(EnumValue_15) |
	                           TestBitMask(EnumValue_16) |
	                           TestBitMask(EnumValue_17) |
	                           TestBitMask(EnumValue_18) |
	                           TestBitMask(EnumValue_19) |
	                           TestBitMask(EnumValue_20) |
	                           TestBitMask(EnumValue_21) |
	                           TestBitMask(EnumValue_22) |
	                           TestBitMask(EnumValue_23) |
	                           TestBitMask(EnumValue_24) |
	                           TestBitMask(EnumValue_25) |
	                           TestBitMask(EnumValue_26) |
	                           TestBitMask(EnumValue_27) |
	                           TestBitMask(EnumValue_28) |
	                           TestBitMask(EnumValue_29) |
	                           TestBitMask(EnumValue_30) |
	                           TestBitMask(EnumValue_31) |
	                           TestBitMask(EnumValue_32) |
	                           TestBitMask(EnumValue_33) |
	                           TestBitMask(EnumValue_34) |
	                           TestBitMask(EnumValue_35) |
	                           TestBitMask(EnumValue_36) |
	                           TestBitMask(EnumValue_37) |
	                           TestBitMask(EnumValue_38) |
	                           TestBitMask(EnumValue_39));
	
	mask.setAllFlags();
	CHECK_EQUAL(false, mask.isEmpty());
	CHECK_EQUAL(true,  mask.checkFlags(allFlags));
	CHECK_EQUAL(true, allFlags == mask);
}


TEST_FIXTURE( SetupBitMask64Fixture, BitMask_Test_64_upper )
{
	const EnumValue EnumValue_Zero  = EnumValue_35;
	const EnumValue EnumValue_One   = EnumValue_36;
	const EnumValue EnumValue_Two   = EnumValue_37;
	const EnumValue EnumValue_Three = EnumValue_38;
	
	// Create with default constructor. (Empty)
	TestBitMask test;
	CHECK_EQUAL(0u,   test.getFlags());
	CHECK_EQUAL(true, test.isEmpty());
	
	// Set Two, check flags, no longer empty.
	test.setFlag(EnumValue_Two);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Two), test.getFlags());
	CHECK_EQUAL(true,                  test.checkFlag(EnumValue_Two));
	CHECK_EQUAL(false,                 test.isEmpty());
	
	// Create second BitMask with different flag (Zero)
	TestBitMask testZero(EnumValue_Zero);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero), testZero.getFlags());
	CHECK_EQUAL(true,                   testZero.checkFlag(EnumValue_Zero));
	
	// Combine the two mask into a new result.
	TestBitMask result = test | testZero;
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero) | (TestBitMask::ValueType(1u) << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Two));
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Zero));
	
	// Set Zero again, nothing should change.
	result.setFlag(EnumValue_Zero);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero) | (TestBitMask::ValueType(1u) << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Two));
	CHECK_EQUAL(true,                                           result.checkFlag(EnumValue_Zero));
	
	// Reset Zero, bit flag should be gone
	result.resetFlag(EnumValue_Zero);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                  result.checkFlag(EnumValue_Two));
	
	// Resetting zero again, nothing should change.
	result.resetFlag(EnumValue_Zero);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Two), result.getFlags());
	CHECK_EQUAL(true,                  result.checkFlag(EnumValue_Two));
	
	// Create two masks with multiple bits.
	const TestBitMask zeroThree(TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_Three));
	const TestBitMask zeroOne(  TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_One)  );
	
	// Test OR
	TestBitMask orResult(zeroThree | zeroOne);
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, orResult.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(true,  orResult.checkFlag(EnumValue_Three));
	
	// Test AND
	TestBitMask andResult(zeroThree & zeroOne);
	CHECK_EQUAL(true,  andResult.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, andResult.checkFlag(EnumValue_Three));
	
	// Test the bitmask functions for multiple flags
	TestBitMask flags(zeroThree);
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree));
	CHECK_EQUAL(true,  flags == zeroThree);
	CHECK_EQUAL(false, flags.checkFlags(zeroOne));
	CHECK_EQUAL(false, flags == zeroOne);
	CHECK_EQUAL(true,  flags.checkAnyFlags(zeroOne));
	
	flags.setFlags(zeroOne);
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree | zeroOne));
	CHECK_EQUAL(true,  flags.checkFlags(zeroThree));
	CHECK_EQUAL(true,  flags.checkFlags(zeroOne));
	CHECK_EQUAL(false, flags == zeroThree);
	CHECK_EQUAL(false, flags == zeroOne);
	CHECK_EQUAL(true,  flags == (zeroThree | zeroOne));
	
	// Test bit toggle
	TestBitMask mask(TestBitMask(EnumValue_Zero) | TestBitMask(EnumValue_Two));
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero) | (TestBitMask::ValueType(1u) << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlag(EnumValue_Two); // Turn it off.
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlag(EnumValue_Two); // Turn it on again.
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero) | (TestBitMask::ValueType(1u) << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	// Test mask toggle.
	const TestBitMask oneTwo(TestBitMask(EnumValue_One) | TestBitMask(EnumValue_Two));
	mask.toggleFlags(oneTwo);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero) | (TestBitMask::ValueType(1u) << EnumValue_One), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
	
	mask.toggleFlags(oneTwo);
	CHECK_EQUAL((TestBitMask::ValueType(1u) << EnumValue_Zero) | (TestBitMask::ValueType(1u) << EnumValue_Two), mask.getFlags());
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Zero ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_One  ));
	CHECK_EQUAL(true,  mask.checkFlag(EnumValue_Two  ));
	CHECK_EQUAL(false, mask.checkFlag(EnumValue_Three));
}


// End SUITE
}
