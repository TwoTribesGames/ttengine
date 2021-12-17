#if !defined(INC_TT_ENGINE_RENDERER_QUADBUFFER_H)
#define INC_TT_ENGINE_RENDERER_QUADBUFFER_H

#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/BufferVtx.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/fwd.h>


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


class QuadBuffer : public D3DResource
{
public:
	// Increased limit, now using 32-bit indices
	// Max batch size possible is now: 715827882
	static const s32 maxBatchSize = 500000; // (Around 11 MB)
	
	/*! \brief Constructor for creating a quad buffer (uses a shared index buffer for rendering)
		\param p_maxQuadCount Maximum nr of quads to be stored
		\param p_texture      Texture for this batchcollection
		\param p_flag         Flags to control the batchcollection
	*/
	QuadBuffer (s32 p_maxQuadCount, const TexturePtr& p_texture = TexturePtr(),
	            BatchFlagQuad p_flag = BatchFlagQuad_None);
	
	/*! \brief Destructor that releases vertex data */
	~QuadBuffer();
	
	/*! \brief Update the quad buffer (must be called to make changes to properties take effect) */
	void applyChanges();
	
	/*! \brief Render this quad buffer on the screen (with an indexed buffer)*/
	void render() const;
	
	/*! \brief Set/Get the texture to use for this batch */
	inline void setTexture(const TexturePtr& p_texture) { m_texture = p_texture; }
	inline const TexturePtr& getTexture() const { return m_texture; }
	
	/*! \brief Sets the batchcollection 
	    \param p_startIndex     the start index for the internal buffer */
	void setCollection(const BatchQuadCollection& p_collection, s32 p_startIndex = 0);
	
	/* Directly fill the quad buffer with the provided quads */
	void fillBuffer(const BatchQuadCollection::iterator& p_begin, const BatchQuadCollection::iterator& p_end);
	
	/*! \brief Clears the batchcollection */
	inline void clear() { m_quads.clear(); }
	
	inline s32 getCapacity() const { return m_capacity; }
	
	/*! \brief Creates device resources (WINDOWS ONLY) */
	virtual void deviceCreated();
	
	/*! \brief Releases device resources (WINDOWS ONLY) */
	virtual void deviceLost();
	
	/*! \brief (Re-)creates device resources (WINDOWS ONLY) */
	virtual void deviceReset();
	
	/*! \brief Destroys device resources (WINDOWS ONLY) */
	virtual void deviceDestroyed();
	
private:
	QuadBuffer(const QuadBuffer& p_rhs);
	QuadBuffer& operator=(const QuadBuffer& p_rhs);
	
	/*! \brief Creates a shared index buffer to be used for batch quad rendering */
	bool generateSharedIndexBuffer();
	
	// Vertex Data
	BatchQuadCollection m_quads;
	bool                m_useVtxColor;
	s32                 m_capacity;
	u32                 m_vertexType;
	TexturePtr          m_texture;
	
	// Win specific
	IDirect3DVertexBuffer9*       m_vertexBuffer;
	static IDirect3DIndexBuffer9* ms_sharedIndexBuffer;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_RENDERER_QUADBUFFER_H)
