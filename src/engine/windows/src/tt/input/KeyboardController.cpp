#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windowsx.h>

#include <tt/app/Application.h>
#include <tt/input/ControllerType.h>
#include <tt/input/KeyboardController.h>
#include <tt/math/math.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace input {

HWND    KeyboardController::ms_window          = 0;
WNDPROC KeyboardController::ms_originalWndProc = 0;

KeyboardController KeyboardController::ms_controller;
bool               KeyboardController::ms_keysDown[Key_Count] = { false };
std::wstring       KeyboardController::ms_charsEntered;

KeyboardController::GetStringCallback KeyboardController::ms_getStringCallback         = 0;
void*                                 KeyboardController::ms_getStringCallbackUserData = 0;
std::wstring                          KeyboardController::ms_getStringText;
Key                                   KeyboardController::ms_getStringAcceptKey        = Key_Enter;
Key                                   KeyboardController::ms_getStringCancelKey        = Key_Escape;


//--------------------------------------------------------------------------------------------------
// Public member functions

bool KeyboardController::isConnected(ControllerIndex p_index)
{
	return p_index == ControllerIndex_One && isInitialized();
}


const KeyboardController& KeyboardController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "KeyboardController has not been initialized yet.");
	TT_ASSERTMSG(p_index == ControllerIndex_One, "Invalid controller index: %d", p_index);
	return ms_controller;
}


void KeyboardController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("KeyboardController has not been initialized yet.");
		return;
	}
	
	// Copy the temporary controller state to the working state
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_controller.keys[i].update(ms_keysDown[i]);
		if (ms_keysDown[i])
		{
			onControllerTypeUsed(input::ControllerType_Keyboard);
		}
	}
	ms_controller.chars = ms_charsEntered;
	ms_charsEntered.clear();
	
	ms_controller.capsLockOn   = ((GetKeyState(VK_CAPITAL) & 0xFF) != 0);
	ms_controller.scrollLockOn = ((GetKeyState(VK_SCROLL ) & 0xFF) != 0);
	ms_controller.numLockOn    = ((GetKeyState(VK_NUMLOCK) & 0xFF) != 0);
}


bool KeyboardController::initialize(HWND p_window)
{
	if (isInitialized())
	{
		TT_PANIC("KeyboardController is already initialized.");
		return false;
	}
	
	if (p_window == 0)
	{
		TT_PANIC("Invalid window handle specified.");
		return false;
	}
	
	ms_window = p_window;
	
	// Set the button containers to an initial state
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_keysDown[i]        = false;
		ms_controller.keys[i] = Button();
	}
	
	// Get the current window procedure
	ms_originalWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(p_window, GWLP_WNDPROC));
	if (ms_originalWndProc == 0)
	{
		TT_PANIC("Retrieving current window procedure failed.");
		return false;
	}
	
	// Set our own window procedure as current
	if (SetWindowLongPtr(p_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndProc)) == 0)
	{
		TT_PANIC("Setting new window procedure failed.");
		ms_originalWndProc = 0;
		return false;
	}
	
	return true;
}


void KeyboardController::deinitialize()
{
	// Restore the original window procedure
	SetWindowLongPtr(ms_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ms_originalWndProc));
	ms_window          = 0;
	ms_originalWndProc = 0;
}


const Button& KeyboardController::getButtonState(BYTE p_keyCode)
{
	// The windows key table uses the upper case ASCII hex values for character keys,
	// so if the given character is lower case make it upper case.
	// FIXME: Should we continue using this hack, or must client code be fixed?
	//        This hack interferes with real key codes in that range.
	//if (p_character > 0x60 && p_character < 0x7B) p_character -= 0x20;
	
	return ms_controller.keys[p_keyCode];
}


bool KeyboardController::isAnyKeyDown()
{
	for (s32 i = 0; i < Key_Count; ++i)
	{
		if (ms_controller.keys[i].down)
		{
			// A key is pressed!
			return true;
		}
	}
	return false;
}


bool KeyboardController::getWideString(KeyboardController::GetStringCallback p_callback,
                                       void*                                 p_callbackUserData,
                                       const std::wstring&                   p_initialString,
                                       Key                                   p_acceptKey,
                                       Key                                   p_cancelKey)
{
	if (isInitialized() == false)
	{
		TT_PANIC("KeyboardController is not initialized.");
		return false;
	}
	
	if (p_callback == 0)
	{
		TT_PANIC("No callback specified.");
		return false;
	}
	
	ms_getStringCallback         = p_callback;
	ms_getStringCallbackUserData = p_callbackUserData;
	ms_getStringText             = p_initialString;
	ms_getStringAcceptKey        = p_acceptKey;
	ms_getStringCancelKey        = p_cancelKey;
	
	resetPressedKeys();
	
	// Send the callback a "starting" update
	ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText, GetStringStatus_Starting);
	
	return true;
}


