#include <tt/code/helpers.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/scene/SceneBlurMgr.h>
#include <tt/engine/scene2d/PlaneScene.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/scene2d/WorldScene.h>


namespace tt {
namespace engine {
namespace scene2d {


bool WorldScene::ms_cullPlanesInUpdate = false;


inline scene::BlurType getBlurType(BlurQuality p_quality, bool p_isClosestLayer)
{
	scene::BlurType result(scene::BlurType_TwoPassConvolution);
	
	switch (p_quality)
	{
	case BlurQuality_NoBlur             : result = scene::BlurType_Triangle;           break;
	case BlurQuality_OnePassThreeSamples: result = scene::BlurType_Triangle;           break;
	case BlurQuality_OnePassFourSamples : result = scene::BlurType_Box;                break;
	case BlurQuality_TwoPassConvolution : result = scene::BlurType_TwoPassConvolution; break;
	default:
		TT_PANIC("Invalid quality type (%d)", p_quality);
	}

	if (p_isClosestLayer && result == scene::BlurType_TwoPassConvolution)
	{
		result = scene::BlurType_Box;
	}
	return result;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

WorldScene::WorldScene()
:
m_sceneNodes(),
m_scheduledForResort(),
m_isUpdating(false),
m_batchCreated(false)
{
}


WorldScene::~WorldScene()
{
}


void WorldScene::update(real p_delta_time)
{
	m_isUpdating = true;

	if (m_batchCreated)
	{
		for (SceneVector::iterator it = m_nonBatchableNodes.begin(); it != m_nonBatchableNodes.end(); ++it)
		{
			(*it)->update(p_delta_time);
		}
	}
	else
	{
		for (SceneVector::iterator it = m_sceneNodes.begin(); it != m_sceneNodes.end(); ++it)
		{
			(*it)->update(p_delta_time);
		}
	}
	m_isUpdating = false;
	
	if (m_scheduledForResort.empty() == false)
	{
		invalidateBatch();
		
		// Remove all nodes that need resorting
		for (SceneVector::iterator it = m_scheduledForResort.begin();
		     it != m_scheduledForResort.end(); ++it)
		{
			remove(*it);
		}
		
		// And re-insert them (so that they get their correct positions)
		for (SceneVector::iterator it = m_scheduledForResort.begin();
		     it != m_scheduledForResort.end(); ++it)
		{
			insert(*it);
		}
		
		m_scheduledForResort.clear();
	}
	
	if (m_batchCreated)
	{
		// Don't waste time with visible set if using batches
		return;
	}
	
	// Determine visible set
	if (ms_cullPlanesInUpdate)
	{
		m_visibleNodes.clear();
	
		for (SceneVector::iterator it = m_sceneNodes.begin(); it != m_sceneNodes.end(); ++it)
		{
			if ((*it)->isPlaneScene())
			{
				PlaneScene* plane = static_cast<PlaneScene*>(*it);
				
				for (renderer::ViewPortContainer::iterator viewPortIt = renderer::ViewPort::getViewPorts().begin();
					viewPortIt != renderer::ViewPort::getViewPorts().end(); ++viewPortIt)
				{
					const scene::CameraPtr camera = viewPortIt->getCamera();
			
					if (camera->isVisible(plane->getBoundingRect(), plane->getPosition().z))
					{
						if (std::find(m_visibleNodes.begin(), m_visibleNodes.end(), plane) == m_visibleNodes.end())
						{
							m_visibleNodes.push_back(*it);
						}
					}
				}
			}
			else
			{
				m_visibleNodes.push_back(*it);
			}
		}
	}
	else
	{
		m_visibleNodes = m_sceneNodes;
	}
}


void WorldScene::render()
{
	if (m_quadBatches.empty() == false)
	{
		renderer::Renderer* renderer(renderer::Renderer::getInstance());
		bool fogEnabled = renderer->isFogEnabled();
		
		for (QuadBatches::const_iterator it = m_quadBatches.begin(); it != m_quadBatches.end(); ++it)
		{
			// Render animated planes first
			for (SceneVector::const_iterator planeIt = it->animatedPlanes.begin(); planeIt != it->animatedPlanes.end(); ++planeIt)
			{
				(*planeIt)->render();
			}
			
			// Render static planes batch
			if (it->staticPlanes != 0)
			{
				if (fogEnabled)
				{
					renderer->setFogEnabled(it->staticState.fogEnabled);
				}

				if (it->staticState.texture != 0)
				{
					it->staticState.texture->setAddressMode(it->staticState.addressModeU, it->staticState.addressModeV);
				}
				renderer->setBlendMode(it->staticState.blendMode);
				it->staticPlanes->render();
			}
		}
		
		renderer->setFogEnabled(fogEnabled);
	}
	else
	{
		for (SceneVector::const_iterator it = m_visibleNodes.begin(); it != m_visibleNodes.end(); ++it)
		{
			(*it)->render();
		}
	}
	
	renderer::MatrixStack::getInstance()->resetTextureMatrix();
}


void WorldScene::renderWithBlurFromBack(const BlurLayers& p_layers, BlurQuality p_quality)
{
	if (p_quality == BlurQuality_NoBlur)
	{
		render();
		return;
	}
	
	SceneVector::const_iterator sceneIt = m_visibleNodes.begin();
	
	if(sceneIt == m_visibleNodes.end()) return;
	
	// Check if any planes are in the blurred area
	real closestLayer = *p_layers.rbegin();
	
	if (p_layers.empty() == false && (*sceneIt)->getDepth() <= closestLayer)
	{
		scene::SceneBlurMgr::beginRender();
		
		// No need to blur until the first plane has been rendered
		bool anythingRendered(false);
		for (BlurLayers::const_iterator blurIt = p_layers.begin(); blurIt != p_layers.end(); ++blurIt)
		{
			while(sceneIt != m_visibleNodes.end() && (*sceneIt)->getDepth() < *blurIt)
			{
				(*sceneIt)->render();
				anythingRendered = true;
				
				++sceneIt;
			}
			
			if (anythingRendered)
			{
				scene::SceneBlurMgr::doBlurPass(
					getBlurType(p_quality, math::realEqual(*blurIt, closestLayer)));
			}
		}
		
		scene::SceneBlurMgr::endRender();
	}
	
	// Non Blur planes
	
	for(; sceneIt != m_visibleNodes.end(); ++sceneIt)
	{
		(*sceneIt)->render();
	}
	
	renderer::MatrixStack::getInstance()->resetTextureMatrix();
}


void WorldScene::renderWithBlurToFront(const BlurLayers& p_layers, BlurQuality p_quality)
{
	if (p_quality == BlurQuality_NoBlur)
	{
		render();
		return;
	}
	
	// Non Blur planes
	SceneVector::const_iterator sceneIt = m_visibleNodes.begin();
	BlurLayers::const_iterator blurIt   = p_layers.begin();
	
	// Render until first blur layer
	while (sceneIt != m_visibleNodes.end() && (blurIt == p_layers.end() || (*sceneIt)->getDepth() < *blurIt))
	{
		(*sceneIt)->render();
		++sceneIt;
	}
	
	if (sceneIt != m_visibleNodes.end())
	{
		// Between blur layers
		for(u32 layer = 0; layer < p_layers.size(); ++layer)
		{
			++blurIt;

			scene::SceneBlurMgr::beginRender();

			renderer::Renderer* renderer(renderer::Renderer::getInstance());
			
			bool anythingRendered(false);
			while(sceneIt != m_visibleNodes.end() && (blurIt == p_layers.end() || (*sceneIt)->getDepth() < *blurIt))
			{
				// NOTE: Blend mode magic
				// To support additive as well as alpha blended planes in the overlay, we need to
				// Set the alpha value to 0 for additive planes at the pixels where alpha != 0
				// This only works if we do the overlay using premultiplied alpha

				if((*sceneIt)->getBlendMode() == renderer::BlendMode_Add)
				{
					renderer->setCustomBlendModeAlpha(renderer::BlendFactor_DstAlpha, renderer::BlendFactor_InvSrcAlpha);
				}
				else
				{
					renderer->setCustomBlendModeAlpha(renderer::BlendFactor_One, renderer::BlendFactor_InvSrcAlpha);
				}
				(*sceneIt)->render();
				anythingRendered = true;
				
				++sceneIt;
			}

			if(anythingRendered)
			{
				scene::SceneBlurMgr::doBlurPass(getBlurType(p_quality, layer == 0));
				
				if(layer > 1)
				{
					for(u32 i = 0; i < (layer - 1); ++i)
					{
						scene::SceneBlurMgr::doBlurPass(getBlurType(p_quality, false));
					}
				}
			}

			scene::SceneBlurMgr::endRender();
		}
	}
	
	renderer::MatrixStack::getInstance()->resetTextureMatrix();
}


void WorldScene::insert(Scene2D* p_node)
{
	TT_NULL_ASSERT(p_node);
	TT_ASSERTMSG(m_isUpdating == false,
	             "Should not call WorldScene::insert while in this WorldScene's update().");
	
	// Search for the first node that is closer than this one
	SceneVector::iterator it = m_sceneNodes.begin();
	
	// NOTE: Sorting on material is very useful to improve batching efficiency, but
	//       the order of planes with equal Z and priority becomes undefined, need data fixes
	//       when we enable this.
#if 0
	const real  depth      = p_node->getDepth();
	const s32   priority   = p_node->getPriority();
	const void* materialID = p_node->getMaterialID();
	
	for ( ; it != m_sceneNodes.end(); ++it)
	{
		const Scene2D* otherNode = (*it);
		const real  otherDepth      = otherNode->getDepth();
		const s32   otherPriority   = otherNode->getPriority();
		const void* otherMaterialID = otherNode->getMaterialID();
		
		if (depth == otherDepth)
		{
			if (priority == otherPriority)
			{
				if (materialID < otherMaterialID)
				{
					break;
				}
			}
			else if (priority < otherPriority)
			{
				break;
			}
		}
		else if (depth < otherDepth)
		{
			break;
		}
	}
#else
		for ( ; it != m_sceneNodes.end(); ++it)
	{
		if (  (p_node->getDepth() < (*it)->getDepth()) ||
		      ( (p_node->getDepth() == (*it)->getDepth()) &&
		        (p_node->getPriority() < (*it)->getPriority()) ) )
		{
			// Node found, exit loop
			break;
		}
	}
#endif
	
	// Insert node
	m_sceneNodes.insert(it, p_node);
	
	// Bind this scene to the node
	p_node->registerScene(this);
}


void WorldScene::remove(Scene2D* p_node)
{
	TT_ASSERTMSG(m_quadBatches.empty(), "Trying to remove node from batched layer.");
	TT_ASSERTMSG(m_isUpdating == false,
	             "Should not call WorldScene::remove while in this WorldScene's update().");
	if (p_node != 0)
	{
		// Remove from scene
		m_sceneNodes.erase(std::remove(m_sceneNodes.begin(), m_sceneNodes.end(), p_node), m_sceneNodes.end());
		
		// Unregister
		p_node->unregisterScene();
	}
}


void WorldScene::rearrange(Scene2D* p_node)
{
	TT_ASSERTMSG(m_quadBatches.empty(), "Trying to rearrange node in batched layer.");
	TT_NULL_ASSERT(p_node);
	if (p_node != 0)
	{
		m_scheduledForResort.push_back(p_node);
	}
}


void WorldScene::removeAll()
{
	// Clear all nodes
	TT_ASSERTMSG(m_isUpdating == false,
	             "Should not call WorldScene::removeAll while in this WorldScene's update().");
	m_sceneNodes.clear();
	m_visibleNodes.clear();
	m_nonBatchableNodes.clear();
	m_quadBatches.clear();
	m_batchCreated = false;
}


void WorldScene::deleteAll()
{
	TT_ASSERTMSG(m_isUpdating == false,
	             "Should not call WorldScene::deleteAll while in this WorldScene's update().");
	for (SceneVector::iterator it = m_sceneNodes.begin(); it != m_sceneNodes.end(); ++it)
	{
		tt::code::helpers::safeDelete(*it);
	}
	m_sceneNodes.clear();
	m_visibleNodes.clear();
	m_nonBatchableNodes.clear();
	m_quadBatches.clear();
	m_batchCreated = false;
}



void WorldScene::createBatchesFromPlanes()
{
	// Only create once
	if (m_batchCreated || m_sceneNodes.empty())
	{
		return;
	}
	
	PlaneBatch planeBatch;
	
	TextureState currentState;
	currentState.addressModeU = renderer::AddressMode_Invalid;
	currentState.addressModeV = renderer::AddressMode_Invalid;
	currentState.blendMode    = renderer::BlendMode_Invalid;
	currentState.fogEnabled   = true;
	
	m_nonBatchableNodes.clear();
	
	for (SceneVector::iterator it = m_sceneNodes.begin(); it != m_sceneNodes.end(); ++it)
	{
		if ((*it)->isSuitableForBatching() == false)
		{
			// Animated plane
			if (planeBatch.empty() || planeBatch.back().staticPlanes.empty() == false)
			{
				ParsedLayer newLayer;
				newLayer.state.blendMode    = renderer::BlendMode_Invalid;
				newLayer.state.addressModeU = renderer::AddressMode_Invalid;
				newLayer.state.addressModeV = renderer::AddressMode_Invalid;
				planeBatch.push_back(newLayer);
			}
			
			m_nonBatchableNodes.push_back(*it);
			planeBatch.back().animatedPlanes.push_back(*it);
		}
		else
		{
#if defined(TT_BUILD_FINAL)
			PlaneScene* plane = static_cast<PlaneScene*>(*it);
#else
			PlaneScene* plane = dynamic_cast<PlaneScene*>(*it);
#endif
			TT_NULL_ASSERT(plane);
			
			if (currentState.texture      != plane->getTexture()      ||
				currentState.addressModeU != plane->getAddressModeU() ||
				currentState.addressModeV != plane->getAddressModeV() ||
				currentState.blendMode    != plane->getBlendMode()    ||
				currentState.fogEnabled   != plane->isFogEnabled())
			{
				currentState.texture      = plane->getTexture();
				currentState.addressModeU = plane->getAddressModeU();
				currentState.addressModeV = plane->getAddressModeV();
				currentState.blendMode    = plane->getBlendMode();
				currentState.fogEnabled   = plane->isFogEnabled();
				
				TT_ASSERT(renderer::isValidAddressMode(currentState.addressModeU));
				TT_ASSERT(renderer::isValidAddressMode(currentState.addressModeV));
				TT_ASSERT(renderer::isValidBlendMode  (currentState.blendMode));
				
				if (planeBatch.empty() || planeBatch.back().staticPlanes.empty() == false)
				{
					// Create a new set of planes for the current TextureState
					ParsedLayer newLayer;
					planeBatch.push_back(newLayer);
				}
			}
			planeBatch.back().staticPlanes.push_back(plane);
			planeBatch.back().state = currentState;
		}
	}
	
	m_quadBatches.clear();
	
	for (PlaneBatch::iterator it = planeBatch.begin(); it != planeBatch.end(); ++it)
	{
		TT_ASSERT(it->staticPlanes.empty() || renderer::isValidAddressMode(it->state.addressModeU));
		TT_ASSERT(it->staticPlanes.empty() || renderer::isValidAddressMode(it->state.addressModeV));
		TT_ASSERT(it->staticPlanes.empty() || renderer::isValidBlendMode  (it->state.blendMode)   );
		
		BatchLayer layer;
		layer.animatedPlanes = it->animatedPlanes;
		layer.staticState    = it->state;
		
		if (it->staticPlanes.empty() == false)
		{
			layer.staticPlanes.reset(new renderer::QuadBuffer(static_cast<s32>(it->staticPlanes.size()), it->state.texture, 
				(tt::engine::renderer::BatchFlagQuad)(renderer::BatchFlagQuad_EmbedGpuBuffer | renderer::BatchFlagQuad_UseVertexColor)));
			renderer::BatchQuadCollection quadCollection(it->staticPlanes.size());
			
			s32 i(0);
			for (Planes::iterator planeIt = it->staticPlanes.begin();
				planeIt != it->staticPlanes.end(); ++planeIt, ++i)
			{
				(*planeIt)->makeBatchQuad(quadCollection[i]);
			}
			
			layer.staticPlanes->setCollection(quadCollection);
			layer.staticPlanes->applyChanges();
		}
		
		m_quadBatches.push_back(layer);
	}
	
	m_batchCreated = true;
}


void WorldScene::invalidateBatch()
{
	m_quadBatches.clear();
	m_batchCreated = false;
}


// Namespace end
}
}
}
