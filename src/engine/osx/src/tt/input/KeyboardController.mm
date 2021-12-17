#if defined(TT_PLATFORM_OSX_MAC) // this file is for Mac OS X (not iOS) builds only

#include <tt/app/TTdevObjCAppView_desktop.h>
#include <tt/input/KeyboardController.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

void*              KeyboardController::ms_appView = 0;
KeyboardController KeyboardController::ms_controller;
bool               KeyboardController::ms_keysDown[Key_Count] = { false };
std::wstring       KeyboardController::ms_charsEntered;


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
	
	// Update controller state
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_controller.keys[i].update(ms_keysDown[i]);
	}
	ms_controller.chars = ms_charsEntered;
	ms_charsEntered.clear();
	
	/*
	ms_controller.capsLockOn   = ;
	ms_controller.scrollLockOn = ;
	ms_controller.numLockOn    = ;
	*/
}


bool KeyboardController::isInitialized()
{
	return ms_appView != 0;
}


bool KeyboardController::initialize(void* p_appView)
{
	if (isInitialized())
	{
		TT_PANIC("KeyboardController is already initialized.");
		return false;
	}
	
	// Nothing special to do here; TTdevObjCAppView drives the input events
	ms_appView = p_appView;
	return true;
}


void KeyboardController::deinitialize()
{
	TT_ASSERTMSG(isInitialized(), "KeyboardController is not initialized.");
	ms_appView = 0;
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
	
	resetPressedKeys();
	
	// Pass the call on to the application view (which will handle the actual input)
	TTdevObjCAppView* view = (TTdevObjCAppView*)ms_appView;
	return [view startWideStringRetrieval:p_callback
	                             userData:p_callbackUserData
	                        initialString:p_initialString
	                            acceptKey:p_acceptKey
	                            cancelKey:p_cancelKey];
}


void KeyboardController::acceptGetString()
{
	if (isInitialized() == false)
	{
		TT_PANIC("KeyboardController is not initialized.");
		return;
	}
	
	// Pass the call on to the application view (which handles the actual input)
	TTdevObjCAppView* view = (TTdevObjCAppView*)ms_appView;
	[view acceptGetString];
}


void KeyboardController::cancelGetString()
{
	if (isInitialized() == false)
	{
		TT_PANIC("KeyboardController is not initialized.");
		return;
	}
	
	// Pass the call on to the application view (which handles the actual input)
	TTdevObjCAppView* view = (TTdevObjCAppView*)ms_appView;
	[view cancelGetString];
}


bool KeyboardController::isGettingWideString()
{
	if (isInitialized() == false)
	{
		TT_PANIC("KeyboardController is not initialized.");
		return false;
	}
	
	// Pass the call on to the application view (which handles the actual input)
	TTdevObjCAppView* view = (TTdevObjCAppView*)ms_appView;
	return [view isRetrievingWideString];
}


void KeyboardController::resetPressedKeys()
{
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_keysDown[i] = false;
	}
}


void KeyboardController::handleViewChanged()
{
	for (s32 i = 0; i < Key_Count; ++i)
	{
		ms_keysDown[i] = false;
	}
}

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_MAC)
