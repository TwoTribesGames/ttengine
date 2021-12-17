#if !defined(INC_TT_ENGINE_RENDERER_TRIANGLEBUFFER_H)
#define INC_TT_ENGINE_RENDERER_TRIANGLEBUFFER_H

#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/PrimitiveCollectionBuffer.h>
#include <tt/engine/renderer/BatchTriangleUV.h>

struct IDirect3DVertexBuffer9;


namespace tt {
namespace engine {
namespace renderer {


// client should create new types of BatchTriangleUV<UVSetCount> when needed
typedef BatchTriangleUV<1> BatchTriangle; // type for backward compatibility

// BatchTriangle collection types for multiple texture coordinate sets (add more if needed)
typedef std::vector<BatchTriangleUV<1> > BatchTriangleCollection;
typedef std::vector<BatchTriangleUV<2> > BatchTriangleCollection2UVSets;
typedef std::vector<BatchTriangleUV<3> > BatchTriangleCollection3UVSets;
typedef std::vector<BatchTriangleUV<4> > BatchTriangleCollection4UVSets;

enum BatchFlagTriangle
{
	BatchFlagTriangle_None,
	BatchFlagTriangle_UseVertexColor
};


class TriangleBuffer : public D3DResource
{
public:
	// At most 21.800 triangles per batch (u16 size)
	static const s32 maxBatchSize = 21800;
	
	/*! \brief Constructor for creating a trianglebuffer
	    \param p_maxTriangleCount Maximum nr of triangles to be stored
	    \param p_uvSize           Number of UV coordinate sets
	    \param p_texture          Texture for this batchcollection
	    \param p_flag             Flags to control the batchcollection */
	TriangleBuffer (s32 p_maxTriangleCount, s32 p_uvSize = 1, const TexturePtr& p_texture = TexturePtr(),
	                BatchFlagTriangle p_flag = BatchFlagTriangle_None);
	
	/*! \brief Destructor that releases vertex data */
	~TriangleBuffer();
	
	/*! \brief Update the trianglebuffer (must be called to make changes to properties take effect) */
	void applyChanges();
	
	/*! \brief Render this trianglebuffer on the screen */
	void render() const;
	
	/*! \brief Set/Get the optional texture to use for this batchcollection 
	    \note  An existing multitexture will be reset */
	void setTexture(const TexturePtr& p_texture);
	inline const TexturePtr& getTexture() const { return m_texture; }
	
	/*! \brief Sets the batchcollection with a templated UVSetCount
	    \param p_collection The templatized collection to copy from
	    \param p_startIndex The start index for the internal buffer */
	template <s32 UVSetCount>
	void setCollection(const std::vector<BatchTriangleUV<UVSetCount> >& p_collection, s32 p_startIndex = 0)
		{ m_triangles.setCollection(p_collection, p_startIndex); }
	
	/*! \brief Clears the batchcollection */
	inline void clear() { m_triangles.clear(); }
	
	/*! \brief Releases device resources (WINDOWS ONLY) */
	virtual void deviceLost();
	
	/*! \brief (Re-)creates device resources (WINDOWS ONLY) */
	virtual void deviceReset();
	
private:
	TriangleBuffer(const TriangleBuffer& p_rhs);
	TriangleBuffer& operator=(const TriangleBuffer& p_rhs);
	
	// Vertex Data
	s32                           m_capacity;
	PrimitiveCollectionBuffer     m_triangles;
	bool                          m_useVtxColor;
	u32                           m_vertexType;
	TexturePtr                    m_texture;
	
	// Win specific
	IDirect3DVertexBuffer9*       m_vertexBuffer;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_RENDERER_TRIANGLEBUFFER_H)
