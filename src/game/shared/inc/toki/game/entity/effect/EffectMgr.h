#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_EFFECTMGR_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_EFFECTMGR_H

#include <tt/cfg/Handle.h>
#include <tt/code/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/math/Vector2.h>
#include <tt/platform/tt_types.h>


#include <toki/game/entity/effect/fwd.h>
#include <toki/game/entity/effect/types.h>
#include <toki/game/entity/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

class EffectMgr
{
public:
	EffectMgr();
	
	void update(real p_elapsedTime);
	tt::engine::renderer::ColorRGB getFogColor(const tt::engine::renderer::ColorRGB& p_defaultColor) const;
	real getFogNear(real p_defaultNear) const;
	real getFogFar( real p_defaultFar) const;
	inline const tt::math::Vector2& getCameraOffset()    const { return m_cameraOffset;    }
	inline const tt::math::Vector2& getDrcCameraOffset() const { return m_drcCameraOffset; }
	tt::math::Vector2 getCameraPosition(   const tt::math::Vector2 p_position) const;
	tt::math::Vector2 getDrcCameraPosition(const tt::math::Vector2 p_position) const;
	real getLightAmbient(real p_defaultAmbient) const;
	real getCameraFov(   real p_defaultFov) const;
	real getDrcCameraFov(real p_defaultFov) const;
	
	void reset();
	
	void addFogColor(const EffectRectHandle&               p_handle,
	                 real                                  p_baseStrength,
	                 const tt::engine::renderer::ColorRGB& p_color);
	void addFogNearFar(const EffectRectHandle& p_handle, real p_baseStrength, real p_near, real p_far);
	void addCameraOffset(   const EffectRectHandle& p_handle, real p_baseStrength,
	                        const tt::math::Vector2& p_offset);
	void addDrcCameraOffset(const EffectRectHandle& p_handle, real p_baseStrength,
	                        const tt::math::Vector2& p_offset);
	void addCameraPosition(   const EffectRectHandle& p_handle, real p_baseStrength,
	                          const tt::math::Vector2& p_position);
	void addDrcCameraPosition(const EffectRectHandle& p_handle, real p_baseStrength,
	                          const tt::math::Vector2& p_position);
	void addLightAmbient(const EffectRectHandle& p_handle, real p_baseStrength, real p_ambient);
	void addCameraFov(   const EffectRectHandle& p_handle, real p_baseStrength, real p_fov);
	void addDrcCameraFov(const EffectRectHandle& p_handle, real p_baseStrength, real p_fov);
	void addColorGrading(const EntityHandle& p_entityForErrorReporting, const EffectRectHandle& p_handle,
	                     real p_baseStrength, const tt::engine::renderer::TexturePtr& p_texture);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	class EffectBase
	{
	public:
		EffectRectHandle rectHandle;
		real             baseStrength;
		
	protected:
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
		EffectBase(const EffectRectHandle& p_rectHandle, real p_baseStrength)
		:
		rectHandle(p_rectHandle),
		baseStrength(p_baseStrength)
		{
			tt::math::clamp(baseStrength, 0.0f, 1.0f);
		}
		EffectBase() : rectHandle(), baseStrength(1.0f) {}
	};
	
	// Fog Color
	class EffectFogColor : public EffectBase
	{
	public:
		EffectFogColor(const EffectRectHandle&               p_rectHandle,
		               real                                  p_baseStrength,
		               const tt::engine::renderer::ColorRGB& p_color)
		: EffectBase(p_rectHandle, p_baseStrength), color(p_color) { }
		EffectFogColor() : EffectBase(), color() { }
		
		tt::engine::renderer::ColorRGB color;
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<EffectFogColor> RectWithFogColors;
	RectWithFogColors              m_fogColors;
	tt::engine::renderer::ColorRGB m_fogColor;
	real                           m_fogColorStrength;
	
