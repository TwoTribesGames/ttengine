#include <tt/fs/fs.h>
#include <tt/fs/File.h>

#include <toki/input/Controller.h>
#include <toki/cfg.h>
#include <toki/savedata/utils.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace input {

static const char* g_userBindingsFilename = "keybindings.json";

void Controller::State::mergeState(const State& p_state)
{
	// Merge is used to collect the input updates for multiple frames into a single frame which the game can use for updates.
	// The important thing is not to miss button presses. For the other stuff the newer p_state can be used.
	
	pointer = p_state.pointer;
	
	accept              .update(accept.down               || p_state.accept.down);
	cancel              .update(cancel.down               || p_state.cancel.down);
	faceUp              .update(faceUp.down               || p_state.faceUp.down);
	faceLeft            .update(faceLeft.down             || p_state.faceLeft.down);
	left                .update(left.down                 || p_state.left.down);
	right               .update(right.down                || p_state.right.down);
	up                  .update(up.down                   || p_state.up.down);
	down                .update(down.down                 || p_state.down.down);
	virusUpload         .update(virusUpload.down          || p_state.virusUpload.down);
	jump                .update(jump.down                 || p_state.jump.down);
	primaryFire         .update(primaryFire.down          || p_state.primaryFire.down);
	secondaryFire       .update(secondaryFire.down        || p_state.secondaryFire.down);
	selectWeapon1       .update(selectWeapon1.down        || p_state.selectWeapon1.down);
	selectWeapon2       .update(selectWeapon2.down        || p_state.selectWeapon2.down);
	selectWeapon3       .update(selectWeapon3.down        || p_state.selectWeapon3.down);
	selectWeapon4       .update(selectWeapon4.down        || p_state.selectWeapon4.down);
	toggleWeapons       .update(toggleWeapons.down        || p_state.toggleWeapons.down);
	demoReset           .update(demoReset.down            || p_state.demoReset.down);
	respawn             .update(respawn.down              || p_state.respawn.down);
	menu                .update(menu.down                 || p_state.menu.down);
	screenSwitch        .update(screenSwitch.down         || p_state.screenSwitch.down);
	startupFailSafeLevel.update(startupFailSafeLevel.down || p_state.startupFailSafeLevel.down);
	
	direction = p_state.direction;
	scroll    = p_state.scroll;
	gyroStick = p_state.gyroStick;
	
	// Debug buttons
	debugCheat          .update(debugCheat.down           || p_state.debugCheat.down);
	debugRestart        .update(debugRestart.down         || p_state.debugRestart.down);
	
	panCamera.update(panCamera.down || p_state.panCamera.down);
	toggleEditor.update(toggleEditor.down || p_state.toggleEditor.down);
	wheelNotches = p_state.wheelNotches;
	
	toggleHudVisible.update (toggleHudVisible.down  || p_state.toggleHudVisible.down );
	
	
	editor.pointer = p_state.editor.pointer;
	editor.pointerLeft.update  (editor.pointerLeft.down   || p_state.editor.pointerLeft.down  );
	editor.pointerMiddle.update(editor.pointerMiddle.down || p_state.editor.pointerMiddle.down);
	editor.pointerRight.update (editor.pointerRight.down  || p_state.editor.pointerRight.down );
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		editor.keys[i].update(editor.keys[i].down || p_state.editor.keys[i].down);
	}
	editor.chars       += p_state.editor.chars;
	editor.capsLockOn   = editor.capsLockOn   || p_state.editor.capsLockOn;
	editor.scrollLockOn = editor.scrollLockOn || p_state.editor.scrollLockOn;
	editor.numLockOn    = editor.numLockOn    || p_state.editor.numLockOn;
}


