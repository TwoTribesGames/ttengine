#if !defined(INC_TT_APP_PLATFORMCALLBACKINTERFACE_H)
#define INC_TT_APP_PLATFORMCALLBACKINTERFACE_H


#include <tt/app/AppOrientation.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace app {

class PlatformCallbackInterface
{
public:
	virtual ~PlatformCallbackInterface() { }
	
	// These are called when entering the OS menu of a platform
	// Currently only useful for Steam (the Steam overlay)
	virtual void onPlatformMenuEnter() { }
	virtual void onPlatformMenuExit()  { }
	
	// Functions called when application becomes inactive/out of focus
	virtual void onAppInactive() { }
	virtual void onAppActive()   { }
	
	// Functions called when the application is paused/resumed (by a call to Application::setPaused)
	virtual void onAppPaused()  { }
	virtual void onAppResumed() { }
	
	// Functions called when application enters/leaves background processing
	// (e.g. on iOS 4 multitasking devices)
	
	/*! \brief Application has been moved to background processing.
	           Save application state here and unload any resources. */
	virtual void onAppEnteredBackground() { }
	
	/*! \brief Application was restored from background to foreground (active).
	           Re-load any resources that were unloaded for background processing. */
	virtual void onAppLeftBackground()    { }
	
	
	// Application orientation notifications (currently applies to iOS only)
	
	/*! \brief Called to query whether the application should rotate to the specified orientation.
	           By default, the app framework supports either landscape orientation.
	    \param p_orientation The new rotation being queried.
	    \return True to allow auto-rotation to the new orientation, false to prohibit it. */
	virtual bool onShouldAutoRotateToOrientation(AppOrientation p_orientation) const
	{ return p_orientation == AppOrientation_LandscapeLeft ||
	         p_orientation == AppOrientation_LandscapeRight; }
	
	/*! \brief Called right before the application starts rotating to a new orientation.
	    \param p_orientation The new orientation that will be rotated to.
	    \param p_duration The duration of the rotation animation, in seconds. */
	virtual void onWillRotateToOrientation(AppOrientation p_orientation, real p_duration)
	{ (void)p_orientation; (void)p_duration; }
	
	/*! \brief Called right after the application has rotating to a new orientation.
	 \param p_fromOrientation The old orientation that was rotated from. */
	virtual void onDidRotateFromOrientation(AppOrientation p_fromOrientation)
	{ (void)p_fromOrientation; }
	
	// Useful for PC games
	virtual void onLostDevice()  { }
	virtual void onResetDevice() { }
	
	// Useful for Multiplayer games
	virtual void onSetPlayerCount(u32 /*p_newCount*/) { }
	
	/*! \brief Called when the app framework wants the application to reload its
	           (currently loaded) assets (e.g. level, config). */
	virtual void onRequestReloadAssets() { }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_PLATFORMCALLBACKINTERFACE_H)
