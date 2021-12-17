#if !defined(INC_TT_ENGINE_RENDERER_UPSCALER_H)
#define INC_TT_ENGINE_RENDERER_UPSCALER_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/scene/fwd.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>


namespace tt {
namespace engine {
namespace renderer {


class UpScaler
{
public:
	// Up-scaler will scale up anything larger than max RT size
	UpScaler(const math::Point2& p_maxRenderTargetSize, bool p_invertY = false);
	~UpScaler() { }
	
	math::Point2 handleResolutionChanged(s32 p_newWidth, s32 p_newHeight);
	
	void beginFrame();
	void endFrame();
	
	inline const math::Vector2& getScaleFactor() { return m_scaling; }
	inline const math::Vector2& getOffset()      { return m_offset;  }
	inline bool isActive() const { return m_active; }
	
	inline void setMaxSize(const math::Point2& p_maxRenderTargetSize) { m_maxSize = p_maxRenderTargetSize; }
	inline void setAspectRatioRange(const math::Vector2& p_range)     { m_aspectRatioRange = p_range; }
	
	void setEnabled(bool p_enabled);
	inline bool isEnabled() const { return m_enabled; } 
	
private:
	bool             m_active;
	bool             m_enabled;
	bool             m_invertY;
	math::Point2     m_maxSize;
	math::Vector2    m_aspectRatioRange;
	math::Vector2    m_scaling;
	math::Vector2    m_offset;
	RenderTargetPtr  m_renderTarget;
	QuadSpritePtr    m_fullScreenQuad;
	scene::CameraPtr m_upscaleCamera;
};


// Namespace end
}
}
}


#endif  // INC_TT_ENGINE_RENDERER_UPSCALER_H