void Controller::State::updateState(const State& p_state, bool p_doNonPlatform)
{
	pointer = p_state.pointer;

	bool stickLeftPressed = false;
	bool stickRightPressed = false;
	bool stickUpPressed = false;
	bool stickDownPressed = false;
	if (p_state.direction.length() > 0.6)
	{
		// Translate analog stick movement to left/right/up/down presses
		const real angle = p_state.direction.getAngleDeg();
		if      (angle > -45.0f  && angle <   45.0f) stickUpPressed    = true;
		else if (angle >= 45.0f  && angle <  135.0f) stickRightPressed = true;
		else if (angle >= 135.0f || angle < -135.0f) stickDownPressed  = true;
		else                                         stickLeftPressed  = true;
	}
	else if (p_state.scroll.length() > 0.6)
	{
		// Translate analog stick movement to left/right/up/down presses
		const real angle = p_state.scroll.getAngleDeg();
		if      (angle > -45.0f  && angle <   45.0f) stickUpPressed    = true;
		else if (angle >= 45.0f  && angle <  135.0f) stickRightPressed = true;
		else if (angle >= 135.0f || angle < -135.0f) stickDownPressed  = true;
		else                                         stickLeftPressed  = true;
	}
	
	/*
	if (p_doNonPlatform)
	{
		// ...
	}
	// */ (void)p_doNonPlatform;
	
	accept              .update(p_state.accept.down);
	cancel              .update(p_state.cancel.down);
	faceUp              .update(p_state.faceUp.down);
	faceLeft            .update(p_state.faceLeft.down);
	
	left                .update(p_state.left.down  || stickLeftPressed );
	right               .update(p_state.right.down || stickRightPressed);
	up                  .update(p_state.up.down    || stickUpPressed   );
	down                .update(p_state.down.down  || stickDownPressed );
	virusUpload         .update(p_state.virusUpload.down);
	jump                .update(p_state.jump.down);
	primaryFire         .update(p_state.primaryFire.down);
	secondaryFire       .update(p_state.secondaryFire.down);
	selectWeapon1       .update(p_state.selectWeapon1.down);
	selectWeapon2       .update(p_state.selectWeapon2.down);
	selectWeapon3       .update(p_state.selectWeapon3.down);
	selectWeapon4       .update(p_state.selectWeapon4.down);
	toggleWeapons       .update(p_state.toggleWeapons.down);
	demoReset           .update(p_state.demoReset.down);
	respawn             .update(p_state.respawn.down);
	menu                .update(p_state.menu.down);
	screenSwitch        .update(p_state.screenSwitch.down);
	startupFailSafeLevel.update(p_state.startupFailSafeLevel.down);
	
	direction = p_state.direction;
	scroll    = p_state.scroll;
	gyroStick = p_state.gyroStick;
	
	// Debug buttons
	debugCheat          .update(p_state.debugCheat.down);
	debugRestart        .update(p_state.debugRestart.down);
	
	// Add directional input to stick.
	if (p_state.left.down)  direction.x -= 1.0f;
	if (p_state.right.down) direction.x += 1.0f;
	if (p_state.up.down)    direction.y += 1.0f;
	if (p_state.down.down)  direction.y -= 1.0f;
	// Make sure we stay in the normalized range.
	direction.normalizeClamp();
	
	panCamera.update(p_state.panCamera.down);
	toggleEditor.update(p_state.toggleEditor.down);
	wheelNotches = p_state.wheelNotches;
	
	toggleHudVisible.update (p_state.toggleHudVisible.down );
	
	
	editor.pointer = p_state.editor.pointer;
	editor.pointerLeft.update  (p_state.editor.pointerLeft.down  );
	editor.pointerMiddle.update(p_state.editor.pointerMiddle.down);
	editor.pointerRight.update (p_state.editor.pointerRight.down );
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		editor.keys[i].update(p_state.editor.keys[i].down);
	}
	editor.chars        = p_state.editor.chars;
	editor.capsLockOn   = p_state.editor.capsLockOn;
	editor.scrollLockOn = p_state.editor.scrollLockOn;
	editor.numLockOn    = p_state.editor.numLockOn;
}


void Controller::State::releaseAll()
{
	pointer.reset();
	
	accept.update(false);
	cancel.update(false);
	faceUp.update(false);
	faceLeft.update(false);
	
	left.update(false);
	right.update(false);
	up.update(false);
	down.update(false);
	virusUpload.update(false);
	jump.update(false);
	primaryFire.update(false);
	secondaryFire.update(false);
	selectWeapon1.update(false);
	selectWeapon2.update(false);
	selectWeapon3.update(false);
	selectWeapon4.update(false);
	toggleWeapons.update(false);
	demoReset.update(false);
	respawn.update(false);
	menu.update(false);
	screenSwitch.update(false);
	
	direction.reset();
	scroll.reset();
	gyroStick.reset();
	
	debugCheat.update(false);
	debugRestart.update(false);
	
	panCamera.update(false);
	toggleEditor.update(false);
	wheelNotches = 0;
	
	toggleHudVisible.update(false);
	
	editor.pointer.reset();
	editor.pointerLeft.update  (false);
	editor.pointerMiddle.update(false);
	editor.pointerRight.update (false);
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		editor.keys[i].update(false);
	}
	editor.chars.clear();
	editor.capsLockOn   = false;
	editor.scrollLockOn = false;
	editor.numLockOn    = false;
}


