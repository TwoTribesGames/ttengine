#define NOMINMAX
#include <spark/SPK.h>

#include <tt/engine/effect/SparkRenderer.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/scene/Camera.h>


namespace tt {
namespace engine {
namespace effect {

void SparkRenderer::initialize()
{
}


void SparkRenderer::shutdown()
{
}


void SparkRenderer::onLostDevice()
{
	// Destroy DX9 device buffers
	SPK::Shared::DX9Info::DX9DestroyAllBuffers();
	SPK::Shared::DX9Info::setDevice( 0 );
}


void SparkRenderer::onResetDevice()
{
	TT_NULL_ASSERT(renderer::getRenderDevice());
	
	// Pass in new device handle
	SPK::Shared::DX9Info::setDevice( renderer::getRenderDevice() );
	
#if SPARK_USE_DX9POINT
	// Set pixel size for point rendering
	renderer::Renderer* renderer(renderer::Renderer::getInstance());
	
	SPK::Shared::DX9PointRenderer::setPixelPerUnit(
		math::degToRad(renderer->getMainCamera()->getFOV()), renderer->getScreenHeight());
#endif
}


}
}
}
