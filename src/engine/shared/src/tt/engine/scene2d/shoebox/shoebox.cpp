#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/ColorAnimationStack2D.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/particles/ParticleTrigger.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/scene/Fog.h>
#include <tt/engine/scene/SceneBlurMgr.h>
#include <tt/engine/scene2d/shoebox/PlaneFollower.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/engine/scene2d/shoebox/ShoeboxPlane.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/scene2d/WorldScene.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/pres/PresentationGroupInterface.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>
#include <tt/xml/util/check.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlFileWriter.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace engine{
namespace scene2d {
namespace shoebox {

//#define SHOEBOX_DEBUG
#ifdef SHOEBOX_DEBUG
	#define SB_Printf(...) TT_Printf(__VA_ARGS__)
#else
	#define SB_Printf(...)
#endif

#define USE_ZERO_LAYER_BATCHING
#if !defined(USE_ZERO_LAYER_BATCHING)
#define USE_BINARY_PLANE_PARTITION
#endif

static const real g_fixedDeltaTime = 1.0f / 30.0f;


std::string Shoebox::ms_shoeboxesPath("shoeboxes/");
std::string Shoebox::ms_texturesPath ("textures/");
std::string Shoebox::ms_particlesPath("particles/");


//--------------------------------------------------------------------------------------------------
// Public member functions

Shoebox::Shoebox(const pres::PresentationMgrPtr& p_presMgr, s32 p_splitPriority)
:
m_textures(),
m_particleCache(),
m_queuedPresentationObjects(),
m_activePresObjects(),
m_background         (new WorldScene),
m_backgroundZero     (new WorldScene),
m_foregroundZeroBack (new WorldScene),
m_foregroundZeroFront(new WorldScene),
m_foreground         (new WorldScene),
m_backPartition(),
m_forePartitionBack(),
m_forePartitionFront(),
m_loadedData(),
m_planeScenesWithID(),
m_planeFollowingParticles(),
m_isHighEndDevice(true),
m_presMgr(p_presMgr),
m_currentTime(0.0f),
m_splitPriority(p_splitPriority),
m_backBlurLayers(),
m_foreBlurLayers(),
m_backBlurQuality(BlurQuality_TwoPassConvolution),
m_frontBlurQuality(BlurQuality_TwoPassConvolution)
#if !defined(TT_BUILD_FINAL)
,
m_layerVisibleFlags()
#endif
{
#if !defined(TT_BUILD_FINAL)
	// All layers are visible by default
	m_layerVisibleFlags.setAllFlags();
#endif
}


Shoebox::~Shoebox()
{
	m_planeFollowingParticles.clear();
	m_planeScenesWithID.clear();
	m_textures.clear();
	m_particleCache.clear();
	m_queuedPresentationObjects.clear();
	m_activePresObjects.clear();
	m_background->deleteAll();
	m_backgroundZero->deleteAll();
	m_foregroundZeroBack->deleteAll();
	m_foregroundZeroFront->deleteAll();
	m_foreground->deleteAll();
	m_backPartition.clear();
	m_forePartitionBack.clear();
	m_forePartitionFront.clear();
	m_loadedData.reset();
	m_backBlurLayers.clear();
	m_foreBlurLayers.clear();
	
	code::helpers::safeDelete(m_background);
	code::helpers::safeDelete(m_backgroundZero);
	code::helpers::safeDelete(m_foregroundZeroBack);
	code::helpers::safeDelete(m_foregroundZeroFront);
	code::helpers::safeDelete(m_foreground);
}


bool Shoebox::load(const std::string&   p_filename,
                   s32                  p_levelWidth,
                   s32                  p_levelHeight,
                   real                 p_scale,
                   const math::Vector3& p_positionOffset,
                   s32                  p_priority,
                   bool                 p_instantCreate)
{
	const std::string filename(makeShoeboxFilename(p_filename));
	
	if (isAllowedToLoadForDeviceType(filename) == false)
	{
		// Ignore this load: pretend it succeeded
		return true;
	}
	
	TT_ERR_CREATE("Loading file '" << p_filename << "' (translated: '" << filename << "'). "
	              "Level width: " << p_levelWidth << "  Level height: " << p_levelHeight);
	
	m_loadedData = ShoeboxData::parse(filename, &errStatus);
	TT_ERR_ASSERT_ON_ERROR();
	if (errStatus.hasError())
	{
		return false;
	}
	
	// If ShoeboxData load succeeded (did not set an error in ErrorStatus),
	// the returned pointer should not be null
	TT_NULL_ASSERT(m_loadedData);
	
	if (p_instantCreate)
	{
		create(p_levelWidth, p_levelHeight, p_scale, p_positionOffset, p_priority);
	}
	
	return true;
}


void Shoebox::create(s32                  p_levelWidth,
                     s32                  p_levelHeight,
                     real                 p_scale,
                     const math::Vector3& p_positionOffset,
                     s32                  p_priority,
                     bool                 p_discardLoadedDataAfterCreate)
{
	TT_ASSERTMSG(m_loadedData != 0, "No shoebox data was loaded: cannot create anything.");
	if (m_loadedData != 0)
	{
		create(*m_loadedData, p_levelWidth, p_levelHeight, p_scale, p_positionOffset, p_priority);
	}
	
	if (p_discardLoadedDataAfterCreate)
	{
		m_loadedData.reset();
	}
}


void Shoebox::create(const ShoeboxData&   p_data,
                     s32                  p_levelWidth,
                     s32                  p_levelHeight,
                     real                 p_scale,
                     const math::Vector3& p_positionOffset,
                     s32                  p_priority)
{
	CreationContext context;
	context.levelWidth     = p_levelWidth;
	context.levelHeight    = p_levelHeight;
	context.scale          = p_scale;
	context.positionOffset = p_positionOffset;
	context.priorityOffset = p_priority;
	
	loadAndCreateIncludes(p_data, context);
	
	createFromData(p_data, context);
	
	// Update after create.
	m_foregroundZeroBack->update(g_fixedDeltaTime);
	m_foregroundZeroFront->update(g_fixedDeltaTime);
	m_foreground->update(g_fixedDeltaTime);
	m_background->update(g_fixedDeltaTime);
	m_forePartitionBack.partition();
	m_forePartitionFront.partition();
	m_backPartition.partition();
	m_forePartitionBack.initialUpdate(g_fixedDeltaTime);
	m_forePartitionFront.initialUpdate(g_fixedDeltaTime);
	m_backPartition.initialUpdate(g_fixedDeltaTime);
}


void Shoebox::copyNeededTextures(const renderer::EngineIDToTextures& p_textures)
{
	renderer::EngineIDToTextures neededTextures;
	
	if (m_loadedData != 0)
	{
		for (ShoeboxData::Planes::iterator planeIt = m_loadedData->planes.begin();
		     planeIt != m_loadedData->planes.end(); ++planeIt)
		{
			EngineID engineID(getEngineID(planeIt->textureFilename));
			
			// If not yet in needed list.
			if (neededTextures.find(engineID.getValue()) == neededTextures.end())
			{
				// Check if it's in p_textures
				renderer::EngineIDToTextures::const_iterator textIt = p_textures.find(engineID.getValue());
				if (textIt != p_textures.end())
				{
					neededTextures.insert(*textIt);
				}
			}
		}
	}
	
	for (renderer::EngineIDToTextures::iterator it = neededTextures.begin(); it != neededTextures.end(); ++it)
	{
		m_textures.insert(*it);
	}
}


renderer::EngineIDToTextures Shoebox::getAllUsedTextures(bool p_loadIncludeFiles) const
{
	renderer::EngineIDToTextures usedTextures(m_textures); // Copy the textures of everything that's created.
	
	// Add the not yet created textures from loaded data.
	getAllUsedTextures(m_loadedData, usedTextures, p_loadIncludeFiles);
	
	return usedTextures;
}


void Shoebox::update(real p_deltaTime)
{
	m_background         ->update(p_deltaTime);
	m_backgroundZero     ->update(p_deltaTime);
	m_foregroundZeroBack ->update(p_deltaTime);
	m_foregroundZeroFront->update(p_deltaTime);
	m_foreground         ->update(p_deltaTime);
	m_backPartition.update(p_deltaTime);
	m_forePartitionBack.update(p_deltaTime);
	m_forePartitionFront.update(p_deltaTime);
	
	// Presentation handling
	m_currentTime += p_deltaTime;
	for (ShoeboxData::PresentationObjects::iterator it = m_queuedPresentationObjects.begin();
	     it != m_queuedPresentationObjects.end(); )
	{
		if (m_currentTime >= (*it).delay)
		{
			pres::PresentationObjectPtr temp(m_presMgr->createPresentationObject((*it).presentationFileName));
			temp->start();
			temp->setPosition((*it).position);
			m_activePresObjects.push_back(temp);
			
			// We don't need the data anymore
			it = m_queuedPresentationObjects.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	for (PresentationObjects::iterator it = m_activePresObjects.begin(); it != m_activePresObjects.end(); )
	{
		if ((*it)->isActive() == false)
		{
			it = m_activePresObjects.erase(it);
		}
		else
		{
			++it;
		}
	}
}


void Shoebox::renderBackground() const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Shoebox Background");
	
#if !defined(TT_BUILD_FINAL)
	if (m_layerVisibleFlags.checkFlag(LayerVisibleFlag_Background) == false) return;
#endif
	
	if(m_backBlurLayers.empty())
	{
		m_background->render();
	}
	else
	{
		m_background->renderWithBlurFromBack(m_backBlurLayers, m_backBlurQuality);
	}
	
	restoreRenderSettings();
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void Shoebox::renderBackgroundZero(const tt::math::VectorRect& p_visibilityRect) const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Shoebox Background Zero");

#if !defined(TT_BUILD_FINAL)
	if (m_layerVisibleFlags.checkFlag(LayerVisibleFlag_BackgroundZero) == false) return;
#endif

#ifdef USE_ZERO_LAYER_BATCHING
	m_backgroundZero->createBatchesFromPlanes();
#endif
	m_backgroundZero->render();
	m_backPartition.render(p_visibilityRect);
	
	restoreRenderSettings();
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void Shoebox::renderForegroundZeroBack(const tt::math::VectorRect& p_visibilityRect) const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Shoebox Foreground Zero Back");

#if !defined(TT_BUILD_FINAL)
	if (m_layerVisibleFlags.checkFlag(LayerVisibleFlag_ForegroundZeroBack) == false) return;
#endif
	
	m_forePartitionBack.render(p_visibilityRect);
#ifdef USE_ZERO_LAYER_BATCHING
	m_foregroundZeroBack->createBatchesFromPlanes();
#endif
	m_foregroundZeroBack->render();
	
	restoreRenderSettings();
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void Shoebox::renderForegroundZeroFront(const tt::math::VectorRect& p_visibilityRect) const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Shoebox Foreground Zero Front");

#if !defined(TT_BUILD_FINAL)
	if (m_layerVisibleFlags.checkFlag(LayerVisibleFlag_ForegroundZeroFront) == false) return;
#endif
	
	m_forePartitionFront.render(p_visibilityRect);
#ifdef USE_ZERO_LAYER_BATCHING
	m_foregroundZeroFront->createBatchesFromPlanes();
#endif
	m_foregroundZeroFront->render();
	
	restoreRenderSettings();
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void Shoebox::renderForeground() const
{
	renderer::Renderer::getInstance()->getDebug()->startRenderGroup("Shoebox Foreground");
	
#if !defined(TT_BUILD_FINAL)
	if (m_layerVisibleFlags.checkFlag(LayerVisibleFlag_Foreground) == false) return;
#endif
	
	if(m_foreBlurLayers.empty())
	{
		m_foreground->render();
	}
	else
	{	
		m_foreground->renderWithBlurToFront(m_foreBlurLayers, m_frontBlurQuality);
	}
	
	restoreRenderSettings();
	renderer::Renderer::getInstance()->getDebug()->endRenderGroup();
}


void Shoebox::renderAll(const tt::math::VectorRect& p_visibilityRect) const
{
#if !defined(TT_BUILD_FINAL)
	if (m_layerVisibleFlags.isEmpty()) return;
#endif
	
	// FIXME: We could save a lot of overhead here if we don't call the render functions separately,
	//        but do the render state setup work once, then render all containers, then do render state cleanup
	renderBackground();
	renderBackgroundZero(p_visibilityRect);
	renderForegroundZeroBack(p_visibilityRect);
	renderForegroundZeroFront(p_visibilityRect);
	renderForeground();
}


void Shoebox::changePositionToRendererSpace(math::Vector3* p_position_OUT,
                                            s32            p_levelWidth,
                                            s32            p_levelHeight)
{
	TT_NULL_ASSERT(p_position_OUT);
	
	p_position_OUT->x = p_position_OUT->x + (p_levelWidth * 0.5f);
	// Y axis is flipped here.
	p_position_OUT->y = -(p_position_OUT->y + (p_levelHeight * 0.5f));
	// Nothing to do for z.
	//p_position_OUT->z = p_position_OUT->z;
}


void Shoebox::changePositionToGameSpace(math::Point2* p_position_OUT,
                                        s32           p_levelWidth,
                                        s32           p_levelHeight)
{
	TT_NULL_ASSERT(p_position_OUT);
	
	p_position_OUT->x = p_position_OUT->x + math::getHalf(p_levelWidth);
	// _NO_ Y axis is flipped here.
	p_position_OUT->y = p_position_OUT->y + math::getHalf(p_levelHeight);
}


renderer::TexturePtr Shoebox::addToTextureCache(const std::string& p_filename)
{
	renderer::EngineIDToTextures::value_type pair = createTextureCacheEntry(p_filename);
	if (pair.second != 0)
	{
		m_textures.insert(pair);
	}
	return pair.second;
}


void Shoebox::setBlurQuality(BlurQuality p_quality)
{
	if (p_quality != m_frontBlurQuality || p_quality != m_backBlurQuality)
	{
		m_frontBlurQuality = p_quality;
		m_backBlurQuality  = p_quality;
		
		real renderTargetRatio(0.4f);
		switch (p_quality)
		{
		case BlurQuality_NoBlur             : renderTargetRatio = 0.10f; break;
		case BlurQuality_OnePassThreeSamples: renderTargetRatio = 0.20f; break;
		case BlurQuality_OnePassFourSamples : renderTargetRatio = 0.25f; break;
		case BlurQuality_TwoPassConvolution : renderTargetRatio = 0.40f; break;
		default:
			TT_PANIC("Invalid blur quality value (%d)", p_quality);
		}
		
		scene::SceneBlurMgr::changeRenderTargetRatio(renderTargetRatio);
	}
}


void Shoebox::invalidateBatches()
{
#if defined(USE_ZERO_LAYER_BATCHING)
	m_backgroundZero     ->invalidateBatch();
	m_foregroundZeroBack ->invalidateBatch();
	m_foregroundZeroFront->invalidateBatch();
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

std::string Shoebox::makeShoeboxFilename(const std::string& p_filename) const
{
	std::string filename(p_filename);
	
	if (filename.find(ms_shoeboxesPath + "device/") != std::string::npos)
	{
		// Found device marker: switch it based on device type
		if (m_isHighEndDevice)
		{
			s32 count = str::replace(filename, ms_shoeboxesPath + "device/", ms_shoeboxesPath + "high/");
			TT_ASSERT(count == 1);
		}
		else
		{
			s32 count = str::replace(filename, ms_shoeboxesPath + "device/", ms_shoeboxesPath + "low/");
			TT_ASSERT(count == 1);
		}
	}
	
	return filename;
}


bool Shoebox::isAllowedToLoadForDeviceType(const std::string& p_filename) const
{
	// Only high end devices should load files in high/.
	return p_filename.find(ms_shoeboxesPath + (m_isHighEndDevice ? "low/" : "high/")) == std::string::npos;
}


void Shoebox::loadAndCreateIncludes(const ShoeboxData& p_data, const CreationContext& p_context)
{
	// Load all includes mentioned in the data
	for (ShoeboxData::Includes::const_iterator it = p_data.includes.begin(); it != p_data.includes.end(); ++it)
	{
		const IncludeData& include(*it);
		
		const std::string filename(makeShoeboxFilename(include.filename + ".shoebox"));
		// FIXME: Do we want this debug output when loading includes?
		//TT_Printf("Found include for shoebox file: '%s'\n", filename.c_str());
		if (isAllowedToLoadForDeviceType(filename))
		{
			TT_ERR_CREATE("Load shoebox include '" << filename << "'.");
			ShoeboxDataPtr includedData = ShoeboxData::parse(filename, &errStatus);
			TT_ERR_ASSERT_ON_ERROR();
			if (errStatus.hasError())
			{
				continue;
			}
			
			if (includedData != 0)
			{
				// Create a modified creation context based on this include's
				// settings and the context that was passed
				CreationContext includeContext(p_context);
				includeContext.positionOffset += include.offset * p_context.scale;
				includeContext.scale          *= include.scale;
				includeContext.priorityOffset += include.priority;
				
				// First recurse to deeper includes
				loadAndCreateIncludes(*includedData, includeContext);
				
				// Create shoebox data for this include
				createFromData(*includedData, includeContext);
			}
		}
	}
}


void Shoebox::createFromData(const ShoeboxData& p_data, const CreationContext& p_context)
{
	// Create all planes
	for (ShoeboxData::Planes::const_iterator it = p_data.planes.begin();
	     it != p_data.planes.end(); ++it)
	{
		createPlane(*it, p_context);
	}
	
	// Create all particle effects
	for (ShoeboxData::Particles::const_iterator it = p_data.particles.begin();
	     it != p_data.particles.end(); ++it)
	{
		createParticle(*it, p_context);
	}
	
	// Copy all presentation objects as queued objects
	m_queuedPresentationObjects = p_data.presentationObjects;
	for (ShoeboxData::PresentationObjects::iterator it = m_queuedPresentationObjects.begin();
	     it != m_queuedPresentationObjects.end(); ++it)
	{
		// But do scale and offset the position based on the creation settings
		PresentationData& data(*it);
		data.position *= p_context.scale;
		data.position += p_context.positionOffset;
		changePositionToRendererSpace(&data.position, p_context.levelWidth, p_context.levelHeight);
	}
}


void Shoebox::createPlane(const PlaneData& p_plane, const CreationContext& p_context)
{
	renderer::TexturePtr texturePtr(addToTextureCache(p_plane.textureFilename));
	if (texturePtr == 0)
	{
		TT_PANIC("Can't add plane because texture '%s' could not be loaded.",
		         p_plane.textureFilename.c_str());
		return;
	}
	
	// Always use filtering on shoebox textures
	texturePtr->setMinificationFilter (renderer::FilterMode_Linear);
	texturePtr->setMagnificationFilter(renderer::FilterMode_Linear);
	
	bool hasVertexColors    = false;
	bool useWholeQuadColors = false;
	
	using renderer::ColorRGB;
	
	if (p_plane.colorWholeQuad.isValid() && p_plane.colorWholeQuad.get() != ColorRGB::white)
	{
		useWholeQuadColors = true;
		hasVertexColors    = true;
		TT_ASSERTMSG(p_plane.colorTopLeft.isValid()    == false && p_plane.colorTopRight.isValid()    == false &&
		             p_plane.colorBottomLeft.isValid() == false && p_plane.colorBottomRight.isValid() == false,
		             "The color for the whole quad was set, but color was also set for an individual vertex!");
	}
	else if (// Check if any of the colors is set.
	    ( p_plane.colorTopLeft.isValid()   || p_plane.colorTopRight.isValid() ||
	      p_plane.colorBottomLeft.isValid()|| p_plane.colorBottomRight.isValid() )
	    &&
	    // If all colors are white the default is good enough.
	    (
	      p_plane.colorTopLeft.get()    != ColorRGB::white || p_plane.colorTopRight.get()    != ColorRGB::white ||
	      p_plane.colorBottomLeft.get() != ColorRGB::white || p_plane.colorBottomRight.get() != ColorRGB::white
	    ) )
	{
		hasVertexColors = true;
	}
	
	if (p_plane.coloranimations != 0)
	{
		if (hasVertexColors == false)
		{
			hasVertexColors    = true;
			useWholeQuadColors = true;
		}
	}
	
	/*
	SB_Printf("Shoebox::addPlanes: Creating plane with texture '%s' on x: %f, y: %f, z: %f. "
	          "With rotation: %f, width: %f and height: %f\n",
	          textureFilename.c_str(), x, y, z, rotation, width, height);
	// */
	
	
	// Translate position.
	math::Vector3 planePos(p_plane.position * p_context.scale);
	planePos += p_context.positionOffset;
	if (p_plane.screenSpace)
	{
		changePositionToRendererSpace(&planePos, 0, 0);
	}
	else
	{
		changePositionToRendererSpace(&planePos, p_context.levelWidth, p_context.levelHeight);
	}
	
	
	const real width  = (p_plane.width.isValid()  ? p_plane.width.get()  : static_cast<real>(texturePtr->getWidth()))  * p_context.scale;
	const real height = (p_plane.height.isValid() ? p_plane.height.get() : static_cast<real>(texturePtr->getHeight())) * p_context.scale;
	
	ShoeboxPlane* plane =
		new ShoeboxPlane(width, height, texturePtr,
		                 planePos, p_plane.rotation,
		                 p_plane.texAnimU, p_plane.texAnimV,
		                 p_plane.texOffsetScale,
		                 p_plane.priority.get() + p_context.priorityOffset, hasVertexColors);
	
	if (p_plane.id.empty() == false)
	{
		if (m_planeScenesWithID.find(p_plane.id) == m_planeScenesWithID.end())
		{
			m_planeScenesWithID[p_plane.id] = plane;
		}
		else
		{
			TT_PANIC("Plane with id '%s' has already been added.", p_plane.id.c_str());
		}
	}
	
	if (math::realEqual(p_plane.cameraSpaceUScale, 0.0f) == false ||
		math::realEqual(p_plane.cameraSpaceVScale, 0.0f) == false)
	{
		const real scaleU = (p_plane.texTopRightU   - p_plane.texTopLeftU) / width  * p_plane.cameraSpaceUScale;
		const real scaleV = (p_plane.texBottomLeftV - p_plane.texTopLeftV) / height * p_plane.cameraSpaceVScale;

		plane->setCameraSpaceScale(scaleU, scaleV);
	}
	
	plane->setTextureCoordinates(p_plane.texTopLeftU,     p_plane.texTopLeftV,
	                             p_plane.texTopRightU,    p_plane.texTopRightV,
	                             p_plane.texBottomRightU, p_plane.texBottomRightV,
	                             p_plane.texBottomLeftU,  p_plane.texBottomLeftV);
	
	const real offsetAnimXMove = p_plane.offsetAnimXMove.get() * p_context.scale;
	const real offsetAnimYMove = p_plane.offsetAnimYMove.get() * p_context.scale;
	
	if (offsetAnimXMove != 0 || offsetAnimYMove != 0)
	{
		plane->setOffsetAnimation(offsetAnimXMove, p_plane.offsetAnimXDuration, p_plane.offsetAnimXTimeOffset,
		                          offsetAnimYMove, p_plane.offsetAnimYDuration, p_plane.offsetAnimYTimeOffset);
	}
	
	plane->setBlendMode       (p_plane.blendMode);
	plane->setAnimations      ((p_plane.animations        != 0) ? p_plane.animations       ->clone() : p_plane.animations       );
	plane->setTextureAnimation((p_plane.textureanimations != 0) ? p_plane.textureanimations->clone() : p_plane.textureanimations);
	plane->setColorAnimation  ((p_plane.coloranimations   != 0) ? p_plane.coloranimations  ->clone() : p_plane.coloranimations  );
	plane->setScreenSpace     (p_plane.screenSpace);
	plane->setVisible         (p_plane.hidden    == false);
	plane->setFogEnabled      (p_plane.ignoreFog == false);
	
	bool useBPP = true;
	(void)useBPP;
	
	if (p_plane.animations != 0)
	{
		if (p_plane.animations->hasZAnimation())
		{
			useBPP = false;
		}
	}
	
	if (hasVertexColors)
	{
		if (useWholeQuadColors)
		{
			plane->setVertexColors(p_plane.colorWholeQuad, p_plane.colorWholeQuad,
			                       p_plane.colorWholeQuad, p_plane.colorWholeQuad);
		}
		else
		{
			plane->setVertexColors(p_plane.colorTopLeft,    p_plane.colorTopRight,
			                       p_plane.colorBottomLeft, p_plane.colorBottomRight);
		}
	}
	
	for (PlaneData::Tags::const_iterator it = p_plane.tags.begin(); it != p_plane.tags.end(); ++it)
	{
		plane->addTag(*it);
	}
	
	const bool shouldAddPlaneToForeground =
			planePos.z > 0.0f || (planePos.z == 0.0f && plane->getPriority() > 0);
	
	if (planePos.z == 0 && p_plane.screenSpace == false)
	{
#ifdef USE_BINARY_PLANE_PARTITION
		if (useBPP)
		{
			if (shouldAddPlaneToForeground)
			{
				if (plane->getPriority() <= m_splitPriority)
				{
					m_forePartitionBack.addPlane(plane);
				}
				else
				{
					m_forePartitionFront.addPlane(plane);
				}
			}
			else
			{
				m_backPartition.addPlane(plane);
			}
		}
		else
#endif
		{
			if (shouldAddPlaneToForeground)
			{
				if (plane->getPriority() <= m_splitPriority)
				{
					m_foregroundZeroBack->insert(plane);
				}
				else
				{
					m_foregroundZeroFront->insert(plane);
				}
			}
			else
			{
				m_backgroundZero->insert(plane);
			}
		}
	}
	else
	{
		if (shouldAddPlaneToForeground)
		{
			m_foreground->insert(plane);
		}
		else
		{
			m_background->insert(plane);
		}
	}
}


void Shoebox::createParticle(const ParticleData& p_particle, const CreationContext& p_context)
{
	if (p_particle.particleFilename.empty())
	{
		TT_PANIC("Can't add particle with an empty name");
		return;
	}
	
	ParticleDataCache particleData(p_particle.particleFilename,
	                               (p_particle.position * p_context.scale) + p_context.positionOffset,
	                               p_particle.scale * p_context.scale,
	                               p_particle.parentID,
	                               p_particle.hidden,
	                               this);
	
	// Copy tags
	for (ParticleData::Tags::const_iterator it = p_particle.tags.begin(); it != p_particle.tags.end(); ++it)
	{
		particleData.addTag(*it);
	}
	
	changePositionToRendererSpace(&particleData.position, p_context.levelWidth, p_context.levelHeight);
	
	if (particleData.startHidden == false)
	{
		particleData.spawn();
	}
	else
	{
		TT_ASSERTMSG(particleData.hasTags(),
		             "Particle found which is can never be started!\n"
		             "Found a particle which starts 'hidden' but which is not tagged!\n"
		             "Effect filename:'%s'", particleData.filename.c_str());
	}
	m_particleCache.push_back(particleData);
}


Shoebox::ParticleDataCache::~ParticleDataCache()
{
	kill();
}


void Shoebox::ParticleDataCache::spawn()
{
	if (effect == 0)
	{
		effect = parentShoebox->spawnParticle(*this);
	}
}


void Shoebox::ParticleDataCache::stop()
{
	kill();
	
	// Can't do stop here because these effects are bound to the shoebox scene.
	// And the temp effect (clones) which are created when we call stop may have a longer lifetime than the scene.
	// This would make m_scene within ParticleTrigger a dangling pointer.
	// A possible solution would be to manage our own temp particles which continue the effect.
	/*
	if (effect != 0)
	{
		effect->stop();
		effect.reset();
	}
	*/
}


void Shoebox::ParticleDataCache::kill()
{
	if (effect != 0)
	{
		effect.reset(); // kill not stop. (Without scene we can't keep the effect.)
	}
}


bool Shoebox::ParticleDataCache::handleEvent(const std::string& p_event, const std::string& /*p_param*/)
{
	if (p_event == "start")
	{
		spawn();
		return true;
	}
	else if (p_event == "stop")
	{
		stop();
		return true;
	}
	
	// Silently accept all unknown events (ShoeboxPlane accepts more events than particles do)
	return true;
}


tt::engine::particles::ParticleEffectPtr Shoebox::spawnParticle(const ParticleDataCache& p_particle)
{
	std::string particlePath(ms_particlesPath + p_particle.filename);
	
	// Check for parent
	PlaneScene* parent = 0;
	PlaneFollowerPtr planeFollower;
	if (p_particle.parentID.empty() == false)
	{
		PlaneSceneIDMapping::const_iterator it = m_planeScenesWithID.find(p_particle.parentID);
		if (it != m_planeScenesWithID.end())
		{
			parent = (*it).second;
		}
		else
		{
			TT_PANIC("Particle with effect file '%s' is parented to unknown parent '%s'.",
				p_particle.filename.c_str(), p_particle.parentID.c_str());
		}
	}
	
	if (parent != 0)
	{
		const tt::math::Vector3 position(p_particle.position - parent->getPosition());
		planeFollower.reset(new PlaneFollower(parent, position));
		m_planeFollowingParticles.push_back(planeFollower);
	}
	
	const bool foreground = (p_particle.position.z > 0.0f);
	
	using tt::engine::particles::ParticleEffectPtr;
	using tt::engine::particles::ParticleMgr;
	
	ParticleMgr* pm = ParticleMgr::getInstance();
	SB_Printf("Shoebox::addParticle: Adding Trigger from file: '%s'\n", particlePath.c_str());
	ParticleEffectPtr pt = pm->createEffectInScene(particlePath,
	                                               (foreground) ? m_foreground : m_background,
	                                               p_particle.position,
	                                               planeFollower,
	                                               p_particle.scale);
	
	if (pt == 0)
	{
		TT_WARN("Could not load shoebox particle effect ('%s').", particlePath.c_str());
		return tt::engine::particles::ParticleEffectPtr();
	}
	
	return pt;
}


void Shoebox::getAllUsedTextures(const ShoeboxDataPtr& p_data, renderer::EngineIDToTextures& p_usedTextures,
                                 bool p_loadIncludeFiles) const
{
	if (p_data == 0)
	{
		return;
	}
	
	// Get textures from this shoebox
	for (ShoeboxData::Planes::const_iterator it = p_data->planes.begin(); it != p_data->planes.end(); ++it)
	{
		EngineID engineID(getEngineID(it->textureFilename));
		
		// If not yet in needed list.
		if (p_usedTextures.find(engineID.getValue()) == p_usedTextures.end())
		{
			p_usedTextures.insert(createTextureCacheEntry(it->textureFilename));
		}
	}
	
	if (p_loadIncludeFiles)
	{
		// Load all includes mentioned in the data
		for (ShoeboxData::Includes::const_iterator it = p_data->includes.begin();
		     it != p_data->includes.end(); ++it)
		{
			const IncludeData& include(*it);
			
			const std::string filename(makeShoeboxFilename(include.filename + ".shoebox"));
			if (isAllowedToLoadForDeviceType(filename))
			{
				TT_ERR_CREATE("Load shoebox include '" << filename << "'.");
				ShoeboxDataPtr includedData = ShoeboxData::parse(filename, &errStatus);
				TT_ERR_ASSERT_ON_ERROR();
				if (errStatus.hasError())
				{
					continue;
				}
				
				// Recursion step
				getAllUsedTextures(includedData, p_usedTextures, true);
			}
		}
	}
}


void Shoebox::restoreRenderSettings() const
{
	renderer::Renderer::getInstance()->setBlendMode(renderer::BlendMode_Blend);
}

// Namespace end
}
}
}
}
