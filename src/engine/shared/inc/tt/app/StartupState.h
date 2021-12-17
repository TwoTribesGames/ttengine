#if !defined(INC_TT_APP_STARTUPSTATE_H)
#define INC_TT_APP_STARTUPSTATE_H


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace app {

/*! \brief What step in the startup sequence the application is/was in. */
enum StartupStep
{
	StartupStep_None,         //!< No known startup step (data not available).
	StartupStep_PreInit,      //!< The very first thing that happens when starting up (before sys init).
	StartupStep_SystemInit,   //!< System (app framework) initialization.
	StartupStep_ClientInit,   //!< Client (AppInterface, e.g. game) initialization.
	StartupStep_Running,      //!< Startup completed, application running normally.
	StartupStep_InBackground, //!< Application has been moved to background processing (iOS).
	StartupStep_Shutdown,     //!< Application shut down normally/cleanly.
	
	StartupStep_Count,
	StartupStep_Invalid
};

enum StartupType
{
	StartupType_NewInstall, //!< Application has been freshly installed on this system.
	StartupType_Upgrade,    //!< Application has been upgraded from an older version.
	StartupType_Normal,     //!< No new install or upgrade, just a normal run.
	
	StartupType_Count,
	StartupType_Invalid
};

inline bool isValidStartupStep(StartupStep p_step) { return p_step >= 0 && p_step < StartupStep_Count; }
const char* getStartupStepName(StartupStep p_step);
StartupStep getStartupStepFromName(const std::string& p_name);

inline bool isValidStartupType(StartupType p_type) { return p_type >= 0 && p_type < StartupType_Count; }
const char* getStartupTypeName(StartupType p_type);
StartupType getStartupTypeFromName(const std::string& p_name);


/*! \brief Provides (low-level) application startup tracking. */
class StartupState
{
public:
	/*! \brief Sets (and immediately stores) the new step in the application startup sequence. */
	void setStartupStep(StartupStep p_step);
	
	/*! \return Which step in the startup sequence the application reached the last time it was started. */
	inline StartupStep getLastStartupStep() const { return m_startupStepFromLastStart; }
	
	/*! \return Which type of startup the current application run is. */
	inline StartupType getStartupType() const { return m_currentStartupType; }
	
	/*! \brief Indicates whether the last app run shut down okay or
	           if the application was somehow terminated abnormally. */
	inline bool didLastRunExitCleanly() const
	{
		return m_startupStepFromLastStart == StartupStep_None         ||
		       m_startupStepFromLastStart == StartupStep_InBackground ||
		       m_startupStepFromLastStart == StartupStep_Shutdown;
	}
	
protected:
	StartupState(s32 p_clientRevision, s32 p_libRevision);
	virtual ~StartupState();
	
	// Should be implemented by platform-specific implementations:
	virtual bool writeStateFileWithData(const u8* p_fileData, s32 p_dataLen) = 0;
	virtual bool readStateFile(u8* p_fileData, s32 p_expectedLen) = 0;
	
	// For derived classes to trigger init (can't call virtual functions from base class constructor)
	void init(); // FIXME: Choose better name
	
private:
	void loadExistingState();
	
	// No copying
	StartupState(const StartupState&);
	StartupState& operator=(const StartupState&);
	
	
	const s32   m_clientRevision;
	const s32   m_libRevision;
	StartupStep m_startupStepFromLastStart;
	StartupType m_currentStartupType;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_STARTUPSTATE_H)
