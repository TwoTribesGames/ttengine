#if !defined(INC_TT_INPUT_MOUSECONTROLLER_H)
#define INC_TT_INPUT_MOUSECONTROLLER_H

#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/Pointer.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

struct MouseController
{
public:
	/*! \brief Indicates whether the controller is connected.
	           Even though you can specify an index here,
	           only index one is valid. */
	static bool isConnected(ControllerIndex p_index);
	
	/*! \brief Returns the state of the controller.
	           Even though you can specify an index here,
	           only index one is valid. */
	static const MouseController& getState(ControllerIndex p_index);
	static void update();
	
	static void updateWindowRects(HWND p_window);
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \param p_window Handle of the window that this controller is associated with.
	    \param p_multipleMiceSupport Whether to support multiple mice.
	    \return True if initialization was successful, false if not. */
	static bool initialize(HWND p_window, bool p_multipleMiceSupport);
	
	/*! \brief Deinitializes the controller. */
	static void deinitialize();
	
	inline static void setDefaultCursor(HCURSOR p_defaultCursor)
	{
		ms_defaultCursor = p_defaultCursor;
		resetToDefaultCursor();
	}
	
	inline static void resetToDefaultCursor()
	{
		setCustomCursor(ms_defaultCursor);
	}
	
	inline static void setCustomCursor(HCURSOR p_cursor)
	{
		ms_cursor = p_cursor;
		
		// Immediately update the cursor, if the pointer is in the window's client area
		if (ms_temporary[0].cursor.valid)
		{
			SetCursor(ms_cursor);
		}
	}
	
	static void setCursorVisible(bool p_visible);
	inline static bool isCursorVisible() { return ms_cursorVisible; }
	
#if !defined(TT_BUILD_FINAL)
	static inline bool isMouseCaptured() { return ms_restrictMouseToWindow; }
	static void toggleMouseCaptured();
	static void setMouseCaptured(bool p_capture);
#endif //#if !defined(TT_BUILD_FINAL)
	
	Pointer cursor;
	Button  left;
	Button  middle;
	Button  right;
	
	s32     wheelNotches;
	
private:
	enum
	{
		MaxSupportedMice   = 4,
		
		CursorCheck_TimerID  = 133742,
		CursorCheck_Interval = 125  //!< Interval in milliseconds
	};
	
	
	MouseController();
	static bool isInitialized();
	static LRESULT CALLBACK wndProc(HWND   p_wnd,    UINT   p_msg,
	                                WPARAM p_wparam, LPARAM p_lparam);
	static void updateCursorFromMessage(s32 p_mouseIndex, LPARAM p_lparam);
	static void offsetCursorPosition(s32 p_mouseIndex);
	static s32 getMouseIndex(HANDLE p_deviceHandle);
	
	
	static HCURSOR         ms_defaultCursor;
	static HCURSOR         ms_cursor;
	static bool            ms_cursorVisible;
	static bool            ms_multipleMiceSupport;
	static s32             ms_availableMice;
	static HANDLE          ms_rdpMouseDevice;
	
	//! Used so mouse handle can be translated back to an array index
	static HANDLE          ms_mouseDevice[MaxSupportedMice];
	
	static MouseController ms_controller[MaxSupportedMice];
	static MouseController ms_temporary[MaxSupportedMice];
	static RECT            ms_clientRect;
	static RECT            ms_windowRect;
	static POINT           ms_screenDimensions;
#if !defined(TT_BUILD_FINAL)
	static bool            ms_restrictMouseToWindow;
#endif
	static HWND            ms_window;
	static WNDPROC         ms_originalWndProc;
	static std::string     ms_originalWindowTitle;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_INPUT_MOUSECONTROLLER_H)
