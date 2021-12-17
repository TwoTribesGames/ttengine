#if !defined(TT_PLATFORM_OSX_IPHONE)

#include <algorithm>

#include <steam/steam_api.h>

#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/File.h>
#include <tt/fs/SteamFileSystem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/thread/thread.h>


namespace tt {
namespace fs {

enum
{
	Retry_Count = 20,  // try 20 times
	Retry_Sleep = 250  // wait 250 ms after each try
};


struct SteamInternal
{
	std::string m_path;       //!< Path of file
	u8*         m_data;       //!< Buffer containing filedata
	size_type   m_size;       //!< File size
	size_type   m_bufferSize; //!< Buffer size
	pos_type    m_offset;     //!< Current file pointer position
	bool        m_write;      //!< Whether file was opened in read mode
	
	
	inline SteamInternal(const std::string& p_path, bool p_write)
	:
	m_path(p_path),
	m_data(0),
	m_size(0),
	m_bufferSize(0),
	m_offset(0),
	m_write(p_write)
	{
	}
	
	inline ~SteamInternal()
	{
		delete[] m_data;
	}
	
	void reserve(size_type p_size)
	{
		while (m_offset + p_size > m_bufferSize)
		{
			m_bufferSize += 1024;
			u8* data = new u8[m_bufferSize];
			mem::copy8(data, m_data, m_size);
			delete[] m_data;
			m_data = data;
		}
	}
	
	size_type write(const void* p_data, size_type p_size)
	{
		reserve(p_size);
		mem::copy8(m_data + m_offset, p_data, p_size);
		m_offset += p_size;
		m_size = std::max(m_size, m_offset);
		return p_size;
	}
	
	size_type read(void* p_buffer, size_type p_size)
	{
		size_type toRead = std::min(p_size, m_size - m_offset);
		mem::copy8(p_buffer, m_data + m_offset, toRead);
		m_offset += toRead;
		return toRead;
	}
	
	void seek(pos_type p_offset, SeekPos p_position)
	{
		switch (p_position)
		{
		case SeekPos_Set: m_offset = std::max(0, std::min(m_size, p_offset));            break;
		case SeekPos_Cur: m_offset = std::max(0, std::min(m_size, m_offset + p_offset)); break;
		case SeekPos_End: m_offset = std::max(0, std::min(m_size, m_size + p_offset));   break;
		default: break;
		}
	}
	
private:
	// No copying
	SteamInternal(const SteamInternal&);
	SteamInternal& operator=(const SteamInternal&);
};


struct SteamDirInternal
{
	inline SteamDirInternal(const std::string& p_filter, int32 p_fileCount)
	:
	filter(p_filter),
	fileCountAtOpen(p_fileCount),
	currentFileIndex(0)
	{
	}
	
	const std::string filter;           //!< The file pattern to filter results with
	const int32       fileCountAtOpen;  //!< File count at the instant of opening the dir (used for sanity checking)
	int32             currentFileIndex;
	
private:
	// No copying
	SteamDirInternal(const SteamDirInternal&);
	SteamDirInternal& operator=(const SteamDirInternal&);
};


//--------------------------------------------------------------------------------------------------
// Public member functions

FileSystemPtr SteamFileSystem::instantiate(fs::identifier p_identifier, ISteamRemoteStorage* p_fs)
{
	FileSystemPtr filesys(new SteamFileSystem(p_identifier, p_fs));
	if (filesys == 0)
	{
		TT_PANIC("Failed to instantiate filesystem.");
		return filesys;
	}
	
	if (fs::registerFileSystem(filesys.get(), p_identifier) == false)
	{
		TT_PANIC("Failed to register filesytem.");
		filesys.reset();
	}
	
	return filesys;
}


SteamFileSystem::~SteamFileSystem()
{
}


// Feature support check functions

bool SteamFileSystem::supportsSaving()
{
	return true;
}


bool SteamFileSystem::supportsDirectories()
{
	return true;
}


bool SteamFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (p_file->getData() != 0)
	{
		TT_PANIC("File already open.");
		return false;
	}
	
	std::string path(p_path);
	if (m_fs == 0)
	{
		path = fs::utils::compactPath(fs::getApplicationDir() + p_path, "\\/");
	}
	
	// parse open mode
	bool readMode  = false;
	bool writeMode = false;
	
