#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windowsx.h>

#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/input/ControllerType.h>
#include <tt/input/MouseController.h>
#include <tt/math/math.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace input {

HCURSOR         MouseController::ms_defaultCursor         = 0;
HCURSOR         MouseController::ms_cursor                = 0;
bool            MouseController::ms_cursorVisible         = true;
bool            MouseController::ms_multipleMiceSupport   = false;
s32             MouseController::ms_availableMice         = 0;
HANDLE          MouseController::ms_rdpMouseDevice        = 0;
HANDLE          MouseController::ms_mouseDevice[MouseController::MaxSupportedMice];
MouseController MouseController::ms_controller[MouseController::MaxSupportedMice];
MouseController MouseController::ms_temporary[MouseController::MaxSupportedMice];
RECT            MouseController::ms_clientRect            = { 0 };
RECT            MouseController::ms_windowRect            = { 0 };
POINT           MouseController::ms_screenDimensions      = { 0 };
#if !defined(TT_BUILD_FINAL)
bool            MouseController::ms_restrictMouseToWindow = false;
#endif //#if !defined(TT_BUILD_FINAL)
HWND            MouseController::ms_window                = 0;
WNDPROC         MouseController::ms_originalWndProc       = 0;
std::string     MouseController::ms_originalWindowTitle;

static const char* const g_rdpMouseID = "\\??\\Root#RDP_MOU#";


//--------------------------------------------------------------------------------------------------
// Public member functions

bool MouseController::isConnected(ControllerIndex p_index)
{
	TT_ASSERTMSG(p_index >= ControllerIndex_One && p_index <= ControllerIndex_Four,
	             "Invalid controller index specified: %d", p_index);
	return isInitialized() && p_index < ms_availableMice;
}


const MouseController& MouseController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "MouseController has not been initialized yet.");
	TT_ASSERTMSG(p_index >= ControllerIndex_One && p_index <= ControllerIndex_Four,
	             "Invalid controller index specified: %d", p_index);
	return ms_controller[p_index];
}


void MouseController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("MouseController has not been initialized yet.");
		return;
	}
	
	for (s32 i = 0; i < ms_availableMice; ++i)
	{
		// Detect usage.
		if (distanceSquared(ms_controller[i].cursor, ms_temporary[i].cursor) > 16 ||
		    ms_temporary[i].left.down                         ||
		    ms_temporary[i].middle.down                       ||
		    ms_temporary[i].right.down                        ||
		    ms_temporary[i].wheelNotches != 0)
		{
			onControllerTypeUsed(input::ControllerType_Keyboard);
		}
		
		// Copy the temporary controller state to the working state
		ms_controller[i].cursor = ms_temporary[i].cursor;
		ms_controller[i].left.update(ms_temporary[i].left.down);
		ms_controller[i].middle.update(ms_temporary[i].middle.down);
		ms_controller[i].right.update(ms_temporary[i].right.down);
		ms_controller[i].wheelNotches = ms_temporary[i].wheelNotches;
		
		// Clear wheel state as there is no message to indicate wheel movement stopped
		ms_temporary[i].wheelNotches = 0;
	}
}