void Controller::State::resetAllAndBlockUntilReleased()
{
	pointer.reset();
	
	accept.resetAndBlockUntilReleased();
	cancel.resetAndBlockUntilReleased();
	faceUp.resetAndBlockUntilReleased();
	faceLeft.resetAndBlockUntilReleased();
	
	left.resetAndBlockUntilReleased();
	right.resetAndBlockUntilReleased();
	up.resetAndBlockUntilReleased();
	down.resetAndBlockUntilReleased();
	virusUpload.resetAndBlockUntilReleased();
	jump.resetAndBlockUntilReleased();
	primaryFire.resetAndBlockUntilReleased();
	secondaryFire.resetAndBlockUntilReleased();
	selectWeapon1.resetAndBlockUntilReleased();
	selectWeapon2.resetAndBlockUntilReleased();
	selectWeapon3.resetAndBlockUntilReleased();
	selectWeapon4.resetAndBlockUntilReleased();
	toggleWeapons.resetAndBlockUntilReleased();
	demoReset.resetAndBlockUntilReleased();
	respawn.resetAndBlockUntilReleased();
	menu.resetAndBlockUntilReleased();
	screenSwitch.resetAndBlockUntilReleased();
	startupFailSafeLevel.resetAndBlockUntilReleased();
	
	direction.reset();
	scroll.reset();
	gyroStick.reset();
	
	// Debug buttons
	debugCheat.resetAndBlockUntilReleased();
	debugRestart.resetAndBlockUntilReleased();
	
	panCamera.resetAndBlockUntilReleased();
	toggleEditor.resetAndBlockUntilReleased();
	wheelNotches = 0;
	
	toggleHudVisible.resetAndBlockUntilReleased();
	
	editor.pointer.reset();
	editor.pointerLeft.resetAndBlockUntilReleased();
	editor.pointerMiddle.resetAndBlockUntilReleased();
	editor.pointerRight.resetAndBlockUntilReleased();
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		editor.keys[i].resetAndBlockUntilReleased();
	}
	editor.capsLockOn   = false;
	editor.scrollLockOn = false;
	editor.numLockOn    = false;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

Controller::Controller()
:
cur(),
prev(),
platform(),
m_rumbleEnabled(true),
m_rumbleStrength(RumbleStrength_Low),
m_rumbleTime(0.0f),
m_rumblePanning(0.0f),
m_gamepadControlScheme(GamepadControlScheme_A1),
m_platformMenuOpen(false),
m_pointerAutoVisibility(true),
m_pointerStationaryTime(0.0f),
m_pointerVisible(true),
m_pointerAutohideTimeout()
{
}


Controller::~Controller()
{
}


void Controller::init()
{
	initRumble();
	
	m_pointerAutohideTimeout = cfg()->getHandleReal("toki.input.pointer_autohide_timeout");
}


void Controller::rumble(RumbleStrength p_strength, real p_durationInSeconds, real p_panning)
{
#ifndef TT_BUILD_FINAL
	if (AppGlobal::isRumbleEnabled() == false)
	{
		return;
	}
#endif
	
	if (m_rumbleEnabled == false || p_strength < m_rumbleStrength)
	{
		// Lower strength rumble effect does not override already active effect
		return;
	}
	
	if (p_durationInSeconds < 0.0f)
	{
		TT_PANIC("Cannot make controller rumble for negative duration (%f seconds).",
		         p_durationInSeconds);
		return;
	}
	if (p_durationInSeconds > 1.0f)
	{
		TT_WARN("Cannot trigger vibration longer than 1 second (requested %f seconds). "
		        "Time was clamped to one second.", p_durationInSeconds);
		p_durationInSeconds = 1.0f;
	}
	
	m_rumbleStrength = p_strength;
	m_rumbleTime     = p_durationInSeconds;
	m_rumblePanning  = p_panning;
	startRumble(p_strength, p_durationInSeconds, p_panning);
}


