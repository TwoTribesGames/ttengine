#if !defined(INC_TOKI_GAME_CAMERAMGR_H)
#define INC_TOKI_GAME_CAMERAMGR_H


#include <tt/code/fwd.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>

#include <toki/game/Camera.h>


namespace toki {
namespace game {

class CameraMgr
{
public:
	struct ViewPortState
	{
		bool                        renderingToDRC;
		bool                        renderingMainCam;
		bool                        renderHudOnly;
		const tt::math::VectorRect* visibilityRect;
		
		
		inline ViewPortState()
		:
		renderingToDRC(false),
		renderingMainCam(false),
		renderHudOnly(false),
		visibilityRect(0)
		{ }
	};
	
	
	//! \param p_configKeyBase The config.xml namespace to use for the default camera settings
	explicit CameraMgr(const std::string& p_configKeyBase = "toki.camera.game");
	
	void update(real                     p_deltaTime,
	            const tt::math::Vector2& p_scrollOffset                = tt::math::Vector2::zero,
	            const tt::math::Vector2& p_effectOffsetTV              = tt::math::Vector2::zero,
	            const tt::math::Vector2& p_effectOffsetDRC             = tt::math::Vector2::zero,
	            bool                     p_applyEffectMgrEffects       = false);
	
	// NOTE: This returns whether both cameras are HUD-only
	bool updateForRender();  // FIXME: All it does for now is calculate visibility rect... rename function?
	
	// Camera state for current viewport (depends on currently rendering viewport; onRenderBegin must have been called first):
	void onRenderBegin();
	inline const ViewPortState& getCurrentViewportState() const { return m_currentViewportState; }
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	inline Camera& getCamera()                         { return m_camera;                }
	inline bool    isDrcCameraEnabled() const          { return m_drcCameraEnabled;      }
	inline void    setDrcCameraEnabled(bool p_enabled)
	{
		if (m_drcCameraEnabled == false && p_enabled)
		{
			m_drcCamera.clear();
		}
		m_drcCameraEnabled = p_enabled;
	}
	inline bool    isDrcCameraMain() const             { return m_drcCamIsMain;          }
	inline void    setDrcCameraAsMain(bool p_enable)   { m_drcCamIsMain = p_enable;      }
	inline Camera& getDrcCamera()                      { return m_drcCamera;             }
	
	inline       Camera& getInputCamera()       { return m_emulateDRC ? m_drcCamera : m_camera; }
	inline const Camera& getInputCamera() const { return m_emulateDRC ? m_drcCamera : m_camera; }
	
	inline bool isEmulatingDRC() const        { return m_emulateDRC;                    }
	inline void setEmulateDRC(bool p_emulate) { m_emulateDRC = p_emulate;               }
	inline void toggleEmulateDRC()            { m_emulateDRC = (m_emulateDRC == false); }
	
	inline const tt::math::VectorRect& getCombinedVisibilityRect() const { return m_combinedVisibilityRect; }
	
private:
	Camera m_camera;
	bool   m_drcCameraEnabled;
	bool   m_drcCamIsMain; // Which of the two cameras is the main camera and which is the sub camera.
	Camera m_drcCamera;
	bool   m_emulateDRC; // If true the DRC camera is shown instead of (TV) camera.
	tt::math::VectorRect m_combinedVisibilityRect;  // combined visibility rects of both cameras
	ViewPortState        m_currentViewportState;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_CAMERAMGR_H)
