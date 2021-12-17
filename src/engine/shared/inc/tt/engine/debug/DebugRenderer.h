#if !defined(INC_TT_ENGINE_DEBUG_DEBUGRENDERER_H)
#define INC_TT_ENGINE_DEBUG_DEBUGRENDERER_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/math/Rect.h>


namespace tt {
namespace engine {
namespace debug {


class DebugRenderer
{
public:
	enum ScreenCaptureMode
	{
		ScreenCaptureMode_None,
		ScreenCaptureMode_ToFile,
		ScreenCaptureMode_ToClipboard
	};
	
	
	// IMPORTANT: When adding new functions, also add them to the TT_BUILD_FINAL section!
	//            Final implementations should be empty, but the DebugRenderer interface
	//            should remain the same between non-final and final builds.
	
#if !defined(TT_BUILD_FINAL)
	~DebugRenderer();
	
	void beginFrame();
	void endFrame();
	
	// Primitive rendering
	void renderLine      (const renderer::ColorRGBA& p_color, const math::Vector3& p_start, const math::Vector3& p_end);
	void renderRect      (const renderer::ColorRGBA& p_color, const math::VectorRect& p_rect, bool p_flipY = false);
	void renderRect      (const renderer::ColorRGBA& p_color, const math::PointRect&  p_rect, bool p_flipY = false);
	void renderSolidRect (const renderer::ColorRGBA& p_color, const math::VectorRect& p_rect, bool p_flipY = false);
	void renderSolidRect (const renderer::ColorRGBA& p_color, const math::PointRect&  p_rect, bool p_flipY = false);
	void renderCircle    (const renderer::ColorRGBA& p_color, const math::Vector2& p_pos, real p_radius);

	inline void renderSolidCircle(const renderer::ColorRGBA& p_color, const math::Vector2& p_pos, real p_radius)
	{ renderSolidCircle(p_color, p_color, p_pos, p_radius); }
	void renderSolidCircle(const renderer::ColorRGBA&     p_centerColor,
	                       const renderer::ColorRGBA&     p_edgeColor,
	                       const math::Vector2& p_pos,
	                       real                 p_radius);
	
	void renderSphere(const math::Vector3& p_position, real p_radius, bool p_wireframe = true);
	void renderText(const std::string& p_message, s32 p_x = 0, s32 p_y = 0,
	                const renderer::ColorRGBA& p_color = renderer::ColorRGB::white);
	void renderText(const std::wstring& p_message, s32 p_x = 0, s32 p_y = 0,
	                const renderer::ColorRGBA& p_color = renderer::ColorRGB::white);
	void printf(s32 p_x, s32 p_y, const char* p_message, ...);
	
	void renderGrid (real p_spacing = 10.0f, s32 p_size = 10);
	void renderAABox (const renderer::ColorRGBA& p_color, const math::Vector3& p_min, const math::Vector3& p_max);
	void renderSolidAABox (const renderer::ColorRGBA& p_color, const math::Vector3& p_min, const math::Vector3& p_max);
	
	void flush();
	
	inline void setSpeed(real p_speed) {m_speed = p_speed;}
	inline real getSpeed() const {return m_speed;}
	
	void toggleAxis (real p_length = 1.0f);
	void toggleSafeFrame();
	void toggleWireFrame();
	
	inline bool wireFrameEnabled() const { return m_displayWireFrame; }
	
	void showAxis();
	void showSafeFrame();
	
	real getFramesPerSecond() const;
	
	renderer::TexturePtr getDummyTexture();
	
	// Debug camera
	// FIXME: Implement debug camera using engine camera, instead of DXUT
	inline void setDebugCameraActive(bool p_active) {m_debugCamActive = p_active;}
	inline bool getDebugCameraActive() const {return m_debugCamActive;}
	//inline CModelViewerCamera* getDebugCamera() const {return m_debugCam;}
	
	void setOverdrawDebugActive(bool p_active);
	inline bool isOverdrawDebugActive() const { return m_overdrawDebugModeActive; }
	
	void setMipmapVisualizerActive(bool p_active);
	inline bool isMipmapVisualizerActive() const { return m_mipmapVisualizerActive; }
	
	// Screenshot
	inline void captureScreen(ScreenCaptureMode p_mode) {m_screenCaptureMode = p_mode;}
	inline ScreenCaptureMode getCaptureMode() const { return m_screenCaptureMode; }
	void setBaseCaptureFilename(const std::string& p_filename);
	
