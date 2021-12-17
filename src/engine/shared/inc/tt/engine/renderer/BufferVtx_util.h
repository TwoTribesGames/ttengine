#if !defined(INC_TT_ENGINE_RENDERER_BUFFERVTX_UTIL_H)
#define INC_TT_ENGINE_RENDERER_BUFFERVTX_UTIL_H

#include <tt/engine/renderer/BufferVtx.h>

namespace tt {
namespace engine {
namespace renderer {


template <s32 UVSetCount>
inline bool operator== (const BufferVtxUV<UVSetCount>& p_lhs, const BufferVtxUV<UVSetCount>& p_rhs)
{
	if (p_lhs.getPosition() != p_rhs.getPosition() ||
	    p_lhs.getColor()    != p_rhs.getColor())
	{
		return false;
	}
	for (s32 i = 0; i < UVSetCount; ++i)
	{
		if (p_lhs.getTexCoord(i) != p_rhs.getTexCoord(i))
		{
			return false;
		}
	}
	return true;
}


template <s32 UVSetCount>
inline std::ostream& operator<<(std::ostream& s, const BufferVtxUV<UVSetCount>& p_vtx)
{
	s << "(pos: " << p_vtx.getPosition() << ", col: " << p_vtx.getColor();
	
	for (s32 i = 0; i < UVSetCount; ++i)
	{
		s << ", tex[" << i << "]: " << p_vtx.getTexCoord(i);
	}
	
	return s << ")";
}


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_BUFFERVTX_UTIL_H
