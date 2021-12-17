#include <tt/app/Application.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/engine/scene/Camera.h>


namespace tt {
namespace engine {
namespace renderer {


static math::Vector3 getPlatformPixelOffset(const math::Point2& p_windowSize, const math::Point2& p_quadSize)
{
	// Handle offset to match texels to pixels (DirectX9 has different orientation)
	math::Vector3 result(0,0,0);
	
	if (p_windowSize.x % 2 == p_quadSize.x % 2)
	{
#if defined(TT_PLATFORM_WIN)
		result.x -= 0.5f;
#endif
	}
	else
	{
#if !defined(TT_PLATFORM_WIN)
		result.x -= 0.5f;
#endif
	}
		
	if (p_windowSize.y % 2 == p_quadSize.y % 2)
	{
#if defined(TT_PLATFORM_WIN)
		result.y -= 0.5f;
#endif
	}
	else
	{
#if !defined(TT_PLATFORM_WIN)
		result.y -= 0.5f;
#endif
	}
	return result;
}


UpScaler::UpScaler(const math::Point2& p_maxRenderTargetSize, bool p_invertY)
:
m_active(false),
m_enabled(true),
m_invertY(p_invertY),
m_maxSize(p_maxRenderTargetSize),
m_aspectRatioRange(0.0f, 9999.0f),
m_scaling(1,1)
{
}


math::Point2 UpScaler::handleResolutionChanged(s32 p_newWidth, s32 p_newHeight)
{
	const math::Point2 currentSize(p_newWidth, p_newHeight);
	
	if (m_enabled == false)
	{
		m_active = false;
		m_scaling.setValues(1,1);
		m_offset .setValues(0,0);
		return currentSize;
	}
	
	// Start with current size
	math::Point2 screenSize(currentSize);
	m_active = false;
	
	const real windowAspectRatio = screenSize.x / static_cast<real>(screenSize.y);
	if (windowAspectRatio < m_aspectRatioRange.x)
	{
		screenSize.y = static_cast<s32>((screenSize.x / m_aspectRatioRange.x) + 0.5f);
	}
	
	if (windowAspectRatio > m_aspectRatioRange.y)
	{
		screenSize.x = static_cast<s32>((screenSize.y * m_aspectRatioRange.y) + 0.5f);
	}
	
	// Check width
	if (m_maxSize.x > 0 && screenSize.x > m_maxSize.x)
	{
		// Resolution is larger than max width: create RT
		const real aspectRatio(screenSize.x / static_cast<real>(screenSize.y));
		
		// Use max resolution for width + compute needed height
		screenSize.setValues(m_maxSize.x, static_cast<s32>((m_maxSize.x / aspectRatio) + 0.5f));
		
		// Unless this results in the max height to be overwritten
		if (m_maxSize.y > 0 && screenSize.y > m_maxSize.y)
		{
			// Use max resolution for height + compute needed width
			screenSize.setValues(static_cast<s32>((m_maxSize.y * aspectRatio) + 0.5f), m_maxSize.y);
		}
	}
	
	// Check height
	else if (m_maxSize.y > 0 && screenSize.y > m_maxSize.y)
	{
		// Resolution is larger than max height: create RT
		const real aspectRatio = screenSize.x / static_cast<real>(screenSize.y);
		
		// Use max resolution for height + compute needed width
		screenSize.setValues(static_cast<s32>((m_maxSize.y * aspectRatio) + 0.5f), m_maxSize.y);
		
		// Unless this results in the max width to be overwritten
		if (m_maxSize.x > 0 && screenSize.x > m_maxSize.x)
		{
			// Use max resolution for width + compute needed height
			screenSize.setValues(m_maxSize.x, static_cast<s32>((m_maxSize.x / aspectRatio) + 0.5f));
		}
	}
	
	// If we modified the screen size, we need a render target
	if(screenSize != currentSize)
	{
		TT_Printf("[RENDERER] Creating target of (%d,%d) for upscaling to (%d,%d)\n",
			screenSize.x, screenSize.y, currentSize.x, currentSize.y);
		
		Renderer* renderer(Renderer::getInstance());
		
		const s32 samples = renderer->getPP()->isActive() ? 0 : renderer->getAASamples();
		
		m_renderTarget = RenderTarget::create(screenSize.x, screenSize.y, samples, true);
		
		const real screenWidth (static_cast<real>(currentSize.x));
		const real screenHeight(static_cast<real>(currentSize.y));
		
		// Create Quad with size of real back buffer
		m_fullScreenQuad = QuadSprite::createQuad(screenWidth, screenHeight, VertexBuffer::Property_Texture0);
		m_fullScreenQuad->setTexture(m_renderTarget->getTexture());
		m_fullScreenQuad->getMaterial()->setBlendMode(BlendFactor_One, BlendFactor_Zero);
		
		// Adjust quad for aspect ratios outside the clamped range so the result won't be distorted
		// but has black bars
		math::Point2 quadSize(currentSize);
		if (windowAspectRatio < m_aspectRatioRange.x)
		{
			quadSize.y = static_cast<s32>((screenWidth / m_aspectRatioRange.x) + 0.5f);
			m_fullScreenQuad->setHeight(static_cast<real>(quadSize.y));
		}
		
		if (windowAspectRatio > m_aspectRatioRange.y)
		{
			quadSize.x = static_cast<s32>((screenHeight * m_aspectRatioRange.y) + 0.5f);
			m_fullScreenQuad->setWidth(static_cast<real>(quadSize.x));
		}
		
		m_fullScreenQuad->setPosition(getPlatformPixelOffset(currentSize, quadSize));
		m_fullScreenQuad->setFlippedVertical(m_invertY);
		m_fullScreenQuad->update();
		
		m_renderTarget->getTexture()->setAddressMode(AddressMode_Clamp, AddressMode_Clamp);
		m_renderTarget->getTexture()->setFilterMode (FilterMode_Linear, FilterMode_Linear);
		
		// And a camera to display it
		m_upscaleCamera = scene::CameraPtr(new scene::Camera(
			math::Vector3::forward, math::Vector3::zero, screenWidth, screenHeight, 0.5f, 4096.0f));
		m_upscaleCamera->update();
		
		m_active = true;
	}
	
	// Compute scaling factors (<= 1.0, 1.0 = no scaling occurs)
	m_scaling.x = screenSize.x / static_cast<real>(currentSize.x);
	m_scaling.y = screenSize.y / static_cast<real>(currentSize.y);
	m_offset.setValues(0,0);
	
	// Compute scaling + offset
	if (windowAspectRatio > m_aspectRatioRange.y)
	{
		// Black bars on the sides
		const s32 widthOnScreen = static_cast<s32>((currentSize.y * m_aspectRatioRange.y) + 0.5f);
		m_offset.setValues(0.5f * (currentSize.x - widthOnScreen), 0);
		m_scaling.x = screenSize.x / static_cast<real>(widthOnScreen);
	}
	
	if (windowAspectRatio < m_aspectRatioRange.x)
	{
		// Black bars on top and bottom
		const s32 heightOnScreen = static_cast<s32>((currentSize.x / m_aspectRatioRange.x) + 0.5f);
		m_offset.setValues(0, 0.5f * (currentSize.y - heightOnScreen));
		m_scaling.y = screenSize.y / static_cast<real>(heightOnScreen);
	}
	//TT_Printf("Offset = [%f,%f], Scaling = [%f,%f]\n", m_offset.x, m_offset.y, m_scaling.x, m_scaling.y);
	
	return screenSize;
}


void UpScaler::beginFrame()
{
	if(m_active && m_renderTarget != 0)
	{
		m_renderTarget->setClearColor(Renderer::getInstance()->getClearColor());
		RenderTargetStack::push(m_renderTarget, ClearFlag_All);
	}
}


void UpScaler::endFrame()
{
	if (m_active && m_renderTarget != 0)
	{
		Renderer *renderer = Renderer::getInstance();
		RenderTargetStack::pop();
		
		// Make sure RT is updated (for AA)
		m_fullScreenQuad->setTexture(m_renderTarget->getTexture());
		
		// Render the result on a fullscreen quad
		TT_NULL_ASSERT(m_upscaleCamera);
		m_upscaleCamera->select();
		
		// Disable overdraw mode, so the overdraw results in the render target will be displayed
		debug::DebugRendererPtr debugRenderer = renderer->getDebug();
		const bool overdrawActive = debugRenderer->isOverdrawDebugActive();
		debugRenderer->setOverdrawDebugActive(false);
		
		const bool depthEnabled = renderer->isZBufferEnabled();
		renderer->setZBufferEnabled(false);
		
		TT_NULL_ASSERT(m_fullScreenQuad);
		m_fullScreenQuad->render();
		renderer->setZBufferEnabled(depthEnabled);
		
		debugRenderer->setOverdrawDebugActive(overdrawActive);
	}
}


void UpScaler::setEnabled(bool p_enabled)
{
	if (p_enabled != m_enabled)
	{
		m_enabled = p_enabled;
		
		app::getApplication()->handleResolutionChanged();
	}
}


// Namespace end
}
}
}
