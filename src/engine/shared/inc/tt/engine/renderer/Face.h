#if !defined(INC_TT_ENGINE_RENDERER_FACE_H)
#define INC_TT_ENGINE_RENDERER_FACE_H


#include <tt/code/ErrorStatus.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace engine {
namespace renderer {

class Face
{
public:
	Face()
	{
		m_vertices[0] = 0;
		m_vertices[1] = 0;
		m_vertices[2] = 0;
	}
	Face(u16 p_v0, u16 p_v1, u16 p_v2)
	{
		m_vertices[0] = p_v0;
		m_vertices[1] = p_v1;
		m_vertices[2] = p_v2;
	}
	~Face() {}
	
	inline bool load(const tt::xml::XmlNode* p_node)
	{
		if (p_node == 0)
		{
			TT_WARN("XmlNode must not be 0");
			return false;
		}
		
		if (p_node->getName() != "face")
		{
			TT_WARN("XmlNode name '%s' invalid, should be 'face'", p_node->getName().c_str());
			return false;
		}
		
		if (p_node->getAttribute("v0").empty() ||
		    p_node->getAttribute("v1").empty() ||
		    p_node->getAttribute("v2").empty())
		{
			TT_WARN("Missing v0, v1 or v2 attribute");
			return false;
		}
		
		TT_ERR_CREATE("Face::load");
		m_vertices[0] = str::parseU16(p_node->getAttribute("v0"), &errStatus);
		m_vertices[1] = str::parseU16(p_node->getAttribute("v1"), &errStatus);
		m_vertices[2] = str::parseU16(p_node->getAttribute("v2"), &errStatus);
		
		if (errStatus.hasError())
		{
			TT_WARN("Error parsing attributes.");
			return false;
		}
		
		if (m_vertices[0] < 1)
		{
			TT_WARN("v0 (%u)out of range", m_vertices[0]);
			return false;
		}
		
		if (m_vertices[1] < 1)
		{
			TT_WARN("v1 (%u)out of range", m_vertices[1]);
			return false;
		}
		
		if (m_vertices[2] < 1)
		{
			TT_WARN("v2 (%u)out of range", m_vertices[2]);
			return false;
		}
		
		--m_vertices[0];
		--m_vertices[1];
		--m_vertices[2];
		return true;
	}
	
	inline u16 getVertex(s32 p_index) const { return m_vertices[p_index]; }
	inline u16 getV0()                const { return m_vertices[0];       }
	inline u16 getV1()                const { return m_vertices[1];       }
	inline u16 getV2()                const { return m_vertices[2];       }
	
	inline void setVertex(s32 p_index, u16 p_v) { m_vertices[p_index] = p_v; }
	inline void setV0(u16 p_v)                  { m_vertices[0]       = p_v; }
	inline void setV1(u16 p_v)                  { m_vertices[1]       = p_v; }
	inline void setV2(u16 p_v)                  { m_vertices[2]       = p_v; }
	
private:
	// Vertex indices
	u16 m_vertices[3];
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_FACE_H
