#if !defined(INC_TOKI_SAVEDATA_UTILS_H)
#define INC_TOKI_SAVEDATA_UTILS_H


#include <string>

#include <tt/fs/types.h>


namespace toki {
namespace savedata {

/*! \brief Initializes the save utilities for use. Must be called before using any other savedata functions.
           Initializes the file system features needed for saving.
           Creates the save data root directory. */
bool initSaveUtils();

/*! \brief Indicates whether the game can save game data.
           This could return false if no room is available for save data, for example. */
bool canSave();

bool mountSaveVolumeForReading();
bool mountSaveVolumeForWriting();
bool unmountSaveVolume();
bool isSaveVolumeMounted();

/*! \brief Converts a relative filename (relative to the save data directory) to an absolute file path.
    \note If the relative filename contains subdirectories, these should be specified using forward slashes!
    \return Absolute file path */
std::string makeSaveFilePath(const std::string& p_relativeFilename, bool p_useSaveFS);

/*! \brief Opens the specified file (relative to the save data directory) for (over)writing.
           Also creates the save data directory (and any others specified in the filename) if these do not exist yet.
    \note If the relative filename contains subdirectories, these should be specified using forward slashes!
    \return Pointer to the opened file if successful. Null pointer if creating any of the directories failed or opening the file for writing failed. */
tt::fs::FilePtr createSaveFile(const std::string& p_relativeFilename, bool p_useSaveFS = true);

/*! \brief Opens the specified file (relative to the save data directory) for reading.
    \note If the relative filename contains subdirectories, these should be specified using forward slashes!
    \return Pointer to the opened file if successful. Null pointer if file does not exist or opening failed. */
tt::fs::FilePtr openSaveFile(const std::string& p_relativeFilename, bool p_useSaveFS = true);

/*! \brief Destroys the specified file (relative to the save data directory) if it exists.
    \note If the relative filename contains subdirectories, these should be specified using forward slashes!
    \return True if file was deleted or did not exist. False if an error occurred. */
bool destroySaveFile(const std::string& p_relativeFilename, bool p_useSaveFS = true);


/*! \brief Renames a specified file (relative to the save data directory) if it exists.
    \note If the relative filename contains subdirectories, these should be specified using forward slashes!
    \return True if file was renamed. False if an error occurred. */
bool renameSaveFile(const std::string& p_relativeSrcFilename, const std::string& p_relativeDstFilename,
                    bool p_useSaveFS);

/*! \brief Commits all save data to the file system (required on CAT).
           Call this after all save data has been written to file(s). */
bool commitSaveData();

// Namespace end
}
}


#endif  // !defined(INC_TOKI_SAVEDATA_UTILS_H)
