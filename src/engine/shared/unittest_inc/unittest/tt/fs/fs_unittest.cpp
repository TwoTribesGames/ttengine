#include <unittestpp/unittestpp.h>

#include <tt/fs/fs.h>
#include <tt/fs/utils/utils.h>

#include <tt/fs/BufferedFileSystem.h>
#include <tt/fs/CrcFileSystem.h>
#if defined(TT_PLATFORM_TWL)
	#include <tt/fs/TwlArchiveFileSystem.h>
#else
	#include <tt/fs/StdFileSystem.h>
#endif
#include <tt/fs/File.h>


//--------------------------------------------------------------------------------------------------
// Helper functions


static const s32 s32Value = -123456789;
static const u32 u32Value =  123456789;
static const s16 s16Value = -9876;
static const u16 u16Value =  9876;
static const s8  s8Value  = -123;
static const u8  u8Value  =  123;

static const tt::fs::size_type ttTypeValuesSize = 14; //!< sizeof all the values above.

static const u8 smallByteArrayValue[] = 
{  5, 143, 201, 116,   8, 147,  96,  51};
static const tt::fs::size_type smallByteArrayValueSize = sizeof(smallByteArrayValue);

static const u8 byteArrayValue[] = 
{  5, 143, 201, 116,   8, 147,  96,  51, 
  52,  73,  21,  87,  36,  89, 154,  32, 
 135,  22,  33,   4,  76, 253, 251, 132, 
  18,  54,  32, 216,  87,  51,  23,  34, 
  86, 151,  95,  65, 211,  57,  65, 222};
static const tt::fs::size_type byteArrayValueSize = sizeof(byteArrayValue);

static u32 guardValue = 753159426;

static const tt::fs::size_type expectedFileSize 
		= ttTypeValuesSize        + sizeof(guardValue) + 
		  smallByteArrayValueSize + sizeof(guardValue) + 
		  byteArrayValueSize      + sizeof(guardValue);


// FIXME: Find a better place to dump these files. (tmp dir?)
#if defined(TT_PLATFORM_WIN)
static const char* testFilePath = "bin/fs_unittest.bin";
#elif defined(TT_PLATFORM_TWL)
static const char* testFilePath = "dataPub:/fs_unittest.bin";
#else
static const char* testFilePath = "fs_unittest.bin";
#endif


// Helper functions to use the same test on multiple filesystems.


static void TestFSWrite(tt::fs::identifier p_fsIdToTest)
{
	CHECK(tt::fs::createSaveRootDir(p_fsIdToTest));
	
	tt::fs::FilePtr testFilePtr = 
			tt::fs::open(testFilePath, tt::fs::OpenMode_Write, p_fsIdToTest);
	
	CHECK(testFilePtr != 0);
#if !defined(TT_BUILD_FINAL)
	CHECK_EQUAL(testFilePath, testFilePtr->getPath());
#endif
	
	CHECK_EQUAL(tt::fs::pos_type(0), testFilePtr->getPosition());
	CHECK( tt::fs::writeInteger(testFilePtr, s32Value) );
	CHECK_EQUAL(tt::fs::pos_type(sizeof(s32Value)), testFilePtr->getPosition());
	CHECK( tt::fs::writeInteger(testFilePtr, u32Value) );
	CHECK( tt::fs::writeInteger(testFilePtr, s16Value) );
	CHECK( tt::fs::writeInteger(testFilePtr, u16Value) );
	CHECK( tt::fs::writeInteger(testFilePtr, s8Value ) );
	CHECK( tt::fs::writeInteger(testFilePtr, u8Value ) );
	
	CHECK_EQUAL(ttTypeValuesSize, testFilePtr->getLength());
	
	CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	
	CHECK_EQUAL(smallByteArrayValueSize, tt::fs::write(testFilePtr, smallByteArrayValue, smallByteArrayValueSize) );
	
	CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	
	CHECK_EQUAL(byteArrayValueSize, tt::fs::write(testFilePtr, byteArrayValue, byteArrayValueSize) );
	
	CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	
	CHECK_EQUAL(expectedFileSize, testFilePtr->getLength());
}


