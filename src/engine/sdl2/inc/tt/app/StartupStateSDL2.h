#if !defined(INC_TT_APP_STARTUPSTATESDL2_H)
#define INC_TT_APP_STARTUPSTATESDL2_H


#include <tt/app/StartupState.h>

namespace tt {
namespace app {

/*! \brief Saves state data to the save data directory. */
class StartupStateSDL2 : public StartupState
{
public:
	StartupStateSDL2(s32 p_clientRevision, s32 p_libRevision);
	virtual ~StartupStateSDL2();
    
    // SDL2 only lazy Initialization
    void initialize(const std::string& p_rootPath);
	
protected:
	virtual bool writeStateFileWithData(const u8* p_fileData, s32 p_dataLen);
	virtual bool readStateFile(u8* p_fileData, s32 p_expectedLen);
	
private:
	std::string m_saveFilename;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_STARTUPSTATESDL2_H)
