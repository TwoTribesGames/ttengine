#include <tt/engine/renderer/device_enumeration.h>
#include <tt/engine/renderer/directx.h>

namespace tt {
namespace engine {
namespace renderer {


Resolutions getSupportedResolutions(bool p_keepDesktopAspectRatio)
{
	Resolutions resolutions;
	
	const DXUTDeviceSettings& deviceSettings = DXUTGetDeviceSettings();
	
	CD3D9Enumeration* d3dEnumeration = DXUTGetD3D9Enumeration();
	TT_NULL_ASSERT(d3dEnumeration);
	
	CD3D9EnumAdapterInfo* adapterInfo = d3dEnumeration->GetAdapterInfo(deviceSettings.d3d9.AdapterOrdinal);
	if (adapterInfo != 0)
	{
		// Get the desktop aspect ratio
		D3DDISPLAYMODE desktopMode;
		DXUTGetDesktopResolution(deviceSettings.d3d9.AdapterOrdinal, &desktopMode.Width, &desktopMode.Height);
		const real desktopAspectRatio = desktopMode.Width / static_cast<real>(desktopMode.Height);
		
		for (int i = 0; i < adapterInfo->displayModeList.GetSize(); i++)
		{
			D3DDISPLAYMODE displayMode = adapterInfo->displayModeList.GetAt(i);
			const real aspectRatio = displayMode.Width / static_cast<real>(displayMode.Height);
			
			if (displayMode.Format == deviceSettings.d3d9.AdapterFormat)
			{
				if (p_keepDesktopAspectRatio == false || math::fabs(desktopAspectRatio - aspectRatio) < 0.05f)
				{
					resolutions.insert(math::Point2(displayMode.Width, displayMode.Height));
				}
			}
		}
	}
	
	return resolutions;
}


// Namespace end
}
}
}
