#if !defined(INC_TT_ENGINE_RENDERER_RENDERTARGETSTACK_H)
#define INC_TT_ENGINE_RENDERER_RENDERTARGETSTACK_H


#include <vector>
#include <tt/engine/renderer/fwd.h>


namespace tt {
namespace engine {
namespace renderer {


class RenderTargetStack
{
public:
	/*! \brief Push a RenderTarget on the stack, this will be set as the active RenderTarget */
	static void push(const RenderTargetPtr& p_renderTarget, u32 p_clearFlags);
	
	/*! \brief Pop a RenderTarget from the stack, the previous RenderTarget will be restored */
	static void pop();
	
	static bool isEmpty() { return ms_renderTargets.empty(); }
	
private:
	typedef std::vector<RenderTargetPtr> RenderTargets;
	static RenderTargets ms_renderTargets;
};



}
}
}

#endif //INC_TT_ENGINE_RENDERER_RENDERTARGETSTACK_H
