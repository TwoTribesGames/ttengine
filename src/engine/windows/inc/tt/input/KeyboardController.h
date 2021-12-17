#if !defined(INC_TT_INPUT_KEYBOARDCONTROLLER_H)
#define INC_TT_INPUT_KEYBOARDCONTROLLER_H

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <map>

#include <tt/input/ControllerIndex.h>
#include <tt/input/Button.h>
#include <tt/input/KeyList.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

struct KeyboardController
{
public:
	enum GetStringStatus
	{
		GetStringStatus_Starting,    // Called immediately after getWideString is called.
		GetStringStatus_CharAdded,   // A character was added to the string.
		GetStringStatus_CharRemoved, // A character was removed from the string.
		GetStringStatus_Complete,    // String retrieval was completed.
		GetStringStatus_Cancelled    // String retrieval was cancelled.
	};
	typedef bool (*GetStringCallback)(void*               p_userData,
	                                  const std::wstring& p_currentString,
	                                  GetStringStatus     p_status);
	
	static bool isConnected(ControllerIndex p_index);
	
	/*! \brief Returns the state of the controller. */
	static const KeyboardController& getState(ControllerIndex p_index);
	
	/*! \brief Updates the state of the keyboard. */
	static void update();
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \param p_window Handle of the window that this controller is associated with.
	    \return True if initialization was successful, false if not. */
	static bool initialize(HWND p_window);
	
	/*! \brief Deinitializes the controller. */
	static void deinitialize();
	
	/*! \brief Retrieve button state for a single keyboard key.
	    \param p_keyCode Can be "VK_RIGHT" (virtual key) or 'A' (char) */
	// FIXME: The p_keyCode parameter should probably be of type tt::input::Key,
	//        but this is more compatible with VK_* usage.
	// FIXME: This function is legacy and should no longer be used (access via getState instead)
	static const Button& getButtonState(BYTE p_keyCode);
	
	/*! \return Whether any key on the keyboard is down (to the knowledge of this controller). */
	static bool isAnyKeyDown();
	
	/*! \brief Retrieves a wide string from the keyboard, blocking regular key input until completed.
	    \param p_callback Callback function that gets updates about string retrieval status.
	    \param p_callbackUserData Optional custom (user) data that will be passed to the callback.
	    \param p_initialString Optional string to start with.
	    \param p_acceptKey The key that will complete the string retrieval.
	    \param p_cancelKey The key that will cancel string retrieval.
	    \return True if string retrieval was successfully started (callback will signal completion), false in case of error. */
	static bool getWideString(GetStringCallback   p_callback,
	                          void*               p_callbackUserData = 0,
	                          const std::wstring& p_initialString    = std::wstring(),
	                          Key                 p_acceptKey        = Key_Enter,
	                          Key                 p_cancelKey        = Key_Escape);
	
	static void acceptGetString();
	static void cancelGetString();
	
	/*! \return True if getWideString is active, false if it is not. */
	inline static bool isGettingWideString() { return ms_getStringCallback != 0; }
	
	/*! \brief Resets the registration of keys that are pressed. */
	static void resetPressedKeys();
	
	
	Button keys[Key_Count];
	std::wstring chars;
	
	bool capsLockOn;   //!< Whether Caps Lock is active
	bool scrollLockOn; //!< Whether Scroll Lock is active
	bool numLockOn;    //!< Whether Num Lock is active
	
private:
	KeyboardController();
	
	static bool isInitialized();
	static LRESULT CALLBACK wndProc(HWND p_wnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam);
	
	static void resetGetStringData();
	
	
	static HWND    ms_window;
	static WNDPROC ms_originalWndProc;
	
	static KeyboardController ms_controller;
	static bool               ms_keysDown[Key_Count]; //!< Whether a specific key code is down
	static std::wstring       ms_charsEntered;
	
	static KeyboardController::GetStringCallback ms_getStringCallback;
	static void*                                 ms_getStringCallbackUserData;
	static std::wstring                          ms_getStringText;
	static Key                                   ms_getStringAcceptKey;
	static Key                                   ms_getStringCancelKey;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_INPUT_KEYBOARDCONTROLLER_H)
