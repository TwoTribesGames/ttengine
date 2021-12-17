#include <algorithm>
#include <sstream>

#include <SDL2/SDL.h>

#include <tt/code/ErrorStatus.h>
#include <tt/doc/ini/IniDocument.h>
#include <tt/input/ControllerType.h>
#include <tt/input/SDLJoypadController.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>


namespace tt {
namespace input {

SDLJoypadController SDLJoypadController::ms_controllers[SupportedControllerCount];
SDLJoypadController SDLJoypadController::ms_temporary  [SupportedControllerCount];
bool                SDLJoypadController::ms_initialized  = false;

#define JOY_POS_CENTER 0
#define JOY_POS_LEFT  -1
#define JOY_POS_RIGHT  1
#define JOY_POS_UP    -1
#define JOY_POS_DOWN   1
#define JOY_DEADZONE   100

// FIXME: Using Xbox 360 controller type here instead of "generic controller",
// so that Toki Tori 2 still thinks we're dealing with the Xbox 360 controller (showing its graphics)
static const ControllerType thisControllerType = ControllerType_Xbox360Controller;


bool SDLJoypadController::isValidControllerIndex(ControllerIndex p_index)
{
	TT_ASSERTMSG(p_index < static_cast<ControllerIndex>(SupportedControllerCount), "Invalid controller index: %d", p_index);
	return p_index < static_cast<ControllerIndex>(SupportedControllerCount);
}


bool SDLJoypadController::isConnected(ControllerIndex p_index)
{
	if (isInitialized() && isValidControllerIndex(p_index))
	{
		return ms_controllers[p_index].m_isConnected;
	}
	return false;
}


const SDLJoypadController& SDLJoypadController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "JoystickController has not been initialized yet.");
	
	if (isInitialized() && isValidControllerIndex(p_index))
	{
		return ms_controllers[p_index];
	}
	return ms_controllers[0];
}

void SDLJoypadController::stopRumble(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "JoystickController has not been initialized yet.");
	
	if (getCurrentControllerType() != ControllerType_GenericController &&
	    getCurrentControllerType() != ControllerType_Xbox360Controller )
	{
		return;
	}
	
	if (ms_controllers[p_index].m_isConnected && ms_controllers[p_index].m_haptic != 0)
	{
		SDL_HapticRumbleStop(ms_controllers[p_index].m_haptic);
	}
}

void SDLJoypadController::setRumble(ControllerIndex p_index, real p_rumble, real p_durationInSeconds)
{
	TT_ASSERTMSG(isInitialized(), "JoystickController has not been initialized yet.");
	
	if (getCurrentControllerType() != ControllerType_GenericController &&
	    getCurrentControllerType() != ControllerType_Xbox360Controller )
	{
		return;
	}
	
	if (ms_controllers[p_index].m_isConnected && ms_controllers[p_index].m_haptic != 0)
	{
		SDL_HapticRumblePlay(ms_controllers[p_index].m_haptic, p_rumble, static_cast<Uint32>(p_durationInSeconds * 1000.0f));
	}
}


