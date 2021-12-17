#include <tt/code/bufferutils_get.h>
#include <tt/code/bufferutils_put.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/CameraMgr.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

CameraMgr::CameraMgr(const std::string& p_configKeyBase)
:
m_camera(p_configKeyBase),
m_drcCameraEnabled(false),
m_drcCamIsMain(false),
m_drcCamera(p_configKeyBase, tt::engine::renderer::ViewPortID_1),
m_emulateDRC(false),
m_combinedVisibilityRect(),
m_currentViewportState()
{
}


void CameraMgr::update(real                     p_deltaTime,
                       const tt::math::Vector2& p_scrollOffset,
                       const tt::math::Vector2& p_effectOffsetTV,
                       const tt::math::Vector2& p_effectOffsetDRC,
                       bool                     p_applyEffectMgrEffects)
{
	m_camera.setScrollOffset(p_scrollOffset);
	m_camera.update(p_deltaTime, p_effectOffsetTV, p_applyEffectMgrEffects);
	
	if (m_emulateDRC) // Update the drcCamera latest so it will override the engine camera.
	{
		if (m_drcCameraEnabled == false)
		{
			m_drcCamera.syncWith(m_camera);
		}
		else
		{
			m_drcCamera.setScrollOffset(p_scrollOffset);
			m_drcCamera.update(p_deltaTime, p_effectOffsetDRC, p_applyEffectMgrEffects);
		}
	}
}


bool CameraMgr::updateForRender()
{
	// In updateForRender we can't cull on a single camera, we need to know where both cameras are.
	// For now we merge both rects if both are active, otherwise we only use the one active camera.
	bool firstCam = true;
	
	if (m_drcCameraEnabled && m_drcCamera.isRenderingHudOnly() == false)
	{
		m_combinedVisibilityRect = m_drcCamera.getCurrentVisibleRect();
		firstCam = false;
	}
	if (m_camera.isRenderingHudOnly() == false)
	{
		if (firstCam)
		{
			m_combinedVisibilityRect = m_camera.getCurrentVisibleRect();
		}
		else
		{
			// Merge both rects.
			m_combinedVisibilityRect = tt::math::merge(m_camera.getCurrentVisibleRect(), m_combinedVisibilityRect);
		}
		firstCam = false;
	}
	
	return firstCam;
}


void CameraMgr::onRenderBegin()
{
	m_currentViewportState.renderingToDRC = m_emulateDRC;
	
	m_currentViewportState.renderingMainCam = (m_currentViewportState.renderingToDRC ^ isDrcCameraMain()) == false;
	
	if (m_currentViewportState.renderingToDRC)
	{
		m_currentViewportState.renderHudOnly  = m_drcCamera.isRenderingHudOnly();
		m_currentViewportState.visibilityRect = &m_drcCamera.getCurrentVisibleRect();
	}
	else
	{
		m_currentViewportState.renderHudOnly  = m_camera.isRenderingHudOnly();
		m_currentViewportState.visibilityRect = &m_camera.getCurrentVisibleRect();
	}
}


void CameraMgr::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_camera.serialize(p_context);
	bu::put(m_drcCameraEnabled, p_context);
	bu::put(m_drcCamIsMain,     p_context);
	m_drcCamera.serialize(p_context);
}


void CameraMgr::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_camera.unserialize(p_context);
	m_drcCameraEnabled = bu::get<bool>(p_context);
	m_drcCamIsMain     = bu::get<bool>(p_context);
	m_drcCamera.unserialize(p_context);
}

// Namespace end
}
}
