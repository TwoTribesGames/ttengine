#include <tt/input/SDLKeyboardController.h>

#include <tt/input/ControllerType.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_types.h>
#include <tt/str/common.h>

#include <SDL2/SDL.h>

namespace tt {
namespace input {

bool                  SDLKeyboardController::ms_initialized = false;
SDLKeyboardController SDLKeyboardController::ms_controller;
bool                  SDLKeyboardController::ms_keysDown[Key_Count] = { false };
std::wstring          SDLKeyboardController::ms_charsEntered;

SDLKeyboardController::GetStringCallback SDLKeyboardController::ms_getStringCallback         = 0;
void*                                    SDLKeyboardController::ms_getStringCallbackUserData = 0;
std::wstring                             SDLKeyboardController::ms_getStringText;
Key                                      SDLKeyboardController::ms_getStringAcceptKey        = Key_Enter;
Key                                      SDLKeyboardController::ms_getStringCancelKey        = Key_Escape;

//--------------------------------------------------------------------------------------------------
// Public member functions

bool SDLKeyboardController::isConnected(ControllerIndex p_index)
{
	return p_index == ControllerIndex_One && isInitialized();
}


const SDLKeyboardController& SDLKeyboardController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "SDLKeyboardController has not been initialized yet.");
	TT_ASSERTMSG(p_index == ControllerIndex_One, "Invalid controller index: %d", p_index);
	return ms_controller;
}


void SDLKeyboardController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLKeyboardController has not been initialized yet.");
		return;
	}
	
	// Update controller state
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
}


bool SDLKeyboardController::isInitialized()
{
	return ms_initialized;
}


bool SDLKeyboardController::initialize()
{
	if (isInitialized())
	{
		TT_PANIC("SDLKeyboardController is already initialized.");
		return false;
	}
	
	ms_initialized = true;
	return true;
}


void SDLKeyboardController::deinitialize()
{
	TT_ASSERTMSG(isInitialized(), "SDLKeyboardController is not initialized.");
	ms_initialized = false;
}

// Map SDL keys to "internal" keys
#define MapScanCode(s, i) case SDL_SCANCODE_##s: ret = i; break
static u8 mapKeyCodes(const SDL_Keysym& sym)
{
	u8 ret = 0;
	switch (sym.scancode)
	{
		MapScanCode(A, 'A');
		MapScanCode(B, 'B');
		MapScanCode(C, 'C');
		MapScanCode(D, 'D');
		MapScanCode(E, 'E');
		MapScanCode(F, 'F');
		MapScanCode(G, 'G');
		MapScanCode(H, 'H');
		MapScanCode(I, 'I');
		MapScanCode(J, 'J');
		MapScanCode(K, 'K');
		MapScanCode(L, 'L');
		MapScanCode(M, 'M');
		MapScanCode(N, 'N');
		MapScanCode(O, 'O');
		MapScanCode(P, 'P');
		MapScanCode(Q, 'Q');
		MapScanCode(R, 'R');
		MapScanCode(S, 'S');
		MapScanCode(T, 'T');
		MapScanCode(U, 'U');
		MapScanCode(V, 'V');
		MapScanCode(W, 'W');
		MapScanCode(X, 'X');
		MapScanCode(Y, 'Y');
		MapScanCode(Z, 'Z');

		MapScanCode(1, '1');
		MapScanCode(2, '2');
		MapScanCode(3, '3');
		MapScanCode(4, '4');
		MapScanCode(5, '5');
		MapScanCode(6, '6');
		MapScanCode(7, '7');
		MapScanCode(8, '8');
		MapScanCode(9, '9');
		MapScanCode(0, '0');
		
		MapScanCode(BACKSPACE, Key_Backspace);
		MapScanCode(TAB, Key_Tab);
		MapScanCode(SPACE, Key_Space);
		MapScanCode(RETURN, Key_Enter);
		MapScanCode(CLEAR, Key_Clear);
		MapScanCode(PAUSE, Key_Break);
		MapScanCode(ESCAPE, Key_Escape);
		
		MapScanCode(APOSTROPHE, Key_Apostrophe);
		MapScanCode(SEMICOLON, Key_Semicolon);
		MapScanCode(MINUS, Key_Minus);
		MapScanCode(COMMA, Key_Comma);
		MapScanCode(PERIOD, Key_Period);
		MapScanCode(SLASH, Key_Slash);
		
		MapScanCode(LEFTBRACKET, Key_LeftSquareBracket);
		MapScanCode(RIGHTBRACKET, Key_RightSquareBracket);
		MapScanCode(BACKSLASH, Key_Backslash);
		MapScanCode(GRAVE, Key_Grave);

		MapScanCode(KP_1, Key_Numpad1);
		MapScanCode(KP_2, Key_Numpad2);
		MapScanCode(KP_3, Key_Numpad3);
		MapScanCode(KP_4, Key_Numpad4);
		MapScanCode(KP_5, Key_Numpad5);
		MapScanCode(KP_6, Key_Numpad6);
		MapScanCode(KP_7, Key_Numpad7);
		MapScanCode(KP_8, Key_Numpad8);
		MapScanCode(KP_9, Key_Numpad9);
		MapScanCode(KP_0, Key_Numpad0);
		
		MapScanCode(KP_PERIOD, Key_Period);
		MapScanCode(KP_MULTIPLY, Key_Multiply);
		MapScanCode(KP_DIVIDE, Key_Slash);
		MapScanCode(KP_MINUS, Key_Minus);
		MapScanCode(KP_PLUS, Key_Plus);

		MapScanCode(UP, Key_Up);
		MapScanCode(DOWN, Key_Down);
		MapScanCode(LEFT, Key_Left);
		MapScanCode(RIGHT, Key_Right);
		MapScanCode(HOME, Key_Home);
		MapScanCode(END, Key_End);
		MapScanCode(INSERT, Key_Insert);
		MapScanCode(DELETE, Key_Delete);
		MapScanCode(PAGEUP, Key_PageUp);
		MapScanCode(PAGEDOWN, Key_PageDown);
		
		MapScanCode(F1, Key_F1);
		MapScanCode(F2, Key_F2);
		MapScanCode(F3, Key_F3);
		MapScanCode(F4, Key_F4);
		MapScanCode(F5, Key_F5);
		MapScanCode(F6, Key_F6);
		MapScanCode(F7, Key_F7);
		MapScanCode(F8, Key_F8);
		MapScanCode(F9, Key_F9);
		MapScanCode(F10, Key_F10);
		MapScanCode(F11, Key_F11);
		MapScanCode(F12, Key_F12);
		
		MapScanCode(NUMLOCKCLEAR, Key_NumLock);
		MapScanCode(CAPSLOCK, Key_CapsLock);
		MapScanCode(SCROLLLOCK, Key_ScrollLock);
		MapScanCode(LCTRL, Key_Control);
		MapScanCode(LSHIFT, Key_Shift);
		MapScanCode(LALT, Key_Control);
//		MapScanCode(LGUI, 0);
		MapScanCode(RCTRL, Key_Control);
		MapScanCode(RSHIFT, Key_Shift);
		MapScanCode(RALT, Key_Control);
//		MapScanCode(RGUI, 0);
		
	default:
		break;
	}
	return ret;
}