static void TestFSRead(tt::fs::identifier p_fsIdToTest)
{
	tt::fs::FilePtr testFilePtr = tt::fs::open(testFilePath, tt::fs::OpenMode_Read, p_fsIdToTest);
	
	CHECK(testFilePtr != 0);
#if !defined(TT_BUILD_FINAL)
	CHECK_EQUAL(testFilePath, testFilePtr->getPath());
#endif
	CHECK_EQUAL(expectedFileSize, testFilePtr->getLength());
	CHECK_EQUAL(tt::fs::pos_type(0), testFilePtr->getPosition());
	
	{
		s32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s32Value, scratch);
	}
	
	CHECK_EQUAL(tt::fs::pos_type(sizeof(s32Value)), testFilePtr->getPosition());
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(u32Value, scratch);
	}
	
	{
		s16 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s16Value, scratch);
	}
	
	{
		u16 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(u16Value, scratch);
	}
	
	{
		s8 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s8Value, scratch);
	}
	
	{
		u8 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(u8Value, scratch);
	}
	
	CHECK_EQUAL(ttTypeValuesSize, testFilePtr->getPosition());
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	
	{
		u8 scratch[smallByteArrayValueSize] = { 0 };
		CHECK_EQUAL(smallByteArrayValueSize, tt::fs::read(testFilePtr, scratch, smallByteArrayValueSize) );
		CHECK_ARRAY_EQUAL( smallByteArrayValue, scratch, smallByteArrayValueSize );
	}
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	
	{
		u8 scratch[byteArrayValueSize] = { 0 };
		CHECK_EQUAL(byteArrayValueSize, tt::fs::read(testFilePtr, scratch, byteArrayValueSize) );
		CHECK_ARRAY_EQUAL( byteArrayValue, scratch, byteArrayValueSize );
	}
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
}


static void TestFSWriteRead(tt::fs::identifier p_fsIdToTest)
{
	TestFSWrite(p_fsIdToTest);
	TestFSRead(p_fsIdToTest);
}


static void TestFSSeek(tt::fs::identifier p_fsIdToTest)
{
	// Create test file.
	TestFSWrite(p_fsIdToTest);
	
	tt::fs::FilePtr testFilePtr = tt::fs::open(testFilePath, tt::fs::OpenMode_Read, p_fsIdToTest);
	
	CHECK(testFilePtr != 0);
#if !defined(TT_BUILD_FINAL)
	CHECK_EQUAL(testFilePath, testFilePtr->getPath());
#endif
	CHECK_EQUAL(expectedFileSize, testFilePtr->getLength());
	CHECK_EQUAL(tt::fs::pos_type(0), testFilePtr->getPosition());
	
	{
		s32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s32Value, scratch);
	}
	
	CHECK_EQUAL(tt::fs::pos_type(sizeof(s32Value)), testFilePtr->getPosition());
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(u32Value, scratch);
	}
	
	// Do some seek testing while staying within the internal buffer of BufferedFileSystem.
	
	// SeekPos_Cur Test (Stay within buffer of BufferedFileSystem.)
	tt::fs::pos_type oldPos = testFilePtr->getPosition();
	CHECK( testFilePtr->seek(sizeof(s16) + sizeof(u16), tt::fs::SeekPos_Cur) );
	CHECK_EQUAL( oldPos + static_cast<tt::fs::pos_type>(sizeof(s16) + sizeof(u16)), testFilePtr->getPosition());
	
	// Seek by-passed the s16 and u16.
	
	{
		s8 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s8Value, scratch);
	}
	
	// SeekPos_Set Test (Stay within buffer of BufferedFileSystem.)
	CHECK( testFilePtr->seek(ttTypeValuesSize, tt::fs::SeekPos_Set) );
	CHECK_EQUAL( ttTypeValuesSize, testFilePtr->getPosition() );
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	
	{
		u8 scratch[smallByteArrayValueSize] = { 0 };
		CHECK_EQUAL(smallByteArrayValueSize, tt::fs::read(testFilePtr, scratch, smallByteArrayValueSize) );
		CHECK_ARRAY_EQUAL( smallByteArrayValue, scratch, smallByteArrayValueSize );
	}
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	
	{
		u8 scratch[byteArrayValueSize] = { 0 };
		const tt::fs::size_type moveBackAmount = 8;
		const tt::fs::size_type byteArraySizeMinus = byteArrayValueSize - moveBackAmount ;
		CHECK_EQUAL(byteArraySizeMinus , tt::fs::read(testFilePtr, scratch, byteArraySizeMinus ) );
		CHECK_ARRAY_EQUAL( byteArrayValue, scratch, byteArraySizeMinus );
		
		CHECK_EQUAL(static_cast<tt::fs::pos_type>(testFilePtr->getLength() - sizeof(u32) - moveBackAmount),
		            testFilePtr->getPosition() );
		
		// Read a bit to fill BufferedFileSystem buffer.
		const tt::fs::size_type readAmount = 4;
		TT_STATIC_ASSERT(readAmount < moveBackAmount);
		
		CHECK_EQUAL(readAmount, tt::fs::read(testFilePtr, scratch, readAmount) );
		CHECK_ARRAY_EQUAL( byteArrayValue + byteArraySizeMinus, scratch, readAmount );
		
		CHECK_EQUAL(static_cast<tt::fs::pos_type>(testFilePtr->getLength() - sizeof(u32) - moveBackAmount + readAmount),
		            testFilePtr->getPosition() );
	}
	
	// SeekPos_End Test (Stay within buffer of BufferedFileSystem.)
	CHECK( testFilePtr->seek( -static_cast<tt::fs::pos_type>(sizeof(u32)), tt::fs::SeekPos_End) );
	CHECK_EQUAL(static_cast<tt::fs::pos_type>(testFilePtr->getLength() - sizeof(u32)),
	            testFilePtr->getPosition() );
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}

	CHECK_EQUAL(static_cast<tt::fs::pos_type>(testFilePtr->getLength()),
	            testFilePtr->getPosition() );
	
	/// End of file ----
	
	// Do some more tests.
	// This time make sure the BufferedFileSystem buffer is filled, 
	// and than seek past the buffer.
	
	// SeekPos_Set Test (by pass buffer of BufferedFileSystem.)
	CHECK( testFilePtr->seek( sizeof(s32), tt::fs::SeekPos_Set) );
	CHECK_EQUAL( static_cast<tt::fs::pos_type>(sizeof(s32)), testFilePtr->getPosition());
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(u32Value, scratch);
	}
	
	// Current position is sizeof(s32) + sizeof(u32).
	
	// File has the following layout:
	// ttTypeValuesSize        + sizeof(guardValue) + 
	// smallByteArrayValueSize + sizeof(guardValue) + 
	// byteArrayValueSize      + sizeof(guardValue);
	
	// SeekPos_Cur Test. (by Pass buffer of BufferedFileSystem.)
	CHECK( testFilePtr->seek( ttTypeValuesSize        + sizeof(guardValue) + 
	                          smallByteArrayValueSize + sizeof(guardValue) + 
	                          byteArrayValueSize
	                          
	                          // Correction for the starting position.
	                          - sizeof(s32) - sizeof(u32), 
	                          tt::fs::SeekPos_Cur));
	CHECK_EQUAL( static_cast<tt::fs::pos_type>(ttTypeValuesSize        + sizeof(guardValue) + 
	                                           smallByteArrayValueSize + sizeof(guardValue) +
	                                           byteArrayValueSize),
	             testFilePtr->getPosition());
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	
	// SeekPos_End Test. (by Pass buffer of BufferedFileSystem.)
	CHECK( testFilePtr->seek( -static_cast<tt::fs::pos_type>(sizeof(u32)), tt::fs::SeekPos_End) );
	CHECK_EQUAL(static_cast<tt::fs::pos_type>(testFilePtr->getLength() - sizeof(u32)),
	            testFilePtr->getPosition() );
	
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	
	CHECK_EQUAL(static_cast<tt::fs::pos_type>(testFilePtr->getLength()),
	            testFilePtr->getPosition() );
}