bool MouseController::initialize(HWND p_window, bool p_multipleMiceSupport)
{
	if (isInitialized())
	{
		TT_PANIC("MouseController is already initialized.");
		return false;
	}
	
	if (p_window == 0)
	{
		TT_PANIC("Invalid window handle specified.");
		return false;
	}
	
	// Set the default cursor
	setDefaultCursor(::LoadCursor(0, IDC_ARROW));
	
	ms_multipleMiceSupport = p_multipleMiceSupport;
	
	// Get the original window title
	{
		char wndTitle[256] = { 0 };
		GetWindowTextA(p_window, wndTitle, 256);
		ms_originalWindowTitle = wndTitle;
	}
	
	// Clear the mouse device handles
	tt::mem::zero8(ms_mouseDevice, sizeof(HANDLE) * MaxSupportedMice);
	
	// Get the window rectangles and screen dimensions
	updateWindowRects(p_window);
	ms_screenDimensions.x = GetSystemMetrics(SM_CXSCREEN);
	ms_screenDimensions.y = GetSystemMetrics(SM_CYSCREEN);
	
	if (ms_multipleMiceSupport)
	{
		// Get the number of connected mice on this system
		ms_availableMice = 0;
		
		UINT rawDeviceCount = 0;
		GetRawInputDeviceList(0, &rawDeviceCount, sizeof(RAWINPUTDEVICELIST));
		RAWINPUTDEVICELIST* rawDeviceList = new RAWINPUTDEVICELIST[rawDeviceCount];
		
		UINT rawDeviceListSize = sizeof(RAWINPUTDEVICELIST) * rawDeviceCount;
		if (GetRawInputDeviceList(rawDeviceList, &rawDeviceListSize,
		                          sizeof(RAWINPUTDEVICELIST)) != rawDeviceCount)
		{
			char reason[256] = { 0 };
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, reason, 256, 0);
			TT_PANIC("Retrieving list of raw input devices failed: %s", reason);
			delete[] rawDeviceList;
			return false;
		}
		
		for (UINT i = 0; i < rawDeviceCount; ++i)
		{
			if (rawDeviceList[i].dwType == RIM_TYPEMOUSE)
			{
				if (ms_availableMice >= MaxSupportedMice)
				{
					TT_WARN("More mice are attached than are supported by MouseController.");
					break;
				}
				
				// Get the length of the device name
				UINT deviceNameLen = 0;
				GetRawInputDeviceInfoA(rawDeviceList[i].hDevice, RIDI_DEVICENAME, 0, &deviceNameLen);
				
				// Retrieve the device name
				char* deviceNameRaw = new char[deviceNameLen + 1];
				tt::mem::zero8(deviceNameRaw, static_cast<tt::mem::size_type>(deviceNameLen + 1));
				GetRawInputDeviceInfoA(rawDeviceList[i].hDevice, RIDI_DEVICENAME,
				                       deviceNameRaw, &deviceNameLen);
				std::string deviceName(deviceNameRaw);
				delete[] deviceNameRaw;
				
				if (deviceName.find(g_rdpMouseID) == 0)
				{
					//TT_Printf("MouseController::initialize: Raw device %u: '%s' is an RDP mouse. Ignoring.\n",
					//          i, deviceName.c_str());
					ms_rdpMouseDevice = rawDeviceList[i].hDevice;
					continue;
				}
				
				TT_Printf("MouseController::initialize: Raw device %u: '%s' is now mouseDevice number: %d\n",
				          i, deviceName.c_str(), ms_availableMice);
				ms_mouseDevice[ms_availableMice] = rawDeviceList[i].hDevice;
				++ms_availableMice;
			}
		}
		
		//TT_Printf("MouseController::initialize: AVAILABLE MICE ON SYSTEM: %d\n", ms_availableMice);
		
		delete[] rawDeviceList;
		rawDeviceList = 0;
	}
	else
	{
		ms_availableMice = 1;
	}
	
	if (ms_availableMice > 1)
	{
		// More than one mouse attached: register for raw mouse input
		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage     = 0x02;
		rid.dwFlags     = 0;
		rid.hwndTarget  = p_window;
		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			char reason[256] = { 0 };
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, reason, 256, 0);
			TT_PANIC("Registering for raw input failed: %s", reason);
			return false;
		}
	}
	
	// Get the current window procedure
	ms_originalWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(p_window, GWLP_WNDPROC));
	if (ms_originalWndProc == 0)
	{
		TT_PANIC("Retrieving current window procedure failed.");
		return false;
	}
	
	// Set our own window procedure as current
	if (SetWindowLongPtr(p_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndProc)) == 0)
	{
		TT_PANIC("Setting new window procedure failed.");
		ms_originalWndProc = 0;
		return false;
	}
	
	ms_window = p_window;
	
	// Reset input data
	MouseController empty;
	for (s32 i = 0; i < MaxSupportedMice; ++i)
	{
		ms_controller[i] = empty;
		ms_temporary[i]  = empty;
	}
	
	if (ms_availableMice == 1)
	{
		// Start a timer to detect whether the mouse left the application window
		SetTimer(ms_window, CursorCheck_TimerID, CursorCheck_Interval, 0);
	}
	
	return true;
}


void MouseController::deinitialize()
{
	// Stop the mouse check timer
	KillTimer(ms_window, CursorCheck_TimerID);
	
	// Restore the original window procedure
	SetWindowLongPtr(ms_window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ms_originalWndProc));
	ms_window          = 0;
	ms_originalWndProc = 0;
}


