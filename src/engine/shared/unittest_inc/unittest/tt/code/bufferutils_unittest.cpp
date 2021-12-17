#include <limits>

#include <unittestpp/unittestpp.h>

#include <tt/code/AutoGrowBuffer.h>
#include <tt/code/bufferutils.h>
#include <tt/code/Handle.h>


SUITE(tt_code)
{

struct SetupAutoGrowBufferFixture
{
	SetupAutoGrowBufferFixture()
	:
	autoGrowBuffer(tt::code::AutoGrowBuffer::create(256, 256))
	{
	}
	tt::code::AutoGrowBufferPtr autoGrowBuffer;
	
private:
	const SetupAutoGrowBufferFixture& operator=(const SetupAutoGrowBufferFixture& p_rhs);
};


TEST_FIXTURE( SetupAutoGrowBufferFixture, BufferUtils_Math )
{
	const tt::math::Vector2    vecPos(-3.3f, 53024.0f);
	const tt::math::Point2     pointPos(53213245, -30453229);
	const tt::math::VectorRect vecRect(tt::math::Vector2(4895.39875f, -3424.93874f), 3409.0f, 34.0f);
	const tt::math::PointRect  pointRect(tt::math::Point2(-92837, 234), 234234, 7643);
	
	namespace bu = tt::code::bufferutils;
	
	{
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		
		bu::put(vecPos,    &context);
		bu::put(pointPos,  &context);
		bu::put(vecRect,   &context);
		bu::put(pointRect, &context);
		
		context.flush();
		
		// TODO: Unittest to verify byte data.
		//CHECK_ARRAY_EQUAL
		
		// TODO: Check buffer size is what is expected.
	}
	
	
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		
		const tt::math::Vector2    loadedVecPos(   bu::get<tt::math::Vector2   >(&context));
		const tt::math::Point2     loadedPointPos( bu::get<tt::math::Point2    >(&context));
		const tt::math::VectorRect loadedVecRect(  bu::get<tt::math::VectorRect>(&context));
		const tt::math::PointRect  loadedPointRect(bu::get<tt::math::PointRect >(&context));
		
		CHECK_EQUAL(vecPos,    loadedVecPos);
		CHECK_EQUAL(pointPos,  loadedPointPos);
		CHECK_EQUAL(vecRect,   loadedVecRect);
		CHECK_EQUAL(pointRect, loadedPointRect);
		
		// TODO: Check nothing is left in buffer.
	}
}


TEST_FIXTURE( SetupAutoGrowBufferFixture, BufferUtils_Enum )
{
	enum TestEnum
	{
		TestEnum_Zero = 0,
		TestEnum_One  = 1,
		TestEnum_NegativeTwo = -2,
		TestEnum_TooLargeForByte = 300
	};
	
	namespace bu = tt::code::bufferutils;
	
	{
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		
		bu::putEnum<u8>(TestEnum_Zero,            &context);
		bu::putEnum<u8>(TestEnum_One,             &context);
		//CHECK_ASSERT(bu::putEnum<u8>(TestEnum_NegativeTwo,     &context));
		bu::putEnum<s8>(TestEnum_NegativeTwo,     &context);
		//CHECK_ASSERT(bu::putEnum<u8>(TestEnum_TooLargeForByte, &context));
		bu::putEnum<u16>(TestEnum_TooLargeForByte, &context);
		
		context.flush();
		
		// TODO: Unittest to verify byte data.
		//CHECK_ARRAY_EQUAL
		
		// TODO: Check buffer size is what is expected.
	}
	
	
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		
		const TestEnum loadedZero           (bu::getEnum<u8 , TestEnum>(&context));
		const TestEnum loadedOne            (bu::getEnum<u8 , TestEnum>(&context));
		const TestEnum loadedNegativeTwo    (bu::getEnum<s8 , TestEnum>(&context));
		const TestEnum loadedTooLargeForByte(bu::getEnum<u16, TestEnum>(&context));
		
		CHECK_EQUAL(TestEnum_Zero           , loadedZero           );
		CHECK_EQUAL(TestEnum_One            , loadedOne            );
		CHECK_EQUAL(TestEnum_NegativeTwo    , loadedNegativeTwo    );
		CHECK_EQUAL(TestEnum_TooLargeForByte, loadedTooLargeForByte);
		
		// TODO: Check nothing is left in buffer.
	}
}


