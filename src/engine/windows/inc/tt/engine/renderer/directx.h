#if !defined(INC_TT_ENGINE_RENDERER_DIRECTX_H)
#define INC_TT_ENGINE_RENDERER_DIRECTX_H


#define NOMINMAX
// Windows includes

#include <tt/engine/renderer/DXUT/DXUT.h>
#include <tt/platform/tt_error.h>



namespace tt {

// Generic COM pointer release helper (to replace DXUT's SAFE_RELEASE)
template<typename T>
inline void safeRelease(T*& p_interface)
{
	if (p_interface != 0)
	{
		p_interface->Release();
		p_interface = 0;
	}
}

namespace engine {
namespace renderer {


// Check result values from D3D calls
bool checkD3DSucceeded(HRESULT p_result);


//--------------------------------------------------------------------------------------------------
// Windows implementation: forward to DXUT

inline IDirect3DDevice9* getRenderDevice(bool p_mustExist = false)
{
	IDirect3DDevice9* device(DXUTGetD3D9Device());
	//TT_ASSERTMSG((p_mustExist == false) || (p_mustExist && device != 0),
	//	"D3D Render Device is not available!");
	return device;
}

#define FP(x) x
#define FPRS(x) x


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_DIRECTX_H)