void MouseController::setCursorVisible(bool p_visible)
{
	if (p_visible != ms_cursorVisible)
	{
		ms_cursorVisible = p_visible;
		SetCursor(ms_cursorVisible ? ms_cursor : 0);
	}
}


#if !defined(TT_BUILD_FINAL)
void MouseController::toggleMouseCaptured()
{
	setMouseCaptured(isMouseCaptured() == false);
}


void MouseController::setMouseCaptured(bool p_capture)
{
	if (ms_restrictMouseToWindow == p_capture)
	{
		return;
	}
	
	ms_restrictMouseToWindow = p_capture;
	
	if (ms_restrictMouseToWindow)
	{
		// Update the window title
		std::string wndTitle(ms_originalWindowTitle + " [MOUSE CAPTURED]");
		SetWindowTextA(ms_window, wndTitle.c_str());
		
		// Make sure the cursor cannot leave the window rect
		ClipCursor(&ms_windowRect);
		
		// Disable the cursor
		SetCursor(0);
	}
	else
	{
		// Update the window title
		SetWindowTextA(ms_window, ms_originalWindowTitle.c_str());
		
		// Cursor is free to go where it pleases
		ClipCursor(0);
		
		// Restore the cursor
		resetToDefaultCursor();
	}
}
#endif  // !defined(TT_BUILD_FINAL)


//--------------------------------------------------------------------------------------------------
// Private member functions

MouseController::MouseController()
:
cursor(),
left(),
middle(),
right(),
wheelNotches(0)
{
}


bool MouseController::isInitialized()
{
	return ms_window != 0;
}


