#include <tt/input/KeyBindings.h>
#include <tt/input/KeyboardController.h>
#include <tt/input/MouseController.h>
#include <tt/input/SDLJoypadController.h>
#include <tt/math/Vector2.h>

#include <toki/input/Controller.h>
#include <toki/AppGlobal.h>

#define USE_SDL 1

namespace toki {
namespace input {

#if USE_SDL
static tt::input::ControllerIndex activeController = tt::input::ControllerIndex_One;
#endif

//--------------------------------------------------------------------------------------------------
// Public member functions

void Controller::update(real p_time)
{
	prev = cur;
	
#if USE_SDL
	// Get first connected controller
	for(u32 i = 0; i < 4; ++i)
	{
		tt::input::ControllerIndex index = static_cast<tt::input::ControllerIndex>(i);

		if(tt::input::SDLJoypadController::isConnected(index))
		{
			activeController = index;
			break;
		}
	}
	
	
	static const real rumbleForce[] =
	{
		0.33f, // RumbleStrength_Low
		0.66f, // RumbleStrength_Medium
		1.0f   // RumbleStrength_High
	};
	TT_STATIC_ASSERT((sizeof(rumbleForce) / sizeof(rumbleForce[0])) == RumbleStrength_Count);
	
	if (m_rumbleTime > 0.0f)
	{
		m_rumbleTime -= p_time;
		
		// Stop condition
		real force = rumbleForce[m_rumbleStrength];
		if (m_rumbleTime <= 0.16666667f)
		{
			force = 0.0f;
			
			m_rumbleStrength = RumbleStrength_Low;
			m_rumbleTime     = 0.0f;
		}
		tt::input::SDLJoypadController::setRumble(activeController, force);
	}
#endif
	
	cur.updateState(platform, false);
	
	updatePointerVisibility(p_time);
}


void Controller::updatePlatformState()
{
	State newInput;
	
	const tt::input::ControllerIndex ttCtrlIndex = tt::input::ControllerIndex_One;
	
	using tt::input::SDLJoypadController;
	using tt::input::MouseController;
	using tt::input::KeyboardController;
	
	// FIXME: With the new multiple game ticks per update frame we should actually update the 
	//        platform controllers here for the second, third, etc.. ticks. 
	//        (the first already update by app.)
	//        This will fix the input staying 'stuck' or not responding quickly with slow framerate.
	
#if USE_SDL
	const SDLJoypadController&  gamepad(SDLJoypadController::getState(activeController));
#else
	const SDLJoypadController gamepad; // empty gamepad
#endif
	const MouseController&    mouse(MouseController::getState(ttCtrlIndex));
	const KeyboardController& kbd(  KeyboardController::getState(ttCtrlIndex));
	
	newInput.toggleHudVisible.update(kbd.keys[tt::input::Key_H].down);
	
	newInput.editor.pointer = mouse.cursor;
	newInput.editor.pointerLeft  .update(mouse.left  .down);
	newInput.editor.pointerMiddle.update(mouse.middle.down);
	newInput.editor.pointerRight .update(mouse.right .down);
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		newInput.editor.keys[i].update(kbd.keys[i].down);
	}
	newInput.editor.chars = kbd.chars;
#if !defined(TT_PLATFORM_OSX)
	// NOTE: No capslock, numlock, scrolllock for OSX
	newInput.editor.capsLockOn   = kbd.capsLockOn;
	newInput.editor.scrollLockOn = kbd.scrollLockOn;
	newInput.editor.numLockOn    = kbd.numLockOn;
#endif	
	
	// Update pointer
	newInput.pointer = mouse.cursor;
	
	// Update buttons
	newInput.action .update(mouse.left.down);
	newInput.accept.update(gamepad.a.down || isKeyDown(BindableAction_Accept));
	newInput.cancel.update(gamepad.b.down || isKeyDown(BindableAction_Cancel));	
	
	const bool rawLeft  = gamepad.left.down  || isKeyDown(BindableAction_Left);
	const bool rawRight = gamepad.right.down || isKeyDown(BindableAction_Right);
	const bool rawUp    = gamepad.up.down    || isKeyDown(BindableAction_Up);
	const bool rawDown  = gamepad.down.down  || isKeyDown(BindableAction_Down);
	