//--------------------------------------------------------------------------------------------------
// The tests


SUITE(tt_fs)
{

struct SetupStdFSFixture
{
	SetupStdFSFixture()
	:
	stdFSID(0),
#if defined(TT_PLATFORM_TWL)
	stdFSPtr(tt::fs::TwlArchiveFileSystem::instantiate(stdFSID, FS_DMA_NOT_USE, false))
#else
	stdFSPtr(tt::fs::StdFileSystem::instantiate(stdFSID))
#endif
	{}
	
	const tt::fs::identifier stdFSID;
	const tt::fs::FileSystemPtr stdFSPtr;
private:
	const SetupStdFSFixture& operator=(const SetupStdFSFixture& p_rhs); // Not implemented.
};


TEST_FIXTURE( SetupStdFSFixture, StdFileSystem_Instantiate )
{
	CHECK(stdFSPtr != 0);
	CHECK_EQUAL(stdFSID, stdFSPtr->getIdentifier());
}


struct StdFSSupportFixture : public SetupStdFSFixture
{
	StdFSSupportFixture()
	:
	stdFSSupportsSaving(true),
	stdFSSupportsAsync(false),
	stdFSSupportsDirectories(false),
	stdFSSupportsCommitting(false)
	{}
	
	const bool stdFSSupportsSaving;
	const bool stdFSSupportsAsync;
	const bool stdFSSupportsDirectories;
	const bool stdFSSupportsCommitting;
};


TEST_FIXTURE( StdFSSupportFixture, StdFileSystem_Supports )
{
	CHECK_EQUAL(stdFSSupportsSaving,      stdFSPtr->supportsSaving());
	CHECK_EQUAL(stdFSSupportsAsync,       stdFSPtr->supportsAsync());
	CHECK_EQUAL(stdFSSupportsDirectories, stdFSPtr->supportsDirectories());
	CHECK_EQUAL(stdFSSupportsCommitting,  stdFSPtr->supportsCommitting());
}


TEST_FIXTURE( SetupStdFSFixture, StdFS_WriteRead )
{
	TestFSWriteRead(stdFSID);
}


TEST_FIXTURE( SetupStdFSFixture, StdFS_Seek )
{
	TestFSSeek(stdFSID);
}

struct BufferedFileSystemFixture : public StdFSSupportFixture
{
	BufferedFileSystemFixture()
	:
	bufferedFSID(1),
	bufferedSize(32),
	bufferedFSPtr(tt::fs::BufferedFileSystem::instantiate(bufferedFSID, stdFSID, bufferedSize))
	{}
	
	const tt::fs::identifier    bufferedFSID;
	const tt::fs::size_type     bufferedSize;
	const tt::fs::FileSystemPtr bufferedFSPtr;
};


TEST_FIXTURE( BufferedFileSystemFixture, StdFileSystem_Instantiate )
{
	CHECK(bufferedFSPtr != 0);
	CHECK_EQUAL(bufferedFSID, bufferedFSPtr->getIdentifier());
}


TEST_FIXTURE( BufferedFileSystemFixture, StdFileSystem_Support )
{
	// The following support should mirror the source fs.
	CHECK_EQUAL(stdFSSupportsSaving,      bufferedFSPtr->supportsSaving());
	CHECK_EQUAL(stdFSSupportsDirectories, bufferedFSPtr->supportsDirectories());
	
	// The following features aren't supported.
	CHECK_EQUAL(false, bufferedFSPtr->supportsAsync());
	CHECK_EQUAL(false, bufferedFSPtr->supportsCommitting());
}


TEST_FIXTURE( BufferedFileSystemFixture, BufferedFileSystemRead )
{
	TestFSWrite(stdFSID);
	TestFSRead(bufferedFSID);
}


TEST_FIXTURE( BufferedFileSystemFixture, BufferedFileSystemWriteRead )
{
	TestFSWriteRead(bufferedFSID);
}


TEST_FIXTURE( BufferedFileSystemFixture, BufferedFileSystemSeek )
{
	TestFSSeek(bufferedFSID);
}

#if 0
TEST_FIXTURE( BufferedFileSystemFixture, BufferedFileSystemMixedWritesAndReads )
{
	// Create standard test file.
	TestFSWrite(bufferedFSID);
	
	// Place this in a variable so its easier to move into a function in the future.
	const tt::fs::identifier p_fsIdToTest = bufferedFSID;
	
	CHECK(tt::fs::createSaveRootDir(p_fsIdToTest));
	
	tt::fs::FilePtr testFilePtr = 
		tt::fs::open(testFilePath, tt::fs::OpenMode_Append, p_fsIdToTest);
	
	CHECK(testFilePtr != 0);
#if !defined(TT_BUILD_FINAL)
	CHECK_EQUAL(testFilePath, testFilePtr->getPath());
#endif
	
	CHECK_EQUAL(expectedFileSize, testFilePtr->getLength());
	CHECK_EQUAL(tt::fs::pos_type(testFilePtr->getLength()), testFilePtr->getPosition());
	
	CHECK(testFilePtr->seekToBegin());
	CHECK_EQUAL(tt::fs::pos_type(0), testFilePtr->getPosition());

	CHECK( tt::fs::writeInteger(testFilePtr, s32Value) );
	CHECK_EQUAL(tt::fs::pos_type(sizeof(s32Value)), testFilePtr->getPosition());
	
	// Replaced the write with a read. (still move the same amount.)
	//CHECK( tt::fs::writeInteger(testFilePtr, u32Value) );
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(u32Value, scratch);
	}
	
	// Replaced the write with a read. (still move the same amount.)
	//CHECK( tt::fs::writeInteger(testFilePtr, s16Value) );
	{
		s16 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s16Value, scratch);
	}
	
	CHECK( tt::fs::writeInteger(testFilePtr, u16Value) );
	
	// Replaced the write with a read. (still move the same amount.)
	//CHECK( tt::fs::writeInteger(testFilePtr, s8Value ) );
	{
		s8 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(s8Value, scratch);
	}
	
	CHECK( tt::fs::writeInteger(testFilePtr, u8Value ) );
	
	CHECK_EQUAL(ttTypeValuesSize, testFilePtr->getLength());
	
	CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	
	CHECK_EQUAL(smallByteArrayValueSize, tt::fs::write(testFilePtr, smallByteArrayValue, smallByteArrayValueSize) );
	
	CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	
	CHECK_EQUAL(byteArrayValueSize, tt::fs::write(testFilePtr, byteArrayValue, byteArrayValueSize) );
	
	CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	
	CHECK_EQUAL(expectedFileSize, testFilePtr->getLength());
}
#endif


