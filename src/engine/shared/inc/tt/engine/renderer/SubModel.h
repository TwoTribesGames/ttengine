#if !defined(INC_TT_ENGINE_RENDERER_SUBMODEL_H)
#define INC_TT_ENGINE_RENDERER_SUBMODEL_H


#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/IndexBuffer.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/scene/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {


class SubModel
{
public:
	SubModel();
	SubModel(const SubModel& p_model);
	~SubModel();
	
	SubModel& operator=(const SubModel& p_rhs);
	
	bool load(const fs::FilePtr& p_file, scene::Model* p_model);
	void render(RenderContext& p_renderContext);
	void renderNormals();
	
	// Trigger re-creation of buffers
	bool updateBuffers();
	
	inline const MaterialPtr& getMaterial() const          { return m_material;       }
	inline void setMaterial(const MaterialPtr& p_material) { m_material = p_material; }
	
	inline s32 getTriangleCount() const    { return m_triangleCount; }
	inline VertexBuffer& getVertexBuffer() { return m_vertices;      }
	
private:
	void destroyBuffers();
	inline void setOwner(scene::Model* p_model) { m_model = p_model; }
	
	
	MaterialPtr   m_material;
	scene::Model* m_model;
	
	// Vertices
	VertexBuffer m_vertices;
	IndexBuffer  m_indices;
	s32          m_triangleCount;
	
	// OpenGL Buffers
	static const u32 maxBuffers = 8; // Max nr of vertex properties + 1
	GLuint m_buffers[maxBuffers];
	
	friend class scene::Model;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_SUBMODEL_H)
