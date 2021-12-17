#include <tt/engine/renderer/directx.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace engine {
namespace renderer {

// NOTE: For Xbox 360, all of the variables and functions declared in directx.h are defined/implemented in XboxApp.cpp

bool checkD3DSucceeded(HRESULT p_result)
{
	if (SUCCEEDED(p_result)) return true;

	std::string errorMsg;

	switch(p_result)
	{
	case D3DERR_INVALIDCALL:
		errorMsg = "D3D Function returned D3DERR_INVALIDCALL: Check parameters.";
		break;

	case D3DERR_OUTOFVIDEOMEMORY:
		errorMsg = "No more video memory (VRAM) available.";
		break;

	case E_OUTOFMEMORY:
		errorMsg = "No more system memory (RAM) available.";
		break;

	default:
		errorMsg = "Unknown error occured in D3D function.";
		break;
	}

	TT_PANIC(errorMsg.c_str());

	return false;
}

// Namespace end
}
}
}
