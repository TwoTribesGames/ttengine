#if !defined(INC_TT_INPUT_KEYBOARDCONTROLLER_H)
#define INC_TT_INPUT_KEYBOARDCONTROLLER_H

#if defined(TT_PLATFORM_OSX_MAC) // this file is for Mac OS X (not iOS) builds only


#include <string>

#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/KeyList.h>


namespace tt {
namespace input {

/*! \brief TT input controller implementation for Mac OS X keyboard events. */
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
	typedef bool (*GetStringCallback)(void* p_userData, const std::wstring& p_currentString, GetStringStatus p_status);
	
	
	static bool isConnected(ControllerIndex p_index);
	
	/*! \brief Returns the state of the controller. */
	static const KeyboardController& getState(ControllerIndex p_index);
	
	static void update();
	
	/*! \brief Indicates whether the controller has been initialized.
	    \return True when initialized, false if not. */
	static bool isInitialized();
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \return True if initialization was successful, false if not. */
	static bool initialize(void* p_appView);
	
	static void deinitialize();
	
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
	
	static bool isGettingWideString();
	
	static void resetPressedKeys();
	
	
	// NOTE: This function is for use by TTdevObjCAppView only!
	// (declaring an Objective C class as friend is impossible, which is why this function is public)
	static inline void setKeyDown(Key p_key, bool p_down) { ms_keysDown[p_key] = p_down; }
	static inline void addTypedChars(const std::wstring& p_chars) { ms_charsEntered += p_chars; }
	static void handleViewChanged();
	
	
	Button       keys[Key_Count];
	std::wstring chars;
	
private:
	static void*              ms_appView;
	static KeyboardController ms_controller;
	static bool               ms_keysDown[Key_Count]; // accumulates key state until update()
	static std::wstring       ms_charsEntered;        // accumulates characters (not key codes) entered until update()
};

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_MAC)

#endif  // !defined(INC_TT_INPUT_KEYBOARDCONTROLLER_H)
