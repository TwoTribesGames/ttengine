#if !defined(INC_TT_ENGINE_RENDERER_BATCHTRIANGLEUV_H)
#define INC_TT_ENGINE_RENDERER_BATCHTRIANGLEUV_H

#include <tt/engine/renderer/BufferVtx.h>


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Struct used by client for a TriangleBuffer */
template <s32 UVSetCount>
struct BatchTriangleUV
{
	BatchTriangleUV(const BufferVtxUV<UVSetCount>& p_a = BufferVtxUV<UVSetCount>(),
	                const BufferVtxUV<UVSetCount>& p_b = BufferVtxUV<UVSetCount>(),
	                const BufferVtxUV<UVSetCount>& p_c = BufferVtxUV<UVSetCount>())
	:
	a(p_a),
	b(p_b),
	c(p_c)
	{ }
	
	BufferVtxUV<UVSetCount> a;
	BufferVtxUV<UVSetCount> b;
	BufferVtxUV<UVSetCount> c;
};


}// Namespace end
}
}


#endif // INC_TT_ENGINE_RENDERER_BATCHTRIANGLEUV_H