TEST_FIXTURE( BufferedFileSystemFixture, BufferedFileSystemWriteException )
{
	// Place this in a variable so its easier to move into a function in the future.
	const tt::fs::identifier p_fsIdToTest = bufferedFSID;
	
	CHECK(tt::fs::createSaveRootDir(p_fsIdToTest));
	
	tt::fs::FilePtr testFilePtr = 
		tt::fs::open(testFilePath, tt::fs::OpenMode_Write, p_fsIdToTest);
	
	CHECK(testFilePtr != 0);
#if !defined(TT_BUILD_FINAL)
	CHECK_EQUAL(testFilePath, testFilePtr->getPath());
#endif
	
	CHECK_EQUAL(tt::fs::pos_type(0), testFilePtr->getPosition());

	// assume BufferedFileSystemFixture buffer size is 32
	for(s32 i = 0; i < 7; ++i)
	{
		// 7 x sizeof(u32) = 7 x 4 = 28
		CHECK( tt::fs::writeInteger(testFilePtr, guardValue) );
	}
	// smallByteValueSize = 8
	// 28 + 8 = 36  --  going over the buffer size with one write.
	CHECK_EQUAL(smallByteArrayValueSize, tt::fs::write(testFilePtr, smallByteArrayValue, smallByteArrayValueSize) );
	
	
	testFilePtr.reset();
	
	testFilePtr = 
		tt::fs::open(testFilePath, tt::fs::OpenMode_Read, p_fsIdToTest);
	
	for(s32 i = 0; i < 7; ++i)
	{
		u32 scratch = 0;
		CHECK( tt::fs::readInteger(testFilePtr, &scratch) );
		CHECK_EQUAL(guardValue, scratch);
	}
	{
		u8 scratch[smallByteArrayValueSize] = { 0 };
		CHECK_EQUAL(smallByteArrayValueSize, tt::fs::read(testFilePtr, scratch, smallByteArrayValueSize) );
		CHECK_ARRAY_EQUAL( smallByteArrayValue, scratch, smallByteArrayValueSize );
	}
}


