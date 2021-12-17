#if !defined(INC_TT_ENGINE_RENDERER_RENDERER_H)
#define INC_TT_ENGINE_RENDERER_RENDERER_H


#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <map>

#include <windows.h>
#include <d3d9types.h>
#include <d3dx9core.h>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>
#include <tt/math/Rect.h>
#include <tt/engine/scene/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/particles/fwd.h>
#include <tt/engine/scene2D/fwd.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/LightManager.h>
#include <tt/engine/renderer/pp/fwd.h>
#include <tt/pres/fwd.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace engine {
namespace renderer {


class TextureStageData;


class Renderer
{
public:
	// Instance management
	static bool createInstance(bool p_ios2xMode = false);
	inline static Renderer* getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	inline static bool hasInstance() {return ms_instance != 0;}
	static void destroyInstance();
	
	// Reinitialize & restore defaults
	void reset(bool p_disableZ = false);
	void setDefaultStates(bool p_disableZ = false);
	
	void setColorMask(u32 p_colorMask);
	u32  getColorMask() const { return m_colorMask; }
	
	/*! \brief Updates the engine by a given amount of time.
	    \param p_elapsedTime The number of seconds to update the engine. */
	void update(real p_elapsedTime);
	
	// Rendering
	bool beginFrame();
	bool endFrame();
	bool present();
	
	// HUD Rendering
	void beginHud();
	void endHud();
	inline bool isRenderingHud() { return m_isRenderingHud; }
	
	/*! \brief Change the clear color */
	void setClearColor(const ColorRGBA& p_color);
	
	/*! \brief Returns the clear color */
	inline const ColorRGBA& getClearColor() const { return m_clearColor; }
	
	/*! \brief Change the active vertex format */
	void setVertexType(u32 p_vertexType);
	void resetVertexType() {m_vertexType = 0;}
	
	/*! \brief Change the current blend mode */
	void setBlendMode(BlendMode p_mode = BlendMode_Blend);
	void setCustomBlendMode(BlendFactor p_src, BlendFactor p_dst);
	
	void setBlendModeAlpha(BlendModeAlpha p_mode = BlendModeAlpha_NoOverride);
	void setCustomBlendModeAlpha(BlendFactor p_srcAlpha, BlendFactor p_dstAlpha);
	void resetCustomBlendModeAlpha();
	inline bool hasSeparateAlphaBlendEnabled() const { return m_separateAlphaBlendEnabled; }
	inline BlendFactor getCustomBlendModeAlphaSrc() const { return m_srcFactorAlpha; }
	inline BlendFactor getCustomBlendModeAlphaDst() const { return m_dstFactorAlpha; }
	
	void setPremultipliedAlphaEnabled(bool p_enabled);
	
	
	//----------- ALPHA TEST -------------
	/*! \brief Enable/disable alpha testing */
	void setAlphaTestEnabled(bool p_enableAlphaTest);
	inline bool isAlphaTestEnabled() const { return m_alphaTestEnabled; }
	
	/*! \brief Set/get the alpha test function */
	void setAlphaTestFunction(AlphaTestFunction p_alphaTestFunction);
	inline AlphaTestFunction getAlphaTestFunction() const { return m_alphaTestFunction; }
	
	/*! \brief Set/get the alpha test value */
	void setAlphaTestValue(u8 p_alphaTestValue);
	inline u8 getAlphaTestValue() const { return m_alphaTestValue; }
	
	/*! \brief Set the alpha test function and value */
	void setAlphaTest(AlphaTestFunction p_alphaTestFunction, u8 p_alphaTestValue);
	//----------- ALPHA TEST end ---------
	
	// Stencil Buffer Control
	void clearStencil     (s32  p_value);
	void setStencilEnabled(bool p_enabled);
	bool isStencilEnabled() const;
	void setStencilFunction (StencilSide p_side, StencilTestFunction p_function, s32 p_reference, s32 p_mask);
	void setStencilOperation(StencilSide p_side, StencilOperation p_stencilFail,
	                         StencilOperation p_zFail, StencilOperation p_stencilPass);
	
	/*! \brief Set the texture to be used for rendering. */
	void setTexture(const TexturePtr& p_texture);
	void resetMultiTexture();
	
	// Camera Management
	scene::CameraPtr setMainCamera(const scene::CameraPtr& p_camera, bool p_update = true, ViewPortID p_viewport = ViewPortID_1);
	scene::CameraPtr getMainCamera(ViewPortID p_viewport = ViewPortID_1) const;
	scene::CameraPtr getHudCamera() const {return m_hudCamera;}
	
	ViewPortID getActiveViewPort() const {return m_currentViewport;}
	
	LightManager& getLightManager() {return m_lightManager;}
	
	s32 getScreenWidth() const; //!< Retrieve current screen width
	s32 getScreenHeight() const; //!< Retrieve current screen height
	inline math::Point2 getBackbufferSize() const { return m_backBufferSize; }
	
