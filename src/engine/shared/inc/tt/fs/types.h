#ifndef INC_TT_FS_TYPES_H
#define INC_TT_FS_TYPES_H

#include <tt/platform/tt_types.h>


namespace tt {
namespace fs {

// forward declaration
class File;
class FileSystem;
class Dir;
class DirEntry;
class MemoryArchive;


// shared pointers
typedef tt_ptr<File>::shared FilePtr;
typedef tt_ptr<FileSystem>::shared FileSystemPtr;
typedef tt_ptr<Dir>::shared DirPtr;
typedef tt_ptr<MemoryArchive>::shared MemoryArchivePtr;


// Typedefs
typedef int identifier;
typedef int size_type;
typedef int pos_type;
typedef long long time_type;
typedef int permission_type;

typedef void (*callback)(FilePtr p_file, void* p_arg);

// Enums
enum SeekPos
{
	SeekPos_Set, //!< Seek from start of the file
	SeekPos_Cur, //!< Seek from current location
	SeekPos_End  //!< Seek from end of the file
};

enum OpenMode
{
	OpenMode_Read      = 1 << 0, //!< Opens in read mode (file must exist)
	OpenMode_Write     = 1 << 1, //!< Opens in write mode (creates and empties file)
	OpenMode_Append    = 1 << 2, //!< Always appends at the end when writing (creates file)
	OpenMode_Truncate  = 1 << 3, //!< Removes the former file contents
	OpenMode_AtEnd     = 1 << 4, //!< Places file pointer at end of file after opening
	OpenMode_Text      = 1 << 5  //!< Replace special characters (windows only)
};

enum Permission
{
	Permission_OwnerRead    = 1 << 0, //!< Owner has read permission
	Permission_OwnerWrite   = 1 << 1, //!< Owner has write permission
	Permission_OwnerExecute = 1 << 2, //!< Owner has execute permission
	Permission_GroupRead    = 1 << 3, //!< Group has read permission
	Permission_GroupWrite   = 1 << 4, //!< Group has write permission
	Permission_GroupExecute = 1 << 5, //!< Group has execute permission
	Permission_OtherRead    = 1 << 6, //!< Other have read permission
	Permission_OtherWrite   = 1 << 7, //!< Other have write permission
	Permission_OtherExecute = 1 << 8  //!< Other have execute permission
};

inline bool hasPermission(permission_type p_type, Permission p_permission)
{
	return (p_type & p_permission) == p_permission;
}

// namespace end
}
}

#endif // INC_TT_FS_TYPES_H
