#if !defined(INC_TT_ENGINE_RENDERER_QUADBUFFER_H)
#define INC_TT_ENGINE_RENDERER_QUADBUFFER_H

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/BufferVtx.h>
#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace engine {
namespace renderer {


struct BatchQuad
{
	BufferVtx bottomLeft;
	BufferVtx topLeft;
	BufferVtx bottomRight;
	BufferVtx topRight;
};
typedef std::vector<BatchQuad> BatchQuadCollection;

enum BatchFlagQuad
{
	BatchFlagQuad_None,
	BatchFlagQuad_UseVertexColor,
	BatchFlagQuad_EmbedGpuBuffer,
};


class QuadBuffer
{
public:
	// At most 10.900 quads per batch (u16 index buffer limit)
	static const s32 maxBatchSize = 50000;
	
	
	/*! \brief Constructor for creating a quad buffer.
	    \param p_size     Maximum nr of quads to be stored.
	    \param p_hasColor Whether to use vertex color or not. */
	QuadBuffer(s32 p_size, const TexturePtr& p_texture = TexturePtr(), BatchFlagQuad p_flag = BatchFlagQuad_None);
	
	/*! \brief Destructor that releases vertex data */
	~QuadBuffer();
	
	/*! \brief Update the quad (must be called to make changes to properties take effect) */
	void applyChanges();
	
	/*! \brief Render this quad on the screen */
	void render() const;
	
	/*! \brief Set/Get the texture to use for this batch */
	inline void setTexture(const TexturePtr& p_texture) {m_texture = p_texture;}
	inline const TexturePtr& getTexture() const { return m_texture; }
	
	/*! \brief Sets the batchcollection 
		\param p_startIndex     the start index for the internal buffer */
	void setCollection(const BatchQuadCollection& p_collection, s32 p_startIndex = 0);
	
	/* Directly fill the quad buffer with the provided quads */
	void fillBuffer(const BatchQuadCollection::iterator& p_begin, const BatchQuadCollection::iterator& p_end);

	/*! \brief clears the quad buffer and depending counters */
	void clear();
	
	inline s32 getCapacity() const { return m_capacity; }
	
private:
	// No copying
	QuadBuffer(const QuadBuffer& p_rhs);
	QuadBuffer& operator=(const QuadBuffer& p_rhs);
	
	void copyToInternalBuffers(
		const BatchQuadCollection::const_iterator& p_begin,
		const BatchQuadCollection::const_iterator& p_end,
		s32 p_startIndex);
	
	// Vertex Data
	bool m_useVtxColor;
	
	s32 m_capacity;
	s32 m_quadCount;
	
	TexturePtr m_texture;
	
	// Dynamically allocated arrays of size (m_capacity * VerticesPerQuad)
	math::Vector3* m_vertexCoordinates;
	math::Vector2* m_textureCoordinates;
	ColorRGBA*     m_colors;
	
	enum { IndicesPerQuad = 6 };
	/*! \brief Index buffer for all possible quads. Shared between all QuadBuffers. */
	static u32  ms_indexBuffer[maxBatchSize * IndicesPerQuad];
	static bool ms_indexBufferInitialized;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_RENDERER_QUADBUFFER_H)
