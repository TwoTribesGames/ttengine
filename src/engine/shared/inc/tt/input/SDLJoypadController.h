#if !defined(INC_TT_INPUT_SDLJOYPADCONTROLLER_H)
#define INC_TT_INPUT_SDLJOYPADCONTROLLER_H


#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <tt/input/ControllerIndex.h>
#include <tt/input/Button.h>
#include <tt/input/Stick.h>
#include <tt/input/Trigger.h>

#include <map>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_haptic.h>

namespace tt {
namespace input {

struct RumbleVibration
{
	int left;
	int right;
};


struct SDLJoypadController
{
public:
	static bool isValidControllerIndex(ControllerIndex p_index);
	static bool isConnected(ControllerIndex p_index);
	static const SDLJoypadController& getState(ControllerIndex p_index);
	static void update();
	static void setRumble(ControllerIndex p_index, real p_rumble, real p_durationInSeconds = .5);
	static void stopRumble(ControllerIndex p_index);
	
	static bool isInitialized() { return ms_initialized; }
	
	static bool initialize();
	
	static void deinitialize();
	
	static int processEvent(const SDL_Event &event);
	
	Stick lstick;
	Stick rstick;
	
	Trigger ltrig;
	Trigger rtrig;
	
	Button left;
	Button right;
	Button up;
	Button down;
	Button a;
	Button b;
	Button x;
	Button y;
	Button l;
	Button r;
	Button back;
	Button guide;
	Button start;
	Button lthumb;
	Button rthumb;
	
	SDLJoypadController() : m_gamepad(0), m_haptic(0), m_isConnected(false), m_instanceID(-1) {}
	
private:
	enum { SupportedControllerCount = 4 };
	SDL_GameController* m_gamepad;
	SDL_Haptic*         m_haptic;
	bool                m_isConnected;
	SDL_JoystickID      m_instanceID;
	
	void open(int device);
	void close();
	static int getControllerIndex(SDL_JoystickID instance);
	
	static SDLJoypadController ms_controllers[SupportedControllerCount];
	static SDLJoypadController ms_temporary  [SupportedControllerCount];
	
	static bool             ms_initialized;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_JOYPADCONTROLLER_H)
