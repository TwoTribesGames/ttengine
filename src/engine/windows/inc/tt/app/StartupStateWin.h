#if !defined(INC_TT_APP_STARTUPSTATEWIN_H)
#define INC_TT_APP_STARTUPSTATEWIN_H


#include <tt/app/StartupState.h>


namespace tt {
namespace app {

/*! \brief Stores state data in registry instead of in a file (simpler implementation). */
class StartupStateWin : public StartupState
{
public:
	StartupStateWin(s32 p_clientRevision, s32 p_libRevision, const std::string& p_appName);
	virtual ~StartupStateWin();
	
protected:
	virtual bool writeStateFileWithData(const u8* p_fileData, s32 p_dataLen);
	virtual bool readStateFile(u8* p_fileData, s32 p_expectedLen);
	
private:
	const std::string m_stateRegKeyPath;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_STARTUPSTATEWIN_H)