	//----------- CULLING -------------
	/*! \brief Set/get the vertex drawing order of a front facing face */
	CullFrontOrder setCullFrontOrder(CullFrontOrder p_cullFrontOrder);
	inline CullFrontOrder getCullFrontOrder() const { return m_cullFrontOrder; }
	
	/*! \brief Set/get the culling mode */
	CullMode setCullMode(CullMode p_cullMode);
	inline CullMode getCullMode() const { return m_cullMode; }
	
	/*! \brief Switches between the last cull mode and no culling */
	void setCullingEnabled(bool p_cullingEnabled);
	inline bool isCullingEnabled() const { return m_cullingEnabled; }
	
	/*! \brief Switches between no backface culling and the default cull mode 
	    \note  DEPRECATED use setCullingEnabled / isCullingEnabled */
	inline void setBackFaceCull(bool p_cull) { setCullingEnabled(p_cull); }
	inline bool getBackFaceCull() const { return m_cullingEnabled; }
	#pragma deprecated(setBackFaceCull, getBackFaceCull)
	//----------- CULLING end ---------

	void setFillMode(FillMode p_fillMode);
	
	// Status
	inline void setDeltaTime(real p_delta) {m_deltaTime = p_delta;}
	inline real getDeltaTime()  const {return m_deltaTime;}
	inline real getTime()       const {return m_time;     }
	
	inline bool isRendering() const {return m_isRendering;}
	
	inline bool isModePAL()    const { return false; }
	bool isWideScreen() const;
	
	// Viewports
	void beginViewPort(ViewPort& p_viewport, bool p_last);
	void endViewPort(ViewPort& p_viewport);
	
	// Lighting
	void setLighting(bool p_enable);
	inline bool isLightingEnabled() const { return m_lightingEnabled; }
	
	// Fog
	void setFog(const scene::FogPtr& p_fog);
	
	void setFogSetting(FogSetting p_setting, real p_value);
	void setFogColor(const ColorRGBA& p_color);
	void setFogMode(FogMode p_mode);
	void setFogEnabled(bool p_enable);
	inline bool isFogEnabled() const { return m_fogEnabled; }
	
	// Anti-Aliasing
	s32  setAntiAliasing(bool p_enable, s32 p_samples = 4);
	inline bool isAAEnabled()  const { return m_useAA;     }
	inline s32  getAASamples() const { return m_samplesAA; }
	
	void setZBufferEnabled(bool p_enabled);
	void setDepthWriteEnabled(bool p_enabled);
	inline bool isZBufferEnabled()    const { return m_isZBufferEnabled; }
	inline bool isDepthWriteEnabled() const { return m_isZWriteEnabled;  }
	inline void setTransparentsWriteZ(bool) {} // Needs implementation
	
	// Transparency support
	void addTransparentObject(real p_distance, scene::SceneObject* p_object, const math::Matrix44& p_matrix);
	void renderTransparents();
	inline bool isRenderingTransparents() const {return m_isRenderingTransparents;}
	
	// Skybox
	inline void setSkybox(const scene::InstancePtr& p_model) {m_skybox = p_model;}
	
	// Info for debugging purposes
	inline const debug::DebugRendererPtr& getDebug() const {return m_debugRenderer;}
	
	// Post-Processing
	inline const pp::PostProcessorPtr& getPP() const {return m_postProcessor;}
	
	// Up-Scaling
	const math::Vector2& getUpScaling() const;
	inline const UpScalerPtr& getUpScaler() const { TT_NULL_ASSERT(m_upScaler); return m_upScaler; }
	inline       UpScalerPtr& getUpScaler()       { TT_NULL_ASSERT(m_upScaler); return m_upScaler; }
	
	inline bool isLowPerformanceMode() const {return m_lowPerformanceMode;}
	inline void setLowPerformanceMode(bool p_enable) {m_lowPerformanceMode = p_enable;}
	
	void setScissorRect(const tt::math::PointRect& p_rect);
	void resetScissorRect();
	
	/*! /Brief Custom viewport setting. Should only be used in special cases.
	           This is done directly in this call, overwriting the beginFrame / beginHUD viewport settings,
	           be sure to restore these viewport settings if there are dependencies on them. */
	void setCustomViewport(s32 p_x, s32 p_y, s32 p_width, s32 p_height, real p_near = -1, real p_far = 1);

	void saveBackbuffer();
	void restoreBackbuffer();

	// Device Management (Windows Only)
	void handleCreateDevice();
	void handleLostDevice();
	bool handleResetDevice();
	void handleDestroyDevice();
	
	// Double-resolution support
	// TODO: Create generic EmulationMode
	inline bool isIOS2xMode() const { return m_ios2xMode; }

	void checkFromRenderThread() const;
	
	u32 getFVFFromVertexType(u32 p_type);

	static void addToDeathRow(IUnknown* p_comObject);
	static void clearDeathRow();
	
private:
	// Constructor -- only create through createInstance() function
	Renderer(bool p_ios2xMode);
	~Renderer();
	