void KeyboardController::acceptGetString()
{
	if (ms_getStringCallback == 0)
	{
		return;
	}
	
	if (ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText, GetStringStatus_Complete))
	{
		resetGetStringData();
	}
}


void KeyboardController::cancelGetString()
{
	if (ms_getStringCallback == 0)
	{
		return;
	}
	
	if (ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText, GetStringStatus_Cancelled))
	{
		resetGetStringData();
	}
}


void KeyboardController::resetPressedKeys()
{
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_keysDown[i] = false;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

KeyboardController::KeyboardController()
:
chars(),
capsLockOn(false),
scrollLockOn(false),
numLockOn(false)
{
}


bool KeyboardController::isInitialized()
{
	return ms_window != 0;
}


LRESULT CALLBACK KeyboardController::wndProc(HWND p_wnd, UINT p_msg, WPARAM p_wparam, LPARAM p_lparam)
{
	// NOTE: Do not modify ms_controller here!
	
	switch (p_msg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (ms_getStringCallback != 0)
		{
			// Ignore/disable regular key input while retrieving a string from the keyboard
			break;
		}
		
		// Add presses to the temp container
		{
			const bool keyIsDown = (p_msg == WM_KEYDOWN || p_msg == WM_SYSKEYDOWN);
			const BYTE keyCode   = static_cast<BYTE>(p_wparam & 0xFF);
			ms_keysDown[keyCode] = keyIsDown;
			
			// If Alt-space is pressed in windowed mode, this will trigger the system menu:
			// reset the pressed keys (we no longer receive key released events once the system menu is opened)
			if (p_msg == WM_SYSKEYDOWN && keyCode == Key_Space &&
			    (app::hasApplication() == false || app::getApplication()->isFullScreen() == false))
			{
				resetPressedKeys();
			}
			
			// If Alt-F4 is pressed, allow default behavior to continue (so that the window actually closes)
			if (p_msg == WM_SYSKEYDOWN && keyCode == Key_F4)
			{
				break;
			}
			
			// If Alt-Enter is pressed, allow default behavior to continue (so that DXUT receives the key combination)
			if (p_msg == WM_SYSKEYDOWN && keyCode == Key_Enter)
			{
				break;
			}
		}
		
		// Always report key input messages as handled
		return 0;
		
	case WM_CHAR:
		{
			const std::wstring::value_type character = static_cast<std::wstring::value_type>(p_wparam);

			// Only interested in pre-translated character input for string retrieval
			if (ms_getStringCallback == 0)
			{
				ms_charsEntered += character;
				break;
			}
			
			if (character == ms_getStringAcceptKey)
			{
				acceptGetString();
			}
			else if (character == ms_getStringCancelKey)
			{
				cancelGetString();
			}
			else if (character == VK_BACK)
			{
				if (ms_getStringText.empty() == false)
				{
					ms_getStringText.erase(ms_getStringText.length() - 1);
				}
				ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText,
				                     GetStringStatus_CharRemoved);
			}
			else
			{
				// Assume anything the user types is a printable character
				ms_getStringText += character;
				if (ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText,
				                         GetStringStatus_CharAdded) == false)
				{
					ms_getStringText.erase(ms_getStringText.length() - 1);
				}
			}
		}
		return 0;
		
	case WM_ACTIVATE:
		if (LOWORD(p_wparam) == WA_INACTIVE)
		{
			// The main application window is being deactivated (user switched to a different window).
			// This window will no longer get key messages. To make sure no button is stuck with down
			// (because an up message might be missed), make all buttons go up.
			
			resetPressedKeys();
		}
		break;
		
	case WM_ACTIVATEAPP:
		if (p_wparam == FALSE)
		{
			// Our application is being deactivated (user switched to a different application).
			// Will no longer get key messages. To make sure no button is stuck with down
			// (because an up message might be missed), make all buttons go up.
			
			resetPressedKeys();
		}
		break;
	}
	
	return CallWindowProc(ms_originalWndProc, p_wnd, p_msg, p_wparam, p_lparam);
}


void KeyboardController::resetGetStringData()
{
	ms_getStringCallback         = 0;
	ms_getStringCallbackUserData = 0;
	ms_getStringText.clear();
	ms_getStringAcceptKey = Key_Enter;
	ms_getStringCancelKey = Key_Escape;
}

// Namespace end
}
}
