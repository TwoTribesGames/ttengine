#ifndef INC_TT_FS_STDFILESYSTEM_H
#define INC_TT_FS_STDFILESYSTEM_H

#include <tt/fs/types.h>
#include <tt/fs/FileSystem.h>


namespace tt {
namespace fs {

class StdFileSystem : public FileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier);
	virtual ~StdFileSystem();
	
	// Feature support check functions
	virtual bool supportsSaving();
	
	// Basic file functions
	virtual bool      open (const FilePtr& p_file, const std::string& p_path, OpenMode p_mode);
	virtual bool      close(File*          p_file);
	virtual size_type read (const FilePtr& p_file, void* p_buffer, size_type p_length);
	virtual size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length);
	virtual bool      flush(const FilePtr& p_file);
	
	// Position / size functions
	virtual bool      seek       (const FilePtr& p_file, pos_type p_offset, SeekPos p_position);
	virtual pos_type  getPosition(const FilePtr& p_file);
	virtual size_type getLength  (const FilePtr& p_file);
	
	// Misc functions
	virtual bool fileExists(const std::string& p_path);
	
private:
	StdFileSystem(identifier p_id);
	
	StdFileSystem(const StdFileSystem& p_rhs);
	StdFileSystem& operator=(const StdFileSystem& p_rhs);
};

// namespace end
}
}

#endif // INC_TT_FS_STDFILESYSTEM_H