void SDLJoypadController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLJoypadController has not been initialized yet.");
		return;
	}
	
	for (s32 i = 0; i < SupportedControllerCount; ++i)
	{
		// Copy the temporary controller state to the working state
		ms_controllers[i].rstick = ms_temporary[i].rstick;
		ms_controllers[i].lstick = ms_temporary[i].lstick;
		ms_controllers[i].ltrig  = ms_temporary[i].ltrig;
		ms_controllers[i].rtrig  = ms_temporary[i].rtrig;
		
		ms_controllers[i].a     .update(ms_temporary[i].a     .down);
		ms_controllers[i].b     .update(ms_temporary[i].b     .down);
		ms_controllers[i].x     .update(ms_temporary[i].x     .down);
		ms_controllers[i].y     .update(ms_temporary[i].y     .down);
		ms_controllers[i].l     .update(ms_temporary[i].l     .down);
		ms_controllers[i].r     .update(ms_temporary[i].r     .down);
		ms_controllers[i].up    .update(ms_temporary[i].up    .down);
		ms_controllers[i].down  .update(ms_temporary[i].down  .down);
		ms_controllers[i].left  .update(ms_temporary[i].left  .down);
		ms_controllers[i].right .update(ms_temporary[i].right .down);
		ms_controllers[i].rthumb.update(ms_temporary[i].rthumb.down);
		ms_controllers[i].lthumb.update(ms_temporary[i].lthumb.down);
		ms_controllers[i].back  .update(ms_temporary[i].back  .down);
		ms_controllers[i].start .update(ms_temporary[i].start .down);
		ms_controllers[i].guide .update(ms_temporary[i].guide .down);
		
		// If a controller button was pressed or a directional stick used,
		// set this controller as the current controller type
		if (ms_controllers[i].a     .down                   ||
		    ms_controllers[i].b     .down                   ||
		    ms_controllers[i].x     .down                   ||
		    ms_controllers[i].y     .down                   ||
		    ms_controllers[i].l     .down                   ||
		    ms_controllers[i].r     .down                   ||
		    ms_controllers[i].up    .down                   ||
		    ms_controllers[i].down  .down                   ||
		    ms_controllers[i].left  .down                   ||
		    ms_controllers[i].right .down                   ||
		    ms_controllers[i].rthumb.down                   ||
		    ms_controllers[i].lthumb.down                   ||
		    ms_controllers[i].back  .down                   ||
		    ms_controllers[i].start .down                   ||
		    ms_controllers[i].lstick.lengthSquared() > 0.1f ||
		    ms_controllers[i].rstick.lengthSquared() > 0.1f)
		{
			onControllerTypeUsed(thisControllerType);
		}
	}
	
}


bool SDLJoypadController::initialize()
{
	if (isInitialized())
	{
		TT_PANIC("JoystickController is already initialized.");
		return false;
	}
	
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0)
	{
		TT_PANIC("Failed to initialize controller subsystems: %s", SDL_GetError());
		return false;
	}
	
	int connectedControllers = SDL_NumJoysticks();
	TT_Printf("SDL reports %d connected controllers...\n", connectedControllers);
	
	if (connectedControllers > SupportedControllerCount)
	{
		connectedControllers = 4;
	}
	
	for (int i = 0; i < connectedControllers; ++i)
	{
		if (SDL_IsGameController(i))
		{
			ms_controllers[i].open(i);
		}
	}
	
	ms_initialized = true;
	return ms_initialized;
}


void SDLJoypadController::deinitialize()
{
	for (int i = 0; i < SupportedControllerCount; ++i)
	{
		ms_controllers[i].close();
	}
	
	ms_initialized = false;
}


void SDLJoypadController::open(int p_device)
{
	m_gamepad = SDL_GameControllerOpen(p_device);
	
	SDL_Joystick* joystick = SDL_GameControllerGetJoystick(m_gamepad);
	
	m_instanceID = SDL_JoystickInstanceID(joystick);
	
	m_isConnected = true;
	
	// Force feedback / rumble support
	if (SDL_JoystickIsHaptic(joystick))
	{
		m_haptic = SDL_HapticOpen(p_device); // We want to use this: SDL_HapticOpenFromJoystick(joystick);
		                                     // but right now (v2.0.0) there is a bug which causes SDL to 
		                                     // crash when called for the second time. (Just connect two xbox controllers)
		                                     // I think it's related to this: http://sdl.5483.n7.nabble.com/what-to-compare-to-SDL-Joy-Event-which-td36091.html
		
		TT_Printf("Haptic Effects: %d\n", SDL_HapticNumEffects(m_haptic));
		TT_Printf("Haptic Query: %x\n",   SDL_HapticQuery     (m_haptic));
		
		if (SDL_HapticRumbleSupported(m_haptic))
		{
			if (SDL_HapticRumbleInit(m_haptic) != 0)
			{
				TT_Printf("Haptic Rumble Init: %s\n", SDL_GetError());
				SDL_HapticClose(m_haptic);
				m_haptic = 0;
			}
		}
		else
		{
			SDL_HapticClose(m_haptic);
			m_haptic = 0;
		}
	}
}


