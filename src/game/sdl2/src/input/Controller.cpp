#include <tt/app/Application.h>
#include <tt/input/KeyBindings.h>
#include <tt/input/KeyboardController.h>
#include <tt/input/MouseController.h>
#include <tt/input/SDLJoypadController.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/input/Controller.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace input {

static tt::input::ControllerIndex activeController = tt::input::ControllerIndex_One;

//--------------------------------------------------------------------------------------------------
// Public member functions


void Controller::update(real p_time)
{
	prev = cur;
	
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
		// Update with fixed time step
		m_rumbleTime -= 1.0f / tt::app::getApplication()->getTargetFPS();
		
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
	
	const SDLJoypadController&  gamepad(SDLJoypadController::getState(activeController));
	const MouseController&    mouse  (MouseController::getState(ttCtrlIndex));
	const KeyboardController& kbd    (KeyboardController::getState(ttCtrlIndex));
	
	newInput.toggleHudVisible.update(kbd.keys[tt::input::Key_H].down);
	
	newInput.editor.pointer = mouse.cursor;
	newInput.editor.pointerLeft.update  (mouse.left.down);
	newInput.editor.pointerMiddle.update(mouse.middle.down);
	newInput.editor.pointerRight.update (mouse.right.down);
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		newInput.editor.keys[i].update(kbd.keys[i].down);
	}
	newInput.editor.chars        = kbd.chars;
	newInput.editor.capsLockOn   = kbd.capsLockOn;
	newInput.editor.scrollLockOn = kbd.scrollLockOn;
	newInput.editor.numLockOn    = kbd.numLockOn;
	
	// Update pointer
	newInput.pointer = mouse.cursor;
	
	// Update buttons
	newInput.accept.update(gamepad.a.down || isKeyDown(BindableAction_Accept));
	newInput.cancel.update(gamepad.b.down || isKeyDown(BindableAction_Cancel) || mouse.right.down);
	newInput.leaderboards.update(gamepad.y.down || kbd.keys[tt::input::Key_2].down); 
	newInput.statistics.update(gamepad.x.down || kbd.keys[tt::input::Key_1].down);
	
	newInput.scroll    = getNormalizedStick(gamepad.rstick); // This needs to be done before the scroll addon below.
	
	const bool rawLeft  = isKeyDown(BindableAction_Left)  || gamepad.left.down;
	const bool rawRight = isKeyDown(BindableAction_Right) || gamepad.right.down;
	const bool rawUp    = isKeyDown(BindableAction_Up)    || gamepad.up.down;
	const bool rawDown  = isKeyDown(BindableAction_Down)  || gamepad.down.down;
	
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
	
	bool isDownPrimaryFire   = false;
	bool isDownSecondaryFire = false;
	bool isDownVirusUpload   = false;
	bool isDownJump          = false;
	
	switch (m_gamepadControlScheme)
	{
	case GamepadControlScheme_A1:
		isDownPrimaryFire   = gamepad.rstick.length() > 0.75f || mouse.left.down;
		isDownSecondaryFire = gamepad.rtrig.value > 0.5f || mouse.right.down;
		isDownVirusUpload   = isKeyDown(BindableAction_Hack) || gamepad.l.down || gamepad.r.down;
		isDownJump          = gamepad.ltrig.value > 0.5f || isKeyDown(BindableAction_Jump);
		break;
		
	case GamepadControlScheme_A2:
		isDownPrimaryFire   = gamepad.rstick.length() > 0.75f || mouse.left.down;
		isDownSecondaryFire = gamepad.rtrig.value > 0.5f || mouse.right.down;
		isDownVirusUpload   = isKeyDown(BindableAction_Hack) || gamepad.ltrig.value > 0.5f || gamepad.r.down;
		isDownJump          = gamepad.l.down || isKeyDown(BindableAction_Jump);
		break;
		
	case GamepadControlScheme_B1:
		isDownPrimaryFire   = gamepad.rtrig.value > 0.5f || mouse.left.down;
		isDownSecondaryFire = gamepad.r.down || mouse.right.down;
		isDownVirusUpload   = isKeyDown(BindableAction_Hack) || gamepad.l.down;
		isDownJump          = gamepad.ltrig.value > 0.5f || isKeyDown(BindableAction_Jump);
		break;
		
	case GamepadControlScheme_B2:
		isDownPrimaryFire   = gamepad.rtrig.value > 0.5f || mouse.left.down;
		isDownSecondaryFire = gamepad.r.down || mouse.right.down;
		isDownVirusUpload   = isKeyDown(BindableAction_Hack) || gamepad.ltrig.value > 0.5f;
		isDownJump          =  gamepad.l.down || isKeyDown(BindableAction_Jump);
		break;
		
	default:
		TT_PANIC("Unhandled control scheme '%d'", m_gamepadControlScheme);
		break;
	}
	
	newInput.primaryFire.update(isDownPrimaryFire);
	newInput.secondaryFire.update(isDownSecondaryFire);
	newInput.virusUpload.update(isDownVirusUpload);
	newInput.jump.update(isDownJump);
	
	newInput.selectWeapon1.update(gamepad.x.down || isKeyDown(BindableAction_SelectWeapon1));
	newInput.selectWeapon2.update(gamepad.a.down || isKeyDown(BindableAction_SelectWeapon2));
	newInput.selectWeapon3.update(gamepad.y.down || isKeyDown(BindableAction_SelectWeapon3));
	newInput.selectWeapon4.update(gamepad.b.down || isKeyDown(BindableAction_SelectWeapon4));
	newInput.toggleWeapons.update(isKeyDown(BindableAction_ToggleWeapons));
	newInput.demoReset.update((gamepad.a.down && gamepad.b.down && gamepad.x.down && gamepad.y.down && gamepad.back.down) || kbd.keys[tt::input::Key_Numpad0].down);
	newInput.respawn.update(
		gamepad.a.down || gamepad.b.down || gamepad.x.down || gamepad.y.down ||
		kbd.keys[tt::input::Key_Enter].down || kbd.keys[tt::input::Key_Space].down || mouse.right.down || mouse.left.down
	);
	
	newInput.menu.update(gamepad.start.down || isKeyDown(BindableAction_OpenMenu));
	
	// Debug buttons