struct CrcFileSystemFixture : public StdFSSupportFixture
{
	CrcFileSystemFixture()
	:
	crcFSID(2),
	crcFSPtr(tt::fs::CrcFileSystem::instantiate(crcFSID, stdFSID))
	{}
	
	const tt::fs::identifier    crcFSID;
	const tt::fs::FileSystemPtr crcFSPtr;
};


TEST_FIXTURE( CrcFileSystemFixture, CrcFileSystemWriteRead )
{
	TestFSWriteRead(crcFSID);
}

TEST_FIXTURE( CrcFileSystemFixture, CrcFileSystemSeek )
{
	TestFSSeek(crcFSID);
}


struct CrcBufferedFileSystemFixture : public BufferedFileSystemFixture
{
	CrcBufferedFileSystemFixture()
	:
	crcFSID(2),
	crcFSPtr(tt::fs::CrcFileSystem::instantiate(crcFSID, bufferedFSID))
	{}
	
	const tt::fs::identifier    crcFSID;
	const tt::fs::FileSystemPtr crcFSPtr;
};


TEST_FIXTURE( CrcBufferedFileSystemFixture, CrcBufferedFileSystemWriteRead )
{
	TestFSWriteRead(crcFSID);
}

TEST_FIXTURE( CrcBufferedFileSystemFixture, CrcBufferedFileSystemSeek )
{
	TestFSSeek(crcFSID);
}


TEST_FIXTURE( StdFSSupportFixture, StdFileSystem_CompactPath )
{
	const std::string      rawPath("./dir/../dir//bla\\bloep");
	const std::string expectedPath("dir/bla/bloep");
	
	std::string result = tt::fs::utils::compactPath(rawPath, "/\\");
	
	CHECK_EQUAL(expectedPath, result);
	
	result = tt::fs::utils::compactPath(rawPath + "/", "/\\");
	
	CHECK_EQUAL(expectedPath + "/", result);
	
	result = tt::fs::utils::compactPath("/" + rawPath, "/\\");
	
	CHECK_EQUAL("/" + expectedPath, result);
}

// End SUITE
}
