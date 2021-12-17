//#define NOMINMAX
//#include <spark/SPK_DX9.h>

#include <tt/engine/effect/SparkRenderer.h>
//#include <tt/engine/renderer/directx.h>
//#include <tt/engine/renderer/Renderer.h>
//#include <tt/engine/scene/Camera.h>


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
	//SPK::DX9::DX9Info::DX9DestroyAllBuffers();
	//SPK::DX9::DX9Info::setDevice( 0 );
}


void SparkRenderer::onResetDevice()
{
	//TT_NULL_ASSERT(renderer::getRenderDevice());

	// Pass in new device handle
	//SPK::DX9::DX9Info::setDevice( renderer::getRenderDevice() );

	// Set pixel size for point rendering
	//renderer::Renderer* renderer(renderer::Renderer::getInstance());

	//SPK::DX9::DX9PointRenderer::setPixelPerUnit(
	//	math::degToRad(renderer->getMainCamera()->getFieldOfView()), renderer->getScreenHeight());
}


}
}
}
