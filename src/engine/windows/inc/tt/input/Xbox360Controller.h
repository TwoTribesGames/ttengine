#if !defined(INC_TT_INPUT_XBOX360CONTROLLER_H)
#define INC_TT_INPUT_XBOX360CONTROLLER_H


#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <XInput.h>

#include <tt/input/ControllerIndex.h>
#include <tt/input/Button.h>
#include <tt/input/Stick.h>
#include <tt/input/Trigger.h>


namespace tt {
namespace input {

struct Xbox360Controller
{
public:
	static bool isConnected(ControllerIndex p_index);
	static const Xbox360Controller& getState(ControllerIndex p_index);
	static void update();
	
	static void setRumble(ControllerIndex p_index, real p_rumble);
	
	
	Stick lstick;
	Stick rstick;
	
	Trigger rtrig;
	Trigger ltrig;
	
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
	Button lthumb;
	Button rthumb;
	Button back;
	Button start;
	
private:
	enum { SupportedControllerCount = XUSER_MAX_COUNT };
	
	static Xbox360Controller ms_controllers[SupportedControllerCount];
	static u32               ms_packets    [SupportedControllerCount];
	static bool              ms_connected  [SupportedControllerCount];
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_XBOX360CONTROLLER_H)
