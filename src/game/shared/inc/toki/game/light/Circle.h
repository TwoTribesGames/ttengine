#if !defined(INC_TESTS_SHADOW_CIRCLE_H)
#define INC_TESTS_SHADOW_CIRCLE_H

#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>


namespace toki {
namespace game {
namespace light {

class Circle
{
public:
	Circle(const tt::math::Vector2& p_centerPos, 
	       real p_radius,
	       const tt::engine::renderer::ColorRGB& p_color);
	
	void setCenterPos(const tt::math::Vector2& p_centerPos) { m_centerPos = p_centerPos; m_geometryReady = false; }
	void setRadius(real p_radius)                           { m_radius    = p_radius;    m_geometryReady = false; }
	
	void update(real p_elapsedTime);
	void render();
	
	
private:
	void calculateCircle();
	
	static const s32 segments       = 30;
	static const s32 primitiveCount = segments + 2;
	
	tt::math::Vector2 m_centerPos;
	real m_radius;
	
	tt::engine::renderer::ColorRGB m_color;
	tt::engine::renderer::TrianglestripBufferPtr m_trianglestripBuffer;
	bool m_geometryReady;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TESTS_SHADOW_CIRCLE_H)
