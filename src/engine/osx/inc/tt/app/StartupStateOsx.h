#if !defined(INC_TT_APP_STARTUPSTATEOSX_H)
#define INC_TT_APP_STARTUPSTATEOSX_H


#include <tt/app/StartupState.h>


namespace tt {
namespace app {

/*! \brief Saves state data to the save data directory in iOS or Mac OS X. */
class StartupStateOsx : public StartupState
{
public:
	StartupStateOsx(s32 p_clientRevision, s32 p_libRevision, const std::string& p_appName);
	virtual ~StartupStateOsx();
	
protected:
	virtual bool writeStateFileWithData(const u8* p_fileData, s32 p_dataLen);
	virtual bool readStateFile(u8* p_fileData, s32 p_expectedLen);
	
private:
	std::string m_saveFilename;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_STARTUPSTATEOSX_H)
