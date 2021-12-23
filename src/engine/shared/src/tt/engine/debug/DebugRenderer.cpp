#include <tt/engine/debug/DebugRenderer.h>

#if defined(TT_BUILD_FINAL)
	// Only update the DebugStats in FINAL builds.
	#include <tt/engine/debug/DebugStats.h>
	namespace tt { namespace engine { namespace debug {
		void DebugRenderer::beginFrame() { DebugStats::clearFrameStats(); }
		void DebugRenderer::endFrame()   { DebugStats::endFrame();        }
	}}} // Namespace end.
#else
#include <tt/engine/debug/DebugFont.h>
#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/debug/screen_capture.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/pp/PostProcessor.h>
#include <tt/math/Box.h>
#include <tt/mem/util.h>
#include <tt/str/str.h>
#include <tt/system/utils.h>


namespace tt {
namespace engine {
namespace debug {


static const s32 dummyTextureSize     = 64;
static const s32 circlePrimitiveCount = 32; // (segments for solid = -2; outline = -1)


DebugRenderer::~DebugRenderer()
{
	DebugFont::destroy();
}


void DebugRenderer::beginFrame()
{
	// FIXME: Cannot do this in constructor, maybe let Renderer do it??
	DebugFont::initialize();

	// Restore book keeping
	DebugStats::clearFrameStats();

	m_quads.clear();
	m_primitiveBuffers.clear();
	m_quadBuffers.clear();
}


void DebugRenderer::endFrame()
{
	// Do sanity checking on the matrix stack
	renderer::MatrixStack::getInstance()->checkIntegrity();
	
	// Screenshots
	if(m_screenCaptureMode != ScreenCaptureMode_None)
	{
		// Make screenshot
		
		if (m_screenCaptureMode == ScreenCaptureMode_ToFile)
		{
			// Determine filename
			std::string filename;
			std::string desktopPath = tt::system::getDesktopPath();
			
			do
			{
				++m_fileCounter;
			
				std::stringstream count;
				count.width(4);
				count.fill('0');
				count << m_fileCounter;
			
				filename = desktopPath;
				filename += m_captureFilename + count.str() + ".png";
			}
			while (tt::fs::fileExists(filename));
			
			// FIXME: Make this configurable somehow... extra modifier key on shortcut perhaps?
			static const bool saveScreenshotWithTransparency = true;
			saveScreenCaptureToFile(filename, saveScreenshotWithTransparency);
		}
		else if (m_screenCaptureMode == ScreenCaptureMode_ToClipboard)
		{
			saveScreenCaptureToClipboard();
		}
		else
		{
			TT_PANIC("Unsupported screencapture mode '%d'", m_screenCaptureMode);
		}
		
		m_screenCaptureMode = ScreenCaptureMode_None;
	}
	
	// Debugging information
	if (m_displayCullingInfo)
	{
		FrameStatistics stats = DebugStats::getStats();
		printf(10,10,"Objects Rendered = %d\nObjects Culled = %d\n Polygons Rendered = %d\n",
		          stats.objectsRendered, stats.objectsCulled, stats.polygonsRendered);
	}

	DebugStats::endFrame();
}


void DebugRenderer::renderLine(const renderer::ColorRGBA& p_color,
	                           const math::Vector3&       p_start,
	                           const math::Vector3&       p_end)
{
	m_lineVertices.push_back(DebugVtx(p_start, p_color, math::Vector2::zero));
	m_lineVertices.push_back(DebugVtx(p_end,   p_color, math::Vector2::zero));
}


void DebugRenderer::renderRect(const renderer::ColorRGBA& p_color, const math::VectorRect& p_rect, bool p_flipY)
{
	tt::math::Vector2 min(p_rect.getMin()    );
	tt::math::Vector2 max(p_rect.getMaxEdge());
	
	// Flip Y here.
	const real sign = p_flipY ? -1.0f : 1.0f;
	tt::math::Vector3 a(min.x, sign * min.y, 0);
	tt::math::Vector3 b(max.x, sign * min.y, 0);
	tt::math::Vector3 c(min.x, sign * max.y, 0);
	tt::math::Vector3 d(max.x, sign * max.y, 0);
	
	renderLine(p_color, a, b);
	renderLine(p_color, a, c);
	renderLine(p_color, c, d);
	renderLine(p_color, d, b);
}


void DebugRenderer::renderRect(const renderer::ColorRGBA& p_color, const math::PointRect& p_rect, bool p_flipY)
{
	renderRect(p_color,
	           math::VectorRect(math::Vector2(p_rect.getPosition()),
	                            static_cast<real>(p_rect.getWidth()),
	                            static_cast<real>(p_rect.getHeight())),
	           p_flipY);
}


void DebugRenderer::renderSolidRect(const renderer::ColorRGBA& p_color, const math::VectorRect& p_rect, bool p_flipY)
{
	const real bottom = p_flipY ? -p_rect.getBottom() : p_rect.getBottom();
	const real top    = p_flipY ? -p_rect.getTop()    : p_rect.getTop();

	renderer::BatchQuad quad;
	quad.bottomLeft. setPosition(p_rect.getLeft(),  bottom, 0);
	quad.bottomRight.setPosition(p_rect.getRight(), bottom, 0);
	quad.topLeft.    setPosition(p_rect.getLeft(),  top,    0);
	quad.topRight.   setPosition(p_rect.getRight(), top,    0);

	quad.topLeft.setColor(p_color);
	quad.topRight.setColor(p_color);
	quad.bottomLeft.setColor(p_color);
	quad.bottomRight.setColor(p_color);

	m_quadVertices.push_back(quad);
}


void DebugRenderer::renderSolidRect(const renderer::ColorRGBA& p_color, const math::PointRect& p_rect, bool p_flipY)
{
	renderSolidRect(p_color,
	                math::VectorRect(math::Vector2(p_rect.getPosition()),
	                                 static_cast<real>(p_rect.getWidth()),
	                                 static_cast<real>(p_rect.getHeight())),
	                p_flipY);
}


void DebugRenderer::renderCircle(const renderer::ColorRGBA& p_color, const math::Vector2& p_pos, real p_radius)
{
	// Unlike solid we don't need the center vertex, so we have 1 extra.
	const real subdivisionAngle = math::twoPi / (circlePrimitiveCount - 1);
	real angle = math::twoPi - subdivisionAngle;

	// First vertex
	const math::Vector3 startPos(p_pos.x, p_pos.y + p_radius, 0);
	m_lineVertices.push_back(DebugVtx(startPos, p_color, math::Vector2::zero));
	
	for (s32 i = 0; i < circlePrimitiveCount-2; ++i)
	{
		DebugVtx vtx;
		vtx.setPosition(p_radius * math::sin(angle) + p_pos.x,
		                p_radius * math::cos(angle) + p_pos.y,
		                0.0f);
		vtx.setColor(p_color);
		angle -=  subdivisionAngle;

		m_lineVertices.push_back(vtx);
		m_lineVertices.push_back(vtx);
	}

	// Last vertex
	m_lineVertices.push_back(DebugVtx(startPos, p_color, math::Vector2::zero));
}


void DebugRenderer::renderSolidCircle(const renderer::ColorRGBA&     p_centerColor,
                                      const renderer::ColorRGBA&     p_edgeColor,
                                      const math::Vector2& p_pos,
                                      real                 p_radius)
{
	const real subdivisionAngle = math::twoPi / (circlePrimitiveCount - 2);
	real angle = math::twoPi - subdivisionAngle;

	// First vertex
	const DebugVtx centerVtx(math::Vector3(p_pos.x, p_pos.y, 0), p_centerColor, math::Vector2::zero);
	const DebugVtx firstVtx (math::Vector3(p_pos.x, p_pos.y + p_radius, 0), p_edgeColor, math::Vector2::zero);

	m_triangleVertices.push_back(firstVtx);
	m_triangleVertices.push_back(centerVtx);

	for (s32 i = 1; i < circlePrimitiveCount-2; ++i)
	{
		DebugVtx vtx;
		vtx.setPosition(p_radius * math::sin(angle) + p_pos.x,
		                p_radius * math::cos(angle) + p_pos.y,
		                0.0f);
		vtx.setColor(p_edgeColor);
		angle -=  subdivisionAngle;

		m_triangleVertices.push_back(vtx);
		m_triangleVertices.push_back(vtx);
		m_triangleVertices.push_back(centerVtx);
	}

	m_triangleVertices.push_back(firstVtx);
}


void DebugRenderer::renderSphere(const math::Vector3& p_position, real p_radius, bool p_wireframe)
{
	(void) p_position;
	(void) p_radius;
	(void) p_wireframe;
	
	// FIXME: No cross-platform implementation
}


void DebugRenderer::renderText(const std::string& p_message, s32 p_x, s32 p_y, const renderer::ColorRGBA& p_color)
{
	DebugFont::setColor(p_color);
	DebugFont::draw(p_message, p_x, p_y);
}


void DebugRenderer::renderText(const std::wstring& p_message, s32 p_x, s32 p_y, const renderer::ColorRGBA& p_color)
{
	return renderText(tt::str::narrow(p_message), p_x, p_y, p_color);
}


void DebugRenderer::printf(s32 p_x, s32 p_y, const char* p_message, ...)
{
	// Prepare buffer for text
	enum { BufferSize = 1024 };
	char buffer[BufferSize] = { 0 };
	
	va_list args;
	va_start(args, p_message);
	
	// Format message into buffer
	vsnprintf(buffer, BufferSize, p_message, args);
	va_end(args);
	
	// Render it
	renderText(buffer, p_x, p_y, renderer::ColorRGB::red);
}


void DebugRenderer::renderGrid(real p_spacing, s32 p_size)
{
	math::Vector3 begin(0, 0, -p_spacing * p_size);
	math::Vector3 end(0, 0, p_spacing * p_size);
	
	for (s32 i = -p_size; i <= p_size; ++i)
	{
		end.x = begin.x = i * p_spacing;
		renderLine(renderer::ColorRGBA(100,100,100,100), begin, end);
	}
	
	begin.x = -p_spacing * p_size;
	end.x   =  p_spacing * p_size;
	
	for (s32 i = -p_size; i <= p_size; ++i)
	{
		end.z = begin.z = i * p_spacing;
		renderLine(renderer::ColorRGBA(100,100,100,100), begin, end);
	}
}


void DebugRenderer::renderAABox(const renderer::ColorRGBA&     p_color,
                                const math::Vector3& p_min,
                                const math::Vector3& p_max)
{
	//                  |
	//      G       H   |
	//      *-------*   |
	//     /|      /|   |
	//    / |     / |   |
	//  C*-------*D |   |
	//   |  *----|--*F  |
	//   | /E    | /    |
	//   |/      |/     |
	//   *-------*      |
	//   A       B      |
	//                  |
	//                  |
	//    Y             |
	//    ^             |
	//    |             |
	//    |             |
	//    *---> X       |
	//   /              |
	// |/_              |
	// Z                |
	// min -> E         |
	// max -> D         |
	//                  |
	//                  |
	
	const math::Vector3 vtxA(p_min.x, p_min.y, p_max.z);
	const math::Vector3 vtxB(p_max.x, p_min.y, p_max.z);
	const math::Vector3 vtxC(p_min.x, p_max.y, p_max.z);
	const math::Vector3 vtxD(p_max.x, p_max.y, p_max.z);
	const math::Vector3 vtxE(p_min.x, p_min.y, p_min.z);
	const math::Vector3 vtxF(p_max.x, p_min.y, p_min.z);
	const math::Vector3 vtxG(p_min.x, p_max.y, p_min.z);
	const math::Vector3 vtxH(p_max.x, p_max.y, p_min.z);

	renderLine(p_color, vtxA, vtxB);
	renderLine(p_color, vtxA, vtxC);
	renderLine(p_color, vtxC, vtxD);
	renderLine(p_color, vtxD, vtxB);
	renderLine(p_color, vtxD, vtxH);
	renderLine(p_color, vtxB, vtxF);
	renderLine(p_color, vtxH, vtxF);
	renderLine(p_color, vtxC, vtxG);
	renderLine(p_color, vtxA, vtxE);
	renderLine(p_color, vtxG, vtxE);
	renderLine(p_color, vtxG, vtxH);
	renderLine(p_color, vtxE, vtxF);
}


void DebugRenderer::renderSolidAABox(const renderer::ColorRGBA&     p_color,
                                     const math::Vector3& p_min,
                                     const math::Vector3& p_max)
{
	//                  |
	//      G       H   |
	//      *-------*   |
	//     /|      /|   |
	//    / |     / |   |
	//  C*-------*D |   |
	//   |  *----|--*F  |
	//   | /E    | /    |
	//   |/      |/     |
	//   *-------*      |
	//   A       B      |
	//                  |
	//                  |
	//    Y             |
	//    ^             |
	//    |             |
	//    |             |
	//    *---> X       |
	//   /              |
	// |/_              |
	// Z                |
	// min -> E         |
	// max -> D         |
	//                  |
	//                  |
	
	renderer::BatchQuad aaBoxQuad[6];

	aaBoxQuad[0].bottomLeft. setPosition(p_min.x, p_min.y, p_max.z); // A
	aaBoxQuad[0].topLeft.    setPosition(p_min.x, p_max.y, p_max.z); // C
	aaBoxQuad[0].topRight.   setPosition(p_max.x, p_max.y, p_max.z); // D
	aaBoxQuad[0].bottomRight.setPosition(p_max.x, p_min.y, p_max.z); // B

	aaBoxQuad[1].bottomLeft. setPosition(p_max.x, p_min.y, p_max.z); // B
	aaBoxQuad[1].topLeft.    setPosition(p_max.x, p_max.y, p_max.z); // D
	aaBoxQuad[1].topRight.   setPosition(p_max.x, p_max.y, p_min.z); // H
	aaBoxQuad[1].bottomRight.setPosition(p_max.x, p_min.y, p_min.z); // F

	aaBoxQuad[2].bottomLeft. setPosition(p_min.x, p_min.y, p_min.z); // E
	aaBoxQuad[2].topLeft.    setPosition(p_min.x, p_max.y, p_min.z); // G
	aaBoxQuad[2].topRight.   setPosition(p_min.x, p_max.y, p_max.z); // C
	aaBoxQuad[2].bottomRight.setPosition(p_min.x, p_min.y, p_max.z); // A

	aaBoxQuad[3].bottomLeft. setPosition(p_max.x, p_min.y, p_min.z); // F
	aaBoxQuad[3].topLeft.    setPosition(p_max.x, p_max.y, p_min.z); // H
	aaBoxQuad[3].topRight.   setPosition(p_min.x, p_max.y, p_min.z); // G
	aaBoxQuad[3].bottomRight.setPosition(p_min.x, p_min.y, p_min.z); // E

	aaBoxQuad[4].bottomLeft. setPosition(p_min.x, p_max.y, p_max.z); // C
	aaBoxQuad[4].topLeft.    setPosition(p_min.x, p_max.y, p_min.z); // G
	aaBoxQuad[4].topRight.   setPosition(p_max.x, p_max.y, p_min.z); // H
	aaBoxQuad[4].bottomRight.setPosition(p_max.x, p_max.y, p_max.z); // D

	aaBoxQuad[5].bottomLeft. setPosition(p_min.x, p_min.y, p_min.z); // E
	aaBoxQuad[5].topLeft.    setPosition(p_min.x, p_min.y, p_max.z); // A
	aaBoxQuad[5].topRight.   setPosition(p_max.x, p_min.y, p_max.z); // B
	aaBoxQuad[5].bottomRight.setPosition(p_max.x, p_min.y, p_min.z); // F
	
	// Set color for all vertices
	for(int i=0; i < 6; ++i)
	{
		aaBoxQuad[i].bottomLeft. setColor(p_color);
		aaBoxQuad[i].topLeft.    setColor(p_color);
		aaBoxQuad[i].topRight.   setColor(p_color);
		aaBoxQuad[i].bottomRight.setColor(p_color);

		m_quadVertices.push_back(aaBoxQuad[i]);
	}
}


void DebugRenderer::flush()
{
	// FIXME: Handle situations where vertex count is higher than maximum allowed by buffers
	if (m_lineVertices.empty() == false)
	{
		using namespace tt::engine::renderer;

		TrianglestripBufferPtr buffer (new TrianglestripBuffer(static_cast<s32>(m_lineVertices.size()), 1, TexturePtr(),
			BatchFlagTrianglestrip_UseVertexColor, TrianglestripBuffer::PrimitiveType_Lines));

		buffer->setCollection(m_lineVertices);
		buffer->applyChanges();
		buffer->render();

		m_lineVertices.clear();
		m_primitiveBuffers.push_back(buffer);
	}

	if (m_triangleVertices.empty() == false)
	{
		using namespace tt::engine::renderer;
		TrianglestripBufferPtr buffer (new TrianglestripBuffer(static_cast<s32>(m_triangleVertices.size()), 1, TexturePtr(),
			BatchFlagTrianglestrip_UseVertexColor, TrianglestripBuffer::PrimitiveType_Triangles));

		buffer->setCollection(m_triangleVertices);
		buffer->applyChanges();
		buffer->render();

		m_triangleVertices.clear();
		m_primitiveBuffers.push_back(buffer);
	}

	if (m_quadVertices.empty() == false)
	{
		using namespace tt::engine::renderer;
		QuadBufferPtr buffer (new QuadBuffer(
			static_cast<s32>(m_quadVertices.size()), TexturePtr(), BatchFlagQuad_UseVertexColor));
		
		buffer->fillBuffer(m_quadVertices.begin(), m_quadVertices.end());
		buffer->render();
		
		m_quadVertices.clear();
		m_quadBuffers.push_back(buffer);
	}
	DebugFont::flush();
}


void DebugRenderer::toggleAxis(real p_length)
{
	m_axisLength = p_length;
	m_showAxis   = (m_showAxis == false);
}


void DebugRenderer::toggleSafeFrame()
{
	switch (m_safeFrame)
	{
		case Frame_None      : m_safeFrame = Frame_ActionSafe; break;
		case Frame_ActionSafe: m_safeFrame = Frame_TitleSafe;  break;
		case Frame_TitleSafe : m_safeFrame = Frame_None;       break;
		default:
			TT_PANIC("Unsupported safe frame mode (%d)", m_safeFrame);
	}
}


void DebugRenderer::toggleWireFrame()
{
	m_displayWireFrame = (m_displayWireFrame == false);

	setOverdrawDebugActive(m_displayWireFrame);
	
	using namespace tt::engine::renderer;
	Renderer::getInstance()->setFillMode(m_displayWireFrame ? FillMode_Wireframe : FillMode_Solid);
}


void DebugRenderer::showAxis()
{
	// Render axis for debugging
	if (m_showAxis) renderAxis();
}


void DebugRenderer::showSafeFrame()
{
	// Draw safe frame here
	if (m_safeFrame != Frame_None)
	{
		if (m_safeFrame == Frame_ActionSafe)
		{
			renderSafeFrame(math::Vector2(0.035f, 0.035f), math::Vector2(0.035f, 0.035f));
			renderText("Action Safe", 20, renderer::Renderer::getInstance()->getScreenHeight() - 20);
		}
		else if (m_safeFrame == Frame_TitleSafe)
		{
			renderSafeFrame(math::Vector2(0.067f, 0.05f), math::Vector2(0.1f, 0.05f));
			renderText("Title Safe", 20, renderer::Renderer::getInstance()->getScreenHeight() - 20);
		}
	}
}


real DebugRenderer::getFramesPerSecond() const
{
	// FIXME: Should probably be managed by a dedicated stats class
	// return DXUTGetFPS();
	return 60.0f;
}


renderer::TexturePtr DebugRenderer::getDummyTexture()
{
	if (m_dummyTexture == 0)
	{
		createDummyTexture();
	}

	return m_dummyTexture;
}


void DebugRenderer::setOverdrawDebugActive(bool p_active)
{
	renderer::FixedFunction::setOverdrawModeEnabled(p_active);
	renderer::Renderer* renderer = renderer::Renderer::getInstance();
	
	// Switch to active => save render states and set new ones
	if (m_overdrawDebugModeActive == false && p_active)
	{
		m_restoreClearColor     = renderer->getClearColor();
		m_restorePostProcessing = renderer->getPP()->isActive();
		
		renderer->setPremultipliedAlphaEnabled(false);
		renderer->setClearColor(renderer::ColorRGB::black);
		renderer->setBlendMode (renderer::BlendMode_Add);
		renderer->getPP()->setActive(false);
		renderer::FixedFunction::setOverdrawColor(getDebugColor());
	}

	// Change the member variable here, because this variable is used by Renderer to check if we are
	// in overdraw mode and changing clear color / blend mode aren't allowed
	bool originalState = m_overdrawDebugModeActive;
	m_overdrawDebugModeActive = p_active;
	
	// Switch to inactive => restore render states
	if (originalState && m_overdrawDebugModeActive == false)
	{
		renderer->setClearColor(m_restoreClearColor);
		renderer->getPP()->setActive(m_restorePostProcessing);
		renderer->setBlendMode (m_restoreBlendMode);
	}
}


void DebugRenderer::setMipmapVisualizerActive(bool p_active)
{
	// FIXME: Implement on other platforms as well
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	renderer::FixedFunction::setMipmapVisualizationEnabled(p_active);
	m_mipmapVisualizerActive = p_active;
	renderer::Renderer* renderer = renderer::Renderer::getInstance();
	if (p_active == false)
	{
		// MARTIJN HACK: Something seriously messes up the visualization if the PP doesn't get reset
		// Marco any idea?
		renderer->getPP()->deinitialize();
	}
#else
	(void)p_active;
#endif
}


renderer::ColorRGBA DebugRenderer::getDebugColor()
{
	if (wireFrameEnabled())
	{
		return renderer::ColorRGB::white;
	}
	
	return renderer::ColorRGBA(renderer::ColorRGB::white, Constants_DebugAlpha);
}


void DebugRenderer::setBaseCaptureFilename(const std::string& p_filename)
{
	m_captureFilename = p_filename;
	
	// Lowercase + replace all illegal characters
	m_captureFilename = str::toLower(m_captureFilename);
	str::replace(m_captureFilename, " ", "_");
	str::replace(m_captureFilename, ":", "_");
	str::replace(m_captureFilename, "/", "_");
	str::replace(m_captureFilename, "\\", "_");
	str::replace(m_captureFilename, "*", "_");
	str::replace(m_captureFilename, "?", "_");
	str::replace(m_captureFilename, "\"", "_");
	str::replace(m_captureFilename, "<", "_");
	str::replace(m_captureFilename, ">", "_");
	str::replace(m_captureFilename, "|", "_");
}


void DebugRenderer::startRenderGroup(const char* p_name)
{
#if defined(TT_PLATFORM_SDL)
	renderer::Renderer::getInstance()->pushMarker(p_name);
#elif defined(TT_PLATFORM_WIN)
	std::wstring eventName(str::widen(std::string(p_name)));
	D3DPERF_BeginEvent(D3DCOLOR_XRGB(255,0,0), eventName.c_str());
#else
	(void) p_name;
#endif
}


void DebugRenderer::endRenderGroup()
{
#if defined(TT_PLATFORM_SDL)
	renderer::Renderer::getInstance()->popMarker();
#elif defined(TT_PLATFORM_WIN)
	D3DPERF_EndEvent();
#endif
}


////////////////////////////////////////////
// Private

DebugRenderer::DebugRenderer()
:
m_displayCullingInfo(false),
m_showAxis(false),
m_displayWireFrame(false),
m_axisLength(1.0f),
m_displayBoundingBoxes(false),
m_displayShadows(false),
m_safeFrame(Frame_None),
m_speed(1.0f),
m_debugCamActive(false),
m_overdrawDebugModeActive(false),
m_mipmapVisualizerActive(false),
m_restoreClearColor(renderer::ColorRGB::black),
m_restorePostProcessing(false),
m_restoreBlendMode(renderer::BlendMode_Blend),
m_screenCaptureMode(ScreenCaptureMode_None),
m_captureFilename("screenshot"),
m_fileCounter(0)
{
}


void DebugRenderer::createDummyTexture()
{
	if(m_dummyTexture == 0)
	{
		m_dummyTexture = renderer::Texture::createForText(dummyTextureSize, dummyTextureSize);
		renderer::TexturePainter p(m_dummyTexture->lock());
		
		for(s32 y = 0; y < dummyTextureSize; ++y)
		{
			for(s32 x = 0; x < dummyTextureSize; ++x)
			{
				if(x == y || (x == dummyTextureSize - y - 1))
				{
					p.setPixel(x,y, renderer::ColorRGB::red);
				}
				else
				{
					p.setPixel(x,y, renderer::ColorRGB::magenta);
				}
			}
		}
	}
}


void DebugRenderer::renderAxis()
{
	renderLine(renderer::ColorRGB::red,   math::Vector3::zero, math::Vector3::unitX * m_axisLength);
	renderLine(renderer::ColorRGB::green, math::Vector3::zero, math::Vector3::unitY * m_axisLength);
	renderLine(renderer::ColorRGB::blue,  math::Vector3::zero, math::Vector3::unitZ * m_axisLength);
}


void DebugRenderer::renderSafeFrame(const math::Vector2& p_range4_3, const math::Vector2& p_range16_9)
{
	renderer::Renderer* renderer = renderer::Renderer::getInstance();
	
	const math::Vector2 screen(static_cast<real>(renderer->getScreenWidth()),
		                       static_cast<real>(renderer->getScreenHeight()));
	math::Vector2 frame;
	
	if (renderer->isWideScreen())
	{
		frame.x = p_range16_9.x * screen.x;
		frame.y = p_range16_9.y * screen.y;
	}
	else
	{
		frame.x = p_range4_3.x * screen.x;
		frame.y = p_range4_3.y * screen.y;
	}
	
	static const renderer::ColorRGBA frameColor(255,0,0,100);

	using math::Vector2;
	using math::VectorRect;

	renderSolidRect(frameColor, VectorRect(Vector2::zero, Vector2(frame.x, screen.y)), true);
	renderSolidRect(frameColor, VectorRect(Vector2(screen.x - frame.x, 0), screen), true);
	renderSolidRect(frameColor, VectorRect(Vector2::zero, Vector2(screen.x, frame.y)), true);
	renderSolidRect(frameColor, VectorRect(Vector2(0, screen.y - frame.y),  screen), true);
	
	renderLine(frameColor, math::Vector3(0, -screen.y / 2, 0), math::Vector3(screen.x, -screen.y / 2, 0));
	renderLine(frameColor, math::Vector3(screen.x / 2, 0, 0),  math::Vector3(screen.x / 2, -screen.y, 0));
	flush();
}


// Namespace end
}
}
}


#endif  // !defined(TT_BUILD_FINAL)