	// No copying
	Renderer(const Renderer&);
	const Renderer& operator=(const Renderer&);
	
	void createDefaultCameras();
	
	void renderSkybox();
	
	// For internal engine use only
	friend class scene::SceneObject;
	friend class scene::Model;
	inline const scene::CameraPtr& getActiveCamera() const { return m_activeCamera; }
	
	// Review all culling parameters and update
	void updateCulling() const;
	void updateBlendModes();
	
	void resetFog();
	
	//--- MULTITEXTURE CONTROL ---
	/*! \brief  Set the texturestage
	    \param  p_resetting This param informs the function to ignore some requirements and costly operations
	    \return Whether the stage is set correctly */
	bool setTextureStage(u32 p_index, const TextureStageData& p_textureStageData, bool p_resetting = false);
	
	/*! \brief Reset the texturestage to its default values 
	    \param p_indexStart  Zero based index of the stage to start resetting from
	    \param p_indexEnd    Zero based index of the stage to stop resetting (non-inclusive) */
	void resetTextureStages(u32 p_indexStart, u32 p_indexEnd);
	inline void resetTextureStage(u32 p_indexStart) { resetTextureStages(p_indexStart, p_indexStart + 1); }
	//--- MULTITEXTURE CONTROL end ---
	
	// Unique renderer instance
	static Renderer* ms_instance;
	
	// Render state
	bool m_isRendering;
	bool m_isRenderingHud;
	
	bool m_useAA;
	bool m_lightingEnabled;
	
	// Fog settings
	bool m_fogEnabled;
	FogMode m_fogMode;
	ColorRGBA m_fogColor;
	float m_fogSettings[FogSetting_Count];
	
	bool m_isZBufferEnabled;
	bool m_isZWriteEnabled;
	real m_deltaTime;
	real m_time;
	u8   m_frame;
	s32  m_samplesAA;
	
	BlendFactor      m_srcFactor;
	BlendFactor      m_dstFactor;
	bool             m_separateAlphaBlendEnabled;
	BlendFactor      m_srcFactorAlpha;
	BlendFactor      m_dstFactorAlpha;
	bool             m_premultipliedAlpha;
	
	bool               m_alphaTestEnabled;
	AlphaTestFunction  m_alphaTestFunction;
	u8                 m_alphaTestValue;
	
	CullFrontOrder     m_cullFrontOrder;
	CullMode           m_cullMode;
	bool               m_cullingEnabled;

	FillMode           m_fillMode;
	u32                m_colorMask;
	
	scene::CameraPtr m_activeCamera;
	scene::CameraPtr m_hudCamera;
	
	// Skybox
	scene::InstancePtr m_skybox;
	
	bool m_lastViewPort;
	ViewPortID m_currentViewport;
	
	LightManager m_lightManager;
	
	u32 m_vertexType;
	
	ColorRGBA m_clearColor;
	
	// Transparancy
	typedef std::pair<scene::SceneObject*, math::Matrix44> RenderObject;
	typedef std::multimap<real, RenderObject> RenderObjects;
	RenderObjects m_transparentObjects;
	bool m_isRenderingTransparents;
	
	// Debug Functionality
	debug::DebugRendererPtr m_debugRenderer;
	
	// Post-Processing
	pp::PostProcessorPtr m_postProcessor;
	bool                 m_postProcessingActive;
	
	// Up-scaling
	math::Point2 m_screenSize;
	math::Point2 m_backBufferSize;
	UpScalerPtr  m_upScaler;
	bool         m_upScalerActive;
	
	// Performance
	bool m_lowPerformanceMode;
	
	// Windows Specific Render Stuff
	IDirect3DDevice9* m_device;
	D3DCOLOR m_d3dClearColor;
	
	D3DVIEWPORT9 m_backBufferViewport;

	IDirect3DSurface9*  m_backBuffer;
	IDirect3DSurface9*  m_backBufferDepth;
	
	const DWORD m_renderThreadID;
	typedef std::vector<IUnknown*> ComDeathRow;
	static ComDeathRow   ms_deathRow;
	static thread::Mutex ms_deathRowMutex;
	
	typedef std::map<u32, u32> VertexAttributeMap;
	VertexAttributeMap m_vertexAttributeMap;
	
	/*! \brief History of MultiTexture stages modified, updated in setMultiTexture(), reset in resetMultiTexture()
	           if stages are modifed, the range ranges from 1 to Multitexture::Constants_maxStages 
	    \note  This will move to MultiTexture when all state changes move to MultiTexture */
	u32 m_multiTextureDirtyStageRange;
	
	// Whether running in double-resolution mode
	const bool m_ios2xMode;
	
	// ParticleTrigger needs to access getActiveCamera for culling
	friend class particles::ParticleTrigger;
	friend class scene2d::PlaneScene;
	friend class pres::FrameAnimation;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_RENDERER_H)
