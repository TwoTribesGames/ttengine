#if !defined(INC_TT_ENGINE_RENDERER_GPU_CAPABILITIES_H)
#define INC_TT_ENGINE_RENDERER_GPU_CAPABILITIES_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {


enum NPOTSupport
{
	NPOTSupport_None,
	NPOTSupport_Limited,
	NPOTSupport_Full
};


/*! \brief Return whether the GPU supports hardware shaders */
bool hasShaderSupport();

/*! \brief Returns the maximum texture width supported by the GPU */
u32 getMaxTextureWidth();

/*! \brief Returns the level of non power-of-two texture support */
NPOTSupport getNonPowerOfTwoSupport();

/*! \brief Returns whether the GPU supports stencil buffer (8-bit 2-sided) */
bool hasStencilBufferSupport();


// Namespace end
}
}
}

#endif //INC_TT_ENGINE_RENDERER_GPU_CAPABILITIES_H