#if !defined(TT_BUILD_FINAL)
	newInput.debugCheat.update(kbd.keys[tt::input::Key_Slash].down || 
	                     (gamepad.ltrig.value > 0.5f && gamepad.rtrig.value > 0.5f && gamepad.l.down && gamepad.r.down && gamepad.lthumb.down && gamepad.rthumb.down));
	newInput.debugRestart.update(kbd.keys[tt::input::Key_Backspace].down);
#endif
	
	// Keys that are not bindable
	newInput.screenSwitch .update(gamepad.back.down || kbd.keys[tt::input::Key_Home].down);
	
	newInput.direction = getNormalizedStick(gamepad.lstick);
	newInput.gyroStick = newInput.direction;
	
	newInput.panCamera.update(mouse.middle.down);
	
	newInput.toggleEditor.update(kbd.keys[tt::input::Key_Tab].down);
	
	newInput.wheelNotches = mouse.wheelNotches;
	
	newInput.startupFailSafeLevel.update(
			(gamepad.a.down && gamepad.b.down && gamepad.x.down && gamepad.y.down &&
			 gamepad.l.down && gamepad.r.down && gamepad.ltrig.value > 0.5f && gamepad.rtrig.value > 0.5f) ||
			kbd.keys[tt::input::Key_F7].down);
	
	// Mouse clicks reset the pointer auto-hide timeout
	if (mouse.left.pressed || mouse.right.pressed)
	{
		m_pointerStationaryTime = 0.0f;
	}
	
	platform.mergeState(newInput);
}


void Controller::setPointerVisible(bool p_visible)
{
	m_pointerVisible = p_visible;
	tt::input::MouseController::setCursorVisible(p_visible);
}


void Controller::setConnectionRequired(bool /*p_required*/)
{
	// Not implemented on this platform
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void Controller::startRumble(RumbleStrength /*p_strength*/, real /*p_durationInSeconds*/)
{
	// Nothing to do here: regular update processing takes care of starting rumble
}


void Controller::stopRumbleImpl(bool p_immediately)
{
	// Only immediately update the rumble setting if requested.
	// Otherwise, regular update processing will take care of stopping rumble.
	if (p_immediately)
	{
		tt::input::SDLJoypadController::setRumble(activeController, 0.0f);
	}
}


bool Controller::isKeyDown(BindableAction p_action) const
{
	KeyMapping::const_iterator it = m_keyMapping.find(p_action);
	if (it != m_keyMapping.end())
	{
		using namespace tt::input;
		const tt::input::ControllerIndex ttCtrlIndex = tt::input::ControllerIndex_One;
		const KeyboardController& kbd (KeyboardController::getState(ttCtrlIndex));
		
#ifndef TT_BUILD_FINAL
		// Disable alt/ctrl and shift in non-final builds as they are used for debug functionality
		if (AppGlobal::allowLevelCreatorDebugFeaturesInGame() &&
			(kbd.keys[tt::input::Key_Control].down || kbd.keys[tt::input::Key_Alt].down || kbd.keys[tt::input::Key_Shift].down))
		{
			return false;
		}
#endif
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
