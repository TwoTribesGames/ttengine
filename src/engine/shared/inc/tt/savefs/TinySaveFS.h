#ifndef INC_SAVEFS_TINYSAVEFS_H
#define INC_SAVEFS_TINYSAVEFS_H

#include <tt/platform/tt_types.h>
#include <tt/savefs/CardTypes.h>

#include <list>


namespace tt {

// forward declarations
namespace fs
{
	struct File;
	struct FileSystem;
}
namespace math
{
	class CRC32;
}


namespace savefs {

// forward declaration
class CardErrorHandler;


class TinySaveFS
{
public:
	static bool init(u32               p_type,
	                 CardType          p_cardType,
	                 CardErrorHandler* p_handler,
	                 u32               p_baseFSType = 0);
	static bool end();
	
	static void enableFlush(bool p_enable);
	static void setAutoFlush(bool p_enable);
	static bool flush();
	
private:
	// memory node used for writing to a file in ram
	struct MemNode
	{
		u32      size;   //!< Size of memory block
		u32      start;  //!< File position corresponding to start of data
		u8*      data;   //!< Memory block
	};
	typedef std::list<MemNode> MemList;
	
	// forward declaration
	struct FileHeader;
	
	// file handle
	struct FileHandle
	{
		FileHeader* info;    //!< File information header (0 == closed)
		bool        write;   //!< Write access allowed
		bool        read;    //!< Read access allowed
		u32         size;    //!< Size of this file
		u32         pointer; //!< Current offset within file
		MemList     data;    //!< Memory list containing file data
	};
	typedef std::list<FileHandle> FileHandles;
	
	// file information from file table
	struct FileHeader
	{
		u32         nameCrc;  //!< Hash of filename
		u32         dataCrc;  //!< Hash of data
		u32         size;     //!< Size of data
		u32         start;    //!< Start of data
		u32         newsize;  //!< Updated size
		u32         newstart; //!< Updated start of data
		FileHandles files;    //!< List of open file handles
	};
	typedef std::list<FileHeader> Files;
	
	 TinySaveFS();
	~TinySaveFS();
	
	static bool isBusy(volatile const fs::File* p_file);
	static bool isSucceeded(volatile const fs::File* p_file);
	static bool openFile(fs::File* p_file, const char* p_path, const char* p_mode);
	static bool closeFile(fs::File* p_file);
	static s32  readFile(fs::File* p_file, void* p_dst, s32 p_len);
	static s32  writeFile(fs::File* p_file, const void* p_src, s32 p_len);
	static bool flush(fs::File* p_file);
	static void cancelFile(fs::File* p_file);
	static bool waitAsync(fs::File* p_file);
	static bool seekFile(fs::File* p_file, s32 p_offset, int origin);
	static u32  getLength(fs::File* p_file);
	static u32  getPosition(fs::File* p_file);
	static bool fileExists(const char* p_path);
	static u64  getCreationTime(fs::File* p_file);
	static u64  getLastWriteTime(fs::File* p_file);
	static u64  getLastAccessTime(fs::File* p_file);
	
	static u32  getCRC(const char* p_str);
	static void remapFiles();
	
	static fs::FileSystem* ms_fs;
	static u32             ms_type;
	static CardType        ms_cardType;
	static u32             ms_cardSize;
	static u8*             ms_mem;
	static u32             ms_freeMem;
	static math::CRC32*    ms_crc;
	static Files           ms_files;
	static bool            ms_autoFlush;
	static bool            ms_flushEnabled;
};

}
}


#endif // INC_SAVEFS_TINYSAVEFS_H
