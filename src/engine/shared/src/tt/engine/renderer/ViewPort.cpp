#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/scene/Camera.h>


namespace tt {
namespace engine {
namespace renderer {

ViewPortContainer ViewPort::ms_viewports;
Layout            ViewPort::ms_layout;


ViewPortSettings::ViewPortSettings(s32 p_topLeftX, s32 p_topLeftY, s32 p_width, s32 p_height)
:
x(p_topLeftX),
y(p_topLeftY),
width(p_width),
height(p_height),
minZ(0),
maxZ(1)
{}


ViewPort::ViewPort(s32 p_topLeftX, s32 p_topLeftY, s32 p_width, s32 p_height)
:
m_settings(p_topLeftX, p_topLeftY, p_width, p_height)
{
}


ViewPort::~ViewPort()
{
}


void ViewPort::begin()
{
	Renderer::getInstance()->setCustomViewport(
		m_settings.x, m_settings.y, m_settings.width, m_settings.height, m_settings.minZ, m_settings.maxZ);
	
	// Upload the camera
	if(m_camera != 0) m_camera->select();
}


void ViewPort::end()
{
}


void ViewPort::setCamera(const scene::CameraPtr& p_camera)
{
	m_camera = p_camera;
	
	if (m_camera != 0)
	{
		real width  = static_cast<real>(m_settings.width );
		real height = static_cast<real>(m_settings.height);
		
		m_camera->setViewPort(static_cast<real>(m_settings.x),
			                  static_cast<real>(m_settings.y),
			                  width, height);
	}
}


const ViewPort& ViewPort::getViewPort(ViewPortID p_viewPortID)
{
	if (hasViewPort(p_viewPortID) == false)
	{
		TT_PANIC("Viewport %d does not exist.", p_viewPortID);
		TT_ASSERT(ms_viewports.empty() == false);
		return ms_viewports.front();
	}
		
	return ms_viewports[p_viewPortID];
}


static scene::CameraPtr createCamera()
{
	static const real defaultNear =    1.0f;
	static const real defaultFar  = 4096.0f;
	static const real defaultFov  =   60.0f;
	
	return scene::Camera::createPerspective(
		math::Vector3(0,0,10), math::Vector3::zero, defaultNear, defaultFar, defaultFov);
}


void ViewPort::createLayout(Layout p_layout)
{
	if (ms_viewports.empty() || ms_layout != p_layout)
	{
		ms_viewports.clear();
		
		// Create new viewports + cameras
		s32 viewportCount(1);
		switch (p_layout)
		{
		case Layout_Standard : viewportCount = 1; break;
		case Layout_LeftRight: viewportCount = 2; break;
		case Layout_TopBottom: viewportCount = 2; break;
		default:
			TT_PANIC("Unsupported viewport layout: %d", p_layout);
		}
		
		for (s32 i = 0; i < viewportCount; ++i)
		{
			ViewPort viewport(0, 0, 640, 480);
			viewport.setCamera(createCamera());
			ms_viewports.push_back(viewport);
		}
	}
	
	ms_layout = p_layout;
	resetLayout();
}


void ViewPort::resetLayout()
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	const real screenWidth  = static_cast<real>(renderer->getScreenWidth ());
	const real screenHeight = static_cast<real>(renderer->getScreenHeight());
	
	switch (ms_layout)
	{
		case Layout_Standard:
		{
			ms_viewports[0].getCamera()->setViewPort(0.0f, 0.0f, screenWidth, screenHeight);
			ms_viewports[0].getCamera()->update();
			ms_viewports[0].modifySettings().width  = renderer->getScreenWidth ();
			ms_viewports[0].modifySettings().height = renderer->getScreenHeight();
			break;
		}
		
		case Layout_LeftRight:
		{
			ms_viewports[0].getCamera()->setViewPort(0.0f, 0.0f, screenWidth/2.0f, screenHeight);
			ms_viewports[0].getCamera()->update();
			ms_viewports[0].modifySettings().width  = renderer->getScreenWidth()/2;
			ms_viewports[0].modifySettings().height = renderer->getScreenHeight();
			
			ms_viewports[1].getCamera()->setViewPort(screenWidth/2.0f, 0, screenWidth/2.0f, screenHeight);
			ms_viewports[1].modifySettings().x      = renderer->getScreenWidth()/2;
			ms_viewports[1].modifySettings().width  = renderer->getScreenWidth()/2;
			ms_viewports[1].modifySettings().height = renderer->getScreenHeight();
			break;
		}
		
		default:
			break;
	}
}


// Namespace end
}
}
}
