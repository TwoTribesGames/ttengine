#if !defined(INC_TT_APP_APPLICATION_H)
#define INC_TT_APP_APPLICATION_H


#include <string>
#include <vector>


#include <tt/fs/types.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_error.h>


namespace tt {

namespace args {
	class CmdLine;
}

namespace app {


// Available debug keys
enum DebugKeys
{
	DebugKeys_None         = 0x0,
	DebugKeys_Function     = 0x1,
	DebugKeys_Normal       = 0x2,

	DebugKeys_All          = DebugKeys_Function | DebugKeys_Normal
};


class StartupState;
class PlatformCallbackInterface;


/*! \brief Interface to provide cross-platform access to application facilities. */
class Application
{
public:
	/*! \return The path to the root of the assets. */
	virtual std::string getAssetRootDir() const = 0;
	
	/*! \return The file system identifier to use for save data. */
	virtual fs::identifier getSaveFsID() const = 0;
	
	/*! \return The command line arguments passed to the application. */
	virtual const args::CmdLine& getCmdLine() const = 0;
	
	/*! \return Information about the application's startup. */
	virtual const StartupState& getStartupState() const = 0;
	
	/*! \brief Switches the application to full-screen or windowed rendering (where available). */
	virtual void setFullScreen(bool p_fullScreen) = 0;
	
	/*! \return Whether the application is in full-screen mode. */
	virtual bool isFullScreen() const = 0;
	
	/*! \brief Set the resolution at which the application should render in fullscreen mode */
	virtual void setFullScreenResolution(const math::Point2& p_resolution) = 0;

	/*! \return The resolution at which the application is rendered in fullscreen mode */
	virtual tt::math::Point2 getFullScreenResolution() const = 0;
	
	virtual tt::math::Point2 getDesktopSize() const
	{ TT_PANIC("getDesktopSize() not supported by this platform."); return tt::math::Point2(); }
	
	/*! \return Whether the application should display debug information (such as FPS, memory usage). */
	virtual bool shouldDisplayDebugInfo() const = 0;
	
	/*! \brief Quits the application.
	    \param p_graceful Exits the application in a graceful fashion (via normal exit logic) if true.
	                      Forcibly exits the application if false. */
	virtual void terminate(bool p_graceful) = 0;
	
	/*! \brief Pauses or unpauses the application, stopping updates (and renders?). */
	virtual void setPaused(bool p_paused) = 0;
	
	/*! \brief Function to check if the application is active/in focus or inactive/out of focus. (See: onAppInactive()) */
	virtual bool isActive() const = 0;
	
	/*! \brief Set which debug keys to handle */
	virtual void setHandleDebugKeys(u32 p_keys) = 0;
	
	/*! \brief Sets whether the platform-specific menu 
	           is allowed (can be opened). This is a 'push' alternative to AppInterface::platformMenuEnabled. */
	virtual void setPlatformMenuEnabled(bool /*p_enabled*/)
	{ TT_WARN("setPlatformMenuEnabled not implemented on this platform."); }
	
	virtual void setPlayerCount(u32 /*p_playerCount*/)
	{ TT_WARN("setPlayerCount not implemented on this platform."); }
	
	virtual void setTargetFPS(u32 p_fps) = 0;
	virtual u32  getTargetFPS() const = 0;
	
	virtual void handleResolutionChanged() {}
	virtual bool onPastIntro() { return true; }
	void registerPlatformCallbackInterface(PlatformCallbackInterface* p_listener);
	void unregisterPlatformCallbackInterface(PlatformCallbackInterface* p_listener);
	
protected:
	typedef std::vector<PlatformCallbackInterface*> PlatformCallbackInterfaces;
	
	Application();
	virtual ~Application();
	
	/*! \brief Makes this application instance available via getApplication. */
	void makeApplicationAvailable();
	
	PlatformCallbackInterfaces m_appListeners;
};


/*! \return Pointer to the application instance. */
Application* getApplication();

/*! \return Whether an application instance is available. */
bool hasApplication();

/*! \return The application's command line (either potentially filtered by Application
            or all command line arguments from CmdLine::getApplicationCmdLine). */
const args::CmdLine& getCmdLine();

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_APPLICATION_H)