TEST_FIXTURE( SetupAutoGrowBufferFixture, BufferUtils_Handle )
{
	typedef tt::code::Handle<s32> HandleType;
	HandleType testHandle(HandleType::createForTesting(2345, 1234));
	HandleType emptyHandle;
	
	namespace bu = tt::code::bufferutils;
	
	{
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		
		bu::putHandle(testHandle , &context);
		bu::putHandle(emptyHandle, &context);
		
		context.flush();
		
		// TODO: Unittest to verify byte data.
		//CHECK_ARRAY_EQUAL
		
		// TODO: Check buffer size is what is expected.
	}
	
	
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		
		const HandleType loadedTest (bu::getHandle<s32>(&context));
		const HandleType loadedEmpty(bu::getHandle<s32>(&context));
		
		CHECK_EQUAL(testHandle , loadedTest );
		CHECK_EQUAL(emptyHandle, loadedEmpty);
		
		// TODO: Check nothing is left in buffer.
	}
}


TEST_FIXTURE( SetupAutoGrowBufferFixture, BufferUtils_RawData )
{
	namespace bu = tt::code::bufferutils;
	
	const u8           testU8      = std::numeric_limits<u8    >::max();
	const u16          testU16     = std::numeric_limits<u16   >::max();
	const u32          testU32     = std::numeric_limits<u32   >::max();
	const u64          testU64     = std::numeric_limits<u64   >::max();
	const s8           testS8      = std::numeric_limits<s8    >::min();
	const s16          testS16     = std::numeric_limits<s16   >::min();
	const s32          testS32     = std::numeric_limits<s32   >::min();
	const s64          testS64     = std::numeric_limits<s64   >::min();
	const real         testReal    = std::numeric_limits<real  >::max();
	const real64       testReal64  = std::numeric_limits<real64>::max();
	const bool         testBool    = true;
	const std::string  testString  = "Test Bufferutils String";
	const std::wstring testWString = L"Test Bufferutils Wide String";
	
	{
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		
		bu::put(testU8     , &context);
		bu::put(testU16    , &context);
		bu::put(testU32    , &context);
		bu::put(testU64    , &context);
		bu::put(testS8     , &context);
		bu::put(testS16    , &context);
		bu::put(testS32    , &context);
		bu::put(testS64    , &context);
		bu::put(testReal   , &context);
		bu::put(testReal64 , &context);
		bu::put(testBool   , &context);
		bu::put(testString , &context);
		bu::put(testWString, &context);
		
		context.flush();
	}
	
	
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		
		const u8           loadedU8      = bu::get<u8          >(&context);
		const u16          loadedU16     = bu::get<u16         >(&context);
		const u32          loadedU32     = bu::get<u32         >(&context);
		const u64          loadedU64     = bu::get<u64         >(&context);
		const s8           loadedS8      = bu::get<s8          >(&context);
		const s16          loadedS16     = bu::get<s16         >(&context);
		const s32          loadedS32     = bu::get<s32         >(&context);
		const s64          loadedS64     = bu::get<s64         >(&context);
		const real         loadedReal    = bu::get<real        >(&context);
		const real64       loadedReal64  = bu::get<real64      >(&context);
		const bool         loadedBool    = bu::get<bool        >(&context);
		const std::string  loadedString  = bu::get<std::string >(&context);
		const std::wstring loadedWString = bu::get<std::wstring>(&context);
		
		CHECK_EQUAL(testU8     , loadedU8     );
		CHECK_EQUAL(testU16    , loadedU16    );
		CHECK_EQUAL(testU32    , loadedU32    );
		CHECK_EQUAL(testU64    , loadedU64    );
		CHECK_EQUAL(testS8     , loadedS8     );
		CHECK_EQUAL(testS16    , loadedS16    );
		CHECK_EQUAL(testS32    , loadedS32    );
		CHECK_EQUAL(testS64    , loadedS64    );
		CHECK_EQUAL(testReal   , loadedReal   );
		CHECK_EQUAL(testReal64 , loadedReal64 );
		CHECK_EQUAL(testBool   , loadedBool   );
		CHECK_EQUAL(testString , loadedString );
		//CHECK_EQUAL(testWString, loadedWString);
		CHECK(testWString == loadedWString);  // FIXME: CHECK_EQUAL doesn't work, because wide string can't be << into narrow stream
		
		// TODO: Check nothing is left in buffer.
	}
}

// End SUITE
}
