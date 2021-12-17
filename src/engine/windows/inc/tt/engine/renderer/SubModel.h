#if !defined(INC_TT_ENGINE_RENDERER_SUBMODEL_H)
#define INC_TT_ENGINE_RENDERER_SUBMODEL_H

// Need to include <windows.h> for DWORD
#define NOMINMAX
#include <windows.h>
#include <d3d9types.h>

#include <tt/engine/physics/CollisionFace.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/IndexBuffer.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/scene/fwd.h>
#include <tt/fs/types.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>

// Forward declarations
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;

// namespace definition
namespace tt {
namespace engine {
namespace renderer {


class SubModel : public D3DResource
{
public:
	SubModel();
	SubModel(const SubModel& p_model);
	~SubModel();
	
	bool load(const fs::FilePtr& p_file, scene::Model* p_model);
	void render(RenderContext& p_renderContext);
	void renderNormals();

	// Trigger re-creation of buffers
	bool updateBuffers();
	
	inline MaterialPtr getMaterial() const {return m_material;}
	inline void        setMaterial(const MaterialPtr& p_material) {m_material = p_material;}
	
	inline s32 getTriangleCount() const {return m_triangleCount;}
	inline VertexBuffer& getVertexBuffer() {return m_vertices;}

	//////////////////////////////////
	// Windows specific

	virtual void deviceCreated();
	virtual void deviceDestroyed();
	
private:
	friend class scene::Model;
	void setOwner(scene::Model* p_model) {m_model = p_model;}

private:
	MaterialPtr m_material;
	scene::Model*  m_model;

	// Vertices
	VertexBuffer m_vertices;
	IndexBuffer  m_indices;
	s32 m_triangleCount;

	// DirectX
	IDirect3DVertexBuffer9* m_vertexBuffer;
	IDirect3DIndexBuffer9*  m_indexBuffer;
};

// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_SUBMODEL_H