void Controller::stopRumble(bool p_immediately)
{
	stopRumbleImpl(p_immediately);
	m_rumbleStrength = RumbleStrength_Low;
	m_rumbleTime     = 0.0f;
	m_rumblePanning  = 0.0f;
}


#if !defined(TT_BUILD_FINAL)

void Controller::debugRender() const
{
}

#endif  // !defined(TT_BUILD_FINAL)


void Controller::onPlatformMenuEnter()
{
	m_platformMenuOpen = true;
}


void Controller::onPlatformMenuExit()
{
	m_platformMenuOpen = false;
}


void Controller::updateCustomKeyBindings(const std::string& p_controllerID, const tt::input::ActionMap& p_bindings)
{
	using namespace tt::input;
	KeyBindings::setCustomControllerBindings(p_controllerID, p_bindings);
	assignKeyBindings();
}


void Controller::saveCustomKeyBindings()
{
	using namespace tt::input;
	tt::fs::FilePtr file = savedata::createSaveFile(g_userBindingsFilename);
	
	if (file == 0)
	{
		// Either not allowed to save or creating the save file failed
		// (underlying code will have triggered an appropriate panic message if so)
		return;
	}
	
	KeyBindings::saveAsJSON(file);
}


void Controller::loadCustomKeyBindings()
{
	// Set permanent members
	using namespace tt::input;
	{
		ActionMap permanents;
		permanents[getBindableActionName(BindableAction_Accept)]  .push_back(Key_Enter);
		permanents[getBindableActionName(BindableAction_Cancel)]  .push_back(Key_Escape);
		permanents[getBindableActionName(BindableAction_OpenMenu)].push_back(Key_Escape);
		permanents[getBindableActionName(BindableAction_Left)]    .push_back(Key_Left);
		permanents[getBindableActionName(BindableAction_Right)]   .push_back(Key_Right);
		permanents[getBindableActionName(BindableAction_Up)]      .push_back(Key_Up);
		permanents[getBindableActionName(BindableAction_Down)]    .push_back(Key_Down);
		permanents[getBindableActionName(BindableAction_Jump)]    .push_back(Key_Up);
		permanents[getBindableActionName(BindableAction_Scroll)]  .push_back(Key_Alt);
		permanents[getBindableActionName(BindableAction_Scroll)]  .push_back(Key_Control);
		KeyBindings::setPermanentControllerBindings(keyboardID, permanents);
	}
	
	const tt::input::RequiredActions requiredActions = 
	{
		getBindableActionName(BindableAction_Hack),
		getBindableActionName(BindableAction_SelectWeapon1),
		getBindableActionName(BindableAction_SelectWeapon2),
		getBindableActionName(BindableAction_SelectWeapon3),
		getBindableActionName(BindableAction_SelectWeapon4)
	};
	
	tt::fs::FilePtr bindingsFile = savedata::openSaveFile(g_userBindingsFilename);
	if (bindingsFile == 0)
	{
		copyDefaultKeyBindings(g_userBindingsFilename);
		// Try again, default file should have been copied now
		bindingsFile = savedata::openSaveFile(g_userBindingsFilename);
	}
	
	if (KeyBindings::createFromJSON(bindingsFile, requiredActions) == false)
	{
		bindingsFile.reset();
		
		copyDefaultKeyBindings(g_userBindingsFilename);
		bindingsFile = savedata::openSaveFile(g_userBindingsFilename);
		// Try again; if this still fails, don't care
		KeyBindings::createFromJSON(bindingsFile, requiredActions);
	}
	
	assignKeyBindings();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

tt::input::Stick Controller::getNormalizedStick(const tt::input::Stick& p_stick) const
{
	tt::input::Stick result;
	
	// Deadzone
	const real deadzoneSize = 0.25f; // FIXME: Add this value to the cfg.
	const real lengthSquared = p_stick.lengthSquared();
	if (lengthSquared > (deadzoneSize * deadzoneSize) + 0.0001f)
	{
		result = p_stick;
		if (lengthSquared > 1.0f)
		{
			result.normalize();
		}
		
		const tt::math::Vector2 normalized = p_stick.getNormalized();
		result -= (normalized * deadzoneSize); // Remove deadzone
		result *= 1.0f / (1.0f - deadzoneSize); // Scale back to full range. (-1.0 - 1.0f)
		
		// Make 100% sure result is normalized (rounding errors of the calculations above can lead to length() > 0
		if (result.lengthSquared() > 1.0f)
		{
			result.normalize();
		}
	}
	
	return result;
}


void Controller::updatePointerVisibility(real p_elapsedTime)
{
	if (m_pointerAutoVisibility == false)
	{
		return;
	}
	
	if (m_platformMenuOpen                      ||
	    prev.pointer.valid != cur.pointer.valid ||
	    prev.pointer.x     != cur.pointer.x     ||
	    prev.pointer.y     != cur.pointer.y)
	{
		// Pointer moved: reset the counter
		m_pointerStationaryTime = 0.0f;
	}
	else
	{
		// FIXME: Remove hardcoded 1/60 timestep; but don't update it with e
		const real minTime = (1.0f / 60.0f);
		// Pointer remained stationary
		m_pointerStationaryTime += p_elapsedTime < minTime ? minTime : p_elapsedTime;
	}
	
	setPointerVisible(
		cur.pointer.valid == false ||
		m_pointerStationaryTime < cfg()->get(m_pointerAutohideTimeout));
}


void Controller::copyDefaultKeyBindings(const char* p_dest)
{
	static const char* defaultBindingsFilename = "config/default_keybindings.json";
	
	tt::fs::FilePtr defaultBindings = tt::fs::open(defaultBindingsFilename, tt::fs::OpenMode_Read);
	if (defaultBindings == 0)
	{
		TT_PANIC("Failed to open default keybindings.");
		return;
	}
	
	tt::fs::FilePtr userFile = savedata::createSaveFile(p_dest);
	if (userFile == 0)
	{
		TT_PANIC("Failed to create user keybindings file.");
		return;
	}
		
	// Copy default keybindings to save directory for user
	tt::code::BufferPtr defaultContent = defaultBindings->getContent();
	userFile->write(defaultContent->getData(), defaultContent->getSize());
}


void Controller::assignKeyBindings()
{
	using namespace tt::input;
	ActionMap keyboardMapping = KeyBindings::getCombinedControllerBindings(keyboardID);
	
	// Assign keyboard mapping to ControllerOne
	input::Controller& controller(AppGlobal::getController(tt::input::ControllerIndex_One));
	input::KeyMapping& mapping(controller.m_keyMapping);
	
	mapping.clear();
	for (s32 i = 0; i < BindableAction_Count; ++i)
	{
		BindableAction actionEnum = static_cast<BindableAction>(i);
		std::string    actionName = getBindableActionName(actionEnum);
		
		if (keyboardMapping.find(actionName) != keyboardMapping.end())
		{
			mapping[actionEnum] = keyboardMapping[actionName];
		}
	}
}


//--------------------------------------------------------------------------------------------------

const char* getBindableActionName(BindableAction p_enum)
{
	switch (p_enum)
	{
	case BindableAction_Accept       : return "accept";
	case BindableAction_Cancel       : return "cancel";
	case BindableAction_Left         : return "left";
	case BindableAction_Right        : return "right";
	case BindableAction_Up           : return "up";
	case BindableAction_Down         : return "down";
	case BindableAction_OpenMenu     : return "open_menu";
	case BindableAction_Scroll       : return "scroll";
	case BindableAction_Jump         : return "jump";
	case BindableAction_Hack         : return "hack";
	case BindableAction_ToggleWeapons: return "toggle_weapons";
	case BindableAction_SelectWeapon1: return "select_weapon_1";
	case BindableAction_SelectWeapon2: return "select_weapon_2";
	case BindableAction_SelectWeapon3: return "select_weapon_3";
	case BindableAction_SelectWeapon4: return "select_weapon_4";
	
	default:
		TT_PANIC("Invalid BindableAction value: %d", p_enum);
		return "";
	}
}


BindableAction getBindableActionFromName(const std::string& p_name)
{
	for (s32 i = 0; i < BindableAction_Count; ++i)
	{
		BindableAction asEnum = static_cast<BindableAction>(i);
		if (p_name == getBindableActionName(asEnum))
		{
			return asEnum;
		}
	}
	return BindableAction_Invalid;
}


// Namespace end
}
}
