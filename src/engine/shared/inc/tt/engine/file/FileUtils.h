#if !defined(INC_TT_ENGINE_FILE_FILEUTILS_H)
#define INC_TT_ENGINE_FILE_FILEUTILS_H

#include <string>
#include <map>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/engine/file/FileType.h>

#include <tt/engine/EngineID.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace file {


const char* getFileTypeExtension(FileType p_type);

class FileUtils
{
public:
	// Instance management
	static bool createInstance();
	inline static FileUtils* getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	inline static bool hasInstance() {return ms_instance != 0;}
	static void destroyInstance();
	
	bool exists(const engine::EngineID& p_id, FileType p_type);
	fs::FilePtr getDataFile(const EngineID& p_id, FileType p_type);
	std::string getFilename(const EngineID& p_id, FileType p_type);
	tt::fs::time_type getLastWriteTime(const EngineID& p_id, FileType p_type);
	
	void generateNamespaceMapping();
	
	/*! \brief Sets the path to prefix to all filenames, for systems that do not support
	           the concept of a 'working directory' (e.g. iPhone). */
	inline void setFileRoot(const std::string& p_path) { m_fileRoot = p_path; }
	
	inline void setShowLoadedFiles(bool p_show) { m_showLoadedFiles = p_show; }
	
private:
	FileUtils();
	inline ~FileUtils() {}

	friend class engine::EngineID;
	u32 getCRC(const std::string& p_name, s32 p_index);
	
	void createCRCTable(u32 p_polynomial, s32 p_index);
	
private:
	static FileUtils* ms_instance;
	
	static const u32 tableEntries = 256;
	
	u32 m_CRCTable[2][tableEntries];
	
	typedef std::map<u32, std::string> NamespaceMap;
	NamespaceMap m_namespaces;
	
	std::string m_fileRoot;
	bool m_showLoadedFiles;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_FILE_FILEUTILS_H
