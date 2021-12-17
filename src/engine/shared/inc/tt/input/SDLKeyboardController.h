#if !defined(INC_TT_INPUT_SDLKEYBOARDCONTROLLER_H)
#define INC_TT_INPUT_SDLKEYBOARDCONTROLLER_H

#include <string>

#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/KeyList.h>

union SDL_Event;

namespace tt {
namespace input {

/*! \brief TT input controller implementation for SDL keyboard events. */
struct SDLKeyboardController
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
	static const SDLKeyboardController& getState(ControllerIndex p_index);

	/*! \brief Updates the state of the keyboard. */
	static void update();
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \return True if initialization was successful, false if not. */
	static bool initialize();
	
	/*! \brief Deinitializes the controller. */
	static void deinitialize();
	
	/*! \brief Processes an SDL2 event
	    \param event A reference to the event to process
	    \return 0 */
	static int processEvent(const SDL_Event &event);
	
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
	
	static bool isGettingWideString();
	
	static void resetPressedKeys();
	
	Button keys[Key_Count];
	std::wstring chars;

	bool capsLockOn;   //!< Whether Caps Lock is active
	bool scrollLockOn; //!< Whether Scroll Lock is active
	bool numLockOn;    //!< Whether Num Lock is active
	
private:
	static void resetGetStringData();
	
	/*! \brief Indicates whether the controller has been initialized.
	    \return True when initialized, false if not. */
	static bool isInitialized();

	static bool                  ms_initialized;
	static SDLKeyboardController ms_controller;
	static bool                  ms_keysDown[Key_Count]; // accumulates key state until update()
	static std::wstring          ms_charsEntered;        // accumulates characters (not key codes) entered until update()
	
	static GetStringCallback ms_getStringCallback;
	static void*             ms_getStringCallbackUserData;
	static std::wstring      ms_getStringText;
	static Key               ms_getStringAcceptKey;
	static Key               ms_getStringCancelKey;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_INPUT_SDLKEYBOARDCONTROLLER_H)