	switch (p_mode & (~OpenMode_AtEnd))
	{
	case OpenMode_Read:
		readMode = true;
		break;
		
	case OpenMode_Write:
	case OpenMode_Write | OpenMode_Truncate:
		writeMode = true;
		break;
		
	case OpenMode_Append:
		p_mode = static_cast<OpenMode>(static_cast<int>(p_mode) | OpenMode_AtEnd);
		// FALL THROUGH
	case OpenMode_Write | OpenMode_Append:
		readMode  = true;
		writeMode = true;
		break;
		
	case OpenMode_Read | OpenMode_Write:
	case OpenMode_Read | OpenMode_Write | OpenMode_Append:
		readMode  = true;
		writeMode = true;
		break;
		
	case OpenMode_Read | OpenMode_Write | OpenMode_Truncate:
		writeMode = true;
		break;
		
	default:
		TT_PANIC("Unsupported open mode.");
		return false;
	}
	
	if (readMode && (writeMode == false))
	{
		if (m_fs != 0)
		{
			if (m_fs->FileExists(path.c_str()) == false)
			{
				TT_PANIC("File '%s' does not exist.", path.c_str());
				return false;
			}
		}
		else
		{
			if (fs::fileExists(path) == false)
			{
				TT_PANIC("File '%s' does not exist.", path.c_str());
				return false;
			}
		}
	}
	
	SteamInternal* intern = new SteamInternal(path, writeMode);
	
	if (readMode)
	{
		if (m_fs != 0)
		{
			int tries = 0;
			size_type size = m_fs->GetFileSize(path.c_str());
			intern->reserve(size);
			intern->m_size = size;
			while (m_fs->FileRead(path.c_str(), intern->m_data, size) != size)
			{
				intern->m_size = 0;
				++tries;
				if (tries >= Retry_Count)
				{
					TT_PANIC("Failed to readMode from file '%s'.", intern->m_path.c_str());
					break;
				}
				TT_Printf("Attempt %2d at reading '%s' failed, retry in %d ms\n", tries, intern->m_path.c_str(), Retry_Sleep * tries);
				tt::thread::sleep(Retry_Sleep * tries);
				intern->m_size = size;
			}
		}
		else
		{
			code::BufferPtr buffer = fs::getFileContent(path);
			intern->reserve(buffer->getSize());
			mem::copy8(intern->m_data, buffer->getData(), buffer->getSize());
			intern->m_size = buffer->getSize();
		}
	}
	
	p_file->setData(reinterpret_cast<void*>(intern));
	
	if ((p_mode & OpenMode_AtEnd) != 0)
	{
		intern->m_offset = intern->m_size;
	}
	return true;
}


bool SteamFileSystem::close(File* p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		TT_WARN("File not open.");
		return false;
	}
	
	SteamInternal* intern = reinterpret_cast<SteamInternal*>(p_file->getData());
	bool success = true;
	if (intern->m_write)
	{
		if (m_fs != 0)
		{
			int tries = 0;
			while (m_fs->FileWrite(intern->m_path.c_str(), intern->m_data, intern->m_size) == false)
			{
				++tries;
				if (tries >= Retry_Count)
				{
					TT_PANIC("Failed to write to file '%s'.", intern->m_path.c_str());
					success = false;
					break;
				}
				TT_Printf("Attempt %2d at writing '%s' failed, retry in %d ms\n", tries, intern->m_path.c_str(), Retry_Sleep * tries);
				tt::thread::sleep(Retry_Sleep * tries);
			}
		}
		else
		{
			// ensure the folder exists
			std::string path(intern->m_path);
			
			if (fs::fileExists(path) == false)
			{
				// see if needed directories exists, if not, create them
				std::string::size_type pos = path.find(fs::getDirSeparator());
				while (pos != std::string::npos)
				{
					std::string dir = path.substr(0, pos);
					if (fs::dirExists(dir) == false)
					{
						fs::createDir(dir);
					}
					pos = path.find(fs::getDirSeparator(), pos + 1);
				}
			}
			fs::FilePtr file = fs::open(path, fs::OpenMode_Write);
			file->write(intern->m_data, intern->m_size);
		}
	}
	
	delete intern;
	p_file->setData(0);
	
	return success;
}


size_type SteamFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	if (p_length == 0)
	{
		return p_length;
	}
	
	SteamInternal* intern = reinterpret_cast<SteamInternal*>(p_file->getData());
	
	return intern->read(p_buffer, p_length);
}


size_type SteamFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	if (p_length == 0)
	{
		return p_length;
	}
	
	SteamInternal* intern = reinterpret_cast<SteamInternal*>(p_file->getData());
	
	return intern->write(p_buffer, p_length);
}


bool SteamFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	SteamInternal* intern = reinterpret_cast<SteamInternal*>(p_file->getData());
	intern->seek(p_offset, p_position);
	
	return true;
}


pos_type SteamFileSystem::getPosition(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return pos_type();
	}
	
	SteamInternal* intern = reinterpret_cast<SteamInternal*>(p_file->getData());
	
	return intern->m_offset;
}


