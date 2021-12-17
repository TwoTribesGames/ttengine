#if !defined(INC_TESTS_SHADOW_LIGHT_H)
#define INC_TESTS_SHADOW_LIGHT_H

#include <algorithm>
#include <vector>
#include <list>

#include <tt/code/fwd.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>

#include <toki/game/light/fwd.h>
#include <toki/game/light/Circle.h>


namespace toki {
namespace game {
namespace light {

#if !defined(TT_BUILD_FINAL)
#define USE_DEBUG_SHAPE
#endif

class LightShape
{
public:
	LightShape(const tt::math::Vector2& p_centerPos, 
	      real p_radius,
	      const tt::engine::renderer::ColorRGB& p_color);
	~LightShape();
	
	inline void setCenterPos(const tt::math::Vector2& p_centerPos)
	{
		m_centerPos = p_centerPos;
#ifdef USE_DEBUG_SHAPE
		m_debug.setCenterPos(p_centerPos);
#endif
	}
	inline const tt::math::Vector2& getCenterPos() const { return m_centerPos; }
	
	inline void setRadius(real p_radius)
	{
		m_radius = p_radius;
#ifdef USE_DEBUG_SHAPE
		m_debug.setRadius(p_radius);
#endif
	}
	inline void setDirection(real p_angle) { TT_ASSERT(p_angle >= 0.0f && p_angle <= tt::math::twoPi); m_direction = p_angle; }
	inline real getDirection() const { return m_direction; }
	inline void setSpread(real p_spread)   { m_halfSpread = p_spread * 0.5f;   }
	inline real getSpread() const { return m_halfSpread * 2.0f; }
	
	inline real getRadius() const { return m_radius; }
	inline const tt::engine::renderer::ColorRGB& getColor() const { return m_color; }
	inline void setColor(const tt::engine::renderer::ColorRGB& p_color) { m_color = p_color; }
	void setTexture(const std::string& p_textureName);
	const std::string& getTextureName() const { return m_textureOverride; }
	
	void update(real p_elapsedTime, const Polygons& p_occluders, u8 p_centerAlpha);
	
	inline void renderDebug()
	{
#ifdef USE_DEBUG_SHAPE
		m_debug.render();
#endif
	}
	void render() const;
	
	bool inSpread(const tt::math::Vector2& p_centerPos, const tt::math::Vector2& p_otherPos) const;
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	void calculateCircle(const Shadows& p_shadows, u8 p_centerAlpha);
	Shadows::iterator insertRightAngleSorted(Shadows& p_list, const Shadow& p_shadow);
	Shadows::iterator insertDistanceSorted(Shadows& p_list, const Shadow& p_shadow);
	
	static const s32 segments = 30;
	
	tt::math::Vector2 m_centerPos;
	real              m_radius;
	real              m_direction;
	real              m_halfSpread;
	
	tt::engine::renderer::ColorRGB m_color;
	tt::engine::renderer::TrianglestripBufferPtr m_trianglestripBuffer;
	std::string                                  m_textureOverride; // It's possible to override which texture the light shape should use.
	Vertices m_rays;
	
	// For optimization purposes; should not be serialized
	Shadows m_shadows;
	Shadows m_distanceSort;
	Shadows m_toRender;
	Shadows m_toAddToRenderOutSideLoop;
	Shadows m_shadowsScratch;
	BoolVector m_backFacingScratch;
	
#ifdef USE_DEBUG_SHAPE
	Circle m_debug;
#endif
};


// Namespace end
}
}
}


#endif  // !defined(INC_TESTS_SHADOW_LIGHT_H)
