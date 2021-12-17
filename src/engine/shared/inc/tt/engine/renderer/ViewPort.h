#ifndef INC_TT_ENGINE_RENDERER_VIEWPORT
#define INC_TT_ENGINE_RENDERER_VIEWPORT

#include <vector>

#include <tt/engine/scene/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace renderer {


struct ViewPortSettings
{
	s32  x;
	s32  y;
	s32  width;
	s32  height;
	real minZ;
	real maxZ;

	ViewPortSettings(s32 p_topLeftX, s32 p_topLeftY, s32 p_width, s32 p_height);
};


class ViewPort
{
public:
	ViewPort(s32 p_topLeftX, s32 p_topLeftY, s32 p_width, s32 p_height);
	~ViewPort();
	
	void begin();
	void end  ();
	
	void setCamera(const scene::CameraPtr& p_camera);
	inline ViewPortSettings& modifySettings() { return m_settings; }
	
	inline const scene::CameraPtr& getCamera()   const { return m_camera;   }
	inline const ViewPortSettings& getSettings() const { return m_settings; }
	
	static inline ViewPortContainer& getViewPorts() { return ms_viewports; }
	
	static const ViewPort& getViewPort(ViewPortID p_viewPortID);
	
	static inline bool hasViewPort(ViewPortID p_viewPortID)
	{
		return p_viewPortID >= 0 &&
			static_cast<ViewPortContainer::size_type>(p_viewPortID) < ms_viewports.size();
	}

	static void createLayout(Layout p_layout);
	static void resetLayout ();
	
private:
	ViewPortSettings m_settings;
	scene::CameraPtr m_camera;

	static ViewPortContainer ms_viewports;
	static Layout            ms_layout;
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_VIEWPORT