	// Debug render section
	void startRenderGroup(const char*);
	void endRenderGroup();
	
#else
	// Empty implementations for Final builds
	DebugRenderer() {}
	~DebugRenderer() {}
	       void beginFrame(); // Almost empty, does update DebugStats.
	       void endFrame();   // Almost empty, does update DebugStats.
	inline void renderQuad(const renderer::ColorRGBA& = renderer::ColorRGB::red) {}
	inline void renderLine(const renderer::ColorRGBA&, const math::Vector3&, const math::Vector3&) {}
	inline void renderRect(const renderer::ColorRGBA&, const math::VectorRect&, bool = false) { }
	inline void renderRect(const renderer::ColorRGBA&, const math::PointRect&, bool = false) { }
	inline void renderSolidRect(const renderer::ColorRGBA&, const math::VectorRect&, bool = false) { }
	inline void renderSolidRect(const renderer::ColorRGBA&, const math::PointRect&, bool = false) { }
	inline void renderCircle(const renderer::ColorRGBA&, const math::Vector2&, real) { }
	inline void renderSolidCircle(const renderer::ColorRGBA&, const math::Vector2&, real) { }
	inline void renderSolidCircle(const renderer::ColorRGBA&, const renderer::ColorRGBA&, const math::Vector2&, real) { }
	inline void renderSphere(const math::Vector3&, real, bool = true) {}
	inline void renderText(const std::string&, s32 = 0, s32 = 0, const renderer::ColorRGBA& = renderer::ColorRGB::white) {}
	inline void renderText(const std::wstring&, s32 = 0, s32 = 0, const renderer::ColorRGBA& = renderer::ColorRGB::white) {}
	inline void printf(s32, s32, const char*, ...) {}
	inline void renderGrid(real = 10.0f, s32 = 10) {}
	inline void renderAABox(const renderer::ColorRGBA&, const math::Vector3&, const math::Vector3&) {}
	inline void renderSolidAABox(const renderer::ColorRGBA&, const math::Vector3&, const math::Vector3&) {}
	inline void flush() {}
	inline void toggleAxis(real = 1.0f) {}
	inline void setSpeed(real) {}
	inline real getSpeed() const { return 1.0f; }
	inline void toggleSafeFrame() {}
	inline void toggleWireFrame() {}
	inline bool wireFrameEnabled() const { return false; }
	inline void showAxis() {}
	inline void showSafeFrame() {}
	inline real getFramesPerSecond() const { return 0.0f; }
	inline renderer::TexturePtr getDummyTexture() {return renderer::TexturePtr();}
	inline void setDebugCameraActive(bool) {}
	inline bool getDebugCameraActive() const {return false;}
	inline void setOverdrawDebugActive(bool) { }
	inline bool isOverdrawDebugActive() const { return false; }
	inline void setMipmapVisualizerActive(bool) {}
	inline bool isMipmapVisualizerActive() const { return false; }
	inline void captureScreen(ScreenCaptureMode) {}
	inline ScreenCaptureMode getCaptureMode() const { return ScreenCaptureMode_None; }
	inline void setBaseCaptureFilename(const std::string&) {}
	inline void startRenderGroup(const char*) {}
	inline void endRenderGroup() {}
#endif
	
	
private:
	// No copying
	DebugRenderer(const DebugRenderer&);
	const DebugRenderer& operator=(const DebugRenderer&);
	
#if !defined(TT_BUILD_FINAL)
	enum Frame
	{
		Frame_None,
		Frame_ActionSafe,
		Frame_TitleSafe
	};
	enum Constants
	{
		Constants_DebugAlpha = 5
	};
	
	// Allow Renderer to instantiate
	friend class renderer::Renderer;
	DebugRenderer();

	void createResources();
	void createDummyTexture();
	
	void renderAxis();
	void renderSafeFrame(const math::Vector2& p_range4_3, const math::Vector2& p_range16_9);

	renderer::ColorRGBA getDebugColor();
	
	bool m_displayCullingInfo;
	bool m_showAxis;
	bool m_displayWireFrame;
	
	real m_axisLength;
	
	bool m_displayBoundingBoxes;
	bool m_displayShadows;
	Frame m_safeFrame;
	
	real m_speed;
	
	renderer::TexturePtr m_dummyTexture;
	
	// Debug Camera
	bool                m_debugCamActive;
	
	bool m_overdrawDebugModeActive;
	bool m_mipmapVisualizerActive;
	renderer::ColorRGBA m_restoreClearColor;
	bool                m_restorePostProcessing;
	renderer::BlendMode m_restoreBlendMode;
	
	// Screenshot
	ScreenCaptureMode m_screenCaptureMode;
	std::string       m_captureFilename;
	u32               m_fileCounter;
	
	// Debug primitives
	typedef std::vector<renderer::Quad2DPtr> Quad2DBuffer;
	Quad2DBuffer m_quads;

	renderer::TrianglestripVertices m_lineVertices;
	renderer::TrianglestripVertices m_triangleVertices;
	renderer::BatchQuadCollection   m_quadVertices;

	typedef std::vector<renderer::TrianglestripBufferPtr> PrimitiveBuffers;
	typedef std::vector<renderer::QuadBufferPtr>          QuadBuffers;

	PrimitiveBuffers m_primitiveBuffers;
	QuadBuffers      m_quadBuffers;

	typedef renderer::BufferVtxUV<1> DebugVtx;
#endif
};

// Namespace end
}
}
}


#endif // INC_TT_ENGINE_DEBUG_DEBUGRENDERER_H