	// Fog Near/Far
	class EffectFogNearFar : public EffectBase
	{
	public:
		EffectFogNearFar(const EffectRectHandle& p_rectHandle, real p_baseStrength,
		                 real p_near, real p_far)
		: EffectBase(p_rectHandle, p_baseStrength), fogNear(p_near), fogFar(p_far) { }
		EffectFogNearFar() : EffectBase(), fogNear(0.0f), fogFar(0.0f) { }
		
		real fogNear;
		real fogFar;
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<EffectFogNearFar> RectWithFogNearFars;
	RectWithFogNearFars m_fogNearFars;
	real                m_fogNear;
	real                m_fogFar;
	real                m_fogNearFarStrength;
	
	// Vector 2 effect
	class EffectVector2 : public EffectBase
	{
	public:
		EffectVector2(const EffectRectHandle& p_rectHandle, real p_baseStrength,
		                   const tt::math::Vector2& p_value)
		: EffectBase(p_rectHandle, p_baseStrength), value(p_value) { }
		EffectVector2() : EffectBase(), value(tt::math::Vector2::zero) { }
		
		tt::math::Vector2 value;
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<EffectVector2> RectWithEffectVector2;
	static tt::math::Vector2 getEffectValue(RectWithEffectVector2&   p_collection,
	                                        real&                    p_effectStrength_OUT,
	                                        const tt::math::Vector2& p_value = tt::math::Vector2::zero);
	static tt::math::Vector2 mixEffect(const tt::math::Vector2& p_defaultValue,
	                                   const tt::math::Vector2& p_effectValue,
	                                   real                     p_effectStrength);
	
	// (Drc)Camera offset
	RectWithEffectVector2 m_cameraOffsets;
	tt::math::Vector2     m_cameraOffset;
	RectWithEffectVector2 m_drcCameraOffsets;
	tt::math::Vector2     m_drcCameraOffset;
	
	// (Drc)Camera position
	RectWithEffectVector2 m_cameraPositions;
	tt::math::Vector2     m_cameraPosition;
	real                  m_cameraPositionStrength;
	real                  m_cameraPositionStrengthTarget;
	RectWithEffectVector2 m_drcCameraPositions;
	tt::math::Vector2     m_drcCameraPosition;
	real                  m_drcCameraPositionStrength;
	real                  m_drcCameraPositionStrengthTarget;
	
	tt::cfg::HandleReal   m_camPosDecayHandle; // Don't serialize this.
	
	// Effects based on a real value.
	class EffectReal : public EffectBase
	{
	public:
		EffectReal(const EffectRectHandle& p_rectHandle, real p_baseStrength, real p_value)
		: EffectBase(p_rectHandle, p_baseStrength), value(p_value) { }
		EffectReal() : EffectBase(), value(0.0f) { }
		
		real value;
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<EffectReal> RectWithEffectReal;
	static void getEffectValues(RectWithEffectReal& p_rects, real& p_effect, real& p_effectStrength);
	static real mixEffect(real p_defaultValue, real p_effectValue, real p_effectStrength);
	
	// Light ambient
	RectWithEffectReal m_lightAmbients;
	real               m_lightAmbient;
	real               m_lightAmbientStrength;
	
	// (Drc)Camera FOV
	RectWithEffectReal m_cameraFoVAmbients;
	real               m_cameraFoVAmbient;
	real               m_cameraFoVAmbientStrength;
	RectWithEffectReal m_drcCameraFoVAmbients;
	real               m_drcCameraFoVAmbient;
	real               m_drcCameraFoVAmbientStrength;
	
	// Color Grading
	class EffectColorGrading : public EffectBase
	{
	public:
		EffectColorGrading(const EffectRectHandle& p_rectHandle, real p_baseStrength,
		                   const tt::engine::renderer::TexturePtr& p_texture);
		EffectColorGrading() : EffectBase(), colorGradingIndex(-1) { }
		
		s32 colorGradingIndex;
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
	};
	typedef std::vector<EffectColorGrading> RectWithEffectColorGrading;
	RectWithEffectColorGrading m_colorGradings;
	
	// Script
	// ...
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_EFFECTMGR_H)