LRESULT CALLBACK MouseController::wndProc(HWND   p_wnd,    UINT   p_msg,
                                          WPARAM p_wparam, LPARAM p_lparam)
{
	// NOTE: Do not modify ms_controller here!
	
	switch (p_msg)
	{
	case WM_MOVE:
	case WM_SIZE:
		updateWindowRects(p_wnd);
		break;
		
	case WM_ACTIVATEAPP:
		if (p_wparam == FALSE)
		{
#if !defined(TT_BUILD_FINAL)
			// Application lost focus; stop capturing the mouse
			setMouseCaptured(false);
#endif //#if !defined(TT_BUILD_FINAL)
		}
		break;
		
	case WM_MOUSEMOVE:
		if (ms_multipleMiceSupport == false || ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			
			ms_temporary[0].left.update(  (p_wparam & MK_LBUTTON) == MK_LBUTTON);
			ms_temporary[0].middle.update((p_wparam & MK_MBUTTON) == MK_MBUTTON);
			ms_temporary[0].right.update( (p_wparam & MK_RBUTTON) == MK_RBUTTON);
		}
		break;
		
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
		if (ms_multipleMiceSupport == false || ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			ms_temporary[0].left.update(true);
		}
		break;
		
	case WM_LBUTTONUP:
		if (ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			ms_temporary[0].left.update(false);
		}
		break;
		
	case WM_MBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
		if (ms_multipleMiceSupport == false || ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			ms_temporary[0].middle.update(true);
		}
		break;
		
	case WM_MBUTTONUP:
		if (ms_multipleMiceSupport == false || ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			ms_temporary[0].middle.update(false);
		}
		break;
		
	case WM_RBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
		if (ms_multipleMiceSupport == false || ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			ms_temporary[0].right.update(true);
		}
		break;
		
	case WM_RBUTTONUP:
		if (ms_multipleMiceSupport == false || ms_availableMice == 1)
		{
			updateCursorFromMessage(0, p_lparam);
			ms_temporary[0].right.update(false);
		}
		break;
		
#if !defined(TT_BUILD_FINAL)
	case WM_KEYDOWN:
		// Bit 30: Specifies the previous key state. The value is 1 if the key is down before the
		//         message is sent, or it is zero if the key is up.
		if (//ms_availableMice > 1 &&
		    p_wparam == VK_SCROLL && (p_lparam & (1 << 30)) == 0)
		{
			if (ms_multipleMiceSupport)
			{
				// Toggle mouse capture
				toggleMouseCaptured();
			}
		}
		break;
#endif //#if !defined(TT_BUILD_FINAL)
		
	case WM_MOUSEWHEEL:
		{
			// Get value of delta parameter
			short delta = GET_WHEEL_DELTA_WPARAM(p_wparam);
			
			if (delta != 0)
			{
				// Store amount of notches (positive = up, negative = down)
				ms_temporary[0].wheelNotches = (s32)(delta / WHEEL_DELTA);
			}
			break;
		}
		
	case WM_INPUT:
		// Handle raw input
		if (ms_multipleMiceSupport && ms_availableMice > 1)
		{
			RAWINPUT rawInput     = { 0 };
			UINT     rawInputSize = sizeof(RAWINPUT);
			if (GetRawInputData((HRAWINPUT)p_lparam, RID_INPUT, &rawInput,
			                    &rawInputSize, sizeof(RAWINPUTHEADER)) != rawInputSize)
			{
				TT_PANIC("Retrieving raw input data failed.");
				return 0;
			}
			
			if (rawInput.header.hDevice == ms_rdpMouseDevice)
			{
				break;
			}
			
			s32 idx = getMouseIndex(rawInput.header.hDevice);
			if (idx == -1)
			{
				break;
			}
			
			const RAWMOUSE& rawMouse(rawInput.data.mouse);
			if ((rawMouse.usFlags & MOUSE_MOVE_RELATIVE) == MOUSE_MOVE_RELATIVE)
			{
				// Received relative mouse movement; offset the coordinates
				ms_temporary[idx].cursor.valid = true;
				ms_temporary[idx].cursor.x    += rawMouse.lLastX;
				ms_temporary[idx].cursor.y    += rawMouse.lLastY;
			}
			else if ((rawMouse.usFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE)
			{
				// Received absolute mouse movement; calculate where the mouse should be
				ms_temporary[idx].cursor.valid = true;
#if !defined(TT_BUILD_FINAL)
				if (ms_restrictMouseToWindow)
				{
					// Interpret the raw coordinates as if they span the client area of the window
					LONG pixelX = (LONG)((ms_clientRect.right  * (LONG)rawMouse.lLastX) >> 16);
					LONG pixelY = (LONG)((ms_clientRect.bottom * (LONG)rawMouse.lLastY) >> 16);
					ms_temporary[idx].cursor.x = pixelX;
					ms_temporary[idx].cursor.y = pixelY;
					
					TT_Printf("MouseController::wndProc: Captured absolute coordinates: (%d, %d)\n",
					          ms_temporary[idx].cursor.x, ms_temporary[idx].cursor.y);
				}
				else
#endif //#if !defined(TT_BUILD_FINAL)
				{
					// Translate the raw coordinates to screen pixel coordinates,
					// then map them to the window rect
					LONG pixelX = (LONG)((ms_screenDimensions.x * (LONG)rawMouse.lLastX) >> 16);
					LONG pixelY = (LONG)((ms_screenDimensions.y * (LONG)rawMouse.lLastY) >> 16);
					ms_temporary[idx].cursor.x = pixelX - ms_windowRect.left;
					ms_temporary[idx].cursor.y = pixelY - ms_windowRect.top;
					
					TT_Printf("MouseController::wndProc: Normal absolute coordinates: (%d, %d)\n",
					          ms_temporary[idx].cursor.x, ms_temporary[idx].cursor.y);
				}
			}
			
			// Update the button status
			USHORT btns = rawMouse.usButtonFlags;
			if ((btns & RI_MOUSE_LEFT_BUTTON_DOWN) == RI_MOUSE_LEFT_BUTTON_DOWN)
			{
				ms_temporary[idx].left.update(true);
			}
			else if ((btns & RI_MOUSE_LEFT_BUTTON_UP) == RI_MOUSE_LEFT_BUTTON_UP)
			{
				ms_temporary[idx].left.update(false);
			}
			
			if ((btns & RI_MOUSE_MIDDLE_BUTTON_DOWN) == RI_MOUSE_MIDDLE_BUTTON_DOWN)
			{
				ms_temporary[idx].middle.update(true);
			}
			else if ((btns & RI_MOUSE_MIDDLE_BUTTON_UP) == RI_MOUSE_MIDDLE_BUTTON_UP)
			{
				ms_temporary[idx].middle.update(false);
			}
			
			if ((btns & RI_MOUSE_RIGHT_BUTTON_DOWN) == RI_MOUSE_RIGHT_BUTTON_DOWN)
			{
				ms_temporary[idx].right.update(true);
			}
			else if ((btns & RI_MOUSE_RIGHT_BUTTON_UP) == RI_MOUSE_RIGHT_BUTTON_UP)
			{
				ms_temporary[idx].right.update(false);
			}
			
			// Clamp the cursor to the client area of the window
			tt::math::clamp(ms_temporary[idx].cursor.x,
			                (s32)ms_clientRect.left, (s32)ms_clientRect.right);
			tt::math::clamp(ms_temporary[idx].cursor.y,
			                (s32)ms_clientRect.top, (s32)ms_clientRect.bottom);
		}
		break;
		
		
	case WM_SETCURSOR:
		// Only respond to setCursor on client area of the window
		if (LOWORD(p_lparam) == HTCLIENT)
		{
#if !defined(TT_BUILD_FINAL)
			if (ms_restrictMouseToWindow)
			{
				SetCursor(0);
			}
			else
#endif
			{
				// Set default cursor
				SetCursor(ms_cursorVisible ? ms_cursor : 0);
			}
			return TRUE;
		}
		break;
		
	case WM_TIMER:
		if (p_wparam == CursorCheck_TimerID && ms_availableMice == 1)
		{
			// Check if cursor is outside of window
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			if (cursorPos.x < ms_windowRect.left || cursorPos.x > ms_windowRect.right ||
			    cursorPos.y < ms_windowRect.top  || cursorPos.y > ms_windowRect.bottom)
			{
				ms_temporary[0].cursor.valid = false;
			}
		}
		break;
	}
	
	return CallWindowProc(ms_originalWndProc, p_wnd, p_msg, p_wparam, p_lparam);
}


void MouseController::updateCursorFromMessage(s32 p_mouseIndex, LPARAM p_lparam)
{
	const int x = static_cast<int>(GET_X_LPARAM(p_lparam));
	const int y = static_cast<int>(GET_Y_LPARAM(p_lparam));
	
	ms_temporary[p_mouseIndex].cursor.x = x;
	ms_temporary[p_mouseIndex].cursor.y = y;
	ms_temporary[p_mouseIndex].cursor.valid =
		x >= ms_clientRect.left && x < ms_clientRect.right &&
		y >= ms_clientRect.top  && y < ms_clientRect.bottom;
	offsetCursorPosition(p_mouseIndex);
}


void MouseController::offsetCursorPosition(s32 p_mouseIndex)
{
	using namespace engine::renderer;
	Renderer* renderer = Renderer::getInstance();
	
	Pointer& cursor(ms_temporary[p_mouseIndex].cursor);
	
	// Handle up-scaling by Renderer (Must down-scale cursor position)
	math::Vector2 scaling = renderer->getUpScaler()->getScaleFactor();
	const math::Vector2 offset  = renderer->getUpScaler()->getOffset();
	if (renderer->isIOS2xMode())
	{
		scaling *= 0.5f;
	}
	cursor.x -= static_cast<s32>(offset.x);
	cursor.y -= static_cast<s32>(offset.y);
	cursor.x  = static_cast<s32>(cursor.x * scaling.x);
	cursor.y  = static_cast<s32>(cursor.y * scaling.y);
}


s32 MouseController::getMouseIndex(HANDLE p_deviceHandle)
{
	for (s32 i = 0; i < ms_availableMice; ++i)
	{
		if (ms_mouseDevice[i] == p_deviceHandle)
		{
			return i;
		}
	}
	
	TT_PANIC("Mouse handle 0x%08X isn't known to MouseController.", p_deviceHandle);
	return -1;
}


void MouseController::updateWindowRects(HWND p_window)
{
	// Get the window rectangles
	GetClientRect(p_window, &ms_clientRect);
	{
		POINT topLeft;
		POINT botRight;
		topLeft.x  = ms_clientRect.left;
		topLeft.y  = ms_clientRect.top;
		botRight.x = ms_clientRect.right;
		botRight.y = ms_clientRect.bottom;
		ClientToScreen(p_window, &topLeft);
		ClientToScreen(p_window, &botRight);
		
		ms_windowRect.left   = topLeft.x;
		ms_windowRect.top    = topLeft.y;
		ms_windowRect.right  = botRight.x;
		ms_windowRect.bottom = botRight.y;
	}
}

// Namespace end
}
}
