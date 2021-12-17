#ifndef INC_TT_FS_STEAMFILESYSTEM_H
#define INC_TT_FS_STEAMFILESYSTEM_H

#if !defined(TT_PLATFORM_OSX_IPHONE)


#include <steam/steam_api.h>

#include <tt/fs/types.h>
#include <tt/fs/FileSystem.h>


namespace tt {
namespace fs {

class SteamFileSystem : public FileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier, ISteamRemoteStorage* p_fs);
	
	virtual ~SteamFileSystem();
	
	
	// Feature support check functions
	
	virtual bool supportsSaving();
	virtual bool supportsDirectories();  // NOTE: So that openDir can be used, but only on root 'dir'
	
	
	// basic file functions
	
	virtual bool      open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode);
	virtual bool      close(File* p_file);
	virtual size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length);
	virtual size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length);
	
	
	// Position / size functions
	
	virtual bool      seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position);
	virtual pos_type  getPosition(const FilePtr& p_file);
	virtual size_type getLength(const FilePtr& p_file);
	
	
	// Directory functions
	
	virtual bool openDir(DirPtr& p_dir, const std::string& p_path, const std::string& p_filter);
	virtual bool closeDir(Dir* p_dir);
	virtual bool readDir(const DirPtr& p_dir, DirEntry& p_entry);
	
	// Create / Destroy functions
	
	virtual bool fileExists(const std::string& p_path);
	virtual bool destroyFile(const std::string& p_path);
	
	
private:
	SteamFileSystem(identifier p_id, ISteamRemoteStorage* p_fs);
	
	SteamFileSystem(const SteamFileSystem& p_rhs);
	SteamFileSystem& operator=(const SteamFileSystem& p_rhs);
	
	
	ISteamRemoteStorage* const m_fs;
};

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)

#endif // INC_TT_FS_STEAMFILESYSTEM_H