void SDLJoypadController::close()
{
	if (m_isConnected)
	{
		m_isConnected = false;
		if (m_haptic != 0)
		{
			SDL_HapticClose(m_haptic);
			m_haptic = 0;
		}
		
		SDL_GameControllerClose(m_gamepad);
		m_gamepad = 0;
	}
}


int SDLJoypadController::getControllerIndex(SDL_JoystickID p_instance)
{
	for (int i = 0; i < SupportedControllerCount; ++i)
	{
		if (ms_controllers[i].m_isConnected && ms_controllers[i].m_instanceID == p_instance)
		{
			return i;
		}
	}
	return -1;
}


int SDLJoypadController::processEvent(const SDL_Event& p_event)
{
	switch (p_event.type)
	{
		case SDL_CONTROLLERAXISMOTION:
		{
			int cIndex = getControllerIndex(p_event.caxis.which);
			if (cIndex < 0) return 0; // unknown controller?
			SDLJoypadController& jc = ms_temporary[cIndex];
			real value;
			
			value = p_event.caxis.value / 32767.0f;
			tt::math::clamp<real>(value, -1, 1);
			
			switch (p_event.caxis.axis)
			{
				case SDL_CONTROLLER_AXIS_LEFTX       : jc.lstick.x    =  value; break;
				case SDL_CONTROLLER_AXIS_LEFTY       : jc.lstick.y    = -value; break;
				case SDL_CONTROLLER_AXIS_RIGHTX      : jc.rstick.x    =  value; break;
				case SDL_CONTROLLER_AXIS_RIGHTY      : jc.rstick.y    = -value; break;
				case SDL_CONTROLLER_AXIS_TRIGGERLEFT : jc.ltrig.value =  value; break;
				case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: jc.rtrig.value =  value; break;
			}
			break;
		}
		
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			int cIndex = getControllerIndex(p_event.caxis.which);
			if (cIndex < 0) return 0; // unknown controller?
			
			SDLJoypadController& jc = ms_temporary[cIndex];
			bool pressed = p_event.cbutton.state == SDL_PRESSED;
			
			switch (p_event.cbutton.button)
			{
				case SDL_CONTROLLER_BUTTON_A            : jc.a     .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_B            : jc.b     .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_X            : jc.x     .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_Y            : jc.y     .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_BACK         : jc.back  .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_GUIDE        : jc.guide .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_START        : jc.start .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_LEFTSTICK    : jc.lthumb.update(pressed); break;
				case SDL_CONTROLLER_BUTTON_RIGHTSTICK   : jc.rthumb.update(pressed); break;
				case SDL_CONTROLLER_BUTTON_LEFTSHOULDER : jc.l     .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: jc.r     .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP      : jc.up    .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN    : jc.down  .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT    : jc.left  .update(pressed); break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT   : jc.right .update(pressed); break;
			}
			break;
		}
		
		case SDL_CONTROLLERDEVICEADDED:
		{
			if (p_event.cdevice.which < SupportedControllerCount)
			{
				SDLJoypadController& jc = ms_controllers[p_event.cdevice.which];
				jc.open(p_event.cdevice.which);
			}
			break;
		}
		
		case SDL_CONTROLLERDEVICEREMOVED:
		{
			int cIndex = getControllerIndex(p_event.cdevice.which);
			if (cIndex < 0) return 0; // unknown controller?
			
			SDLJoypadController& jc = ms_controllers[cIndex];
			jc.close();
			break;
		}
	}
	return 0;
}

// Namespace end
}
}

