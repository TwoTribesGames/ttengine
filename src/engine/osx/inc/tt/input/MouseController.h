#if !defined(INC_TT_INPUT_MOUSECONTROLLER_H)
#define INC_TT_INPUT_MOUSECONTROLLER_H

#if defined(TT_PLATFORM_OSX_MAC)  // only available on Mac OS X, not iOS


#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/KeyList.h>
#include <tt/input/Pointer.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

class MouseCursor;
typedef tt_ptr<MouseCursor>::shared MouseCursorPtr;


struct MouseController;
class MouseCursor
{
public:
	enum SystemCursor
	{
		SystemCursor_Arrow,
		SystemCursor_IBeam,
		SystemCursor_Crosshair,
		SystemCursor_ClosedHand,
		SystemCursor_OpenHand,
		SystemCursor_PointingHand,
		SystemCursor_ResizeLeft,
		SystemCursor_ResizeRight,
		SystemCursor_ResizeLeftRight,
		SystemCursor_ResizeUp,
		SystemCursor_ResizeDown,
		SystemCursor_ResizeUpDown,
		
		SystemCursor_Count
	};
	
	
	static MouseCursorPtr create(const std::string& p_windowsCursorFilename, s32 p_cursorIndex = 0);
	static MouseCursorPtr create(const std::string& p_imageFilename, const math::Point2& p_hotSpot);
	static MouseCursorPtr create(SystemCursor p_systemCursor);
	~MouseCursor();
	
	inline void* getPlatformCursor() const { return m_cursor; }
	
private:
	MouseCursor(void* p_cursor, bool p_haveOwnership);
	
	// No copying
	MouseCursor(const MouseCursor&);
	MouseCursor& operator=(const MouseCursor&);
	
	
	void* m_cursor;         // actually an NSCursor pointer
	bool  m_haveOwnership;  // do we need to release the cursor object?
	
	friend struct MouseController;
};


/*! \brief TT input controller implementation for Mac OS X mouse events. */
struct MouseController
{
public:
	static bool isConnected(ControllerIndex p_index);
	
	/*! \brief Returns the state of the controller. */
	static const MouseController& getState(ControllerIndex p_index);
	
	static void update();
	
	/*! \brief Indicates whether the controller has been initialized.
	    \return True when initialized, false if not. */
	static bool isInitialized();
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \return True if initialization was successful, false if not. */
	static bool initialize(void* p_appView);
	
	static void deinitialize();
	
	static void setCustomCursor(const MouseCursorPtr& p_cursor);
	static void setCursorVisible(bool p_visible);
	static inline bool isCursorVisible() { return ms_cursorVisible; }
	
	// NOTE: This function is for use by TTdevObjCAppView only!
	// (declaring an Objective C class as friend is impossible, which is why this function is public)
	static inline MouseController& getTemporaryState() { return ms_temporary; }
	static void handleViewChanged();
	
	static inline bool wasCustomCursorSet() { return ms_customCursorWasSet; }
	static inline const MouseCursorPtr& getCustomCursor() { return ms_customCursor; }
	
	
	Button  left;
	Button  middle;
	Button  right;
	Pointer cursor;
	
	s32     wheelNotches;
	
	// Three-finger swipe gestures
	Button trackpadSwipeLeft;
	Button trackpadSwipeRight;
	Button trackpadSwipeUp;
	Button trackpadSwipeDown;
	
private:
	// Should not instantiate directly
	MouseController();
	
	
	static bool ms_initialized;
	static MouseController ms_controller;
	static MouseController ms_temporary;  // stores updated (from event) state until update()
	
	static bool           ms_customCursorWasSet; // whether a custom cursor has been used ever
	static MouseCursorPtr ms_customCursor;
	static bool           ms_cursorVisible; // whether the cursor should be visible
	
	static void* ms_appView; // pointer to Objective C class TTdevObjCAppView, for custom cursor support
};

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_MAC)

#endif  // !defined(INC_TT_INPUT_MOUSECONTROLLER_H)
