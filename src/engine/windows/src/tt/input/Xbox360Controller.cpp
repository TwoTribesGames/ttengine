#include <algorithm>

#include <tt/input/ControllerType.h>
#include <tt/input/Xbox360Controller.h>

namespace tt {
namespace input {

Xbox360Controller Xbox360Controller::ms_controllers[SupportedControllerCount];
u32               Xbox360Controller::ms_packets    [SupportedControllerCount] = { 0 };
bool              Xbox360Controller::ms_connected  [SupportedControllerCount] = { false, false, false, false };


static const u64 rumbleTimeoutInSeconds = 30;


bool Xbox360Controller::isConnected(ControllerIndex p_index)
{
	return ms_connected[p_index];
}


const Xbox360Controller& Xbox360Controller::getState(ControllerIndex p_index)
{
	return ms_controllers[p_index];
}


void Xbox360Controller::update()
{
	// FIXME: XInputGetState() takes a lot of time for unconnected controllers
	//        We should keep track of controller (dis)connection events

	for (DWORD i = 0; i < SupportedControllerCount; ++i)
	{
		XINPUT_STATE state = { 0 };
		if (XInputGetState(i, &state) == ERROR_SUCCESS)
		{
			ms_packets[i] = state.dwPacketNumber;
			Xbox360Controller& xc = ms_controllers[i];
			
			xc.lstick.x = state.Gamepad.sThumbLX / real(32767);
			xc.lstick.x = std::max(real(-1), xc.lstick.x);
			xc.lstick.y = state.Gamepad.sThumbLY / real(32767);
			xc.lstick.y = std::max(real(-1), xc.lstick.y);
			
			xc.rstick.x = state.Gamepad.sThumbRX / real(32767);
			xc.rstick.x = std::max(real(-1), xc.rstick.x);
			xc.rstick.y = state.Gamepad.sThumbRY / real(32767);
			xc.rstick.y = std::max(real(-1), xc.rstick.y);
			
			xc.ltrig.value = state.Gamepad.bLeftTrigger / real(255);
			xc.rtrig.value = state.Gamepad.bRightTrigger / real(255);
			
			WORD buttons = state.Gamepad.wButtons;
			
			xc.left.update((buttons & XINPUT_GAMEPAD_DPAD_LEFT) == XINPUT_GAMEPAD_DPAD_LEFT);
			xc.right.update((buttons & XINPUT_GAMEPAD_DPAD_RIGHT) == XINPUT_GAMEPAD_DPAD_RIGHT);
			xc.up.update((buttons & XINPUT_GAMEPAD_DPAD_UP) == XINPUT_GAMEPAD_DPAD_UP);
			xc.down.update((buttons & XINPUT_GAMEPAD_DPAD_DOWN) == XINPUT_GAMEPAD_DPAD_DOWN);
			xc.a.update((buttons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A);
			xc.b.update((buttons & XINPUT_GAMEPAD_B) == XINPUT_GAMEPAD_B);
			xc.x.update((buttons & XINPUT_GAMEPAD_X) == XINPUT_GAMEPAD_X);
			xc.y.update((buttons & XINPUT_GAMEPAD_Y) == XINPUT_GAMEPAD_Y);
			xc.l.update((buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) == XINPUT_GAMEPAD_LEFT_SHOULDER);
			xc.r.update((buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) == XINPUT_GAMEPAD_RIGHT_SHOULDER);
			xc.lthumb.update((buttons & XINPUT_GAMEPAD_LEFT_THUMB) == XINPUT_GAMEPAD_LEFT_THUMB);
			xc.rthumb.update((buttons & XINPUT_GAMEPAD_RIGHT_THUMB) == XINPUT_GAMEPAD_RIGHT_THUMB);
			xc.start.update((buttons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START);
			xc.back.update((buttons & XINPUT_GAMEPAD_BACK) == XINPUT_GAMEPAD_BACK);
			
			if(buttons != 0 || xc.lstick.lengthSquared() > 0.1f || xc.rstick.lengthSquared() > 0.1f)
			{
				onControllerTypeUsed(ControllerType_Xbox360Controller);
			}
			
			ms_connected[i] = true;
		}
		else
		{
			ms_connected[i] = false;
		}
	}
}


void Xbox360Controller::setRumble(ControllerIndex p_index, real p_rumble)
{
	// If controller hasn't been used for a while, don't rumble
	if (getCurrentControllerType() != ControllerType_Xbox360Controller)
	{
		return;
	}
	
	if (p_rumble < 0.0f)
	{
		p_rumble = 0.0f;
	}
	
	if (p_rumble > 1.0f)
	{
		p_rumble = 1.0f;
	}
	
	XINPUT_VIBRATION rumble;
	rumble.wLeftMotorSpeed  = static_cast<WORD>(65535.0 * p_rumble);
	rumble.wRightMotorSpeed = static_cast<WORD>(65535.0 * p_rumble);
	XInputSetState(p_index, &rumble);
}

// Namespace end
}
}
