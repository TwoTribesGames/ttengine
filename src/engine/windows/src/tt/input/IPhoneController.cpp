#include <tt/platform/tt_printf.h>

#include <tt/input/IPhoneController.h>
#include <tt/input/MouseController.h>
#include <tt/input/KeyboardController.h>


namespace tt {
namespace input {


IPhoneController IPhoneController::ms_controller;

bool g_isInitialzed = false;

//--------------------------------------------------------------------------------------------------
// Public member functions

bool IPhoneController::isConnected(ControllerIndex p_index)
{
	return p_index == ControllerIndex_One ? 
		MouseController::isConnected(ControllerIndex_One) : false;
}


const IPhoneController& IPhoneController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "IPhoneController has not been initialized yet.");
	TT_ASSERTMSG(p_index == ControllerIndex_One,
	             "Invalid controller index specified: %d", p_index);
	return ms_controller;
}


void IPhoneController::update()
{
	TT_ASSERTMSG(isInitialized(), "IPhoneController has not been initialized yet.");
	
	const auto mouseState = MouseController::getState(ControllerIndex_One);
	const auto kbdState = KeyboardController::getState(ControllerIndex_One);
	
	ms_controller.touch1Status.update(mouseState.left.down);

	if (ms_controller.touch1Status.down)
	{
		ms_controller.touch1Location = mouseState.cursor;
	}
	
	// Second finger logic
	if (kbdState.keys[tt::input::Key_Alt].down)
	{
		if (ms_controller.touch2Status.down == false)
		{
			ms_controller.touch2Location = mouseState.cursor;
		}
		ms_controller.touch2Status.update(true);
	}
	else if (kbdState.keys[tt::input::Key_Shift].down)
	{
		static tt::math::Point2 start;
		if (ms_controller.touch2Status.down == false)
		{
			start = mouseState.cursor;
		}
		tt::math::Point2 diff(mouseState.cursor - start);
		
		ms_controller.touch2Location.x = start.x + diff.x;
		ms_controller.touch2Location.y = start.y + diff.y;
		
		ms_controller.touch2Location.y += 108;
		ms_controller.touch2Status.update(true);
	}
	else if (kbdState.keys[tt::input::Key_Control].down)
	{
		static tt::math::Point2 start;
		if (ms_controller.touch2Status.down == false)
		{
			start = mouseState.cursor;
		}
		
		// Move away from start in opposite direction
		tt::math::Point2 diff(mouseState.cursor - start);

		ms_controller.touch2Location.x = mouseState.cursor.x - diff.x * 2;
		ms_controller.touch2Location.y = mouseState.cursor.y - diff.y * 2;

		ms_controller.touch2Status.update(true);
	}
	else
	{
		ms_controller.touch2Status.update(false);
	}
}


bool IPhoneController::initialize()
{
	g_isInitialzed = true;
	return true;
}


void IPhoneController::deinitialize()
{
}

//--------------------------------------------------------------------------------------------------
// Private member functions

IPhoneController::IPhoneController()
:
touch1Status(),
touch2Status(),
touch1Location(),
touch2Location()
{
}


bool IPhoneController::isInitialized()
{
	return g_isInitialzed;
}


// Namespace end
}
}