	newInput.scroll    = getNormalizedStick(gamepad.rstick); // This needs to be done before the scroll addon below.
	
	// While pressing the modifiers the direction is mapped to scroll.
	if (isKeyDown(BindableAction_Scroll))
	{
		if (rawLeft)
		{
			newInput.scroll.x -= 1.0f;
		}
		if (rawRight)
		{
			newInput.scroll.x += 1.0f;
		}
		if (rawUp)
		{
			newInput.scroll.y += 1.0f;
		}
		if (rawDown)
		{
			newInput.scroll.y -= 1.0f;
		}
		newInput.scroll.normalizeClamp();
		
		// Disable directional movement
		newInput.left .update(false);
		newInput.right.update(false);
		newInput.up   .update(false);
		newInput.down .update(false);
	}
	else
	{
		newInput.left .update(rawLeft);
		newInput.right.update(rawRight);
		newInput.up   .update(rawUp);
		newInput.down .update(rawDown);
	}
	
	
	// Keys that are bindable
	newInput.stomp  .update(isKeyDown(BindableAction_Stomp) || gamepad.b.down || gamepad.x.down || gamepad.rtrig.value > 0.5f || gamepad.ltrig.value > 0.5f);
	newInput.whistle.update(isKeyDown(BindableAction_Whistle) || gamepad.a.down || gamepad.y.down);
	newInput.menu.update(isKeyDown(BindableAction_OpenMenu) || gamepad.start.down);
	
	// Keys that are not bindable
	newInput.restart      .update(kbd.keys[tt::input::Key_Backspace].down);
	newInput.tokiDex      .update(kbd.keys[tt::input::Key_Slash].down ||
	                              kbd.keys[tt::input::Key_SlashQuestionmark].down);
	newInput.tokiDexCamera.update(kbd.keys[tt::input::Key_Period].down || gamepad.l.down || gamepad.r.down);
	
	newInput.panCamera           .update(mouse.middle.down || mouse.right.down);
	newInput.toggleEditor        .update(kbd.keys[tt::input::Key_Tab].down);
	newInput.startupFailSafeLevel.update(kbd.keys[tt::input::Key_F7].down);
	newInput.wheelNotches = mouse.wheelNotches;
	
	newInput.screenSwitch.update(gamepad.back.down || kbd.keys[tt::input::Key_Home].down);
	
	newInput.direction = getNormalizedStick(gamepad.lstick);
	newInput.gyroStick = newInput.direction;
	
	
	// Mouse clicks reset the pointer auto-hide timeout
	if (mouse.left.pressed || mouse.right.pressed)
	{
		m_pointerStationaryTime = 0.0f;
	}
	
	platform.mergeState(newInput);
}


void Controller::setConnectionRequired(bool /*p_required*/)
{
	// Not implemented on this platform
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void Controller::setPointerVisible(bool p_visible)
{
	m_pointerVisible = p_visible;
	tt::input::MouseController::setCursorVisible(p_visible);
}


void Controller::startRumble(RumbleStrength /*p_strength*/, real /*p_durationInSeconds*/)
{
	// Nothing to do here: regular update processing takes care of starting rumble
}


void Controller::stopRumbleImpl(bool p_immediately)
{
#if USE_SDL
	// Only immediately update the rumble setting if requested.
	// Otherwise, regular update processing will take care of stopping rumble.
	if (p_immediately)
	{
		tt::input::SDLJoypadController::setRumble(activeController, 0.0f);
	}
#endif
}



bool Controller::isKeyDown(BindableAction p_action) const
{
	KeyMapping::const_iterator it = m_keyMapping.find(p_action);
	if (it != m_keyMapping.end())
	{
		using namespace tt::input;
		const tt::input::ControllerIndex ttCtrlIndex = tt::input::ControllerIndex_One;
		const KeyboardController& kbd (KeyboardController::getState(ttCtrlIndex));
		
		for (KeyList::const_iterator keyIt = it->second.begin(); keyIt != it->second.end(); ++keyIt)
		{
			// Ignore the space key in level creator mode, because it is used for mouse scrolling
			if (AppGlobal::allowLevelCreatorDebugFeaturesInGame() && (*keyIt) == Key_Space) continue;
			
			if (kbd.keys[*keyIt].down) return true;
		}
	}
	return false;
}


// Namespace end
}
}