size_type SteamFileSystem::getLength(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	SteamInternal* intern = reinterpret_cast<SteamInternal*>(p_file->getData());
	
	return intern->m_size;
}


bool SteamFileSystem::openDir(DirPtr& p_dir, const std::string& p_path, const std::string& p_filter)
{
	if (p_dir == 0)
	{
		TT_PANIC("No dir specified.");
		return false;
	}
	
	if (p_path.empty() == false)
	{
		TT_PANIC("Can only open (unnamed) root directory on Steam FS.");
		return 0;
	}
	
	if (p_dir->getData() != 0)
	{
		TT_PANIC("Dir already open.");
		return false;
	}
	
	SteamDirInternal* intern = new SteamDirInternal(p_filter, m_fs->GetFileCount());
	p_dir->setData(reinterpret_cast<void*>(intern));
	
	return true;
}


bool SteamFileSystem::closeDir(Dir* p_dir)
{
	if (p_dir == 0)
	{
		TT_PANIC("No dir specified.");
		return false;
	}
	
	if (p_dir->getData() == 0)
	{
		TT_PANIC("Dir not open.");
		return false;
	}
	
	SteamDirInternal* intern = reinterpret_cast<SteamDirInternal*>(p_dir->getData());
	delete intern;
	p_dir->setData(0);
	
	return true;
}


bool SteamFileSystem::readDir(const DirPtr& p_dir, DirEntry& p_entry)
{
	if (validate(p_dir) == false)
	{
		return false;
	}
	
	SteamDirInternal* intern = reinterpret_cast<SteamDirInternal*>(p_dir->getData());
	
	const int32 fileCount = m_fs->GetFileCount();
	TT_ASSERTMSG(intern->fileCountAtOpen == fileCount,
	             "Steam FS file count changed between openDir (count %d) and readDir (count %d). "
	             "Results may be undefined.", intern->fileCountAtOpen, fileCount);
	
	// Find and read new directory entry
	for ( ; intern->currentFileIndex < fileCount; )
	{
		// Get basic entry details
		int32       fileSize = 0;
		const char* fileName = m_fs->GetFileNameAndSize(intern->currentFileIndex, &fileSize);
		
		// Increment current file index now, since it must be incremented
		// if filter doesn't match AND if correct entry was found
		intern->currentFileIndex++;
		
		// Apply filter to entry (skip any entries that do not match the filter)
		if (utils::matchesFilter(fileName, intern->filter) == false)
		{
			continue;
		}
		
		// Found an acceptabe entry; fill in the details and stop the search
		p_entry.clear();
		
		p_entry.setName(fileName);
		p_entry.setSize(static_cast<size_type>(fileSize));
		p_entry.setIsDirectory(false);
		p_entry.setIsHidden(false);
		
		// FIXME: Steam API only provides one time stamp: which entry stat should we map it to?
		const int64 timestamp = m_fs->GetFileTimestamp(fileName);
		p_entry.setCreationTime(timestamp);
		p_entry.setAccessTime(timestamp);
		p_entry.setWriteTime(timestamp);
		
		return true;
	}
	
	// Could not get new entry
	return false;
}


bool SteamFileSystem::fileExists(const std::string& p_path)
{
	if (p_path.empty())
	{
		return false;
	}
	
	if (m_fs != 0)
	{
		return m_fs->FileExists(p_path.c_str());
	}
	else
	{
		return fs::fileExists(fs::utils::compactPath(fs::getApplicationDir() + p_path, "\\/"));
	}
}


bool SteamFileSystem::destroyFile(const std::string& p_path)
{
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (m_fs != 0)
	{
		return m_fs->FileDelete(p_path.c_str());
	}
	else
	{
		return fs::destroyFile(fs::utils::compactPath(fs::getApplicationDir() + p_path, "\\/"));
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

SteamFileSystem::SteamFileSystem(fs::identifier p_id, ISteamRemoteStorage* p_fs)
:
FileSystem(p_id),
m_fs(p_fs)
{
	//TT_Printf("SteamFileSystem::SteamFileSystem: Initialized Steam file system with ID %d.\n", p_id);
	
	// NOTE: This check does not make sense: IsCloudEnabledForApp indicates if the user has enabled
	//       Steam Cloud for their account for this app, not whether the application is Cloud-enabled.
	//       The Cloud API will still work exactly the same, even if the user disabled Cloud for themselves.
	//TT_ASSERTMSG(m_fs == 0 || m_fs->IsCloudEnabledForApp(),
	//             "Application wants to use Steam Cloud, but the cloud is not enabled for this application.");
}

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)
