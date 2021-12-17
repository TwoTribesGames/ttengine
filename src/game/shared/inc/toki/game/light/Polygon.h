#if !defined(INC_TESTS_SHADOW_POLYGON_H)
#define INC_TESTS_SHADOW_POLYGON_H

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>

#include <toki/game/light/fwd.h>
#include <toki/game/light/Circle.h>


namespace toki {
namespace game {
namespace light {


class Polygon
{
public:
	Polygon(const tt::math::Vector2& p_pos, 
	        const tt::engine::renderer::ColorRGBA& p_color,
	        const Vertices& p_vertices,
	        bool p_renderLines);
	
	void update(real p_elapsedTime);
	void render();

#ifndef TT_BUILD_FINAL
	inline void renderDebug() { m_debug.render(); }
#endif
	
	inline const Vertices& getVerticesModelSpace() const { return m_vertices; }
	inline const Vertices& getVerticesWorldSpace() const { return m_verticesWorldSpace; }
	inline const Vertices& getNormals()  const { return m_normals;  }
	
	inline tt::math::Vector2 getBoundingSphereMidPoint() const { return m_boundingSphereMidPoint + m_pos; }
	inline real getBoundingSphereRadius() const                { return m_boundingSphereRadius; }
	
	inline void setPosition(const tt::math::Vector2 p_pos)
	{
		if (m_pos != p_pos)
		{
			m_pos = p_pos;
			calculateVertices();
			m_geometryReady = false;
		}
	}
	
	inline const tt::engine::renderer::ColorRGBA& getColor() const { return m_color; }
	inline void setColor(const tt::engine::renderer::ColorRGBA& p_color)
	{
		if (m_color != p_color)
		{
			m_color = p_color;
			m_geometryReady = false;
		}
	}
	
private:
	void calculateVertices();
	void prepareGeometry();
	
	tt::math::Vector2 m_pos;
	tt::engine::renderer::ColorRGBA m_color;
	Vertices m_vertices;
	Vertices m_verticesWorldSpace;
	Vertices m_normals;
	tt::engine::renderer::TrianglestripBufferPtr m_trianglestripBuffer;
	bool                                         m_geometryReady;
	
	tt::math::Vector2 m_boundingSphereMidPoint;
	real              m_boundingSphereRadius;
	
#ifndef TT_BUILD_FINAL
	Circle m_debug;
#endif
};


// Namespace end
}
}
}


#endif  // !defined(INC_TESTS_SHADOW_POLYGON_H)
