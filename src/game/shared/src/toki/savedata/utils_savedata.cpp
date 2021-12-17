#include <tt/app/Application.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/savedata/utils.h>


namespace toki {
namespace savedata {

static bool g_saveDataInitSuccessful = false;


// Private helper function to handle both the "create file for writing" and "open for reading" cases
static tt::fs::FilePtr openFile(const std::string& p_relativeFilename,
                                bool               p_forWriting,
                                tt::fs::identifier p_fsID)
{
	TT_ASSERTMSG(p_relativeFilename.find('\\') == std::string::npos,
	             "Relative save filename '%s' contains backslashes. Use forward slashes as directory separators!",
	             p_relativeFilename.c_str());
	
	if (p_relativeFilename.empty())
	{
		TT_PANIC("No filename specified.");
		return tt::fs::FilePtr();
	}
	
	if (tt::app::hasApplication() == false)
	{
		TT_PANIC("Cannot create or open save file if no Application (app framework) instance is available.");
		return tt::fs::FilePtr();
	}
	
	tt::str::Strings subdirComponents(tt::str::explode(p_relativeFilename, "/"));
	
	const std::string filenameOnly(subdirComponents.back());
	subdirComponents.pop_back();
	
	std::string pathToFile(tt::fs::getSaveRootDir(p_fsID));
	if (subdirComponents.empty() == false)
	{
		pathToFile += tt::str::implode(subdirComponents, "/") + "/";
	}
	
	// Check if the save root directory exists
	if (pathToFile.empty() == false && tt::fs::dirExists(pathToFile, p_fsID) == false)
	{
		if (p_forWriting == false)
		{
			// Save data directory does not exist. Silently return a null pointer
			// (failure to open save data is not an error condition)
			return tt::fs::FilePtr();
		}
		
		// When opening for writing, also create the directories leading up to the save file
		if (tt::fs::utils::createDirRecursive(pathToFile, p_fsID) == false)
		{
			TT_PANIC("Creating save data directory '%s' failed.", pathToFile.c_str());
			return tt::fs::FilePtr();
		}
	}
	
	// Compose the full path to the save file
	const std::string fullPath(pathToFile + filenameOnly);
	
	// For reading, ensure the file exists before trying to open it
	if (p_forWriting == false && tt::fs::fileExists(fullPath, p_fsID) == false)
	{
		// Save file does not exist. Silently return a null pointer
		// (failure to open save data is not an error condition)
		return tt::fs::FilePtr();
	}
	
	return tt::fs::open(fullPath, p_forWriting ? tt::fs::OpenMode_Write : tt::fs::OpenMode_Read, p_fsID);
}


bool initSaveUtils()
{
	g_saveDataInitSuccessful = tt::fs::createSaveRootDir(tt::app::getApplication()->getSaveFsID());
	return g_saveDataInitSuccessful;
}


bool canSave()
{
	return g_saveDataInitSuccessful;
}


bool mountSaveVolumeForReading()
{
	return canSave() &&
	       tt::fs::mountSaveVolume(tt::fs::OpenMode_Read, tt::app::getApplication()->getSaveFsID());
}


bool mountSaveVolumeForWriting()
{
	return canSave() &&
	       tt::fs::mountSaveVolume(tt::fs::OpenMode_Write, tt::app::getApplication()->getSaveFsID());
}


bool unmountSaveVolume()
{
	return canSave() &&
	       tt::fs::unmountSaveVolume(tt::app::getApplication()->getSaveFsID());
}


bool isSaveVolumeMounted()
{
	return canSave() &&
	       tt::fs::isSaveVolumeMounted(tt::app::getApplication()->getSaveFsID());
}


std::string makeSaveFilePath(const std::string& p_relativeFilename, bool p_useSaveFS)
{
	TT_ASSERTMSG(p_relativeFilename.find('\\') == std::string::npos,
	             "Relative save filename '%s' contains backslashes. Use forward slashes as directory separators!",
	             p_relativeFilename.c_str());
	
	if (p_relativeFilename.empty())
	{
		TT_PANIC("No filename specified.");
		return "";
	}
	
	if (tt::app::hasApplication() == false)
	{
		TT_PANIC("Cannot make save file path if no Application (app framework) instance is available.");
		return "";
	}
	
	const tt::fs::identifier saveFsID = p_useSaveFS ? tt::app::getApplication()->getSaveFsID() : 0;
	return (tt::fs::getSaveRootDir(saveFsID) + p_relativeFilename);
}


tt::fs::FilePtr createSaveFile(const std::string& p_relativeFilename, bool p_useSaveFS)
{
	if (p_useSaveFS && canSave() == false) return tt::fs::FilePtr();
	const tt::fs::identifier fsID = p_useSaveFS ? tt::app::getApplication()->getSaveFsID() : 0;
	return openFile(p_relativeFilename, true, fsID);
}


tt::fs::FilePtr openSaveFile(const std::string& p_relativeFilename, bool p_useSaveFS)
{
	if (p_useSaveFS && canSave() == false) return tt::fs::FilePtr();
	const tt::fs::identifier fsID = p_useSaveFS ? tt::app::getApplication()->getSaveFsID() : 0;
	return openFile(p_relativeFilename, false, fsID);
}


bool destroySaveFile(const std::string& p_relativeFilename, bool p_useSaveFS)
{
	if (p_useSaveFS && canSave() == false) return false;
	
	const tt::fs::identifier saveFsID = p_useSaveFS ? tt::app::getApplication()->getSaveFsID() : 0;
	const std::string fullPath(makeSaveFilePath(p_relativeFilename, p_useSaveFS));
	
	return (tt::fs::fileExists(fullPath, saveFsID) == false) ||
	       tt::fs::destroyFile(fullPath, saveFsID);
}


bool renameSaveFile(const std::string& p_relativeSrcFilename, const std::string& p_relativeDstFilename,
                    bool p_useSaveFS)
{
	if (p_useSaveFS && canSave() == false) return false;
	
	TT_ASSERTMSG(p_relativeSrcFilename.find('\\') == std::string::npos,
	             "Relative source filename '%s' contains backslashes. Use forward slashes as directory separators!",
	             p_relativeSrcFilename.c_str());
	
	TT_ASSERTMSG(p_relativeDstFilename.find('\\') == std::string::npos,
	             "Relative destination filename '%s' contains backslashes. Use forward slashes as directory separators!",
	             p_relativeDstFilename.c_str());
	
	if (p_relativeSrcFilename.empty())
	{
		TT_PANIC("No source filename specified.");
		return false;
	}
	
	if (p_relativeDstFilename.empty())
	{
		TT_PANIC("No destination filename specified.");
		return false;
	}
	
	if (tt::app::hasApplication() == false)
	{
		TT_PANIC("Cannot rename save file if no Application (app framework) instance is available.");
		return false;
	}
	
	const tt::fs::identifier saveFsID = p_useSaveFS ? tt::app::getApplication()->getSaveFsID() : 0;
	const std::string saveRootDir(tt::fs::getSaveRootDir(saveFsID));
	const std::string fullSrcPath(saveRootDir + p_relativeSrcFilename);
	
	if (tt::fs::fileExists(fullSrcPath, saveFsID) == false)
	{
		return false;
	}
	
	tt::str::Strings subdirComponents(tt::str::explode(p_relativeDstFilename, "/"));
	
	const std::string filenameOnly(subdirComponents.back());
	subdirComponents.pop_back();
	
	std::string pathToFile(saveRootDir);
	if (subdirComponents.empty() == false)
	{
		pathToFile += tt::str::implode(subdirComponents, "/") + "/";
	}
	
	// Check if the save root directory exists
	if (pathToFile.empty() == false && tt::fs::dirExists(pathToFile, saveFsID) == false)
	{
		// When opening for writing, also create the directories leading up to the save file
		if (tt::fs::utils::createDirRecursive(pathToFile, saveFsID) == false)
		{
			TT_PANIC("Creating save data directory '%s' failed.", pathToFile.c_str());
			return false;
		}
	}
	
	// Compose the full path to the destination file
	const std::string fullDstPath(pathToFile + filenameOnly);
	
	return tt::fs::moveFile(fullSrcPath, fullDstPath, false, saveFsID);
}


bool commitSaveData()
{
	if (canSave() == false) return false;
	return tt::fs::commit(tt::app::getApplication()->getSaveFsID());
}

// Namespace end
}
}
