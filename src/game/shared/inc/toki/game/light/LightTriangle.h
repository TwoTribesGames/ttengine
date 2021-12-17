#if !defined(INC_TESTS_SHADOW_LIGHTTRIANGLE_H)
#define INC_TESTS_SHADOW_LIGHTTRIANGLE_H

#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>


#include <toki/game/light/fwd.h>


namespace toki {
namespace game {
namespace light {


class LightTriangle
{
public:
	LightTriangle(const LightShape& p_parent,
				  const tt::math::Vector2& p_left,
				  const tt::math::Vector2& p_right);
	
	void update(real p_elapsedTime, const Polygons& p_occluders);
	
	//inline void renderDebug() { m_debug.render(); }
	void render();
	
	
private:
	void calculateCircle();
	
	//--------------------
	//                   |
	//  left       right |
	//     *         *   |
	//      \       /    |
	//       \     /     |
	//        \   /      |
	//         \ /       |
	// centerPos *       |
	//                   |
	//--------------------
	
	const LightShape&      m_parent;
	tt::math::Vector2 m_left;      // The left side of the triangle
	tt::math::Vector2 m_right;     // The right side of the triangle.
	
	tt::engine::renderer::TrianglestripBufferPtr m_trianglestripBuffer;

	LightTriangle(const LightTriangle& p_rhs); // No copy
	const LightTriangle& operator=(const LightTriangle& p_rhs); // No assigment.
};


// Namespace end
}
}
}


#endif  // !defined(INC_TESTS_SHADOW_LIGHTTRIANGLE_H)
