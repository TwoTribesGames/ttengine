#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>


namespace tt {
namespace fs {
namespace utils {

bool matchesFilter(const char* p_name, const char* p_filter);


bool matchesFilter(const std::string& p_name, const std::string& p_filter)
{
	return matchesFilter(p_name.c_str(), p_filter.c_str());
}


bool createDirRecursive(const std::string& p_dir, identifier p_identifier)
{
	std::string normalizedDir(compactPath(p_dir, "\\/", p_identifier));
	std::string dirSep(1, getDirSeparator(p_identifier));
	
	const bool pathStartsWithDirSep = str::startsWith(normalizedDir, dirSep);
	
	const str::Strings allDirectories(str::explode(normalizedDir, dirSep));
	
	normalizedDir.clear();
	if (pathStartsWithDirSep) normalizedDir = dirSep;
	for (str::Strings::const_iterator it = allDirectories.begin(); it != allDirectories.end(); ++it)
	{
		normalizedDir += *it + dirSep;
		
		if (dirExists(normalizedDir, p_identifier) == false)
		{
			// Directory path does not exist; attempt to create it
			if (createDir(normalizedDir, p_identifier) == false)
			{
				return false;
			}
		}
	}
	
	return true;
}


bool destroyDirRecursive(const std::string& p_dir, identifier p_identifier)
{
	bool result = clearDirRecursive(p_dir, p_identifier);
	return result && destroyDir(p_dir, p_identifier);
}


bool clearDirRecursive(const std::string& p_dir, identifier p_identifier)
{
	const std::string dirSep(1, getDirSeparator(p_identifier));
	DirPtr dir = openDir(p_dir);
	
	if (dir == 0)
	{
		return false;
	}
	
	DirEntry entry;
	bool result = true;
	while (dir->read(entry))
	{
		if (entry.getName() == "." || entry.getName() == "..")
		{
			continue;
		}
		
		const std::string fullPath(p_dir + dirSep + entry.getName());
		if (entry.isDirectory())
		{
			result = result && destroyDirRecursive(fullPath, p_identifier);
		}
		else
		{
			result = result && destroyFile(fullPath);
		}
	}
	
	return result;
}


// Internal raw string implementation (std::string version is for convenience)
bool matchesFilter(const char* p_name, const char* p_filter)
{
	TT_NULL_ASSERT(p_name);
	TT_NULL_ASSERT(p_filter);
	
	while (p_filter != 0 && *p_filter != 0)
	{
		// Check for end of name string
		if (*p_name == 0)
		{
			// Run through the rest of the filter
			for ( ;; )
			{
				if (*p_filter == 0)
				{
					// End of filter; name matches filter
					return true;
				}
				else if (*p_filter != '*')
				{
					// More characters were expected; no match
					return false;
				}
				
				++p_filter;
			}
		}
		
		if (*p_filter == '?')
		{
			// Matches any character
		}
		else if (*p_filter == '*')
		{
			++p_filter;
			for ( ;; )
			{
				// Try again for every substring left in the name
				if (matchesFilter(p_name, p_filter))
				{
					return true;
				}
				
				if (*p_name == 0)
				{
					return false;
				}
				
				++p_name;
			}
		}
		else if (*p_filter != *p_name)
		{
			// Simple character mismatch
			return false;
		}
		
		++p_filter;
		++p_name;
	}
	
	// End of the filter should be the end of the name
	return *p_name == 0;
}


std::string addSubdirToPath(const std::string& p_path, const std::string& p_subDir)
{
	TT_ASSERTMSG(p_path.find("\\") == std::string::npos,
	             "Found \\ in path: '%s'. Only use /!",
	             p_path.c_str());
	
	TT_ASSERTMSG(p_subDir.find("\\") == std::string::npos,
	             "Found \\ in sub dir: '%s'. Only use /!",
	             p_path.c_str());
	
	TT_ASSERTMSG(tt::str::startsWith(p_subDir, "/") == false ||
	             tt::str::endsWith(p_subDir, "/")   == false,
	             "Sub dir '%s' may not start or end with '/'!",
	             p_subDir.c_str());
	
	std::string::size_type pos = p_path.rfind("/");
	
	if (pos == std::string::npos)
	{
		// Not found
		return p_subDir + "/" + p_path; 
	}
	else
	{
		std::string dir  = p_path.substr(0, pos);
		std::string file = p_path.substr(pos + 1);
		
		return dir + "/" + p_subDir + "/" + file;
	}
}


std::string getExtension(const std::string& p_path, identifier /*p_identifier*/)
{
	if (p_path.empty())
	{
		return std::string();
	}
	
	// find last path separator
	std::string::size_type dirpos = p_path.find_last_of("\\/");
	std::string::size_type extpos = p_path.find_last_of('.');
	
	if (extpos == std::string::npos || (dirpos != std::string::npos && dirpos > extpos))
	{
		return std::string();
	}
	
	return p_path.substr(extpos + 1);
}


std::string getFileTitle(const std::string& p_path, identifier p_identifier)
{
	std::string filename(getFileName(p_path, p_identifier));
	std::string::size_type extpos = filename.find_last_of('.');
	
	if (extpos == std::string::npos)
	{
		return filename;
	}
	
	return filename.substr(0, extpos);
}


std::string getFileName(const std::string& p_path, identifier /*p_identifier*/)
{
	if (p_path.empty())
	{
		return std::string();
	}
	
	// find last path separator
	std::string::size_type dirpos = p_path.find_last_of("\\/");
	
	if (dirpos == std::string::npos)
	{
		return p_path;
	}
	
	std::string filename(p_path.substr(dirpos + 1));
	
	// check for . and .. special case
	if (filename == "." || filename == "..")
	{
		return std::string();
	}
	
	return filename;
}


time_type getFileCreationTime(const std::string& p_path, identifier p_identifier)
{
	FilePtr file = open(p_path, OpenMode_Read, p_identifier);
	if (file == 0)
	{
		TT_PANIC("Cannot open '%s' to get creation time. File doesn't exist or is in use.", p_path.c_str());
		return -1;
	}
	
	return file->getCreationTime();
}


time_type getFileAccessTime(const std::string& p_path, identifier p_identifier)
{
	FilePtr file = open(p_path, OpenMode_Read, p_identifier);
	if (file == 0)
	{
		TT_PANIC("Cannot open '%s' to get access time. File doesn't exist or is in use.", p_path.c_str());
		return -1;
	}
	
	return file->getAccessTime();
}


time_type getFileWriteTime(const std::string& p_path, identifier p_identifier)
{
	FilePtr file = open(p_path, OpenMode_Read, p_identifier);
	if (file == 0)
	{
		TT_PANIC("Cannot open '%s' to get write time. File doesn't exist or is in use.", p_path.c_str());
		return -1;
	}
	
	return file->getWriteTime();
}


std::string getLastDirectoryName(const std::string& p_path, identifier p_identifier)
{
	if (p_path.empty())
	{
		return std::string();
	}
	
	std::string path(p_path);
	
	// Strip trailing directory separators
	while (path.length() > 1 && (path.at(path.length() - 1) == '/' || path.at(path.length() - 1) == '\\'))
	{
		path.erase(path.length() - 1);
	}
	
	// now we can make use of the getFileName method to retrieve the directory name
	return getFileName(path, p_identifier);
}


std::string getDirectory(const std::string& p_path, identifier /*p_identifier*/)
{
	if (p_path.empty())
	{
		return std::string();
	}
	
	// find last path separator
	std::string::size_type dirpos = p_path.find_last_of("\\/");
	
	if (dirpos == std::string::npos)
	{
		return std::string();
	}
	
	return p_path.substr(0, dirpos + 1);
}


std::string getParentDirectory(const std::string& p_directory, identifier /*p_identifier*/)
{
	if (p_directory.empty())
	{
		return std::string();
	}
	
	std::string path(p_directory);
	
	// Strip trailing directory separators
	while (path.length() > 1 && (path.at(path.length() - 1) == '/' || path.at(path.length() - 1) == '\\'))
	{
		path.erase(path.length() - 1);
	}
	
	// find last path separator
	std::string::size_type dirpos = path.find_last_of("\\/");
	
	if (dirpos == std::string::npos)
	{
		return std::string();
	}
	
	return path.substr(0, dirpos + 1);
}


std::string compactPath(const std::string& p_path, const std::string& p_separators, identifier p_identifier)
{
	std::string path(p_path);
	const std::string::value_type sep = getDirSeparator(p_identifier);
	const std::string sepstr(1, sep);
	
	// Convert any separator characters in the path to the "native" separator type
	if (p_separators.empty() == false)
	{
		for (std::string::const_iterator it = p_separators.begin(); it != p_separators.end(); ++it)
		{
			if (*it == sep)
			{
				continue;
			}
			str::replace(path, std::string(1, *it), sepstr);
		}
	}
	
	// Remove any double separators.
	{
		const bool startWithSeparator = str::startsWith(path, sepstr);
		const bool endsWithSeparator = str::endsWith(path, sepstr);
		str::Strings pieces(str::explode(path, sepstr));
		path  = (startWithSeparator) ? sepstr : "";
		path += str::implode(pieces, sepstr);
		path += (endsWithSeparator)  ? sepstr : "";
	}
	
	// Strip off "current dir" characters (dir ".")
	for (;;)
	{
		while (path.find("." + sepstr) == 0)
		{
			path.erase(0, 2);
		}
		
		const std::string::size_type pos = path.find(sepstr + "." + sepstr);
		if (pos == std::string::npos)
		{
			break;
		}
		path.erase(pos + 1, 2);
	}
	
	for (;;)
	{
		// make sure the path does not begin with ".."
		if (path.find(".." + sepstr) == 0)
		{
			TT_PANIC("Attempt to compact invalid path '%s' (starts with ..).\n", p_path.c_str());
			return std::string();
		}
		
		const std::string::size_type pos = path.find(sepstr + ".." + sepstr);
		if (pos == std::string::npos)
		{
			return path;
		}
		
		if (pos == 0)
		{
			TT_PANIC("Attempt to compact invalid path '%s' (cannot do .. from root.", p_path.c_str());
			return std::string();
		}
		
		// Find directory preceding ..
		std::string::size_type begin = path.rfind(sep, pos - 1);
		if (begin == std::string::npos)
		{
			// No directory separator found
			begin = 0;
		}
		else
		{
			// Leave the found separator intact
			++begin;
		}
		
		// Erase preceding directory and ..
		path.erase(begin, (pos - begin) + 4);
	}
}


std::string makeCorrectFSPath(const std::string& p_path)
{
	return compactPath(p_path, "\\/");
}


str::StringSet getFilesInDir(const std::string& p_path, const std::string& p_filter)
{
	DirPtr dir = openDir(p_path, p_filter);
	
	if (dir == 0)
	{
		return str::StringSet();
	}
	
	str::StringSet sortedFiles;
	DirEntry       entry;

	while (dir->read(entry))
	{
		if (entry.isDirectory() == false)
		{
			sortedFiles.insert(getFileTitle(entry.getName()));
		}
	}
	
	return sortedFiles;
}


str::Strings getRecursiveFileList(const std::string& p_rootPath,
                                  const std::string& p_filter,
                                  bool               p_addRootToPath)
{
	TT_ASSERT(p_rootPath.empty() == false && *p_rootPath.rbegin() == '/');
	
	// First make an alphabetical list of all subdirs
	str::StringSet sortedDirs;
	{
		DirPtr dir = openDir(p_rootPath);
		if (dir == 0)
		{
			return tt::str::Strings();
		}
		
		DirEntry entry;
		while (dir->read(entry))
		{
			if (entry.isDirectory()    &&
			    entry.getName() != "." &&
			    entry.getName() != "..")
			{
				sortedDirs.insert(entry.getName());
			}
		}
	}
	
	// Recurse into each subdir first (directories should be listed first)
	str::Strings fileList;
	for (str::StringSet::iterator dirIt = sortedDirs.begin(); dirIt != sortedDirs.end(); ++dirIt)
	{
		str::Strings subDirFiles(getRecursiveFileList(p_rootPath + *dirIt + "/", p_filter, true));
		if (p_addRootToPath == false)
		{
			// Strip the root path from all the sub entries (needed to be added for proper recursion)
			for (str::Strings::iterator it = subDirFiles.begin(); it != subDirFiles.end(); ++it)
			{
				(*it).erase(0, p_rootPath.length());
			}
		}
		
		fileList.reserve(subDirFiles.size());
		fileList.insert(fileList.end(), subDirFiles.begin(), subDirFiles.end());
	}
	
	// Now add all files at the current level
	str::StringSet ourFiles(getFilesInDir(p_rootPath, p_filter));
	fileList.reserve(fileList.size() + ourFiles.size());
	for (str::StringSet::iterator fileIt = ourFiles.begin(); fileIt != ourFiles.end(); ++fileIt)
	{
		if (p_addRootToPath)
		{
			fileList.push_back(p_rootPath + *fileIt);
		}
		else
		{
			fileList.push_back(*fileIt);
		}
	}
	
	return fileList;
}


// Namespace end
}
}
}