int SDLKeyboardController::processEvent(const SDL_Event& event)
{
	switch (event.type) {
		case SDL_KEYDOWN:
			if (ms_getStringCallback != 0) {
				int mappedKey = mapKeyCodes(event.key.keysym);
				if (mappedKey == ms_getStringAcceptKey)
				{
					acceptGetString();
				}
				else if (mappedKey == ms_getStringCancelKey)
				{
					cancelGetString();
				}
				else if (mappedKey == Key_Tab)
				{
					// Ignore tab character
					break;
				}
				else if (mappedKey == Key_Backspace)
				{
					if (ms_getStringText.empty() == false)
					{
					    ms_getStringText.erase(ms_getStringText.length() - 1);
					}
					ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText,
					                     GetStringStatus_CharRemoved);
				}
				break;
			}
			// this falls through to the KEYUP event
		case SDL_KEYUP:
			if (ms_getStringCallback == 0)
			{
				bool state = event.key.state == SDL_PRESSED;
				ms_keysDown[mapKeyCodes(event.key.keysym)] = state;
			}
			break;
		case SDL_TEXTINPUT:
			if (ms_getStringCallback == 0)
			{
				ms_charsEntered += tt::str::utf8ToUtf16(event.text.text);
			}
			else
			{
				ms_getStringText += tt::str::utf8ToUtf16(event.text.text);
				if (ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText,
										 GetStringStatus_CharAdded) == false)
				{
					ms_getStringText.erase(ms_getStringText.length() - 1);
				}
			}
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				resetPressedKeys();
			}
			break;
	}
	return 0;
}


bool SDLKeyboardController::getWideString(SDLKeyboardController::GetStringCallback p_callback,
                                          void*                                 p_callbackUserData,
                                          const std::wstring&                   p_initialString,
                                          Key                                   p_acceptKey,
                                          Key                                   p_cancelKey)
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLKeyboardController is not initialized.");
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
	
	ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText,
	                     GetStringStatus_Starting);
	
	return true;
}


void SDLKeyboardController::acceptGetString()
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLKeyboardController is not initialized.");
		return;
	}
	
	if (ms_getStringCallback == 0)
	{
		TT_Printf("acceptGetString called with out any callback?");
		return;
	}
	
	if (ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText, GetStringStatus_Complete))
	{
		resetGetStringData();
	}
}

void SDLKeyboardController::cancelGetString()
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLKeyboardController is not initialized.");
		return;
	}
	
	if (ms_getStringCallback == 0)
	{
		TT_Printf("acceptGetString called with out any callback?");
		return;
	}
	
	if (ms_getStringCallback(ms_getStringCallbackUserData, ms_getStringText, GetStringStatus_Cancelled))
	{
		resetGetStringData();
	}
}


bool SDLKeyboardController::isGettingWideString()
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLKeyboardController is not initialized.");
		return false;
	}
	
	return ms_getStringCallback != 0;
}

void SDLKeyboardController::resetGetStringData()
{
	ms_getStringCallback         = 0;
	ms_getStringCallbackUserData = 0;
	ms_getStringText.clear();
	ms_getStringAcceptKey        = Key_Enter;
	ms_getStringCancelKey        = Key_Escape;
}

void SDLKeyboardController::resetPressedKeys()
{
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_keysDown[i] = false;
	}
}

// Namespace end
}
}
