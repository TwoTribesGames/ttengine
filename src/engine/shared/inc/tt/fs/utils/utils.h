#if !defined(INC_TT_FS_UTILS_UTILS_H)
#define INC_TT_FS_UTILS_UTILS_H


#include <string>

#include <tt/fs/types.h>
#include <tt/str/str_types.h>


namespace tt {
namespace fs {
namespace utils {

bool matchesFilter(const std::string& p_name, const std::string& p_filter);

/*! \brief Creates a directory, creating each parent directory as necessary.
    \return True if entire directory structure was created successfully, false if it was not. */
bool createDirRecursive(const std::string& p_dir, identifier p_identifier = 0);

/*! \brief Destroys a directory, destroying each subdirectory as necessary.
    \return True if entire directory structure was destroyed successfully, false if it was not. */
bool destroyDirRecursive(const std::string& p_dir, identifier p_identifier = 0);

/*! \brief Clears a directory, destroying each subdirectory and file as necessary.
    \return True if entire directory structure was cleared successfully, false if it was not. */
bool clearDirRecursive(const std::string& p_dir, identifier p_identifier = 0);

/*! \brief Splits a path into a dir and a file and then add a subdir between them. */
std::string addSubdirToPath(const std::string& p_path, const std::string& p_subDir);

/*! \brief Retrieves extension part of a path.
    \param p_path The path to retrieve the extension of.
    \param p_identifier Filesystem to use.
    \return The extension on success, empty string on fail.*/
std::string getExtension(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves filetitle part of a path.
    \param p_path The path to retrieve the filetitle of.
    \param p_identifier Filesystem to use.
    \return Filename without the extension, empty string on fail.*/
std::string getFileTitle(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves filename part of a path.
    \param p_path The path to retrieve the filename of.
    \param p_identifier Filesystem to use.
    \return Filename with the extension, empty string on fail.*/
std::string getFileName(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Gets the time of creation of the file.
\param p_path The path to retrieve the filename of.
\param p_identifier Filesystem to use.
\return The time of creation of the file.*/
time_type getFileCreationTime(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Gets the time of last access of the file.
\param p_path The path to retrieve the filename of.
\param p_identifier Filesystem to use.
\return The time of access of the file.*/
time_type getFileAccessTime(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Gets the time of last write of the file.
\param p_path The path to retrieve the filename of.
\param p_identifier Filesystem to use.
\return The time of write of the file.*/
time_type getFileWriteTime(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves directory part of a path.
    \param p_path The path to retrieve the directory part of.
    \param p_identifier Filesystem to use.
    \return The directory part on success, empty string on fail.*/
std::string getDirectory(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves last directoryname of a path.
    \param p_path The path to retrieve the directoryname of.
    \param p_identifier Filesystem to use.
    \return The last directoryname on success, empty string on fail.*/
std::string getLastDirectoryName(const std::string& p_path, identifier p_identifier = 0);

std::string getParentDirectory(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Compacts a path (resolves .. and .)
    \param p_path The path to compact
    \param p_separators Optional list of valid separators
    \param p_identifier Filesystem to use.
    \return The compacted path, or an empty string on failure.*/
std::string compactPath(const std::string& p_path,
                        const std::string& p_separators = std::string(),
                        identifier         p_identifier = 0);

/*! \brief Compacts a path (resolves .. and .) and makes sure it is a correct FS path
    \param p_path The path to correct
    \return The corrected path, or an empty string on failure.*/
std::string makeCorrectFSPath(const std::string& p_path);


/*! \brief Get a sorted list of all files in a directory
	\param p_path The path to get the filenames from
	\param p_filter A filter to select specific files only
	\return Sorted list of all filenames that contain the filter */
str::StringSet getFilesInDir(const std::string& p_path, const std::string& p_filter = "*");


/*! \brief Get a sorted list of all files in a directory and its subdirectories
	\param p_rootPath The root path to get the filenames from
	\param p_filter A filter to select specific files only
	\param p_addRootToPath Needed for proper recursion
	\return Sorted list of all filenames that contain the filter */
str::Strings getRecursiveFileList(const std::string& p_rootPath,
	                              const std::string& p_filter = "*",
	                              bool p_addRootToPath = false);

// Namespace end
}
}
}


#endif  // !defined(INC_TT_FS_UTILS_UTILS_H)
