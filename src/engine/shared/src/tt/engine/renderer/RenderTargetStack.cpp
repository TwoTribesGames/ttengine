#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/RenderTarget.h>
#include <tt/engine/renderer/RenderTargetStack.h>


namespace tt {
namespace engine {
namespace renderer {


RenderTargetStack::RenderTargets RenderTargetStack::ms_renderTargets;


void RenderTargetStack::push(const RenderTargetPtr& p_renderTarget, u32 p_clearFlags)
{
	if (ms_renderTargets.empty())
	{
		Renderer::getInstance()->saveBackbuffer();
	}
	else
	{
		ms_renderTargets.back()->setInactive();
	}
	ms_renderTargets.push_back(p_renderTarget);
	p_renderTarget->setActive(p_clearFlags);
}


void RenderTargetStack::pop()
{
	TT_ASSERTMSG(ms_renderTargets.empty() == false, "Cannot pop render target from empty stack.");
	ms_renderTargets.back()->setInactive();
	ms_renderTargets.pop_back();
	if (ms_renderTargets.empty())
	{
		Renderer::getInstance()->restoreBackbuffer();
	}
	else
	{
		ms_renderTargets.back()->setActive(ClearFlag_DontClear);
	}
}

}
}
}
