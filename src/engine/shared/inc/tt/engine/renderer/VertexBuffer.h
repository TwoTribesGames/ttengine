#if !defined(INC_TT_ENGINE_RENDERER_VERTEXBUFFER_H)
#define INC_TT_ENGINE_RENDERER_VERTEXBUFFER_H

#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/math/Vector2.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/fs/types.h>

namespace tt {
namespace engine {
namespace renderer {

class VertexBuffer
{
public:
	enum Property
	{
		// All vertices have at least a position
		Property_Position  = 0,
		Property_Normal    = (1 << 0),
		Property_Diffuse   = (1 << 1),
		Property_Texture0  = (1 << 2),
		Property_Texture1  = (1 << 3),
		Property_Texture2  = (1 << 4),
		Property_Texture3  = (1 << 5),
		
		Property_Max       = (1 << 6)
	};
	
	static const u32 PropertyTextureMask = 0x3C; // Masks all texture channels

	// Create clearer named types
	typedef math::Vector3 Position;
	typedef math::Vector3 Normal;
	typedef math::Vector2 TexCoord;

	typedef std::vector<Position>  PositionBuffer;
	typedef std::vector<Normal>    NormalBuffer;
	typedef std::vector<ColorRGBA> ColorBuffer;
	typedef std::vector<TexCoord>  TexCoordBuffer;

	VertexBuffer();
	VertexBuffer(u32 p_type);
	VertexBuffer(const VertexBuffer& p_rhs);

	void setVertexType(u32 p_type);
	bool load(const fs::FilePtr& p_file, s32 vertexCount);

	inline bool hasProperty(Property p_property)
	{
		return (m_vertexType & p_property) != 0;
	}

	inline PositionBuffer& getPositions() {return m_positions;}
	inline NormalBuffer&   getNormals()   {return m_normals;}
	inline ColorBuffer&    getColors()    {return m_colors;}
	inline TexCoordBuffer& getTexCoords(s32 p_channel) {return m_texcoords[p_channel];}

	inline s32 getVertexSize() const {return m_vertexSize;}
	inline u32 getVertexType() const {return m_vertexType;}
	inline s32 getPropertyCount() const {return m_propertyCount;}

private:
	void computeVertexSize();

	// Vertex data containers
	PositionBuffer m_positions;
	NormalBuffer   m_normals;
	ColorBuffer    m_colors;
	TexCoordBuffer m_texcoords[4];

	// Extra information
	u32 m_vertexType; // Combination of properties
	s32 m_vertexSize; // Total size of single vertex
	s32 m_propertyCount; // Number of properties
};

// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_VERTEXBUFFER_H
