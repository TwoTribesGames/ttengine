#include <algorithm>

#include <tt/code/bufferutils.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/movement/SurroundingsSurvey.h>
#include <toki/game/AttributeDebugView.h>
#include <toki/game/Game.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>



namespace toki {
namespace game {
namespace fluid {

static const level::CollisionTypes g_fluidKillers(
		level::CollisionTypes(level::CollisionType_Solid_FluidKill) |
		level::CollisionTypes(level::CollisionType_FluidKill));


//--------------------------------------------------------------------------------------------------
// Public member functions

FluidMgrPtr FluidMgr::create(const level::AttributeLayerPtr& p_levelLayer)
{
	return FluidMgrPtr(new FluidMgr(p_levelLayer));
}


void FluidMgr::update(real p_deltaTime)
{
	using namespace toki::utils;
	m_sectionProfiler.startFrameUpdate();
	
	// Do fluid simulation
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_Grow);

	updateFluids(p_deltaTime);
	
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_FluidParticlesMgr);
	updateEntityEffects();
	
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_NotifyTileChange);
	
	// Notify entities where fluids changed
	for (entity::EntityHandleSet::iterator it = m_notifiedEntities.begin();
	     it != m_notifiedEntities.end(); ++it)
	{
		entity::Entity* entity = (*it).getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			entity->onTileChange();
		}
	}
	m_notifiedEntities.clear();
	
	m_graphicsMgr.update(p_deltaTime, m_sectionProfiler);
	
	m_sectionProfiler.stopFrameUpdate();
}


void FluidMgr::updateForRender(const tt::math::VectorRect& p_visibilityRect)
{
	//using namespace toki::utils;
	//m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_FluidGraphicsMgrMisc);
	// Update the actual graphics manager if it is enabled. Otherwise update the debug rendering
	if (m_graphicsMgr.isEnabled())
	{
		m_graphicsMgr.updateForRender(m_activeLayer, m_particlesMgr,
		                              m_sectionProfiler, p_visibilityRect, m_falls, m_flows);
	}
#if defined(TT_PLATFORM_WIN)
	else
	{
		m_activeView->update();
	}
#endif
	
	updateAudio(p_visibilityRect);
	
	//m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_FluidParticlesMgr);
	m_particlesMgr.update();
}


void FluidMgr::render() const
{
	if (m_graphicsMgr.isEnabled() == false) // Only render debug graphics if actual graphics are not enabled
	{
#if defined(TT_PLATFORM_WIN)
		m_activeView->render();
#endif
		renderFluids();
	}
}


void FluidMgr::onTileChange(const tt::math::Point2& p_position)
{
	handleTileChanged(p_position);

	m_stillTiles.erase(p_position);
	m_stillTilesSurface.erase(p_position);
	m_sourceTiles.erase(p_position);
	
	// if this position became a source add it
	const level::CollisionType collisionType = m_levelLayer->getCollisionType(p_position);
	if (level::isFluidSource(collisionType))
	{
		m_sourceTiles.insert(p_position);
	}
	// else if this is a still add it to the stills
	else if (level::isFluidStill(collisionType))
	{
		m_stillTiles.insert(p_position);
	}
}


void FluidMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_FluidMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the FluidMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Active fluid tile layer
	bu::put(static_cast<s32>(m_activeLayer->getWidth()),  &context);
	bu::put(static_cast<s32>(m_activeLayer->getHeight()), &context);
	bu::put(m_activeLayer->getRawData(), static_cast<size_t>(m_activeLayer->getLength()), &context);
	
	// All data per fluid type: fluid flow type data
	bu::put(static_cast<u32>(FluidType_Count), &context);
	bu::put(static_cast<u32>(NodeType_Count),  &context);  // avoid repeating node type count for each fluid type
	for (s32 fluidTypeIndex = 0; fluidTypeIndex < FluidType_Count; ++fluidTypeIndex)
	{
		const FluidType fluidType = static_cast<FluidType>(fluidTypeIndex);
		bu::put(getFluidTypeName(fluidType), &context);
		
		// Fluid flow type data per node type:
		for (s32 nodeTypeIndex = 0; nodeTypeIndex < NodeType_Count; ++nodeTypeIndex)
		{
			const NodeType nodeType = static_cast<NodeType>(nodeTypeIndex);
			bu::put(getNodeTypeName(nodeType), &context);
			
			const FluidFlowTypeData& typeData(m_fluidFlowTypeData[fluidType][nodeType]);
			
			bu::put(typeData.timeTillNextUpdate, &context);
			
			bu::put(static_cast<u32>(typeData.toFlowList.size()), &context);
			for (Point2s::const_iterator it = typeData.toFlowList.begin();
			     it != typeData.toFlowList.end(); ++it)
			{
				bu::put(*it, &context);
			}
		}
	}
	
	// Warp pairs
	u32 validWarpPairCount = 0;
	for (WarpPairs::const_iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if ((*it).second.isValid)
		{
			++validWarpPairCount;
		}
	}
	bu::put(validWarpPairCount, &context);
	
	for (WarpPairs::const_iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if ((*it).second.isValid)
		{
			const WarpID&   warpID((*it).first);
			const WarpPair& warpPair((*it).second);
			
			bu::put(warpID.getValue(),  &context);
			bu::put(warpPair.warpRect1, &context);
			bu::put(warpPair.warpRect2, &context);
		}
	}
	
	// Fluid Falls
	for(s32 i = 0; i < m_activeLayer->getWidth(); ++i)
	{
		// Number of falls in this column
		bu::put(static_cast<s32>(m_falls[i].size()), &context);

		for(FluidFalls::const_iterator it = m_falls[i].begin(); it != m_falls[i].end(); ++it)
		{
			// Serialize fall
			bu::put(it->area, &context);
			bu::put(static_cast<u8>(it->type), &context);
			bu::put(it->warpID.getValue(), &context);
			
			bu::put(it->startTile, &context);
			bu::put(it->growTile,  &context);
			bu::put(static_cast<u8>(it->state),    &context);
			bu::put(static_cast<u8>(it->fallType), &context);
		}
	}

	// Fluid Flows
	for(s32 i = 0; i < m_activeLayer->getHeight(); ++i)
	{
		// Number of flows in this row
		bu::put(static_cast<s32>(m_flows[i].size()), &context);

		for(FluidFlows::const_iterator it = m_flows[i].begin(); it != m_flows[i].end(); ++it)
		{
			// Serialize fall
			bu::put(it->area, &context);
			bu::put(static_cast<u8>(it->type), &context);
			bu::put(it->warpID.getValue(), &context);
			
			bu::put(it->growLeft,  &context);
			bu::put(it->growRight, &context);
			bu::put(static_cast<u8>(it->stateLeft),  &context);
			bu::put(static_cast<u8>(it->stateRight), &context);
			bu::put(it->feedPointCount,  &context);
			bu::put(it->currentLeft, &context);
			bu::put(it->currentRight, &context);
		}

		// Number of feed points in this row
		bu::put(static_cast<s32>(m_feedPoints[i].size()), &context);
		for(FeedPoints::const_iterator it = m_feedPoints[i].begin(); it != m_feedPoints[i].end(); ++it)
		{
			bu::put(*it, &context);
		}
	}

	// Warp tiles
	bu::put(static_cast<s32>(m_warpTiles.size()), &context);
	for(Point2Set::const_iterator it = m_warpTiles.begin(); it != m_warpTiles.end(); ++it)
	{
		bu::put(*it, &context);
	}
	
	context.flush();
}


void FluidMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr,
                           const toki::game::entity::EntityMgr& p_entityMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_FluidMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the FluidMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Active fluid tile layer
	const s32 layerWidth  = bu::get<s32>(&context);
	const s32 layerHeight = bu::get<s32>(&context);
	level::AttributeLayerPtr activeLayer = level::AttributeLayer::create(layerWidth, layerHeight);
	if (activeLayer == 0)
	{
		TT_PANIC("Could not create new tile layer (size %d x %d) for fluid simulation.",
		         layerWidth, layerHeight);
		return;
	}
	bu::get(activeLayer->getRawData(), static_cast<size_t>(activeLayer->getLength()), &context);
	
	m_activeLayer = activeLayer;
	m_simulationLayer = level::AttributeLayer::create(layerWidth, layerHeight);
	m_simulationLayer->clear();
	
#if defined(TT_PLATFORM_WIN)
	m_activeView->setAttributeLayer(m_activeLayer);
	m_simulationView->setAttributeLayer(m_simulationLayer);
#endif
	
	// All data per fluid type: fluid flow type data
	// - First clear the current data (in case not everything is loaded)
	for (u32 fluidTypeIndex = 0; fluidTypeIndex < FluidType_Count; ++fluidTypeIndex)
	{
		for (u32 nodeTypeIndex = 0; nodeTypeIndex < NodeType_Count; ++nodeTypeIndex)
		{
			m_fluidFlowTypeData[fluidTypeIndex][nodeTypeIndex].timeTillNextUpdate = 0.0f;
			m_fluidFlowTypeData[fluidTypeIndex][nodeTypeIndex].toFlowList.clear();
		}
	}
	
	// - Load the settings from serialization data
	const u32 fluidTypeCount = bu::get<u32>(&context);
	const u32 nodeTypeCount  = bu::get<u32>(&context);
	for (u32 fluidTypeIndex = 0; fluidTypeIndex < fluidTypeCount; ++fluidTypeIndex)
	{
		const std::string fluidTypeName = bu::get<std::string>(&context);
		const FluidType   fluidType     = getFluidTypeFromName(fluidTypeName);
		
		TT_ASSERTMSG(isValidFluidType(fluidType),
		             "Loaded unsupported fluid type '%s' from serialization data.",
		             fluidTypeName.c_str());
		
		// Fluid flow type data per node type:
		for (u32 nodeTypeIndex = 0; nodeTypeIndex < nodeTypeCount; ++nodeTypeIndex)
		{
			const std::string nodeTypeName = bu::get<std::string>(&context);
			const NodeType    nodeType     = getNodeTypeFromName(nodeTypeName);
			
			TT_ASSERTMSG(isValidNodeType(nodeType),
			             "Loaded unsupported node type '%s' from serialization data, for fluid type '%s'.",
			             nodeTypeName.c_str(), fluidTypeName.c_str());
			
			// If both types are valid, write to the actual storage; otherwise, use dummy storage
			FluidFlowTypeData  dummyTypeData;
			FluidFlowTypeData& typeData((isValidFluidType(fluidType) && isValidNodeType(nodeType)) ?
				m_fluidFlowTypeData[fluidType][nodeType] :
				dummyTypeData);
			
			typeData.timeTillNextUpdate = bu::get<real>(&context);
			
			const u32 toFlowListSize = bu::get<u32>(&context);
			typeData.toFlowList.reserve(static_cast<Point2s::size_type>(toFlowListSize));
			for (u32 pointIndex = 0; pointIndex < toFlowListSize; ++pointIndex)
			{
				typeData.toFlowList.push_back(bu::get<tt::math::Point2>(&context));
			}
		}
	}
	
	// Warp pairs
	m_warpPairs.clear();
	
	const u32 warpPairCount = bu::get<u32>(&context);
	for (u32 i = 0; i < warpPairCount; ++i)
	{
		const WarpID warpID(bu::get<WarpID::ValueType>(&context));
		
		WarpPair warpPair;
		warpPair.isValid   = true;
		warpPair.warpRect1 = bu::get<tt::math::PointRect>(&context);
		warpPair.warpRect2 = bu::get<tt::math::PointRect>(&context);
		
		m_warpPairs[warpID] = warpPair;
	}
	
	// Fluid Falls
	m_falls.clear();
	m_falls.resize(layerWidth);
	for(s32 i = 0; i < layerWidth; ++i)
	{
		// Number of falls in this column
		s32 fallCount = bu::get<s32>(&context);

		for(s32 j = 0; j < fallCount; ++j)
		{
			Fall fall;

			// Unserialize fall
			fall.area   = bu::get<tt::math::VectorRect>(&context);
			fall.type   = static_cast<FluidType>(bu::get<u8>(&context));
			fall.warpID = WarpID(bu::get<WarpID::ValueType>(&context));

			fall.startTile = bu::get<tt::math::Point2>(&context);
			fall.growTile  = bu::get<tt::math::Point2>(&context);
			fall.state     = static_cast<FlowState>(bu::get<u8>(&context));
			fall.fallType  = static_cast<FallType>(bu::get<u8>(&context));

			m_falls[i].push_back(fall);
		}
	}

	// Fluid Flows
	m_flows.clear();
	m_flows.resize(layerHeight);
	m_feedPoints.clear();
	m_feedPoints.resize(layerHeight);

	for(s32 i = 0; i < layerHeight; ++i)
	{
		// Number of flows in this row
		s32 flowCount = bu::get<s32>(&context);

		for(s32 j = 0; j < flowCount; ++j)
		{
			// Unserialize flow
			tt::math::VectorRect area = bu::get<tt::math::VectorRect>(&context);
			FluidType type            = static_cast<FluidType>(bu::get<u8>(&context));

			Flow flow(area, type);
			flow.warpID = WarpID(bu::get<WarpID::ValueType>(&context));

			flow.growLeft  = bu::get<tt::math::Point2>(&context);
			flow.growRight = bu::get<tt::math::Point2>(&context);
			flow.stateLeft  = static_cast<FlowState>(bu::get<u8>(&context));
			flow.stateRight = static_cast<FlowState>(bu::get<u8>(&context));
			flow.feedPointCount = bu::get<s32>(&context);
			flow.currentLeft  = bu::get<s32>(&context);
			flow.currentRight = bu::get<s32>(&context);

			m_flows[i].push_back(flow);
		}

		s32 feedPointCount = bu::get<s32>(&context);
		for(s32 j = 0; j < feedPointCount; ++j)
		{
			m_feedPoints[i].push_back(bu::get<s32>(&context));
		}
	}

	m_warpTiles.clear();
	s32 warpTileCount = bu::get<s32>(&context);
	for(s32 i = 0; i < warpTileCount; ++i)
	{
		m_warpTiles.insert(bu::get<tt::math::Point2>(&context));
	}

	// Recreate source and still tile containers
	collectSourceTiles();
	
	{
		clearAllEntityFluidResources();
		
		// (Both EntityMgr and FluidMgr need to be done unserializing at this point.
		// This is needed to refill m_entityFluidResources
		
		const entity::Entity* entity = p_entityMgr.getFirstEntity();
		for (s32 i = 0; i < p_entityMgr.getActiveEntitiesCount(); ++i, ++entity)
		{
			TT_NULL_ASSERT(entity);
			if (entity->shouldUpdateSurvey()) // Can't use getSurvey if we didn't update it.
			{
				const fluid::FluidTypes& fluidTouching     = entity->getSurvey().getTouchFluidTypes();
				const fluid::FluidTypes& waterfallTouching = entity->getSurvey().getTouchWaterfallTypes();
				for (s32 fluid = 0; fluid < fluid::FluidType_Count; ++fluid)
				{
					fluid::FluidType fluidType = static_cast<fluid::FluidType>(fluid);
					
					if (fluidTouching.checkFlag(fluidType))
					{
						onEntityEntersImpl(*entity, fluid::EntityFluidEffectType_Surface, fluidType, false);
					}
					if (waterfallTouching.checkFlag(fluidType))
					{
						onEntityEntersImpl(*entity, fluid::EntityFluidEffectType_Fall, fluidType, false);
					}
				}
			}
		}
	}
	
	// FIXME: After loading all data, ensure the generated data is updated
	m_useSimulationLayer = false;
	//simulate();
}


void FluidMgr::handleLevelResized()
{
	// recreate fluid layers
	m_activeLayer     = level::AttributeLayer::create(m_levelLayer->getWidth(), m_levelLayer->getHeight());
	m_simulationLayer = level::AttributeLayer::create(m_levelLayer->getWidth(), m_levelLayer->getHeight());
	m_activeLayer->clear();
	m_simulationLayer->clear();
	
	// re collect the source tiles in the level
	collectSourceTiles();
	
	m_changedTiles.clear();
	m_dirty = true;
	
	m_graphicsMgr.handleLevelResized();
	
	resetFluidSimulation(false);
}


void FluidMgr::setFluidLayerVisible(bool p_visible) 
{
#if defined(TT_PLATFORM_WIN)
	m_activeView->setVisible(p_visible);
	m_simulationView->setVisible(p_visible);
#else
	(void)p_visible;
#endif
}


void FluidMgr::fillSurvey(movement::SurroundingsSurvey& p_survey,
                          const tt::math::PointRect&    p_entityRegisteredTileRect,
                          const entity::Entity&         p_entity) const
{
	FluidTypes notFallingTouch;
	FluidType  notFallingInside;
	FluidTypes fallTouch;  // Waterfall etc
	FluidType  fallInside; // Waterfall etc
	
	// Determine "inside"
	getFluidTypes(p_survey.getSnappedTiles().getMin(),
	              p_survey.getSnappedTiles().getMaxInside(),
	              notFallingTouch,
	              notFallingInside,
	              fallTouch,
	              fallInside);
	
	p_survey.setInsideFluidTypeForAllTiles(notFallingInside);
	p_survey.setInsideWaterfallTypeForAllTiles(fallInside);
	
	// Determine "touch" (uses different rect, hence the second call)
	getFluidTypes(p_entityRegisteredTileRect.getMin(),
	              p_entityRegisteredTileRect.getMaxInside(),
	              notFallingTouch,
	              notFallingInside,
	              fallTouch,
	              fallInside);
	
	p_survey.setTouchFluidTypes(    notFallingTouch);
	p_survey.setTouchWaterfallTypes(fallTouch);
	
	// Use the touch fluid types set above to determine if we're in water
	
	movement::SurveyResult flowDirection = movement::SurveyResult_Invalid;
	const movement::SurveyResult onWaterResult = checkForSurveyOnWater(
			p_survey, p_entityRegisteredTileRect, p_entity, &flowDirection, FluidType_Water);
	p_survey.setOnWaterResult(onWaterResult);
	p_survey.setWaterDirection(flowDirection);
	
	movement::SurveyResult onLavaResult = checkForSurveyOnWater(
			p_survey, p_entityRegisteredTileRect, p_entity, &flowDirection, FluidType_Lava);
	
	// Translate the water results to lava.
	switch(onLavaResult)
	{
	case movement::SurveyResult_OnWaterLeft:      onLavaResult = movement::SurveyResult_OnLavaLeft;      break;
	case movement::SurveyResult_OnWaterRight:     onLavaResult = movement::SurveyResult_OnLavaRight;     break;
	case movement::SurveyResult_OnWaterStatic:    onLavaResult = movement::SurveyResult_OnLavaStatic;    break;
	case movement::SurveyResult_NotInWater:       onLavaResult = movement::SurveyResult_NotInLava;       break;
	case movement::SurveyResult_SubmergedInWater: onLavaResult = movement::SurveyResult_SubmergedInLava; break;
	default:
		TT_PANIC("Unknown water result: %d, can't translate to lava result!\n");
		break;
	}
	p_survey.setOnLavaResult(onLavaResult);
}


void FluidMgr::updateEntityEffects()
{
	for (s32 fluidEffect = 0; fluidEffect < EntityFluidEffectType_Count; ++fluidEffect)
	{
		EntityFluidEffectType effectType = static_cast<EntityFluidEffectType>(fluidEffect);
		for (s32 fluid = 0; fluid < FluidType_Count; ++fluid)
		{
			FluidType fluidType = static_cast<FluidType>(fluid);
			EntityToFluidResource& entitiesToResource = modifyEntityFluidResource(effectType, fluidType);
			for (EntityToFluidResource::iterator it = entitiesToResource.begin();
			     it != entitiesToResource.end();)
			{
				const entity::EntityHandle& handle   = (*it).first;
				const entity::Entity* entity = handle.getPtr();
				if (entity == 0 || entity->hasFluidSettings(fluidType) == false)
				{
					EntityToFluidResource::iterator copyIt = it;
					++it;
					entitiesToResource.erase(copyIt);
					continue;
				}
				EntityFluidResource& resource = (*it).second;
				
				bool updatedRects = false;
				if (resource.rectsDirty)
				{
					// Check if we really need to update rects.
					if (resource.needsParticleUpdate ||
					    resource.needsWaveUpdate)
					{
						// Update rects for this type.
						resource.tileRects = updateEntityEffectsRects(*entity, effectType, fluidType);
						updatedRects       = true;
					}
					else
					{
						// We expect the rects to have never been added.
						TT_ASSERT(resource.tileRects.empty());
					}
					resource.rectsDirty = false;
				}
				
				if (resource.needsParticleUpdate)
				{
					ParticleEffects::iterator particleIt = resource.particleEffects.begin();
					for (PointRects::iterator rectIt = resource.tileRects.begin(); rectIt != resource.tileRects.end(); ++rectIt)
					{
						tt::math::VectorRect intersectionRect;
						const tt::math::VectorRect& entityWorldRect(entity->getWorldRect());
						const real entityRectWidth = entityWorldRect.getWidth();
						tt::math::Vector2 worldPos = entityWorldRect.getPosition();
						worldPos.x += (entityWorldRect.getWidth() - entityRectWidth) * 0.5f;
						const tt::math::VectorRect entityRect(worldPos,
						                                      entityRectWidth,
						                                      entityWorldRect.getHeight());
						const bool intersection = entityRect.intersects(level::tileToWorld(*rectIt), &intersectionRect);
						if (intersection)
						{
							// Update rect of particleIt ;
							using tt::math::Vector2;
							using tt::math::Vector3;
							
							const Vector2 pos2D(intersectionRect.getCenterPosition());
							const Vector3 pos3D(pos2D, 0.0f);
							const real    width = intersectionRect.getWidth();
							
							tt::engine::particles::ParticleEffectPtr effect;
							bool newEffect = false;
							if (particleIt != resource.particleEffects.end())
							{
								effect = (*particleIt);
								++particleIt;
							}
							else
							{
								FluidSettingsPtr settings = entity->getFluidSettings(fluidType);
								TT_NULL_ASSERT(settings);
								if (settings != 0)
								{
									effect = tt::engine::particles::ParticleMgr::getInstance()->createEffect(
									         settings->getParticles(effectType).triggerFileName,
									         pos3D);
								}
								
								if (effect == 0)
								{
									continue;
								}
								newEffect = true;
								resource.particleEffects.push_back(effect);
								particleIt = resource.particleEffects.end(); // Reset iterator because it's invalid after the push_back.
								resource.originalParticleCount.clear();
								
								tt::engine::particles::ParticleTrigger* trigger = effect->getTrigger();
								TT_NULL_ASSERT(trigger);
								
								resource.originalParticleCount.reserve(trigger->getEmitterCount());
								for (int i = 0; i < trigger->getEmitterCount(); ++i)
								{
									tt::engine::particles::EmitterSettings& emitterSettings
										= trigger->getEmitter(i)->getSettings();
									
									resource.originalParticleCount.push_back(emitterSettings.emission.particles);
									
									TT_ASSERTMSG(emitterSettings.emission.area_type ==
									             tt::engine::particles::EmissionBehavior::AreaType_Rectangle,
									             "Expecte particles with rectangle as areatype! (File: '%s')",
									             settings->getParticles(effectType).triggerFileName.c_str());
								}
							}
							
							TT_NULL_ASSERT(effect);
							
							// set the emission rect width and position of all the emitters of the trigger
							tt::engine::particles::ParticleTrigger* trigger = effect->getTrigger();
							TT_NULL_ASSERT(trigger);
							
							tt::engine::particles::Range<real> rectWidth;
							const real halfWidth = width * 0.5f;
							rectWidth.high =  halfWidth;
							rectWidth.low  = -halfWidth;
							
							for (int i = 0; i < trigger->getEmitterCount(); ++i)
							{
								tt::engine::particles::EmitterSettings& emitterSettings
									= trigger->getEmitter(i)->getSettings();
								
								emitterSettings.emission.rect_width = rectWidth;
								emitterSettings.emission.particles  = resource.originalParticleCount[i] * width;
								trigger->getEmitter(i)->setPosition(pos2D);
							}
							
							// If it is a new effect make sure to spawn it
							if (newEffect)
							{
								effect->spawn();
							}
						}
					}
					// Didn't use the remaining particle effects.
					while (particleIt != resource.particleEffects.end())
					{
						(*particleIt)->stop();
						particleIt = resource.particleEffects.erase(particleIt);
					}
				}
				
				if (resource.needsWaveUpdate &&
				    updatedRects)
				{
					PointRects::iterator waveRectIt = resource.prevTileRectsWaves.begin();
					for (PointRects::iterator rectIt = resource.tileRects.begin(); rectIt != resource.tileRects.end(); ++rectIt)
					{
						const tt::math::PointRect& tileRect = (*rectIt);
						if (waveRectIt != resource.prevTileRectsWaves.end())
						{
							tt::math::PointRect& prevTileRect = (*waveRectIt);
							if (tileRect != prevTileRect)
							{
								startEntityWave(tileRect, &prevTileRect, *entity, fluidType);
								prevTileRect = tileRect;
							}
						}
						else
						{
							startEntityWave(tileRect, 0, *entity, fluidType);
							
							resource.prevTileRectsWaves.push_back(tileRect);
							waveRectIt = resource.prevTileRectsWaves.end();
						}
					}
				}
				
				++it;
			}
		}
	}
}


void FluidMgr::onEntityEntersFluid(const entity::Entity& p_entity, FluidType p_type)
{
		const fluid::FluidSettingsPtr fluidSettings = p_entity.getFluidSettings(p_type);
	if (fluidSettings == 0)
	{
		return;
	}
	
	// Handle surface particlese
	onEntityEntersImpl(p_entity, EntityFluidEffectType_Surface, p_type, true);
	
	// Play enter sound
	if (fluidSettings->enterSound.empty() == false)
	{
		toki::audio::AudioPlayer::getInstance()->playPositionalEffectCue("Effects",
				fluidSettings->enterSound, p_entity.getHandle());
	}
	
	// Check for splash
	if (fluidSettings->enterParticleEffect.empty() == false)
	{
		tt::engine::particles::ParticleEffectPtr effect = 
			p_entity.spawnParticle(entity::Entity::SpawnType_OneShot, fluidSettings->enterParticleEffect,
				p_entity.getPosition(), false, 0.0f, true,
				game::ParticleLayer_UseLayerFromParticleEffect);
		
		if (effect != 0 && effect->getTrigger() != 0)
		{
			const tt::math::VectorRect& collisionRect(p_entity.getCollisionRect());
			const real scale = collisionRect.getWidth();
			tt::engine::particles::Range<real> rectWidth;
			const real halfWidth = scale * 0.5f;
			rectWidth.high =  halfWidth;
			rectWidth.low  = -halfWidth;
			
			// Update emission rectangle and total particles of all emitters
			tt::engine::particles::ParticleTrigger* trigger = effect->getTrigger();
			const s32 count = trigger->getEmitterCount();
			for (s32 i = 0; i < count; ++i)
			{
				tt::engine::particles::ParticleEmitter* emitter = trigger->getEmitter(i);
				tt::engine::particles::EmitterSettings& emitterSettings =  emitter->getSettings();
				emitterSettings.emission.rect_width = rectWidth;
				emitterSettings.emission.particles *= scale;
				emitter->changeMaxParticles(static_cast<s32>(emitterSettings.emission.particles));
			}
		}
	}
}


void FluidMgr::onEntityExitsFluid(const entity::Entity& p_entity, FluidType p_type)
{
	const fluid::FluidSettingsPtr fluidSettings = p_entity.getFluidSettings(p_type);
	if (fluidSettings == 0)
	{
		return;
	}
	
	if (fluidSettings->exitSound.empty() == false)
	{
		toki::audio::AudioPlayer::getInstance()->playPositionalEffectCue("Effects",
				fluidSettings->exitSound, p_entity.getHandle());
	}
		
	onEntityExitsImpl(p_entity, EntityFluidEffectType_Surface, p_type);
	
	// Check for splash
	// FIXME: Code duplication with onEntityEntersFluid
	if (fluidSettings->exitParticleEffect.empty() == false)
	{
		tt::engine::particles::ParticleEffectPtr effect = 
			p_entity.spawnParticle(entity::Entity::SpawnType_OneShot, fluidSettings->exitParticleEffect,
				tt::math::Vector2::zero, true, 0.0f, false,
				game::ParticleLayer_UseLayerFromParticleEffect);
		
		if (effect != 0 && effect->getTrigger() != 0)
		{
			const tt::math::VectorRect& collisionRect(p_entity.getCollisionRect());
			const real scale = collisionRect.getWidth();
			tt::engine::particles::Range<real> rectWidth;
			const real halfWidth = scale * 0.5f;
			rectWidth.high =  halfWidth;
			rectWidth.low  = -halfWidth;
			
			// Update emission rectangle and total particles of all emitters
			tt::engine::particles::ParticleTrigger* trigger = effect->getTrigger();
			const s32 count = trigger->getEmitterCount();
			for (s32 i = 0; i < count; ++i)
			{
				tt::engine::particles::ParticleEmitter* emitter = trigger->getEmitter(i);
				tt::engine::particles::EmitterSettings& emitterSettings =  emitter->getSettings();
				emitterSettings.emission.rect_width = rectWidth;
				emitterSettings.emission.particles *= scale;
				emitter->changeMaxParticles(static_cast<s32>(emitterSettings.emission.particles));
			}
		}
	}
}


void FluidMgr::onEntityEntersFall(const entity::Entity& p_entity, FluidType p_type)
{
	onEntityEntersImpl(p_entity, EntityFluidEffectType_Fall, p_type, false);
}


void FluidMgr::onEntityExitsFall(const entity::Entity& p_entity, FluidType p_type)
{
	onEntityExitsImpl(p_entity, EntityFluidEffectType_Fall, p_type);
}


void FluidMgr::scheduleEntityEffectsRectsUpdate(const entity::Entity& p_entity,
                                                EntityFluidEffectType p_entityFluidEffectType,
                                                FluidType             p_fluidType)
{
	EntityToFluidResource& entityResources = modifyEntityFluidResource(p_entityFluidEffectType, p_fluidType);
	
	EntityToFluidResource::iterator it = entityResources.find(p_entity.getHandle());
	if (it != entityResources.end())
	{
		(*it).second.rectsDirty = true;
	}
}


void FluidMgr::addFluidWarp(const tt::math::VectorRect& p_rect, const std::string& p_name)
{
	WarpID warpID(p_name);
	
	WarpPairs::iterator pairIt(m_warpPairs.find(warpID));
	
	if (pairIt == m_warpPairs.end())
	{
		m_warpPairs[warpID].warpRect1 = level::worldToTile(p_rect);
	}
	else if (pairIt->second.isValid == false)
	{
		pairIt->second.warpRect2 = level::worldToTile(p_rect);
		
		if (pairIt->second.warpRect2 == pairIt->second.warpRect1)
		{
			return;
		}
		
		const tt::math::PointRect& rect1 = pairIt->second.warpRect1;
		const tt::math::PointRect& rect2 = pairIt->second.warpRect2;
		if (rect1.getHeight() != rect2.getHeight() ||
		    rect1.getWidth()  != rect2.getWidth())
		{
			TT_PANIC("Second fluid warp rect, named '%s' [%d,%d], is not the same size as the first rect [%d,%d].",
			         p_name.c_str(), rect2.getWidth(), rect2.getHeight(), rect1.getWidth(), rect1.getHeight());
			m_warpPairs.erase(pairIt);
			return;
		}
		
		pairIt->second.isValid = true;
		
		// add the tiles to the changed list
		addTileRectToChangedList(pairIt->second.warpRect1);
		addTileRectToChangedList(pairIt->second.warpRect2);
		
		setWarpTiles(pairIt->second.warpRect1, m_activeLayer, true);
		setWarpTiles(pairIt->second.warpRect2, m_activeLayer, true);
	}
	else
	{
		TT_PANIC("More than two fluid warps with the same name '%s' defined", p_name.c_str());
	}
}


void FluidMgr::deleteFluidWarpPair(const std::string& p_name)
{
	WarpPairs::iterator pairIt(m_warpPairs.find(WarpID(p_name)));
	
	if (pairIt != m_warpPairs.end() && pairIt->second.isValid)
	{
		// add the tiles to the changed list
		addTileRectToChangedList(pairIt->second.warpRect1);
		addTileRectToChangedList(pairIt->second.warpRect2);
		
		setWarpTiles(pairIt->second.warpRect1, m_activeLayer, false);
		setWarpTiles(pairIt->second.warpRect2, m_activeLayer, false);
		
		m_warpPairs.erase(pairIt);
	}
}


void FluidMgr::clearAllEntityFluidResources()
{
	for (s32 effect = 0; effect < EntityFluidEffectType_Count; ++effect)
	{
		for (s32 fluid = 0; fluid < FluidType_Count; ++fluid)
		{
			m_entityFluidResources[effect][fluid].clear();
		}
	}
}


void FluidMgr::onEntityEntersImpl(const entity::Entity& p_entity, EntityFluidEffectType p_effectType,
                                  FluidType p_fluidType, const bool p_breakingSurface)
{
	FluidSettingsPtr settings = p_entity.getFluidSettings(p_fluidType);
	if (settings == 0)
	{
		return;
	}
	
	// Start a sound
	tt::audio::player::SoundCuePtr cue;
	const std::string& soundStr = settings->getSound(p_effectType);
	if (soundStr.empty() == false)
	{
		cue = audio::AudioPlayer::getInstance()->playPositionalEffectCue("Effects", soundStr, p_entity.getHandle());
	}
	
	// We only add this entity to resources if we need to update it's particles and/or if it has a cue.
	bool needsParticleUpdate = settings->getParticles(p_effectType).enabled && 
	                           settings->getParticles(p_effectType).triggerFileName.empty() == false;
	
	if (cue != 0 || needsParticleUpdate || settings->waveGenerationEnabled)
	{
		EntityToFluidResource& entityResources = modifyEntityFluidResource(p_effectType, p_fluidType);
		EntityFluidResource& resource = entityResources[p_entity.getHandle()];
		TT_ASSERT(resource.soundCue == 0); // We expect this to be freshly created. If not we might need to remove/stop the old stuff first.
		
		resource.soundCue            = cue;
		resource.needsParticleUpdate = needsParticleUpdate;
		resource.needsWaveUpdate     = settings->waveGenerationEnabled;
		
		if (p_effectType      == EntityFluidEffectType_Surface &&
		    p_breakingSurface == false) // Suppress initial wave.
		{
			resource.tileRects          = updateEntityEffectsRects(p_entity, p_effectType, p_fluidType);
			resource.prevTileRectsWaves = resource.tileRects;
		}
	}
}


void FluidMgr::onEntityExitsImpl(const entity::Entity& p_entity, EntityFluidEffectType p_effectType,
                                 FluidType p_fluidType)
{
	EntityToFluidResource& entityResources = modifyEntityFluidResource(p_effectType, p_fluidType);
	
	EntityToFluidResource::iterator it = entityResources.find(p_entity.getHandle());
	
	if (it != entityResources.end())
	{
		EntityFluidResource& resource = (*it).second;
		if (resource.soundCue != 0)
		{
			resource.soundCue->stop();
		}
		for (ParticleEffects::iterator particleIt = resource.particleEffects.begin(); particleIt != resource.particleEffects.end(); ++particleIt)
		{
			(*particleIt)->stop();
		}
		entityResources.erase(it);
	}
}


FluidMgr::PointRects FluidMgr::updateEntityEffectsRects(const entity::Entity& p_entity,
                                                        EntityFluidEffectType p_effectType,
                                                        FluidType             p_fluidType)
{
	using namespace tt::math;
	const PointRect entityTileRect = p_entity.getRegisteredTileRect();
	      Point2    minPos         = entityTileRect.getMin();
	      Point2    maxPos         = entityTileRect.getMaxInside();
	
	switch (p_effectType)
	{
	case EntityFluidEffectType_Surface:
		// We need to check the tile above which needs to be empty.
		maxPos.y += 1;
		break;
	case EntityFluidEffectType_Fall:
		// For falls we're only interessted in the top layer of tiles.
		minPos.y = maxPos.y;
		break;
	default:
		break;
	}
	
	PointRects result;
	
	const s32 width = m_activeLayer->getWidth();
	// Skip invalid positions
	if (minPos.x < 0      || minPos.y < 0                        ||
	    maxPos.x >= width || maxPos.y >= m_activeLayer->getHeight())
	{
		return result;
	}
	
	bool tileRectStarted = false;
	s32  tileRectStartPos = -1;
	s32  tileRectEndPos   = -1;
	
	// Step through each tile and check collision type
	const u8* attributes = m_activeLayer->getRawData();
	const u8* columnPtr  = &attributes[(minPos.y * width) + minPos.x];
	for (tt::math::Point2 tilePos = minPos; tilePos.y <= maxPos.y; ++tilePos.y, columnPtr += width)
	{
		const u8* rowPtr = columnPtr;
		for (tilePos.x = minPos.x; tilePos.x <= maxPos.x; ++tilePos.x, ++rowPtr)
		{
			const FluidFlowType flowType  = getFluidFlowType(*rowPtr);
			const FluidType     fluidType = getFluidType(    *rowPtr);
			
#if defined(TT_BUILD_DEV)
			TT_ASSERTMSG(rowPtr == &attributes[(tilePos.y * width) + tilePos.x],
			             "rowPtr(%p) isn't where it should be %p for tile x: %d, y: %d",
			             rowPtr, &attributes[(tilePos.y * width) + tilePos.x], tilePos.x, tilePos.y);
			TT_ASSERT(flowType  == m_activeLayer->getFluidFlowType(tilePos));
			TT_ASSERT(fluidType == m_activeLayer->getFluidType(    tilePos));
#endif
			
			if (fluidType == p_fluidType)
			{
				bool effectFound = false;
				switch (p_effectType)
				{
				case EntityFluidEffectType_Surface:
					// We want to do another check for still fluids.
					// or it was none. (Maybe it's because we're a hermit and we're overriding the tiles.)
					if (isStill(flowType) || flowType == FluidFlowType_None)
					{
						effectFound = (m_stillTilesSurface.find(tilePos) != m_stillTilesSurface.end());
					}
					else
					{
						TT_ASSERT(flowType != FluidFlowType_None); // Was already checked above.
						effectFound = isFalling(flowType) == false; // Fall is not a valid surface type
					}
					break;
				case EntityFluidEffectType_Fall:
					effectFound = isFall(flowType);
					break;
				default:
					TT_PANIC("Unknown EntityFluidEffectType: %d\n", p_effectType);
					continue;
				}
				
				if (effectFound)
				{
					// Found something
					if (tileRectStarted)
					{
						tileRectEndPos = tilePos.x;
					}
					else
					{
						tileRectStarted  = true;
						tileRectStartPos = tilePos.x;
						tileRectEndPos   = tilePos.x;
					}
					continue;
				}
			}
			// Nothing found on this tile.
			if (tileRectStarted)
			{
				tileRectStarted = false;
				result.push_back(PointRect(Point2(tileRectStartPos, tilePos.y),
				                           Point2(tileRectEndPos  , tilePos.y)));
			}
		}
		// End of the row. (We don't bother checking if we can merge rects.)
		if (tileRectStarted)
		{
			tileRectStarted = false;
			result.push_back(PointRect(Point2(tileRectStartPos, tilePos.y),
			                           Point2(tileRectEndPos  , tilePos.y)));
		}
	}
	
	if (p_effectType == EntityFluidEffectType_Surface &&
	    result.empty() == false)
	{
		// HACK-ish.
		// We (mis)use the fact that we didn't merge the rects.
		// We should get the different rows in which we found surface tiles.
		// We only want surface particles on the surface (no fluid tiles above)
		// So we check to make sure we didn't find any in the top most row.
		// We also only return 1 rect.
		
		PointRect topRect = result.back();
		result.clear();
		
		if (topRect.getMaxInside().y < maxPos.y)
		{
			result.push_back(topRect);
			return result;
		}
	}
	
	return result;
}


void FluidMgr::resetLevel()
{
	// raise flagse
	m_dirty       = true;
	m_preGenerate = true;
	
	// clear warp pairs
	m_warpPairs.clear();
	m_warpTiles.clear();
	
	m_notifiedEntities.clear();
	
	// clear the active layer
	m_activeLayer->clear();
	
	// (re)set timings
	for (s32 fluidType = 0; fluidType < FluidType_Count; ++fluidType)
	{
		for (s32 nodeType = 0; nodeType < NodeType_Count; ++nodeType)
		{
			m_fluidFlowTypeData[fluidType][nodeType].timeTillNextUpdate = 
					getFlowTime(static_cast<FluidType>(fluidType), 
					            static_cast<NodeType>(  nodeType));
		}
	}
	
	m_soundCues.clear();
	m_soundCuesForFalls.clear();
	
	m_particlesMgr.reset();
	
	clearAllEntityFluidResources();
	
	updateFlowTimes();
	
	addStillWater();
	
	// FIXME: I think more of the members should be reset here. (Please check if we're missing some here.)
}


tt::math::Point2 FluidMgr::getWarpedPosition(const tt::math::Point2& p_startPos)
{
	for (WarpPairs::iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if(it->second.warpRect1.contains(p_startPos))
		{
			return p_startPos + it->second.warpRect2.getPosition() - it->second.warpRect1.getPosition();
		}
		if(it->second.warpRect2.contains(p_startPos))
		{
			return p_startPos + it->second.warpRect1.getPosition() - it->second.warpRect2.getPosition();
		}
	}
	TT_PANIC("Position has no warp pair");
	return tt::math::Point2::zero;
}


bool FluidMgr::isSameWarp(const tt::math::Point2& p_pos1, const tt::math::Point2& p_pos2)
{
	for (WarpPairs::iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if(it->second.warpRect1.contains(p_pos1))
		{
			return it->second.warpRect1.contains(p_pos2);
		}
		if(it->second.warpRect2.contains(p_pos1))
		{
			return it->second.warpRect2.contains(p_pos2);
		}
	}
	return false;
}


void FluidMgr::notifyTileChange(const tt::math::Point2& p_position)
{
	const entity::EntityHandleSet& entities = m_tileRegMgr.getRegisteredEntityHandles(p_position);
	
	m_notifiedEntities.insert(entities.begin(), entities.end());
		
	tt::math::Point2 tileAbove(p_position.x, p_position.y + 1);
	// Notify entites floating on the surface
	if (m_tileRegMgr.contains(tileAbove))
	{
		const entity::EntityHandleSet& surfaceEntities = m_tileRegMgr.getRegisteredEntityHandles(tileAbove);
		m_notifiedEntities.insert(surfaceEntities.begin(), surfaceEntities.end());
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

FluidMgr::FluidMgr(const level::AttributeLayerPtr& p_levelLayer)
:
m_levelLayer(p_levelLayer),
m_activeLayer(level::AttributeLayer::create(p_levelLayer->getWidth(),
                                            p_levelLayer->getHeight())),
m_simulationLayer(level::AttributeLayer::create(p_levelLayer->getWidth(),
                                                p_levelLayer->getHeight())),
#if defined(TT_PLATFORM_WIN)
m_activeView    (AttributeDebugView::create(m_activeLayer,     AttributeDebugView::ViewMode_Fluids)),
m_simulationView(AttributeDebugView::create(m_simulationLayer, AttributeDebugView::ViewMode_Fluids)),
#endif
m_fluidFlowTypeData(),
m_flowTypeTimings(),
//m_flowTypeTimes(),
m_dirty(true),
m_preGenerate(true),
m_checkActiveLayerType(false),
m_useSimulationLayer(false),
m_activeFluidType(FluidType_Invalid),
m_currentNodeType(NodeType_Invalid),
m_currentPosition(),
m_changedTiles(),
m_sourceTiles(),
m_stillTiles(),
m_toFlowScratch(),
m_toFlowActiveLayerScratch(),
m_soundCues(),
m_soundCuesForFalls(),
m_warpPairs(),
m_notifiedEntities(),
m_graphicsMgr(),
m_particlesMgr(),
m_tileRegMgr(AppGlobal::getGame()->getTileRegistrationMgr()),
m_sectionProfiler("FluidMgr - update")
{
	m_activeLayer->clear();
	m_simulationLayer->clear();
	
	// Get the interval settings
	m_flowTypeTimings[FluidType_Water][NodeType_Fall ] = cfg()->getHandleReal("toki.fluids.water.grow_interval.fall_single");
	m_flowTypeTimings[FluidType_Water][NodeType_Left ] = cfg()->getHandleReal("toki.fluids.water.grow_interval.left");
	m_flowTypeTimings[FluidType_Water][NodeType_Right] = cfg()->getHandleReal("toki.fluids.water.grow_interval.right");
	
	m_flowTypeTimings[FluidType_Lava][NodeType_Fall ]  = cfg()->getHandleReal("toki.fluids.lava.grow_interval.fall_single");
	m_flowTypeTimings[FluidType_Lava][NodeType_Left ]  = cfg()->getHandleReal("toki.fluids.lava.grow_interval.left");
	m_flowTypeTimings[FluidType_Lava][NodeType_Right]  = cfg()->getHandleReal("toki.fluids.lava.grow_interval.right");
	m_flowTypeTimings[FluidType_Lava][NodeType_Lvl2 ]  = cfg()->getHandleReal("toki.fluids.lava.grow_interval.lvl2");
	
	updateFlowTimes();
	
	collectSourceTiles();
}


void FluidMgr::collectSourceTiles()
{
	m_sourceTiles.clear();
	m_stillTiles.clear();
	
	tt::math::Point2 pos(0, 0);
	for (pos.y = 0; pos.y < m_levelLayer->getHeight(); ++pos.y)
	{
		for (pos.x = 0; pos.x < m_levelLayer->getWidth(); ++pos.x)
		{
			const level::CollisionType collisionType = m_levelLayer->getCollisionType(pos);
			if (level::isFluidSource(collisionType))
			{
				m_sourceTiles.insert(pos);
			}
			else if (level::isFluidStill(collisionType))
			{
				m_stillTiles.insert(pos);
			}
		}
	}
}


void FluidMgr::simulate()
{
	/*
	using namespace utils;
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_SimulateMisc);
	m_checkActiveLayerType = (m_preGenerate == false);
	m_useSimulationLayer   = true;
	m_simulationLayer->clear();
	
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_SimulateSetWarpTiles);
	setAllWarpTiles(m_simulationLayer);

	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_SimulateFluids);

	// run simulation for each fluid type
	for (s32 fluidType = 0; fluidType < FluidType_Count; ++fluidType)
	{
		m_activeFluidType = static_cast<FluidType>(fluidType);
		real flowTimingsBackup[NodeType_Count];
		
		for (s32 i = 0; i < NodeType_Count; ++i)
		{
			// create timing backup to restore after simulation
			flowTimingsBackup[i] = m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate;
			
			
			// reset data for simulation
			m_fluidFlowTypeData[m_activeFluidType][i].toFlowList.clear();
			m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate = 
					getFlowTime(m_activeFluidType, static_cast<NodeType>(i));
		}
		
		// add the fluidSources in the level as source positions
		for (Point2Set::const_iterator it = m_sourceTiles.begin(); it != m_sourceTiles.end(); ++it)
		{
			// check if this source is the same fluidType as the current active type
			if (getFluidType(m_levelLayer->getCollisionType(*it)) != m_activeFluidType)
			{
				continue;
			}
			
			// Check if we have collision over the source. (Entity with collision moved over it.)
			if (m_tileRegMgr.isSolid(*it))
			{
				continue;
			}
			
			// place tile
			placeFluidTile(*it, FluidFlowType_Fall);
			
			static const NodeType fallType(getNodeType(FluidFlowType_Fall));

			// check if the flow should warp
			if (m_activeLayer->isWarpTile(*it))
			{
				addWarpedNode(*it, getNodeType(FluidFlowType_Fall));
			}
			else
			{
				addToToflowList(*it, fallType);
			}
		}
		
		// add the stillTiles
		for (Point2Set::iterator it = m_stillTiles.begin(); it != m_stillTiles.end(); ++it)
		{
			// check if this source is the same fluidType as the current active type
			if (getFluidType(m_levelLayer->getCollisionType(*it)) != m_activeFluidType)
			{
				continue;
			}
			
			// place tile
			m_simulationLayer->setFluidFlowType(*it, FluidFlowType_Still);
			m_simulationLayer->setFluidType    (*it, m_activeFluidType);
			
			// check if the flow should warp
			if (m_activeLayer->isWarpTile(*it))
			{
				addWarpedNode(*it, getNodeType(FluidFlowType_Fall));
			}
		}

		bool continueExpanding = true;
		while (continueExpanding)
		{
			// find smallest timeleft
			FluidFlowTypeData* smallestTime(&m_fluidFlowTypeData[m_activeFluidType][NodeType_Fall]);
		
			for (s32 i = 0; i < NodeType_Count; ++i)
			{
				if (m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate < smallestTime->timeTillNextUpdate)
				{
					smallestTime = &m_fluidFlowTypeData[m_activeFluidType][i];
				}
			}
			
			// Grow by this fluidtype's time
			updateGrowth(smallestTime->timeTillNextUpdate);
			
			
			continueExpanding = false;
			
			for (s32 i = 0; i < NodeType_Count; ++i)
			{
				// when the new list is empty the growing is finished
				if (m_fluidFlowTypeData[m_activeFluidType][i].toFlowList.empty() == false)
				{
					continueExpanding = true;
					break;
				}
			}
		}
		
		// after simulation add the removed nodes back
		for (s32 i = 0; i < NodeType_Count; ++i)
		{
			m_fluidFlowTypeData[m_activeFluidType][i].toFlowList.swap(
					m_toFlowActiveLayerScratch[m_activeFluidType][i]);
			m_toFlowActiveLayerScratch[m_activeFluidType][i].clear();
			
			// reset the timings to the backup
			m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate = flowTimingsBackup[i];
		}
	}
	m_activeFluidType = FluidType_Invalid;
	
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_SimulateDiffCheck);
	// Check difference between simulation and active layer for dead water tiles
	s32 height = m_activeLayer->getHeight();
	s32 width  = m_activeLayer->getWidth();
	const u8* activeBuffer     = m_activeLayer->getRawData();
	const u8* simulationBuffer = m_simulationLayer->getRawData();
	
	for (tt::math::Point2 pos(0, 0); pos.y < height; ++pos.y)
	{
		for (pos.x = 0; pos.x < width; ++pos.x, ++activeBuffer, ++simulationBuffer)
		{
			// Check for difference between active and simulation buffer
			if (*activeBuffer != *simulationBuffer)
			{
				// Notify entities of this change
				notifyTileChange(pos);
				notifyTileChange(tt::math::Point2(pos.x, pos.y + 1)); // Notify entites floating on the surface
				
				const FluidFlowType activeFlowType     = getFluidFlowType(*activeBuffer);
				const FluidFlowType simulationFlowType = getFluidFlowType(*simulationBuffer);
				
				if (isLvl2(activeFlowType) || isLvl2(simulationFlowType))
				{
					// In the case of level 2 changes, notify the entities on the tile below.
					notifyTileChange(tt::math::Point2(pos.x, pos.y - 1), false);
				}
				
				if (isIgnorableFlowType(activeFlowType) == false &&
					isIgnorableFlowType(simulationFlowType))
				{
					// Spawn removed fluid particle
					m_particlesMgr.triggerParticle(pos, FluidParticlesMgr::TileOrientation_CenterCenter,
					                               FluidParticlesMgr::ParticleType_RemovedTile,
					                               getFluidType(*activeBuffer));
					
					SoundCues::iterator it = m_soundCues.find(pos);
					if (it != m_soundCues.end())
					{
						TT_NULL_ASSERT(it->second);
						if (it->second != 0)
						{
							it->second->stop();
						}
						m_soundCues.erase(it);
					}
				}
			}
		}
	}
	
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_SimulateSwap);
	std::swap(m_simulationLayer, m_activeLayer);
	std::swap(m_simulationView,  m_activeView);
	
	m_sectionProfiler.startFrameUpdateSection(FluidMgrSection_SimulateMisc);
	m_preGenerate          = false;
	m_checkActiveLayerType = false;
	m_useSimulationLayer   = false;
	*/
}


void FluidMgr::updateGrowth(real p_deltaTime)
{
	TT_ASSERT(isValidFluidType(m_activeFluidType));
	
	for (s32 i = 0; i < NodeType_Count; ++i)
	{
		m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate -= p_deltaTime;
		while (m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate <= 0.0f)
		{
			const NodeType nodetype = static_cast<NodeType>(i);
			
			tickGrowth(nodetype);
			
			m_fluidFlowTypeData[m_activeFluidType][i].timeTillNextUpdate += 
					getFlowTime(m_activeFluidType, nodetype);
		}
	}
}


void FluidMgr::tickGrowth(NodeType)
{
	/*
	using namespace tt::math;
	
	TT_ASSERT(isValidFluidType(m_activeFluidType));
	
	Point2s& toFlowList(m_fluidFlowTypeData[m_activeFluidType][p_nodeType].toFlowList);
	
	// Setup new list to work with
	m_toFlowScratch.clear();
	m_toFlowScratch.insert(toFlowList.begin(), toFlowList.end());
	toFlowList.clear();
	
	m_currentNodeType = p_nodeType;
	
	for (Point2Set::iterator it = m_toFlowScratch.begin(); it != m_toFlowScratch.end() ; ++it)
	{
		// Assert there is no solid at this position, there should never be a solid.
		TT_ASSERT(m_tileRegMgr.isSolid(*it) == false);
		
		m_currentPosition = *it;
		
		switch (p_nodeType)
		{
		case NodeType_Fall:
			{
				const Point2        below((*it) - Point2::unitY);
				const FluidFlowType belowType = getFlowType(below);
				
				// stop on kill fluids
				if (m_levelLayer->contains(below) == false ||
				    g_fluidKillers.checkAnyFlags(m_tileRegMgr.getCollisionTypesFromRegisteredTiles(below, m_levelLayer)))
				{
					break;
				}
				
				if (m_tileRegMgr.isSolid(below) == false && isIgnorableFlowTypeOrOverflow(belowType))
				{
					// continue going down when not blocked
					placeFlow(FluidFlowType_Fall, below);
				}
				else if (isIgnorableFlowTypeOrOverflow(belowType) == false)
				{
					// when we hit a fall, don't continue because there already is a fall
					if (belowType == FluidFlowType_Fall) break;
					
					// Hit fluid on fall. Check if this is a basin(lvl2).
					if ((fillLvl2(below) || fillLvl2(below - Point2::unitY)) == false)
					{
						// no lvl2 here.
						switch (belowType)
						{
						case FluidFlowType_Left:
							// convert to still to the right
							for (Point2 marker(below); 
							     getFlowType(marker) == FluidFlowType_Left ||
							     getFlowType(marker) == FluidFlowType_LeftLvl2;
							     marker += Point2::right)
							{
								if (getFlowType(marker) == FluidFlowType_LeftLvl2)
								{
									fillLvl2(marker - Point2::unitY);
									break;
								}
								placeFluidTile(marker, FluidFlowType_Still);
							}
							break;
							
						case FluidFlowType_Right:
							// convert to still to the left
							for (Point2 marker(below); 
							     getFlowType(marker) == FluidFlowType_Right ||
							     getFlowType(marker) == FluidFlowType_RightLvl2;
							     marker += Point2::left)
							{
								if (getFlowType(marker) == FluidFlowType_RightLvl2)
								{
									fillLvl2(marker - Point2::unitY);
									break;
								}
								placeFluidTile(marker, FluidFlowType_Still);
							}
							break;
							
						default:
							break;
						}
						
						setTileBelowFall(below);
						
						// place left & right
						placeAdjacentFlow(below, NodeType_Left);
						placeAdjacentFlow(below, NodeType_Right);
					}
				}
				else // blocked by solid
				{
					// determine by surroundings what this tile should become
					setTileBelowFall(*it);
					
					// place left & right
					placeAdjacentFlow(*it, NodeType_Left);
					placeAdjacentFlow(*it, NodeType_Right);
				}
			}
			break;
			
		case NodeType_Left:
		case NodeType_Right:
			{
				
				Point2 adjacentDir(p_nodeType == NodeType_Left ? Point2::left : Point2::right);
				Point2 adjacent((*it) + adjacentDir);
				
				Point2 belowAdjacent(adjacent - Point2::unitY);
				Point2 below(*it - Point2::unitY);
				Point2 belowPrevAdjacent((*it) - adjacentDir - Point2::unitY);
				
				// stop on kill fluids
				if (m_levelLayer->contains(adjacent) == false ||
				    g_fluidKillers.checkAnyFlags(AppGlobal::getGame()->getTileRegistrationMgr().getCollisionTypesFromRegisteredTiles(adjacent, m_levelLayer)))
				{
					break;
				}
				
				// when not a fall (just a 1 tile gap) flow into it without stopping the sideways flow
				if (m_tileRegMgr.isSolid(below) == false &&
				    m_tileRegMgr.isSolid(belowPrevAdjacent) &&
				    (m_tileRegMgr.isSolid(adjacent) || // check if the next step would be a fall
				     m_activeLayer->isWarpTile(adjacent) || // case where a warp interupts an overflow
				     getFlowType(adjacent) == FluidFlowType_Fall || // case where a fall interupts an overflow
				     m_tileRegMgr.isSolid(belowAdjacent)))
				{
					placeFlow(FluidFlowType_Fall, below);
				}
				
				
				// Fall if there is no solid below this tile and below the previous tile
				if (m_tileRegMgr.isSolid(below)             == false &&
				    m_tileRegMgr.isSolid(belowPrevAdjacent) == false)
				{
					// create double down flow (Fall)
					Point2 leftBelow(p_nodeType == NodeType_Left ? below : belowPrevAdjacent);
					Point2 rightBelow(leftBelow + Point2::right);
					
					if (isIgnorableFlowTypeOrOverflow(getFlowType(leftBelow)))
					{
						// continue going down when not blocked
						placeFlow(FluidFlowType_Fall, leftBelow);
					}
					
					if (isIgnorableFlowTypeOrOverflow(getFlowType(rightBelow)))
					{
						// continue going down when not blocked
						placeFlow(FluidFlowType_Fall, rightBelow);
					}
					
					bool isLeftBelowFluid  = (isIgnorableFlowTypeOrOverflow(getFlowType(leftBelow))  == false);
					bool isrightBelowFluid = (isIgnorableFlowTypeOrOverflow(getFlowType(rightBelow)) == false);
					
					if (isLeftBelowFluid || isrightBelowFluid) // one off the falls is blocked by other fluids
					{
						fillLvl2(leftBelow);
						if (m_tileRegMgr.isSolid(leftBelow - Point2::unitY))  setTileBelowFall(leftBelow);
						if (m_tileRegMgr.isSolid(rightBelow - Point2::unitY)) setTileBelowFall(rightBelow);
						
					}
				}
				else
				{
					placeAdjacentFlow(*it, p_nodeType);
				}
			}
			break;
			
		case NodeType_Lvl2:
			{
				// The 2nd level needs to expand from the incoming flows.
				// lvl1 has already been filled
				
				Point2 leftEdge(m_currentPosition);
				
				// find the left edge of the bassin
				for (Point2 marker(m_currentPosition); m_tileRegMgr.isSolid(marker) == false;
				     marker += tt::math::Point2::left)
				{
					leftEdge = marker;
				}
				
				Point2    rightEdge(leftEdge);
				Point2Set incomingFlows;
				
				// go to the right edge of the bassin and find any incoming flows on the way
				for (Point2 marker(leftEdge);
				     m_tileRegMgr.isSolid(marker) == false; marker += tt::math::Point2::right)
				{
					rightEdge = marker;
					FluidFlowType flowTypeLvl2 = getFlowType(marker + Point2::unitY);
					if (isBassinInputType(flowTypeLvl2) ||
					    flowTypeLvl2 == FluidFlowType_Left || flowTypeLvl2 == FluidFlowType_Right ||
					    flowTypeLvl2 == FluidFlowType_LeftOverFlow || flowTypeLvl2 == FluidFlowType_RightOverFlow)
					{
						incomingFlows.insert(marker + Point2::unitY);
					}
					if (flowTypeLvl2 == FluidFlowType_Fall)
					{
						placeFluidTileLvl2(marker + Point2::unitY, FluidFlowType_StillUnderFall);
					}
				}
				
				bool hasExpanded = false;
				bool reachedSimulationEnd = false;
				
				// make all incoming flows expand 1 tile left & right
				for (Point2Set::iterator incomingIt = incomingFlows.begin();
				     incomingIt != incomingFlows.end(); ++incomingIt)
				{
					expandLvl2(FluidFlowType_RightLvl2, *incomingIt, rightEdge.x, &hasExpanded, &reachedSimulationEnd);
					expandLvl2(FluidFlowType_LeftLvl2,  *incomingIt, leftEdge.x,  &hasExpanded, &reachedSimulationEnd);
				}
				
				if (hasExpanded)
				{
					// call this again next tick
					addToToflowList(*it, p_nodeType);
				}
				else if (reachedSimulationEnd)
				{
					m_toFlowActiveLayerScratch[m_activeFluidType][m_currentNodeType].push_back(m_currentPosition);
				}
			}
			break;
			
		default:
			TT_PANIC("Unexpected FluidFlowType: %d", p_nodeType);
			break;
		}
	}
	*/
}


real FluidMgr::getFlowTime(FluidType p_fluidType, NodeType p_nodeType) const
{
	return m_flowTypeTimes[p_fluidType][p_nodeType];
}


FluidFlowType FluidMgr::getFlowType(const tt::math::Point2& p_position) const
{
	const level::AttributeLayerPtr& workingLayer(m_useSimulationLayer ? m_simulationLayer : m_activeLayer);
	
	if (workingLayer->contains(p_position) &&
	    workingLayer->getFluidType(p_position) == m_activeFluidType)
	{
		return workingLayer->getFluidFlowType(p_position);
	}
	return FluidFlowType_None;
}


void FluidMgr::placeFluidTile(const tt::math::Point2& p_position, FluidType p_type, FluidFlowType p_flowType, bool p_playSound)
{
	TT_ASSERT(m_levelLayer->contains(p_position));

	if (m_stillTiles.find(p_position) != m_stillTiles.end() || m_tileRegMgr.isSolid(p_position)) return;

	const FluidFlowType oldFlowType  = m_activeLayer->getFluidFlowType(p_position);
	const FluidType     oldFluidType = m_activeLayer->getFluidType(p_position);
	
	switch (p_type)
	{
	case FluidType_Water:
	{
		if ((oldFlowType != p_flowType || p_type != oldFluidType) && p_playSound)
		{
			playAudio(p_position);
		}
			
		// water stops when reaching lava
		if (oldFluidType == FluidType_Lava && m_preGenerate == false)
		{
			m_particlesMgr.triggerParticle(p_position, FluidParticlesMgr::TileOrientation_TopCenter,
				                            FluidParticlesMgr::ParticleType_WaterLavaCollision, 
				                            FluidType_Water);
			return;
		}
		break;
	}
		
	case FluidType_Lava:
	{
		if ((oldFlowType != p_flowType || p_type != oldFluidType) && p_playSound)
		{
			playAudio(p_position);
		}
			
		if (oldFlowType != FluidFlowType_None &&
			oldFluidType == FluidType_Water && m_preGenerate == false)
		{
			m_particlesMgr.triggerParticle(p_position, FluidParticlesMgr::TileOrientation_CenterCenter,
				                            FluidParticlesMgr::ParticleType_WaterLavaCollision, 
				                            FluidType_Lava);
		}
		break;
	}
		
	default:
		break;
	}
	
	/*
	if ((oldFlowType == FluidFlowType_None || p_type != oldFluidType) && m_preGenerate == false)
	{
		// TEMP: start a burst particle in the center of the new tile
		m_particlesMgr.triggerParticle(p_position, FluidParticlesMgr::TileOrientation_CenterCenter,
		                               FluidParticlesMgr::ParticleType_NewTile, m_activeFluidType);
	}
	*/
	
	// Check if fluid tile changed
	if (m_useSimulationLayer == false) // Only check when not using simulation layer.
	                                   // (onTileChanges will be called when checking for differences 
	                                   //  between active and simulation layer.)
	{
		
		// Check difference in fluid tile
		if (oldFluidType != m_activeFluidType || oldFlowType  != p_flowType)
		{
			// Notify entities of tile change
			notifyTileChange(p_position);
		}
	}
	
	m_activeLayer->setFluidType(p_position, p_type);
	m_activeLayer->setFluidFlowType(p_position, p_flowType);

	// */
}


void FluidMgr::placeFluidTileLvl2(const tt::math::Point2& p_position, FluidFlowType p_flowType)
{
	if (m_activeLayer->isWarpTile(p_position))
	{
		addWarpedNode(p_position, getNodeType(p_flowType));
	}
	placeFluidTile(p_position, m_activeFluidType, p_flowType);
}


void FluidMgr::addToToflowList(const tt::math::Point2& p_position, NodeType p_nodeType)
{
	TT_ASSERT(isValidFluidType(m_activeFluidType));
	const level::AttributeLayerPtr& workingLayer(m_useSimulationLayer ? m_simulationLayer : m_activeLayer);
	
	// water stops when reaching lava
	if (m_activeFluidType == FluidType_Water && 
	    workingLayer->getFluidType(p_position) == FluidType_Lava)
	{
		return;
	}
	
	m_fluidFlowTypeData[m_activeFluidType][p_nodeType].toFlowList.push_back(p_position);
}


void FluidMgr::setTileBelowFall(const tt::math::Point2& p_position, FluidType p_type)
{
	using tt::math::Point2;
	
	// check surroundings to see what this tile should be (still, left or right)
	
	const Point2 left (p_position + Point2::left);
	const Point2 right(p_position + Point2::right);
	const bool   isLeftSolid  = m_tileRegMgr.isSolid(left);
	const bool   isRightSolid = m_tileRegMgr.isSolid(right);
	
	// dead end
	if (isLeftSolid && isRightSolid)
	{
		placeFluidTile(p_position, p_type, FluidFlowType_Still);
		return;
	}
	
	const Point2 aboveleft (left  + Point2::unitY);
	const Point2 aboveright(right + Point2::unitY);
	
	// single fall
	if (isIgnorableFlowType(m_activeLayer->getFluidFlowType(aboveleft)) &&
		isIgnorableFlowType(m_activeLayer->getFluidFlowType(aboveright)))
	{
		placeFluidTile(p_position, p_type, FluidFlowType_Still, true);
		return;
	}
	
	// multi fall
	bool   makeRight = false;
	Point2 fallLeftEdge(p_position);
	
	// is solid to the left?
	for (Point2 marker(p_position);
		isIgnorableFlowType(m_activeLayer->getFluidFlowType(marker + Point2::unitY)) == false &&
	     m_tileRegMgr.isSolid(marker - Point2::unitY);
	     marker += Point2::left)
	{
		fallLeftEdge = marker;
		if (m_tileRegMgr.isSolid(marker + Point2::left))
		{
			makeRight = true;
			break;
		}
	}
	
	bool   makeLeft = false;
	Point2 fallRightEdge(p_position);
	
	// is solid to the right?
	for (Point2 marker(p_position);
	     isIgnorableFlowType(m_activeLayer->getFluidFlowType(marker + Point2::unitY)) == false &&
	     m_tileRegMgr.isSolid(marker - Point2::unitY);
	     marker += Point2::right)
	{
		fallRightEdge = marker;
		if (m_tileRegMgr.isSolid(marker + Point2::right))
		{
			makeLeft = true;
			break;
		}
	}
	
	FluidFlowType typeToMake = FluidFlowType_Invalid;
	if (makeLeft && makeRight)
	{
		typeToMake = FluidFlowType_Still;
	}
	else if (makeLeft)
	{
		typeToMake = FluidFlowType_Left;
	}
	else if (makeRight)
	{
		typeToMake = FluidFlowType_Right;
	}
	
	
	real half = fallLeftEdge.x + (fallRightEdge.x - fallLeftEdge.x) * 0.5f;
	
	for (Point2 marker(fallLeftEdge); marker.x <= fallRightEdge.x;
	     marker += Point2::right)
	{
		if (typeToMake == FluidFlowType_Invalid)
		{
			// invalid means there is no solid to the left and to the right
			// So split left & right below the fall
			real markerReal = static_cast<real>(marker.x);
			if(markerReal < half)
			{
				placeFluidTile(marker, p_type, FluidFlowType_Left, true);
			}
			else if(markerReal > half)
			{
				placeFluidTile(marker, p_type, FluidFlowType_Right, true);
			}
			else
			{
				// an odd amount of streams. The middle one becoms still.
				placeFluidTile(marker, p_type, FluidFlowType_Still, true);
			}
		}
		else
		{
			placeFluidTile(marker, p_type, typeToMake, true);
		}
	}
}


bool FluidMgr::canStartLvl2(const tt::math::Point2& p_lvl1Pos) const
{
	tt::math::Point2 leftEdge(p_lvl1Pos);
	tt::math::Point2 rightEdge(p_lvl1Pos);
	
	// go to left edge
	for (tt::math::Point2 marker(p_lvl1Pos);
	     m_tileRegMgr.isSolid(marker) == false;
	     marker += tt::math::Point2::left)
	{
		// Check for 2 wide gaps.
		const tt::math::Point2 below(    marker - tt::math::Point2::unitY);
		const tt::math::Point2 left(     marker + tt::math::Point2::left);
		const tt::math::Point2 leftbelow( below + tt::math::Point2::left);
		
		if (m_tileRegMgr.isSolid(below    ) == false &&
		    m_tileRegMgr.isSolid(leftbelow) == false &&
		    m_tileRegMgr.isSolid(left     ) == false)
		{
			return false;
		}
		
		leftEdge = marker;
	}
	
	// go to right edge
	for (tt::math::Point2 marker(p_lvl1Pos);
	     m_tileRegMgr.isSolid(marker) == false;
	     marker += tt::math::Point2::right)
	{
		// Check for 2 wide gaps.
		const tt::math::Point2 below(     marker - tt::math::Point2::unitY);
		const tt::math::Point2 right(     marker + tt::math::Point2::right);
		const tt::math::Point2 rightbelow( below + tt::math::Point2::right);
		
		if (m_tileRegMgr.isSolid(below     ) == false &&
		    m_tileRegMgr.isSolid(rightbelow) == false &&
		    m_tileRegMgr.isSolid(right     ) == false)
		{
			return false;
		}
		
		rightEdge = marker;
	}
	
	if (rightEdge == leftEdge) return false;
	
	// check if lvl1 is fully filled
	for (tt::math::Point2 marker(leftEdge);
	     marker != rightEdge;
	     marker += tt::math::Point2::right)
	{
		if (isIgnorableFlowType(getFlowType(marker)))
		{
			return false;
		}
	}
	// Temp: disable lvl2 by always returning false
	return false;
}


bool FluidMgr::fillLvl2(const tt::math::Point2&)
{
	/*
	using tt::math::Point2;
	
	if (canStartLvl2(p_lvl1Pos) == false) return false;
	// if we can start a lvl2, this means the given position has no 2 wide gaps or bigger below it 
	// between the first solid on the left and the first solid to the right (edges of the bassin).
	// At the 2nd level there can still be solids in the bassin or no solids at the edges. 
	// (Communicating vessels & spilling)
	// Everything between the most left in flow and the most right in flow should become still.
	
	Point2 leftEdge(p_lvl1Pos);
	
	// find the left edge of the bassin
	for (Point2 marker(p_lvl1Pos); m_tileRegMgr.isSolid(marker) == false; 
	     marker += tt::math::Point2::left)
	{
		leftEdge = marker;
	}
	
	// lava fills level two in his own way. Add the left edge of the bassin.
	if (m_activeFluidType == FluidType_Lava)
	{
		addToToflowList(leftEdge, NodeType_Lvl2);
		return true;
	}
	
	Point2 rightEdge(p_lvl1Pos);
	
	// find the right edge of the bassin
	for (Point2 marker(p_lvl1Pos); m_tileRegMgr.isSolid(marker) == false;
	     marker += tt::math::Point2::right)
	{
		rightEdge = marker;
	}
	
	// find leftmost incoming flow
	const Point2 aboveLeftEdge (leftEdge + Point2::unitY);
	const Point2 aboveRightEdge(rightEdge + Point2::unitY);
	Point2       leftmostIncoming;
	bool         incomingOnLeftEdge = false;
	
	FluidFlowType aboveLeftEdgeType = getFlowType(aboveLeftEdge);
	
	if (isBassinInputType(aboveLeftEdgeType) ||
	    aboveLeftEdgeType == FluidFlowType_RightOverFlow || aboveLeftEdgeType == FluidFlowType_RightLvl2)
	{
		leftmostIncoming   = aboveLeftEdge;
		incomingOnLeftEdge = true;
	}
	else
	{
		for (leftmostIncoming = aboveLeftEdge;
		     isBassinInputType(getFlowType(leftmostIncoming)) == false && 
		     leftmostIncoming != aboveRightEdge;
		     leftmostIncoming += tt::math::Point2::right)
		{ }
	}
	
	// find rightmost incoming flow
	Point2        rightmostIncoming;
	bool          incomingOnRightEdge = false;
	FluidFlowType aboveRightEdgeType  = getFlowType(aboveRightEdge);
	
	if (isBassinInputType(aboveRightEdgeType) ||
	    aboveRightEdgeType == FluidFlowType_LeftOverFlow || aboveRightEdgeType == FluidFlowType_LeftLvl2)
	{
		rightmostIncoming   = aboveRightEdge;
		incomingOnRightEdge = true;
	}
	else
	{
		for (rightmostIncoming = aboveRightEdge;
		     isBassinInputType(getFlowType(rightmostIncoming)) == false &&
		     rightmostIncoming != aboveLeftEdge;
		     rightmostIncoming += tt::math::Point2::left)
		{ }
	}
	
	const tt::math::Point2 leftSpill (leftEdge  + tt::math::Point2::left  + tt::math::Point2::unitY);
	const tt::math::Point2 rightSpill(rightEdge + tt::math::Point2::right + tt::math::Point2::unitY);
	
	if (leftmostIncoming == aboveLeftEdge &&
	    getFlowType(leftSpill) == FluidFlowType_Right && 
	    isIgnorableFlowType(getFlowType(aboveLeftEdge + Point2::unitY)) == false)
	{
		// convert to still
		for (Point2 marker(leftSpill);
		     getFlowType(marker) == FluidFlowType_Right ||
		     getFlowType(marker) == FluidFlowType_RightLvl2; marker += Point2::left)
		{
			if (getFlowType(marker) == FluidFlowType_RightLvl2)
			{
				fillLvl2(marker - Point2::unitY);
				break;
			}
			placeFluidTile(marker, FluidFlowType_Still);
		}
	}
	
	if (rightmostIncoming == aboveRightEdge && 
	    getFlowType(rightSpill) == FluidFlowType_Left && 
	    isIgnorableFlowType(getFlowType(aboveLeftEdge + Point2::unitY)) == false)
	{
		// convert to still
		for (Point2 marker(rightSpill);
		     getFlowType(marker) == FluidFlowType_Left ||
		     getFlowType(marker) == FluidFlowType_LeftLvl2; marker += Point2::right)
		{
			if (getFlowType(marker) == FluidFlowType_LeftLvl2)
			{
				fillLvl2(marker - Point2::unitY);
				break;
			}
			placeFluidTile(marker, FluidFlowType_Still);
		}
	}
	
	
	// When the spill position is still make it the outer incoming position so all the tiles become still
	if (getFlowType(rightSpill) == FluidFlowType_Still || getFlowType(rightSpill) == FluidFlowType_Left)
	{
		rightmostIncoming = rightSpill + Point2::left;
	}
	if (getFlowType(leftSpill) == FluidFlowType_Still || getFlowType(leftSpill) == FluidFlowType_Right)
	{
		leftmostIncoming = leftSpill + Point2::right;
	}
	
	// spill over edges
	bool isIncomingFromSpill = incomingOnLeftEdge && isIgnorableFlowType(getFlowType(leftSpill)) == false;
	bool isSpillTileSolid    = m_tileRegMgr.isSolid(leftSpill);
	
	if (isIncomingFromSpill == false && isSpillTileSolid == false && getFlowType(rightSpill) != FluidFlowType_Still)
	{
		placeFlow(FluidFlowType_Left, leftSpill);
	}
	
	isIncomingFromSpill = incomingOnRightEdge && isIgnorableFlowType(getFlowType(rightSpill)) == false;
	isSpillTileSolid    = m_tileRegMgr.isSolid(rightSpill);
	
	if (isIncomingFromSpill == false && isSpillTileSolid == false && getFlowType(rightSpill) != FluidFlowType_Still)
	{
		placeFlow(FluidFlowType_Right, rightSpill);
	}
	
	
	// start left until leftmost incoming
	for (Point2 marker(aboveLeftEdge); marker.x < leftmostIncoming.x;
	     marker += tt::math::Point2::right)
	{
		if (m_tileRegMgr.isSolid(marker) == false)
		{
			placeFluidTileLvl2(marker, FluidFlowType_LeftLvl2);
		}
		placeFluidTile(marker - Point2::unitY, FluidFlowType_Left);
	}
	
	// Skip converting to still if there is only one incoming and it is at the edge
	if ((leftmostIncoming == rightmostIncoming &&
	    (leftmostIncoming == aboveLeftEdge || leftmostIncoming == aboveRightEdge) &&
	    isBassinInputType(getFlowType(leftmostIncoming)) == false) == false)
	{
		// between leftmost and rightmost incoming (including under the falls)
		for (Point2 marker(leftmostIncoming); marker.x <= rightmostIncoming.x;
		     marker += tt::math::Point2::right)
		{
			if (m_tileRegMgr.isSolid(marker) == false)
			{
				if (isBassinInputType(getFlowType(marker)) ||
				    (incomingOnLeftEdge  && marker == leftmostIncoming) ||
				    (incomingOnRightEdge && marker == rightmostIncoming))
				{
					placeFluidTileLvl2(marker, FluidFlowType_StillUnderFall);
				}
				else
				{
					placeFluidTileLvl2(marker, FluidFlowType_Still);
				}
			}
			placeFluidTile(marker - Point2::unitY, FluidFlowType_Still);
		}
	}
	else if (getFlowType(leftmostIncoming) == FluidFlowType_RightOverFlow ||
	         getFlowType(leftmostIncoming) == FluidFlowType_RightLvl2)
	{
		placeFluidTileLvl2(leftmostIncoming, FluidFlowType_RightLvl2);
		placeFluidTile(leftmostIncoming - Point2::unitY, FluidFlowType_Right);
	}
	else if (getFlowType(rightmostIncoming) == FluidFlowType_LeftOverFlow ||
	         getFlowType(leftmostIncoming) == FluidFlowType_LeftLvl2)
	{
		placeFluidTileLvl2(rightmostIncoming, FluidFlowType_LeftLvl2);
		placeFluidTile(rightmostIncoming - Point2::unitY, FluidFlowType_Left);
	}
	
	// between rightmost incoming and right edge
	for (Point2 marker(rightmostIncoming + Point2::right); marker.x <= aboveRightEdge.x;
	     marker += tt::math::Point2::right)
	{
		if (m_tileRegMgr.isSolid(marker) == false)
		{
			placeFluidTileLvl2(marker, FluidFlowType_RightLvl2);
		}
		placeFluidTile(marker - Point2::unitY, FluidFlowType_Right);
	}
	*/
	return true;
}


void FluidMgr::expandLvl2(FluidFlowType p_lvl2DirType, const tt::math::Point2& p_incomingPos,
                          s32 p_edgePosX, bool* p_hasExpandedOut, bool* p_reachedSimulationEndOut)
{
	TT_ASSERT(p_lvl2DirType == FluidFlowType_RightLvl2 || p_lvl2DirType == FluidFlowType_LeftLvl2);
	
	using tt::math::Point2;
	Point2 direction(p_lvl2DirType == FluidFlowType_RightLvl2 ? Point2::right : Point2::left);
	
	// Move to where we already expanded
	for (Point2 marker(p_incomingPos + direction);
	     (p_lvl2DirType == FluidFlowType_RightLvl2) ? marker.x <= p_edgePosX : marker.x >= p_edgePosX;
	     marker += direction)
	{
		// reached the end of the expansion?
		if (getFlowType(marker) != p_lvl2DirType &&
		    m_tileRegMgr.isSolid(marker) == false)
		{
			if (isIgnorableFlowType(getFlowType(marker)))
			{
				const bool isOnActiveLayer =
						(m_activeLayer->getFluidFlowType(marker) == p_lvl2DirType ||
						 m_activeLayer->getFluidFlowType(marker) == FluidFlowType_Still) &&
						m_activeLayer->getFluidType(marker)      == FluidType_Lava;
				
				if (m_checkActiveLayerType == false || isOnActiveLayer)
				{
					// continue expanding normally
					placeFluidTileLvl2(marker, p_lvl2DirType);
					*p_hasExpandedOut = true;
				}
				else
				{
					*p_reachedSimulationEndOut = true;
				}
			}
			else // other fluid in the way
			{
				// convert to still
				for (Point2 toStillMarker(marker - direction);
				     getFlowType(toStillMarker) == p_lvl2DirType;
				     toStillMarker -= direction)
				{
					placeFluidTileLvl2(toStillMarker, FluidFlowType_Still);
					placeFluidTile(toStillMarker - Point2::unitY, m_activeFluidType, FluidFlowType_Still);
				}
				placeFluidTileLvl2(p_incomingPos, FluidFlowType_StillUnderFall);
				placeFluidTile(p_incomingPos - Point2::unitY, m_activeFluidType, FluidFlowType_StillUnderFall);
			}
			return;
		}
	}
	
	// Add spill if possible
	Point2 spillPos(Point2(p_edgePosX, p_incomingPos.y) + direction);
	FluidFlowType spillPosType(getFlowType(spillPos));
	FluidFlowType oppositeDirectionType(p_lvl2DirType == FluidFlowType_RightLvl2 ? 
			FluidFlowType_Left : FluidFlowType_Right);
	
	if (m_tileRegMgr.isSolid(spillPos) == false &&
	    isIgnorableFlowType(spillPosType))
	{
		placeFlow(p_lvl2DirType == FluidFlowType_RightLvl2 ? FluidFlowType_Right : FluidFlowType_Left,
		          spillPos);
	}
	else if (spillPosType == oppositeDirectionType)
	{
		// convert to still
		for (Point2 toStillMarker(spillPos - direction);
		     getFlowType(toStillMarker) == p_lvl2DirType;
		     toStillMarker -= direction)
		{
			placeFluidTileLvl2(toStillMarker, FluidFlowType_Still);
			placeFluidTile(toStillMarker - Point2::unitY, m_activeFluidType, FluidFlowType_Still);
		}
		placeFluidTileLvl2(p_incomingPos, FluidFlowType_StillUnderFall);
		placeFluidTile(p_incomingPos - Point2::unitY, m_activeFluidType, FluidFlowType_StillUnderFall);
	}
}


void FluidMgr::placeFlow(FluidFlowType p_newFluidFlowType, const tt::math::Point2& p_newPosition)
{
	TT_ASSERT(isValidFluidType(m_activeFluidType));
	
	const bool newPositionHasFluid = m_activeLayer->getFluidType(p_newPosition)     == m_activeFluidType &&
	                                 m_activeLayer->getFluidFlowType(p_newPosition) != FluidFlowType_None;
	const bool newPositionIsValid = (m_checkActiveLayerType == false) || newPositionHasFluid;
	
	if (newPositionIsValid)
	{
		if (level::isFluidStill(m_tileRegMgr.getCollisionTypeFromRegisteredTiles(p_newPosition, m_levelLayer)))
		{
			return;
		}
		
		
		if (p_newFluidFlowType == FluidFlowType_Left || p_newFluidFlowType == FluidFlowType_Right)
		{
			using tt::math::Point2;
			// check if these should become Overflow tiles
			const Point2 below     (p_newPosition - Point2::unitY);
			const Point2 belowLeft (below + Point2::left);
			const Point2 belowRight(below + Point2::right);
			
			if ( m_tileRegMgr.isSolid(below)      == false &&
			    (m_tileRegMgr.isSolid(belowLeft ) == false ||
			     m_tileRegMgr.isSolid(belowRight) == false))
			{
				FluidFlowType overFlowType = (p_newFluidFlowType == FluidFlowType_Left) ? 
						FluidFlowType_LeftOverFlow : FluidFlowType_RightOverFlow;
				
				placeFluidTile(p_newPosition, m_activeFluidType, overFlowType);
			}
			else
			{
				placeFluidTile(p_newPosition, m_activeFluidType, p_newFluidFlowType);
			}
		}
		else
		{
			// place tile
			placeFluidTile(p_newPosition, m_activeFluidType, p_newFluidFlowType);
		}
		
		// check if the flow should warp
		if (m_activeLayer->isWarpTile(p_newPosition) && isSameWarp(m_currentPosition, p_newPosition) == false)
		{
			addWarpedNode(p_newPosition, getNodeType(p_newFluidFlowType));
		}
		else
		{
			addToToflowList(p_newPosition, getNodeType(p_newFluidFlowType));
		}
		
	}
	else
	{
		m_toFlowActiveLayerScratch[m_activeFluidType][m_currentNodeType].push_back(m_currentPosition);
	}
}


void FluidMgr::placeAdjacentFlow(const tt::math::Point2& p_position, NodeType p_nodeType)
{
	using tt::math::Point2;
	
	const Point2 adjacentDir  (p_nodeType == NodeType_Left ? Point2::left : Point2::right);
	const Point2 adjacent     (p_position + adjacentDir);
	const Point2 below        (p_position - Point2::unitY);
	const Point2 belowAdjacent(adjacent - Point2::unitY);
	
	if (m_tileRegMgr.isSolid(adjacent) == false &&
	    isIgnorableFlowType(getFlowType(adjacent)))
	{
		// nothing is blocking so keep flowing
		placeFlow(getFluidFlowType(p_nodeType), adjacent);
	}
	else
	{
		// something is blocking
		
		FluidFlowType adjacentType = getFlowType(adjacent);
		FluidFlowType oppositeFlowType =
				(p_nodeType == NodeType_Left) ? FluidFlowType_Right : FluidFlowType_Left;
		FluidFlowType oppositeOverFlowType =
				(p_nodeType == NodeType_Left) ? FluidFlowType_RightOverFlow : FluidFlowType_LeftOverFlow;
		FluidFlowType oppositeLvl2FlowType =
				(p_nodeType == NodeType_Left) ? FluidFlowType_RightLvl2 : FluidFlowType_LeftLvl2;
		
		// special case when flowing over a gap where a single fall is falling in from above
		if (adjacentType == FluidFlowType_Fall &&
		    m_tileRegMgr.isSolid(below) &&
		    m_tileRegMgr.isSolid(below + adjacentDir) == false &&
		    m_tileRegMgr.isSolid(below + adjacentDir + adjacentDir) &&
		    m_tileRegMgr.isSolid(adjacent + adjacentDir) == false)
		{
			// start flow on other side of the gap
			placeFlow(getFluidFlowType(p_nodeType), adjacent + adjacentDir);
		}
		else if (adjacentType == oppositeFlowType || adjacentType == oppositeLvl2FlowType)
		{
			// check for 2 wide gap
			if (m_tileRegMgr.isSolid(below)         == false &&
			    m_tileRegMgr.isSolid(belowAdjacent) == false)
			{
				// create double down flow (Fall)
				Point2 leftBelow(p_nodeType == NodeType_Left ? belowAdjacent : below);
				
				placeFlow(FluidFlowType_Fall, leftBelow);
				placeFlow(FluidFlowType_Fall, leftBelow + Point2::right);
			}
			else
			{
				// make the opposite flow still. (the flow where we're coming from will
				// be made still by the opposite flow)
				for (Point2 marker(adjacent); 
				     getFlowType(marker) == oppositeFlowType || getFlowType(marker) == oppositeOverFlowType || 
				     getFlowType(marker) == oppositeLvl2FlowType;
				     marker += adjacentDir)
				{
					if (getFlowType(marker) == oppositeLvl2FlowType)
					{
						fillLvl2(marker - Point2::unitY);
						break;
					}
					// if a fluid is above this tile, setting this tile is handled by that
					if (isIgnorableFlowType(getFlowType(marker + Point2::unitY)))
					{
						placeFluidTile(marker, m_activeFluidType, FluidFlowType_Still);
					}
				}
			}
		}
		else if (adjacentType == oppositeOverFlowType)
		{
			if (m_tileRegMgr.isSolid(below) == false)
			{
				placeFlow(FluidFlowType_Fall, below);
			}
		}
		else if (adjacentType == FluidFlowType_Fall &&
		         m_tileRegMgr.isSolid(below) == false)
		{
			placeFlow(FluidFlowType_Fall, below);
		}
		
		fillLvl2(p_position);
	}
}


void FluidMgr::addWarpedNode(const tt::math::Point2& p_sourcePosition, NodeType /*p_nodeType*/)
{
	using tt::math::Point2;
	
	const tt::math::Point2 warpedPos(getWarpedPosition(p_sourcePosition));
	
	// don't place a fluid at the warped position if there is already a fluidtile there.
	if (isIgnorableFlowType(getFlowType(warpedPos)) == false) return;
	
	// stop when warping to a solid
	if (m_tileRegMgr.isSolid(warpedPos))
	{
		return;
	}
	
	placeFluidTile(warpedPos, m_activeFluidType, FluidFlowType_Fall);
	
	addToToflowList(warpedPos, NodeType_Fall);
}


void FluidMgr::playAudio(const tt::math::Point2& p_position)
{
	SoundCues::const_iterator it = m_soundCues.find(p_position);
	if (it == m_soundCues.end())
	{
		const tt::math::Vector2 pos(level::tileToWorld(p_position));
		const tt::math::Vector3 worldPosition(pos.x, pos.y, 0.0f);
		using namespace tt::audio;
		player::SoundCuePtr cue = audio::AudioPlayer::getInstance()->playPositionalEffectCue("Effects",
				(m_activeFluidType == FluidType_Water) ? "ambience_waterfall" : "ambience_lavafall",
				worldPosition);
		
		if (cue != 0)
		{
			m_soundCues.insert(std::make_pair(p_position, cue));
		}
	}
}


void FluidMgr::addTileToChangedList(const tt::math::Point2& p_pos)
{
	using namespace tt::math;
	
	m_changedTiles.insert(p_pos);
	
	// and adjacent tiles
	if (m_levelLayer->contains(p_pos + Point2( 0,  1))) m_changedTiles.insert(p_pos + Point2( 0,  1));
	if (m_levelLayer->contains(p_pos + Point2( 0, -1))) m_changedTiles.insert(p_pos + Point2( 0, -1));
	if (m_levelLayer->contains(p_pos + Point2(-1,  0))) m_changedTiles.insert(p_pos + Point2(-1,  0));
	if (m_levelLayer->contains(p_pos + Point2( 1,  0))) m_changedTiles.insert(p_pos + Point2( 1,  0));
}


void FluidMgr::addTileRectToChangedList(const tt::math::PointRect& p_rect)
{
	using namespace tt::math;
	
	for (Point2 pos(p_rect.getPosition()); p_rect.contains(pos); ++pos.y)
	{
		for (; p_rect.contains(pos); ++pos.x)
		{
			addTileToChangedList(pos);
		}
		pos.x = p_rect.getPosition().x;
	}
}


void FluidMgr::setAllWarpTiles(const level::AttributeLayerPtr& p_layer)
{
	for (WarpPairs::iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if(it->second.isValid == false) continue;
		
		// If any of the two rects is (partially) outside the layer, then ignore both.
		if (p_layer->contains(it->second.warpRect1.getMaxInside()) == false ||
		    p_layer->contains(it->second.warpRect2.getMaxInside()) == false)
		{
			TT_WARN("Warp pair found outside of level. Ignoring.");
			continue;
		}
		
		setWarpTiles(it->second.warpRect1, p_layer, true);
		setWarpTiles(it->second.warpRect2, p_layer, true);
	}
}


void FluidMgr::setWarpTiles(const tt::math::PointRect& p_rect, const level::AttributeLayerPtr& p_layer,
                            bool p_isWarpTile)
{
	for (tt::math::Point2 pos(p_rect.getPosition()); p_rect.contains(pos); ++pos.y)
	{
		for (; p_rect.contains(pos); ++pos.x)
		{
			if (p_layer->contains(pos) == false)
			{
				TT_PANIC("Tile coordinates (%d, %d) out of bounds (max (%d, %d))!",
				         pos.x, pos.y, p_layer->getWidth()- 1, p_layer->getHeight() - 1);
				continue;
			}
			
			TT_ASSERTMSG(p_layer->isWarpTile(pos) != p_isWarpTile, 
			             "Tile at position x:%d y:%d has already been %s as warp tile.",
			              pos.x, pos.y, p_isWarpTile ? "set" : "cleared");
			
			p_layer->setWarpTile(pos, p_isWarpTile);
			
			// Keep track of warp tiles for fast check
			if(p_isWarpTile)
			{
				m_warpTiles.insert(pos);
				handleWarpPlaced(pos);
			}
			else
			{
				m_warpTiles.erase(pos);
				handleWarpRemoved(pos);
			}
		}
		pos.x = p_rect.getPosition().x;
	}
}


void FluidMgr::getFluidTypes(const tt::math::Point2&         p_minPos,
                             const tt::math::Point2&         p_maxPos,
                             FluidTypes&                     p_notFalling_OUT,
                             FluidType&                      p_notFallingAllTiles_OUT,
                             FluidTypes&                     p_fall_OUT,
                             FluidType&                      p_fallAllTiles_OUT) const
{
	TT_NULL_ASSERT(m_activeLayer);
	TT_ASSERT(p_minPos.x <= p_maxPos.x);
	TT_ASSERT(p_minPos.y <= p_maxPos.y);
	
	const s32 width = m_activeLayer->getWidth();
	const tt::math::Point2 levelMax(width - 1, m_activeLayer->getHeight() - 1);
	
	// Start out with "none" or "not all the same" values
	p_notFalling_OUT         = FluidTypes();
	p_fall_OUT               = FluidTypes();
	p_notFallingAllTiles_OUT = FluidType_Invalid;
	p_fallAllTiles_OUT       = FluidType_Invalid;
	
	// Check if whole rect is outside of level
	if (p_minPos.x > levelMax.x || p_minPos.y > levelMax.y ||
	    p_maxPos.x < 0          || p_maxPos.y < 0)
	{
		return;
	}
	
	bool resultNotFallingAllTilesSet = false;
	bool resultFallAllTilesSet       = false;
	
	// Clamp rect to level size (outside of level can have no fluids)
	tt::math::Point2 minPos(p_minPos);
	tt::math::Point2 maxPos(p_maxPos);
	
	bool clamped = tt::math::clamp(minPos.x, s32(0), levelMax.x);
	clamped      = tt::math::clamp(minPos.y, s32(0), levelMax.y) || clamped;
	clamped      = tt::math::clamp(maxPos.x, s32(0), levelMax.x) || clamped;
	clamped      = tt::math::clamp(maxPos.y, s32(0), levelMax.y) || clamped;
	/*
	if (clamped)
	{
		resultAllTilesSet = true;
		resultAllTiles    = CollisionType_Solid;
		result.setFlag(CollisionType_Solid);
	}
	*/
	
	// Step through each tile and check collision type
	const u8* attributes = m_activeLayer->getRawData();
	const u8* columnPtr  = &attributes[(minPos.y * width) + minPos.x];
	for (tt::math::Point2 tilePos = minPos; tilePos.y <= maxPos.y; ++tilePos.y, columnPtr += width)
	{
		const u8* rowPtr = columnPtr;
		for (tilePos.x = minPos.x; tilePos.x <= maxPos.x; ++tilePos.x, ++rowPtr)
		{
			const FluidFlowType flow              = getFluidFlowType(*rowPtr);
			const FluidType     colType           = getFluidType(*rowPtr);
			const FluidType     colTypeNotFalling = (flow != FluidFlowType_None && isFalling(flow) == false) ? colType : FluidType_Invalid;
			const FluidType     colTypeFall       = (isFall(flow))                  ? colType : FluidType_Invalid;
			
			if (isValidFluidType(colTypeNotFalling)) p_notFalling_OUT.setFlag(colTypeNotFalling);
			if (isValidFluidType(colTypeFall))       p_fall_OUT      .setFlag(colTypeFall);
			
			if (resultNotFallingAllTilesSet == false) // Is this the first type found?
			{
				resultNotFallingAllTilesSet = true;
				p_notFallingAllTiles_OUT    = colTypeNotFalling;
			}
			// Martijn: make sure to ignore solid tiles because otherwise result is FluidType_Invalid
			// while entity is effectively still under water
			else if (p_notFallingAllTiles_OUT != colTypeNotFalling &&   // Is this type different from the previous types?
			         level::isSolid(m_levelLayer->getCollisionType(tilePos)) == false)
			{
				// Not all tiles are the same type
				p_notFallingAllTiles_OUT = FluidType_Invalid;
			}
			
			if (resultFallAllTilesSet == false) // Is this the first type found?
			{
				resultFallAllTilesSet = true;
				p_fallAllTiles_OUT    = colTypeFall;
			}
			else if (p_fallAllTiles_OUT != colTypeFall) // Is this type different from the previous types?
			{
				// Not all tiles are the same type
				p_fallAllTiles_OUT = FluidType_Invalid;
			}
			
#if defined(TT_BUILD_DEV)
			TT_ASSERTMSG(rowPtr == &attributes[(tilePos.y * width) + tilePos.x],
			             "rowPtr(%p) isn't where it should be %p for tile x: %d, y: %d",
			             rowPtr, &attributes[(tilePos.y * width) + tilePos.x], tilePos.x, tilePos.y);
#endif
		}
	}
}


movement::SurveyResult FluidMgr::checkForSurveyOnWater(const movement::SurroundingsSurvey& /*p_survey*/,
                                                       const tt::math::PointRect&    p_entityRegisteredTileRect,
                                                       const entity::Entity&         p_entity,
                                                       movement::SurveyResult*       p_flowDirection_OUT,
                                                       FluidType p_fluidType) const
{
	*p_flowDirection_OUT = movement::SurveyResult_Invalid;
	
	// The counter used to decide the direction.
	const s32 submergeDepth = p_entity.getSubmergeDepth();
	
	// Get the rect from 1 row below the entity to the top.
	tt::math::Point2 minPos(p_entityRegisteredTileRect.getMin().x, p_entityRegisteredTileRect.getMin().y - 1);
	tt::math::Point2 maxPos(p_entityRegisteredTileRect.getMaxInside());
	TT_ASSERT(maxPos.y >= minPos.y);
	TT_ASSERT(submergeDepth <= maxPos.y - minPos.y);
	// For onWater to be true no water tiles should be found in tiles above this.
	tt::math::Point2 minPosAboveWater(minPos.x, minPos.y + submergeDepth + 1);
	
	if (maxPos.y < minPosAboveWater.y)
	{
		// Also check the tile above this entity if we're so submerged we need to to get the proper submerged reporting.
		// (e.g. entity height == submerged depth.)
		maxPos.y += 1;
	}
	
	bool HACK_touchingFall = false;
	bool submergedInWater = false;
	
	if (minPosAboveWater.y <= maxPos.y)
	{
		FluidType  notNeededInside; // dummy variables for the types we don't care about
		FluidTypes notNeededTouch;
		FluidTypes CHANGE_ME_TO_notNeededTouch; // Remove this line!
		FluidTypes fluidsAbove;
		getFluidTypes(minPosAboveWater, maxPos, fluidsAbove,
		              notNeededInside, CHANGE_ME_TO_notNeededTouch, notNeededInside);
		
		if (fluidsAbove.checkFlag(p_fluidType))
		{
			// Found fluids above, expected nothing here.
			// This means we're not (floating) onWater.
			submergedInWater = true;
		}
		
		HACK_touchingFall = CHANGE_ME_TO_notNeededTouch.isEmpty() == false;
	}
	
	if (submergedInWater == false)
	{
		// Don't lower max when we are submerged.
		maxPos.y = minPosAboveWater.y - 1; // The top of the water is one row below the minPosAboveWater.
	}
	TT_ASSERT(minPos.x <= maxPos.x);
	TT_ASSERT(minPos.y <= maxPos.y);
	
	const tt::math::Point2 levelMax(m_activeLayer->getWidth()  - 1,
	                                m_activeLayer->getHeight() - 1);
	
	// Check if whole rect is outside of level (outside of level can have no fluids)
	if (minPos.x > levelMax.x || minPos.y > levelMax.y ||
	    maxPos.x < 0          || maxPos.y < 0)
	{
		TT_ASSERT(submergedInWater == false);
		// No fluid outside of level.
		return movement::SurveyResult_NotInWater;
	}
	
	// Clamp rect to level size (outside of level can have no fluids)
	bool clamped = tt::math::clamp(minPos.x, s32(0), levelMax.x);
	clamped      = tt::math::clamp(minPos.y, s32(0), levelMax.y) || clamped;
	clamped      = tt::math::clamp(maxPos.x, s32(0), levelMax.x) || clamped;
	clamped      = tt::math::clamp(maxPos.y, s32(0), levelMax.y) || clamped;
	/* FIXME: What to do here? If clamped, do we expect fluids outside of the level? Or nothing?
	if (clamped)
	{
		result = CollisionType_Solid;
	}
	*/
	
	s32 leftCount   = 0;
	s32 rightCount  = 0;
	bool foundWaterInTopTile = false;
	
	// Step through each tile and check collision type.
	for (tt::math::Point2 tilePos = minPos; tilePos.y <= maxPos.y; ++tilePos.y)
	{
		tilePos.y = maxPos.y;
		for (tilePos.x = minPos.x; tilePos.x <= maxPos.x; ++tilePos.x)
		{
			FluidFlowType flowType = m_activeLayer->getFluidFlowType(tilePos);
			if (flowType == FluidFlowType_None ||                    // Ignore none
			    isFalling(flowType)            ||                    // Ignore falling fluids
			    m_activeLayer->getFluidType(tilePos) != p_fluidType) // Ignore non-water
			{
				continue;
			}
			if (tilePos.y == maxPos.y)
			{
				foundWaterInTopTile = true;
			}
			// Count direction. Top tiles count twice as much.
			switch (flowType)
			{
			case FluidFlowType_Left:
			case FluidFlowType_LeftLvl2:
				++leftCount;
				break;
			case FluidFlowType_Right:
			case FluidFlowType_RightLvl2:
				++rightCount;
				break;
			default:
				break;
			}
		}
	}
	
	*p_flowDirection_OUT = movement::SurveyResult_WaterFlowStatic;
	if (leftCount > rightCount)
	{
		*p_flowDirection_OUT = movement::SurveyResult_WaterFlowLeft;
	}
	else if (rightCount > leftCount)
	{
		*p_flowDirection_OUT = movement::SurveyResult_WaterFlowRight;
	}
	else if (rightCount > 0 || HACK_touchingFall) // HACK: Do this special logic when touching waterfalls to work around missing flow direction below falls. (Remove when that if fixed.)
	{
		TT_ASSERT(rightCount == leftCount);
		// We are in flow directions, but they cancel each other out.
		
		// Keep the previous direction.
		*p_flowDirection_OUT = (p_entity.flowToTheRight()) ? movement::SurveyResult_WaterFlowRight :
		                                                     movement::SurveyResult_WaterFlowLeft;
	}
	
	if (submergedInWater)
	{
		return movement::SurveyResult_SubmergedInWater;
	}
	
	if (foundWaterInTopTile == false)
	{
		*p_flowDirection_OUT = movement::SurveyResult_Invalid;
		// No water found at submerge depth so we are still too high, or the tiles too low.
		return movement::SurveyResult_NotInWater;
	}
	
	{
		const tt::math::VectorRect worldTileRect(p_entity.calcWorldRect());
		
		const real entityBottom = worldTileRect.getMin().y;
		const real tileBottom   = static_cast<real>(level::worldToTile(entityBottom));
		const real diff         = entityBottom - tileBottom;
		
		// Not close enough the bottom edge. (Allow a little bit below so when rising up from below it's on water.)
		if (diff < 0.0f || diff > 0.15f) // FIXME: Close enough is this magic number.
		{
			*p_flowDirection_OUT = movement::SurveyResult_Invalid;
			return movement::SurveyResult_NotInWater;
		}
	}
	
	// Entity is on the water surface. (This is only valid when down orientation is down.)
	if (p_entity.getOrientationDown() != movement::Direction_Down)
	{
		*p_flowDirection_OUT = movement::SurveyResult_Invalid;
		return movement::SurveyResult_NotInWater;
	}
	
	switch (*p_flowDirection_OUT)
	{
	case movement::SurveyResult_WaterFlowLeft:   return movement::SurveyResult_OnWaterLeft;
	case movement::SurveyResult_WaterFlowRight:  return movement::SurveyResult_OnWaterRight;
	case movement::SurveyResult_WaterFlowStatic: return movement::SurveyResult_OnWaterStatic;
	default:
		TT_PANIC("Unexpected flow direction found: %d", *p_flowDirection_OUT);
		return movement::SurveyResult_OnWaterStatic;
	}
}


void FluidMgr::updateFlowTimes()
{
	for (s32 i = 0; i < FluidType_Count; ++i)
	{
		for(s32 j = 0; j < NodeType_Count; ++j)
		{
			if(m_flowTypeTimings[i][j].isValid())
			{
				m_flowTypeTimes[i][j] = cfg()->get(m_flowTypeTimings[i][j]);
			}
			else
			{
				m_flowTypeTimes[i][j] = 1.0f;
			}
		}
	}

	m_drainFallSpeed[FluidType_Water] = 1.0f / cfg()->getRealDirect("toki.fluids.water.grow_interval.drain_fall");
	m_drainFlowSpeed[FluidType_Water] = 1.0f / cfg()->getRealDirect("toki.fluids.water.grow_interval.drain_flow");
	m_drainFallSpeed[FluidType_Lava ] = 1.0f / cfg()->getRealDirect("toki.fluids.lava.grow_interval.drain_fall");
	m_drainFlowSpeed[FluidType_Lava ] = 1.0f / cfg()->getRealDirect("toki.fluids.lava.grow_interval.drain_flow");
}


void FluidMgr::startEntityWave(const tt::math::PointRect& p_newRect,
                               const tt::math::PointRect* p_oldRect,
                               const entity::Entity&      p_entity,
                               FluidType                  p_fluidType)
{
	const FluidSettingsPtr fluidSettings = p_entity.getFluidSettings(p_fluidType);
	if (fluidSettings == 0)
	{
		return;
	}
	
	const tt::math::Point2 centerPos(p_newRect.getCenterPosition().x, p_newRect.getBottom());
	
	if (p_oldRect == 0)
	{
		startWave(centerPos, 
		          fluidSettings->fallInPool.positionOffset, 
		          fluidSettings->fallInPool.width, 
		          fluidSettings->fallInPool.strength, 
		          fluidSettings->fallInPool.duration);
		return;
	}
	
	TT_NULL_ASSERT(p_oldRect);
	const tt::math::Point2 moveDirection(p_newRect.getPosition() - p_oldRect->getPosition());
	
	const tt::math::Point2 leftTilePos (p_newRect.getLeft(),  centerPos.y);
	const tt::math::Point2 rightTilePos(p_newRect.getRight(), centerPos.y);
	
	if (moveDirection.x > 0 || p_oldRect->getWidth() != p_newRect.getWidth())
	{
		startWave(rightTilePos,
		          1.0f - fluidSettings->forwardInPool.positionOffset, 
		          fluidSettings->forwardInPool.width, 
		          fluidSettings->forwardInPool.strength,
		          fluidSettings->forwardInPool.duration);
		startWave(leftTilePos,
		          fluidSettings->behindInPool.positionOffset, 
		          fluidSettings->behindInPool.width, 
		          fluidSettings->behindInPool.strength, 
		          fluidSettings->behindInPool.duration);
	}
	else if (moveDirection.x < 0)
	{
		startWave(rightTilePos,
		          1.0f - fluidSettings->behindInPool.positionOffset, 
		          fluidSettings->behindInPool.width, 
		          fluidSettings->behindInPool.strength, 
		          fluidSettings->behindInPool.duration);
		startWave(leftTilePos,
		          fluidSettings->forwardInPool.positionOffset, 
		          fluidSettings->forwardInPool.width, 
		          fluidSettings->forwardInPool.strength, 
		          fluidSettings->forwardInPool.duration);
	}
}


//--------------------------------------------------------------------------------------------------

bool FluidMgr::hasFluidSource(const tt::math::Point2& p_position, FluidType p_type) const
{
	if(m_levelLayer->contains(p_position))
	{
		level::CollisionType collisionType = m_levelLayer->getCollisionType(p_position);
		return level::isFluidSource(collisionType) && p_type == getFluidType(collisionType);
	}
	return false;
}


bool FluidMgr::hasFluidKill(const tt::math::Point2& p_position) const
{
	if(m_levelLayer->contains(p_position))
	{
		return g_fluidKillers.checkAnyFlags(
			m_tileRegMgr.getCollisionTypesFromRegisteredTiles(p_position, m_levelLayer));
	}

	// Consider level outsides as fluid killers
	return true;
}


void FluidMgr::addFlow(const tt::math::Point2& p_position, FluidType p_type, WarpID p_warpID)
{
	TT_ASSERTMSG(m_levelLayer->contains(p_position),
		"Trying to add a fluid flow outside the level (%d,%d)", p_position.x, p_position.y);

	tt::math::VectorRect startRect(level::tileToWorld(tt::math::PointRect(p_position, 1, 1)));
	Flow newFlow(startRect, p_type);
	newFlow.warpID = p_warpID;

	// Insert flow at correct location (sorted by starting position)
	FluidFlows& flows = m_flows[p_position.y];
	bool insertedFlow(false);
	for(FluidFlows::iterator it = flows.begin(); it != flows.end() && insertedFlow == false; ++it)
	{
		// Prevent detecting neighbour tiles
		startRect.setWidth(startRect.getWidth() * 0.9f);
		startRect.translate(tt::math::Vector2(0.05f, 0));

		if(it->area.contains(startRect) && it->stateLeft != FlowState_Drain)
		{
			if(it->type == p_type)
			{
				// Already part of flow, add feed point
				it->feedPointCount++;

				insertFeedPoint(p_position);
				computeFlowDirection(*it);

				if(p_position.x < it->currentLeft)  it->currentLeft  = p_position.x;
				if(p_position.x > it->currentRight) it->currentRight = p_position.x + 1;

				return;
			}
			else if(it->type == FluidType_Lava)
			{
				// There is lava here, do not create flow
				return;
			}
		}
		else if(it->growLeft.x >= newFlow.growRight.x)
		{
			flows.insert(it, newFlow);
			insertedFlow = true;
			setFlowTile(p_position, p_type, FluidFlowType_Still);
		}
	}
	if(insertedFlow == false)
	{
		flows.push_back(newFlow);
		setFlowTile(p_position, p_type, FluidFlowType_Still);
	}

	// Register feed point
	insertFeedPoint(p_position);
}


bool FluidMgr::expandFlow(Flow& p_flow, Flow* p_adjacentFlow, real p_elapsedTime)
{
	// Sanity check
	if(p_flow.feedPointCount <= 0 && p_flow.stateLeft != FlowState_Drain)
	{
		drainFlow(p_flow);
	}

	if(p_flow.stateLeft == FlowState_Drain)
	{
		real changeInHeight = m_drainFlowSpeed[p_flow.type] * p_elapsedTime;

		if(p_flow.area.getHeight() >= changeInHeight)
		{
			p_flow.area.setHeight(p_flow.area.getHeight() - changeInHeight);
		}
		else
		{
			p_flow.stateLeft  = FlowState_Done;
			p_flow.stateRight = FlowState_Done;
			return false;
		}
	}
	else
	{
		if(p_flow.stateLeft < FlowState_Done)
		{
			expandFlow(p_flow, p_adjacentFlow, p_elapsedTime, tt::math::Point2::left);
		}
		if(p_flow.stateRight < FlowState_Done)
		{
			expandFlow(p_flow, p_adjacentFlow, p_elapsedTime, tt::math::Point2::right);
		}
	}
	return true;
}


void FluidMgr::expandFlow(Flow& p_flow, Flow* p_adjacentFlow, real p_elapsedTime, const tt::math::Point2& p_direction)
{
	using tt::math::Point2;
	using tt::math::PointRect;

	TT_ASSERTMSG(p_direction == Point2::left || p_direction == Point2::right,
		"Only Point2::left and Point2::right are valid directions, got (%d,%d)\n",
		p_direction.x, p_direction.y);

	bool expandToLeft(p_direction == Point2::left);
	bool expandToRight(expandToLeft == false);

	// Figure out direction parameters
	FlowState& flowState = expandToLeft ? p_flow.stateLeft : p_flow.stateRight;
	Point2&    growPoint = expandToLeft ? p_flow.growLeft  : p_flow.growRight;
	Point2 opposingDirection = expandToLeft ? Point2::right : Point2::left;

	bool expansionAreaFilled =
		(expandToLeft  && p_flow.area.getLeft() < p_flow.growLeft.x) ||
		(expandToRight && p_flow.area.getRight() > (p_flow.growRight.x + 1));
		
	if(expansionAreaFilled)
	{
		growPoint += p_direction;
		Point2 target(growPoint);

		const bool targetSolid = m_tileRegMgr.isSolid(target);

		switch(flowState) 
		{
			case FlowState_Expand:
			{
				if(hasFluidKill(target) || target.x < 0 || target.x >= m_levelLayer->getWidth())
				{
					flowState = FlowState_BlockedByKill;
				}
				else if(targetSolid)
				{
					flowState = FlowState_BlockedBySolid;
				}
				else if(m_stillTiles.find(target) != m_stillTiles.end())
				{
					flowState = FlowState_BlockedByStill;
				}
				else if(m_warpTiles.find(target) != m_warpTiles.end() && allowedToEnterWarp(p_flow.warpID, target))
				{
					flowState = FlowState_EnterWarp;
				}
				else if(m_tileRegMgr.isSolid(target + Point2::up) == false)
				{
					// Detected empty space below expansion area

					const bool nextTargetSolid(m_tileRegMgr.isSolid(target + Point2(p_direction.x,  0)));

					if (m_tileRegMgr.isSolid(target + Point2(p_direction.x, -1)) && nextTargetSolid == false)
					{
						// Single tile gap, keep expanding but generate fall at this location
						// Once it has been filled
						break;
					}

					// Check out the amount of space we have to expand
					// based on this we create either asingle or a double tile overflow

					if (nextTargetSolid == false && m_tileRegMgr.isSolid(target + Point2( 0,-2)) == false)
					{
						flowState = FlowState_OverflowDoubleFirstTile;
					}
					else
					{
						flowState = FlowState_OverflowSingle;
					}
				}
				else if(expandToRight && p_adjacentFlow != 0 &&
					p_adjacentFlow->growLeft.x <= target.x && p_adjacentFlow->area.getLeft() < p_flow.area.getRight()) 
				{
					// Reached adjacent flow
					if(p_flow.type == FluidType_Lava && p_adjacentFlow->type == FluidType_Water)
					{
						// Keep expanding
						p_adjacentFlow->area.setLeft(p_flow.area.getRight());
					}
					else if(p_flow.type == FluidType_Water && p_adjacentFlow->type == FluidType_Lava)
					{
						// Shrink
						p_flow.area.setRight(p_adjacentFlow->area.getLeft());
					}
				}

				// If the previous tile was a above a single tile gap, generate fall
				if(m_tileRegMgr.isSolid(target + Point2(opposingDirection.x,-1)) == false)
				{
					// If the area below the target is also empty
					// this should become a 2 wide overflow
					if(m_tileRegMgr.isSolid(target + Point2::up) == false)
					{
						flowState = FlowState_OverflowDoubleSecondTile;
					}
					else
					{
						addFall(growPoint + opposingDirection, p_flow.type, FallType_FromFlowMiddle);
					}
				}

				break;
			}

			case FlowState_OverflowSingle:
			{
				// Done overflowing
				flowState = FlowState_DoneOverflowSingle;

				// Spawn 1 wide waterfall
				addFall(growPoint + opposingDirection, p_flow.type, FallType_FromOverflowSingle);
				break;
			}

			case FlowState_OverflowDoubleFirstTile:
			{
				if(targetSolid || hasFluidKill(target))
				{
					flowState = FlowState_OverflowSingle;
					growPoint += opposingDirection;
				}
				else
				{
					// Move 1 tile along
					flowState = FlowState_OverflowDoubleSecondTile;
				}
				break;
			}

			case FlowState_OverflowDoubleSecondTile:
			{
				flowState = FlowState_DoneOverflowDouble;

				// Determine overflow tiles
				const Point2 outerOverflow = growPoint + opposingDirection;
				const Point2 innerOverflow = outerOverflow + opposingDirection;

				// Spawn inner overflow fall
				addFall(innerOverflow, p_flow.type, FallType_FromOverflowDoubleFirst);

				if(m_warpTiles.find(outerOverflow) == m_warpTiles.end())
				{
					// Spawn outer overflow fall
					addFall(outerOverflow, p_flow.type, FallType_FromOverflowDoubleSecond);
				}
				else
				{
					// Warp detected at overflow edge
					addFallFromWarp(growPoint, p_flow.type);
					flowState = FlowState_DoneEnterWarp;
				}
				break;
			}

			case FlowState_EnterWarp:
				flowState = FlowState_DoneEnterWarp;
				addFallFromWarp(target, p_flow.type);
				break;

			default:
				break;
		}

		// Place fluid tiles
		if(flowState < FlowState_Done)
		{
			m_activeLayer->setFluidType(target, p_flow.type);
		}

		computeFlowDirection(p_flow);

		if(p_flow.type == FluidType_Lava && flowState < FlowState_Done)
		{
			// Check if we are crossing a waterfall here
			handleFallObstructed(target, true);
		}
	}

	if(flowState < FlowState_Done)
	{
		NodeType nodeType = expandToLeft ? NodeType_Left : NodeType_Right;

		real changeInSize = p_elapsedTime * (1.0f / m_flowTypeTimes[p_flow.type][nodeType]);

		// Clamp change to max 1 tile per update
		tt::math::clamp(changeInSize, 0.0f, 0.45f);

		p_flow.area.setWidth(p_flow.area.getWidth() + changeInSize);

		if(expandToLeft)
		{
			p_flow.area.translate(tt::math::Vector2(-changeInSize,0));
		}
	}
	else
	{
		// FIXME: Only do this once
		// Done flowing => Clamp to tile boundaries
		if(expandToLeft)
		{
			p_flow.area.setLeft(static_cast<real>(p_flow.growLeft.x + 1));
		}
		else
		{
			p_flow.area.setRight(static_cast<real>(p_flow.growRight.x));
		}
	}
}


void FluidMgr::addFall(const tt::math::Point2& p_position, FluidType p_type, FallType p_fallType, WarpID p_warpID)
{
	TT_ASSERTMSG(m_levelLayer->contains(p_position),
		"Trying to add a fluid fall outside the level (%d,%d)", p_position.x, p_position.y);

	// Prevent spawning falls directly above solid
	if (p_fallType != FallType_FromSource && p_fallType != FallType_FromWarp &&
		m_tileRegMgr.isSolid(p_position + tt::math::Point2(0,-1)))
	{
		return;
	}

	Fall newFall;
	newFall.startTile = p_position;
	newFall.growTile  = p_position;
	newFall.state     = FlowState_Expand;
	newFall.type      = p_type;
	newFall.fallType  = p_fallType;

	tt::math::PointRect startRect(p_position, 1, 1);

	newFall.area = level::tileToWorld(startRect);

	if(p_fallType == FallType_FromWarp)
	{
		newFall.area.setHeight(0);
		newFall.area.translate(tt::math::Vector2(0,1));
		newFall.warpID = p_warpID;
	}

	// Insert fall at correct location (sorted by starting position)
	FluidFalls& falls = m_falls[p_position.x];
	bool insertedFall(false);
	for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
	{
		if(it->startTile.y == newFall.startTile.y)
		{
			// Already has a fall at this location
			return;
		}
		else if(it->startTile.y > newFall.startTile.y)
		{
			falls.insert(it, newFall);
			insertedFall = true;
			break;
		}
	}
	if(insertedFall == false)
	{
		falls.push_back(newFall);
	}

	if(p_fallType == FallType_FromSource || p_fallType == FallType_FromWarp)
	{
		setFallTile(p_position, p_type, false);
	}
}


void FluidMgr::removeFallsFromFlow(Flow& p_flow)
{
	const s32 firstTile = p_flow.growLeft.x;
	const s32 lastTile  = p_flow.growRight.x;
	const s32 flowRow   = p_flow.growLeft.y;

	for(s32 x = firstTile; x <= lastTile; ++x)
	{
		TT_ASSERT(x < static_cast<s32>(m_falls.size()) && x >= 0);

		FluidFalls& falls = m_falls[x];
		for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
		{
			if(it->startTile.y == flowRow)
			{
				if(hasFluidSource(tt::math::Point2(x, flowRow), p_flow.type) == false)
				{
					drainFall(*it);
				}

				// Break out of inner for loop (search for fall complete)
				break;
			}

			// Handle water blocked by lava flow
			if (p_flow.type == FluidType_Lava && it->type == FluidType_Water &&
				it->state == FlowState_BlockedByLava && it->growTile.y == flowRow)
			{
				it->state = FlowState_Expand;
				it->growTile.y++;
			}
		}
	}
}


bool FluidMgr::expandFall(Fall& p_fall, real p_elapsedTime)
{
	// Sanity check
	if(p_fall.fallType == FallType_FromSource)
	{
		if(hasFluidSource(p_fall.startTile, p_fall.type) == false)
		{
			drainFall(p_fall);
		}
	}
	else if(p_fall.fallType == FallType_FromWarp)
	{
		if(m_warpTiles.find(p_fall.startTile) == m_warpTiles.end())
		{
			drainFall(p_fall);
		}
	}
	else
	{
		if(m_activeLayer->getFluidFlowType(p_fall.startTile) == FluidFlowType_None)
		{
			drainFall(p_fall);
		}
	}

	if(p_fall.state < FlowState_Done)
	{
		// Check if we have filled up our growth area
		if(p_fall.area.getPosition().y <= p_fall.growTile.y)
		{
			// HACK: Restore prev fall tile when expanding
			if(p_fall.startTile.y != p_fall.growTile.y && p_fall.state == FlowState_Expand)
			{
				setFallTile(p_fall.growTile, p_fall.type, true);
			}

			// Move growth area a tile down
			--p_fall.growTile.y;

			if(p_fall.state == FlowState_EnterWarp)
			{
				p_fall.state = FlowState_DoneEnterWarp;
				addFallFromWarp(p_fall.growTile, p_fall.type);
			}
			else
			{
				p_fall.state = setFallTile(p_fall.growTile, p_fall.type, false);
			}

			if (p_fall.state != FlowState_DoneEnterWarp && 
				m_warpTiles.find(p_fall.growTile) != m_warpTiles.end() &&
				allowedToEnterWarp(p_fall.warpID, p_fall.growTile))
			{
				p_fall.state = FlowState_EnterWarp;
			}

			if(p_fall.state == FlowState_BlockedBySolid)
			{
				// Fall must be from source or fall must be larger than 1 tile
				if (p_fall.fallType == FallType_FromSource ||
					p_fall.fallType == FallType_FromWarp   ||
					p_fall.startTile.y > (p_fall.growTile.y + 1))
				{
					// Spawn new flow at bottom of fall
					addFlow(p_fall.growTile + tt::math::Point2::down, p_fall.type, p_fall.warpID);
				}
			}

			if(p_fall.state >= FlowState_Done)
			{
				// Clamp fall area to tile boundaries
				p_fall.area.setTop   (static_cast<real>(p_fall.growTile.y  + 1));
				p_fall.area.setBottom(static_cast<real>(p_fall.startTile.y + 1));
			}

			if(p_fall.state == FlowState_Expand && p_fall.type == FluidType_Lava)
			{
				// Check for water flows
				splitWaterFlowByLava(p_fall.growTile);
			}
		}

		if(p_fall.state == FlowState_Expand || p_fall.state == FlowState_EnterWarp)
		{
			// Grow fall smoothly into grow area
			real height = p_fall.area.getHeight();
			real changeInHeight = p_elapsedTime * (1.0f / m_flowTypeTimes[p_fall.type][NodeType_Fall]);

			tt::math::clamp(changeInHeight, 0.0f, 0.5f);

			// We need to translate the rect, because level space is inverted in Y direction
			// otherwise area grows upside down
			p_fall.area.setHeight(height + changeInHeight);
			p_fall.area.translate(tt::math::Vector2(0, -changeInHeight));
		}
	}
	else if(p_fall.state == FlowState_Drain)
	{
		real changeInHeight = p_elapsedTime * m_drainFallSpeed[p_fall.type];

		if(changeInHeight < p_fall.area.getHeight())
		{
			p_fall.area.setHeight(p_fall.area.getHeight() - changeInHeight);
		}
		else
		{
			p_fall.state = FlowState_Done;
			return false;
		}
	}

	return true;
}


void FluidMgr::drainFall(Fall& p_fall)
{
	if(p_fall.state == FlowState_Drain) return;

	if(p_fall.fallType != FallType_FromWarp)
	{
		p_fall.area.setHeight(p_fall.area.getHeight() - 1.0f);
	}

	removeFallTiles(p_fall.startTile.x, p_fall.startTile.y, p_fall.growTile.y);

	// Set start tile at bottom location to prevent issues with respawning source tiles
	p_fall.startTile.y = p_fall.growTile.y;

	p_fall.state = FlowState_Drain;
}


void FluidMgr::handleFallObstructed(const tt::math::Point2& p_position, bool p_waterOnly)
{
	FluidFalls& falls = m_falls[p_position.x];
	for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
	{
		if(p_waterOnly && it->type != FluidType_Water) continue;

		if(it->startTile.y == p_position.y)
		{
			// Blocking spawn point of waterfall, drain waterfall
			drainFall(*it);
		}
		else if(it->growTile.y == p_position.y)
		{	
			// Move grow tile 1 up to trigger recheck on next simulation step
			if(it->state == FlowState_Expand)
			{
				it->growTile.y++;

				if(it->growTile.y >= it->startTile.y)
				{
					drainFall(*it);
				}
			}
		}
		else if(it->area.contains(tt::math::Vector2(p_position.x + 0.5f, p_position.y + 0.5f)))
		{
			if(it->state == FlowState_Drain)
			{
				it->area.setBottom(std::max(it->area.getTop(), static_cast<real>(p_position.y)));
				continue;
			}

			// If this waterfall was done expanding, remove the feed tile
			if(it->state == FlowState_BlockedBySolid || it->state == FlowState_DoneEnterWarp)
			{
				handleFeedPointRemoved(it->growTile + tt::math::Point2::down, it->type);
			}

			// Blocking waterfall in the middle, place grow point above solid to trigger
			// recheck in next simulation step

			const s32 endTile = std::max(s32(0), it->growTile.y);
			removeFallTiles(p_position.x, p_position.y, endTile);

			it->growTile.y = p_position.y + 1;

			if(it->growTile.y >= it->startTile.y && hasFluidSource(it->startTile, it->type) == false)
			{
				// Prevent 1 tile waterfall from flow feeding that same flow
				falls.erase(it);
				return;
			}

			it->area.setPosition(tt::math::Vector2(it->growTile));
			it->area.setHeight(static_cast<float>(it->startTile.y - it->growTile.y + 1));
			it->state = FlowState_Expand;
		}
	}
}


void FluidMgr::addFallFromWarp(const tt::math::Point2& p_sourcePosition, FluidType p_type)
{
	for(WarpPairs::iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if(it->second.isValid)
		{
			if(it->second.warpRect1.contains(p_sourcePosition))
			{
				// Start fall at corresponding position in warpRect2
				tt::math::Point2 offset = p_sourcePosition - it->second.warpRect1.getPosition();
				addFall(it->second.warpRect2.getPosition() + offset, p_type, FallType_FromWarp, it->first);
			}
			else if(it->second.warpRect2.contains(p_sourcePosition))
			{
				// Start fall at corresponding position in warpRect2
				tt::math::Point2 offset = p_sourcePosition - it->second.warpRect2.getPosition();
				addFall(it->second.warpRect1.getPosition() + offset, p_type, FallType_FromWarp, it->first);
			}
		}
	}
}


void FluidMgr::handleTileChanged(const tt::math::Point2& p_position)
{
	if(m_tileRegMgr.isSolid(p_position))
	{
		handleSolidPlaced(p_position);
	}
	else
	{
		handleSolidRemoved(p_position);
	}
}


inline void setLeft(tt::math::VectorRect& p_rect, real p_left)
{
	p_rect.setLeftRight(p_left, std::max(p_left, p_rect.getRight()));
}


inline void setRight(tt::math::VectorRect& p_rect, real p_right)
{
	p_rect.setLeftRight(p_rect.getLeft(), std::max(p_right, p_rect.getLeft()));
}


Flow FluidMgr::splitFlow(const FluidFlows::iterator& p_originalFlow, const tt::math::Point2 p_splitPoint)
{
	// Copy the current flow object
	Flow leftPart(*p_originalFlow);

	// Resize flow areas
	real splitX = static_cast<real>(p_splitPoint.x);
	setLeft(p_originalFlow->area, splitX);
	setRight(leftPart.area, splitX);

	// Set grow areas
	leftPart.growRight = p_splitPoint;
	p_originalFlow->growLeft.x = p_splitPoint.x - 1;

	// Count feed points
	s32 originalFeedPointCount = p_originalFlow->feedPointCount;
	s32 leftFeedPointCount = countFeedPoints(p_splitPoint.y, leftPart.growLeft.x + 1, p_splitPoint.x);
	s32 rightFeedPointCount = originalFeedPointCount - leftFeedPointCount;

	p_originalFlow->feedPointCount = rightFeedPointCount;
	leftPart.feedPointCount        = leftFeedPointCount;
	
	// These states should be overwritten by calling function to more appropriate states
	p_originalFlow->stateLeft = FlowState_Done;
	leftPart.stateRight       = FlowState_Done;
	
	if(leftPart.feedPointCount == 0)
	{
		drainFlow(leftPart);
	}

	return leftPart;
}


void FluidMgr::splitWaterFlowByLava(const tt::math::Point2 p_splitPoint)
{
	TT_ASSERTMSG(m_levelLayer->contains(p_splitPoint),
		"Trying to split a fluid outside the level (%d,%d)", p_splitPoint.x, p_splitPoint.y);

	FluidFlows& flows = m_flows[p_splitPoint.y];
	for(FluidFlows::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		if(it->type != FluidType_Water) continue;

		if(it->area.contains(tt::math::Vector2(p_splitPoint.x + 0.5f, p_splitPoint.y + 0.5f)))
		{
			// Water here, split water flow
			Flow leftPart = splitFlow(it, p_splitPoint);
			
			it->stateLeft = FlowState_BlockedByLava;
			it->growLeft.x++;

			if(it->area.getWidth() >= 1.0f)
			{
				it->area.setLeft(it->area.getLeft() + 1.0f);
			}

			removeFlowTile(p_splitPoint, FluidType_Water);

			if(leftPart.feedPointCount > 0)
			{
				leftPart.stateRight = FlowState_BlockedByLava; 
			}

			// Do not drain 1 tile flows visibly
			if(leftPart.area.getWidth() <= 1.0f)
			{
				leftPart.area.setHeight(0);
			}

			if(leftPart.area.getWidth() > 0)
			{
				flows.insert(it, leftPart);
			}

			if(it->area.getWidth() <= 0)
			{
				flows.erase(it);
			}
			break;
		}
	}
}


void FluidMgr::drainFlow(Flow& p_flow)
{
	if(p_flow.stateLeft == FlowState_DoneEnterWarp)
	{
		removeFallFromPairedWarp(p_flow.growLeft);
	}
	
	if(p_flow.stateRight == FlowState_DoneEnterWarp)
	{
		removeFallFromPairedWarp(p_flow.growRight);
	}

	if(p_flow.area.getWidth() > 0)
	{
		removeFlowTiles(p_flow);
	}

	// Restore fall tiles that overlapped overflows
	if(p_flow.stateLeft == FlowState_OverflowSingle || p_flow.stateLeft == FlowState_OverflowDoubleFirstTile)
	{
		restoreFallTile(p_flow.growLeft);
	}
	else if(p_flow.stateLeft == FlowState_OverflowDoubleSecondTile)
	{
		restoreFallTile(p_flow.growLeft);
		restoreFallTile(p_flow.growLeft + tt::math::Point2::right);
	}
	else if(p_flow.stateLeft == FlowState_DoneOverflowSingle)
	{
		restoreFallTile(p_flow.growLeft + tt::math::Point2::right);
	}
	else if(p_flow.stateLeft == FlowState_DoneOverflowDouble)
	{
		restoreFallTile(p_flow.growLeft + tt::math::Point2::right);
		restoreFallTile(p_flow.growLeft + 2 * tt::math::Point2::right);
	}

	if(p_flow.stateRight == FlowState_OverflowSingle || p_flow.stateRight == FlowState_OverflowDoubleFirstTile)
	{
		restoreFallTile(p_flow.growRight);
	}
	else if(p_flow.stateRight == FlowState_OverflowDoubleSecondTile)
	{
		restoreFallTile(p_flow.growRight);
		restoreFallTile(p_flow.growRight + tt::math::Point2::left);
	}
	else if(p_flow.stateRight == FlowState_DoneOverflowSingle)
	{
		restoreFallTile(p_flow.growRight + tt::math::Point2::left);
	}
	else if(p_flow.stateRight == FlowState_DoneOverflowDouble)
	{
		restoreFallTile(p_flow.growRight + tt::math::Point2::left);
		restoreFallTile(p_flow.growRight + 2 * tt::math::Point2::left);
	}

	// If done growing adjust grow tiles to be within the flow
	// Otherwise falls can be removed that do not belong to this flow
	if(p_flow.stateLeft >= FlowState_Done)
	{
		p_flow.growLeft.x++;
	}
	if(p_flow.stateRight >= FlowState_Done)
	{
		p_flow.growRight.x--;
	}

	// Set the current points to outer edges (we don't want currents in draining fluids)
	p_flow.currentLeft  = p_flow.growLeft.x + 1;
	p_flow.currentRight = p_flow.growRight.x;

	p_flow.stateLeft = p_flow.stateRight = FlowState_Drain;
}


void FluidMgr::shrinkFlowFromLeft(Flow& p_flow, real p_newLeftEdge)
{
	if(p_flow.area.getRight() > p_newLeftEdge)
	{
		p_flow.area.setLeft(p_newLeftEdge);

		if(p_newLeftEdge > (p_flow.growLeft.x + 1))
		{
			p_flow.growLeft.x++;
		}
	}
	else
	{
		p_flow.area.setWidth(0);
		p_flow.feedPointCount = 0;
		drainFlow(p_flow);
	}
}


void FluidMgr::shrinkFlowFromRight(Flow& p_flow, real p_newRightEdge)
{
	if(p_flow.area.getLeft() < p_newRightEdge)
	{
		p_flow.area.setRight(p_newRightEdge);

		if(p_newRightEdge < p_flow.growRight.x)
		{
			p_flow.growRight.x--;
		}
	}
	else
	{
		p_flow.area.setWidth(0);
		p_flow.feedPointCount = 0;
		drainFlow(p_flow);
	}
}


void FluidMgr::handleFlowObstructed(const tt::math::Point2& p_position)
{
	FluidFlows& flows = m_flows[p_position.y];
	for(FluidFlows::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		if(it->stateLeft == FlowState_Drain) continue;

		// Handle right edge (double overflow cases)
		if(p_position.x == it->growRight.x)
		{
			if(it->stateRight == FlowState_OverflowDoubleSecondTile)
			{
				it->stateRight = FlowState_OverflowSingle;
				it->growRight.x--;
			}
			else if(it->stateRight == FlowState_Expand)
			{
				it->growRight.x--;
			}
			else if(it->stateRight == FlowState_OverflowSingle)
			{
				it->stateRight = FlowState_Expand;
				it->growRight.x--;
			}
		}
		else if(p_position.x == (it->growRight.x - 1) && it->stateRight == FlowState_DoneOverflowDouble)
		{
			it->stateRight = FlowState_DoneOverflowSingle;
			it->area.setWidth(it->area.getWidth() - 1);
			it->growRight.x--;
		}
		else if(p_position.x == (it->growRight.x + 1) && it->stateRight == FlowState_OverflowDoubleFirstTile)
		{
			it->stateRight = FlowState_OverflowSingle;
		}
		else if(p_position.x == it->growRight.x &&
			(it->stateRight == FlowState_DoneOverflowDouble ||
			 it->stateRight == FlowState_DoneOverflowSingle))
		{
			// Do nothing
		}
		else if(p_position.x == it->growLeft.x)
		{
			if(it->stateLeft == FlowState_OverflowDoubleSecondTile)
			{
				it->stateLeft = FlowState_OverflowSingle;
				it->growLeft.x++;
			}
			else if(it->stateLeft == FlowState_Expand)
			{
				it->growLeft.x++;
			}
			else if(it->stateLeft == FlowState_OverflowSingle)
			{
				it->stateLeft = FlowState_Expand;
				it->growLeft.x++;
			}
		}
		else if(p_position.x == (it->growLeft.x + 1) && it->stateLeft == FlowState_DoneOverflowDouble)
		{
			it->stateLeft = FlowState_DoneOverflowSingle;
			it->area.setWidth(it->area.getWidth() - 1);
			it->area.translate(tt::math::Vector2(1,0));
			it->growLeft.x++;
		}
		else if(p_position.x == (it->growLeft.x - 1) && it->stateLeft == FlowState_OverflowDoubleFirstTile)
		{
			it->stateLeft = FlowState_OverflowSingle;
		}
		else if(p_position.x == it->growLeft.x &&
			(it->stateLeft == FlowState_DoneOverflowDouble ||
			 it->stateLeft == FlowState_DoneOverflowSingle))
		{
			// Do nothing
		}
		else if(p_position.x > it->growLeft.x && p_position.x < it->growRight.x)
		{
			// Split flow

			removeFlowTiles(*it);
			Flow leftPart = splitFlow(it, p_position);
			
			it->stateLeft = FlowState_Expand;
			it->growLeft.x+=2;
			setLeft(it->area, it->area.getLeft() + 1.0f);
			computeFlowDirection(*it);

			// What if the obstructing tile was a feed point
			FeedPoints& feedPoints = m_feedPoints[p_position.y];
			if(std::find(feedPoints.begin(), feedPoints.end(), p_position.x) != feedPoints.end())
			{
				// NOTE: The actual feed point will be removed by the fall
				it->feedPointCount--;
			}

			if(leftPart.feedPointCount > 0)
			{
				leftPart.stateRight = FlowState_Expand;
				leftPart.growRight.x--;
				computeFlowDirection(leftPart);
			}
			else if(leftPart.area.getWidth() <= 1.0f)
			{
				leftPart.area.setHeight(0);
				drainFlow(leftPart);
			}

			if(leftPart.growLeft.x <= leftPart.growRight.x)
			{
				flows.insert(it, leftPart);
			}
			break;
		}
	}
}


FlowState FluidMgr::setFallTile(const tt::math::Point2& p_position, FluidType p_type, bool p_forceUpdate)
{
	if(p_position.y < 0) return FlowState_BlockedByKill;

	//const level::CollisionType collisionType = m_activeLayer->getCollisionType(p_position);
	const FluidFlowType        oldFlowType   = m_activeLayer->getFluidFlowType(p_position);
	const FluidType            oldFluidType  = m_activeLayer->getFluidType    (p_position);

	FluidType     newFluidType(p_type);
	FluidFlowType newFlowType (FluidFlowType_Fall);
	FlowState     newFlowState(FlowState_Expand);
	bool updateTile(p_forceUpdate);

	if (g_fluidKillers.checkAnyFlags(m_tileRegMgr.getCollisionTypesFromRegisteredTiles(p_position, m_levelLayer)))
	{
		newFlowState = FlowState_BlockedByKill;
	}
	else if(m_stillTiles.find(p_position + tt::math::Point2::down) != m_stillTiles.end())
	{
		newFlowState = FlowState_BlockedByStill;
		updateTile = false;
	}
	else if(m_tileRegMgr.isSolid(p_position))
	{
		newFlowState = FlowState_BlockedBySolid;
	}
	else if(oldFlowType == FluidFlowType_None)
	{
		// No fluids yet
		newFlowState = FlowState_Expand;
		updateTile = true;
	}
	else if(oldFluidType == FluidType_Water)
	{
		// Lava overtakes water
		if(p_type == FluidType_Lava)
		{
			newFlowState = FlowState_Expand;
			updateTile = true;
		}
	}
	else if(oldFluidType == FluidType_Lava)
	{
		if(p_type == FluidType_Water)
		{
			newFlowState = FlowState_BlockedByLava;
		}
	}
	else if(m_stillTiles.find(p_position) != m_stillTiles.end())
	{
		// Reached still water ==> stop
		newFlowState = FlowState_BlockedByStill;
	}

	if(oldFluidType == FluidType_Water && newFluidType == FluidType_Lava)
	{
		if(oldFlowType == FluidFlowType_Fall)
		{
			handleFallObstructed(p_position, true);
		}
	}

	if (updateTile && (newFluidType != oldFluidType || newFlowType != oldFlowType))
	{
		placeFluidTile(p_position, p_type, newFlowType);
	}

	return newFlowState;
}


void FluidMgr::restoreFallTile(const tt::math::Point2& p_position)
{
	TT_ASSERTMSG(m_levelLayer->contains(p_position),
	             "Can't restoreFallTile because p_position (%d, %d) not inside levelLayer (%d, %d).",
	             p_position.x, p_position.y, m_levelLayer->getWidth(), m_levelLayer->getHeight());
	
	FluidFalls& falls = m_falls[p_position.x];
	for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
	{
		if(p_position.y < it->startTile.y && p_position.y >= it->growTile.y)
		{
			setFallTile(p_position, it->type, true);
		}
	}
}


void FluidMgr::removeFallTiles(s32 p_column, s32 p_startY, s32 p_endY)
{
	TT_ASSERT(p_column >= 0 && p_column < static_cast<s32>(m_falls.size()));

	FluidFalls& falls = m_falls[p_column];
	for(s32 y = p_startY; y >= p_endY; --y)
	{
		s32 fallCount(0);
		for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
		{
			if(it->startTile.y >= y && it->growTile.y <= y)
			{
				fallCount++;
			}
		}

		// If this is the last fall at this location, remove tile
		if(fallCount == 1)
		{
			removeFallTile(tt::math::Point2(p_column, y));
		}
	}
}


void FluidMgr::removeFallTile(const tt::math::Point2& p_position)
{
	if(p_position.y < 0) return;

	FluidType oldFluidType = m_activeLayer->getFluidType(p_position);

	if(m_activeLayer->getFluidFlowType(p_position) == FluidFlowType_Fall)
	{
		m_activeLayer->setFluidType    (p_position, FluidType_Water);
		m_activeLayer->setFluidFlowType(p_position, FluidFlowType_None);
		notifyTileChange(p_position);

		if(oldFluidType == FluidType_Lava && hasFluidSource(p_position, FluidType_Water))
		{
			// Restore water sources that were blocked by lava
			addFall(p_position, FluidType_Water, FallType_FromSource);
		}
	}
}


void FluidMgr::setFlowTile(const tt::math::Point2& p_position, FluidType p_type, FluidFlowType p_flowType)
{
	const FluidType oldFluidType = m_activeLayer->getFluidType(p_position);
	const FluidType newFluidType(p_type);

	if(oldFluidType == FluidType_Lava && newFluidType == FluidType_Water)
	{
		return;
	}

	placeFluidTile(p_position, p_type, p_flowType);
}


void FluidMgr::removeFlowTiles(const Flow& p_flow)
{
	// If still expanding include grow tiles, otherwise exclude these

	s32 startTile = p_flow.growLeft.x;
	if(p_flow.stateLeft >= FlowState_Done)
	{
		startTile++;
	}
	else
	{
		// Restore grow tiles to area extremes
		startTile = static_cast<s32>(tt::math::floor(p_flow.area.getLeft()));
	}

	s32 endTile = p_flow.growRight.x;
	if(p_flow.stateRight >= FlowState_Done)
	{
		endTile--;
	}
	else
	{
		endTile = static_cast<s32>(tt::math::floor(p_flow.area.getRight()));
	}

	for(s32 x = startTile; x <= endTile; ++x)
	{
		removeFlowTile(tt::math::Point2(x, p_flow.growLeft.y), p_flow.type);
	}
}


void FluidMgr::removeFlowTile(const tt::math::Point2& p_position, FluidType p_type)
{
	// Don't let water flows remove lava tiles
	if(p_type == FluidType_Water && m_activeLayer->getFluidType(p_position) == FluidType_Lava)
	{
		return;
	}

	FluidFlowType oldFlowType = m_activeLayer->getFluidFlowType(p_position);

	if(oldFlowType > FluidFlowType_Fall)
	{
		m_activeLayer->setFluidFlowType(p_position, FluidFlowType_None);
		m_activeLayer->setFluidType(p_position, FluidType_Water);
		notifyTileChange(p_position);
	}
}


void FluidMgr::computeFlowUnderFall(s32 p_start, s32 p_end, bool p_fallToLeft, bool p_fallToRight, tt::math::Point2& p_tile)
{
	TT_ASSERT(p_start < p_end);

	const bool leftSolid  = m_tileRegMgr.isSolid(tt::math::Point2(p_start - 1, p_tile.y));
	const bool rightSolid = m_tileRegMgr.isSolid(tt::math::Point2(p_end + 1,   p_tile.y));

	FluidFlowType typeToMake = FluidFlowType_Invalid;
	if (leftSolid && rightSolid)
	{
		typeToMake = FluidFlowType_Still;
	}
	else if (rightSolid)
	{
		typeToMake = p_fallToLeft ? FluidFlowType_Still : FluidFlowType_Left;
	}
	else if (leftSolid)
	{
		typeToMake = p_fallToRight ? FluidFlowType_Still : FluidFlowType_Right;
	}
	
	real half = p_start + (p_end - p_start) * 0.5f;
	
	for (p_tile.x = p_start; p_tile.x <= p_end; ++p_tile.x)
	{
		if (typeToMake == FluidFlowType_Invalid)
		{
			// Invalid means there is no solid to the left and to the right
			// So split left & right below the fall
			const real tileX = static_cast<real>(p_tile.x);

			if(tileX < half)
			{
				setFlowType(p_tile, p_fallToLeft ? FluidFlowType_Still : FluidFlowType_Left);
			}
			else if(tileX > half)
			{
				setFlowType(p_tile, p_fallToRight ? FluidFlowType_Still : FluidFlowType_Right);
			}
			else
			{
				// an odd amount of streams. The middle one becoms still.
				setFlowType(p_tile, FluidFlowType_Still);
			}
		}
		else
		{
			setFlowType(p_tile, typeToMake);
		}
	}
}


void FluidMgr::computeFlowDirection(Flow& p_flow)
{
	if(p_flow.feedPointCount == 0)
	{
		return;
	}

	// NOTE: Brute force recomputation of the flow tiles
	const FeedPoints& feedPoints = m_feedPoints[p_flow.growLeft.y];

	FeedPoints::const_iterator nextFeedPoint = feedPoints.begin();

	// Search first feed point in the stream
	while(nextFeedPoint != feedPoints.end() && *nextFeedPoint < p_flow.growLeft.x)
	{
		++nextFeedPoint;
	}

	if(nextFeedPoint == feedPoints.end())
	{
		// No feed points found!!
		p_flow.feedPointCount = 0;
		return;
	}

	// Determine first tile
	tt::math::Point2 tile(p_flow.growLeft);
	if(p_flow.stateLeft >= FlowState_Done) ++tile.x;

	const tt::math::Point2 firstTile(tile);

	// Single Overflow
	if( p_flow.stateLeft == FlowState_DoneOverflowSingle ||
		p_flow.stateLeft == FlowState_OverflowSingle ||
		p_flow.stateLeft == FlowState_OverflowDoubleFirstTile)
	{
		setFlowType(tile, FluidFlowType_LeftOverFlow);
		++tile.x;
	}
	// Double overflow
	else if(p_flow.stateLeft == FlowState_DoneOverflowDouble ||
		    p_flow.stateLeft == FlowState_OverflowDoubleSecondTile)
	{
		setFlowType(tile, FluidFlowType_LeftOverFlow);
		++tile.x;
		setFlowType(tile, FluidFlowType_LeftOverFlow);
		++tile.x;
	}

	// Now fill up with left tiles until the first feed point is found
	while(tile.x < *nextFeedPoint)
	{
		setFlowType(tile, FluidFlowType_Left);
		++tile.x;
	}
	p_flow.currentLeft = tile.x;

	// Left most feed point => Fill tiles under fall
	s32 leftMostFeedPoint = *nextFeedPoint;
	++nextFeedPoint;

	if(nextFeedPoint != feedPoints.end() && leftMostFeedPoint + 1 == *nextFeedPoint)
	{
		// Multi fall
		s32 fallEnd = leftMostFeedPoint;

		while(nextFeedPoint != feedPoints.end() && *nextFeedPoint == fallEnd + 1)
		{
			fallEnd = *nextFeedPoint;
			++nextFeedPoint;
		}

		const bool fallToRight = (nextFeedPoint != feedPoints.end() && *nextFeedPoint < p_flow.growRight.x);

		computeFlowUnderFall(leftMostFeedPoint, fallEnd, false, fallToRight, tile);
	}
	else
	{
		// Single fall
		setFlowType(tile, FluidFlowType_Still);
		++tile.x;
	}

	// Are there more feed points ?
	while(nextFeedPoint != feedPoints.end() && *nextFeedPoint < p_flow.growRight.x)
	{
		// Another feed point detected, fill with still until next fall
		while(tile.x < *nextFeedPoint)
		{
			setFlowType(tile, FluidFlowType_Still);
			++tile.x;
		}

		// Handle next fall
		s32 fallStart = *nextFeedPoint;
		++nextFeedPoint;

		if(nextFeedPoint != feedPoints.end() && fallStart + 1 == *nextFeedPoint)
		{
			// Multi fall
			s32 fallEnd = fallStart;

			while(nextFeedPoint != feedPoints.end() && *nextFeedPoint == fallEnd + 1)
			{
				fallEnd = *nextFeedPoint;
				++nextFeedPoint;
			}

			const bool fallToRight = (nextFeedPoint != feedPoints.end() && *nextFeedPoint < p_flow.growRight.x);

			computeFlowUnderFall(fallStart, fallEnd, true, fallToRight, tile);
		}
		else
		{
			// Single fall
			setFlowType(tile, FluidFlowType_Still);
			++tile.x;
		}
	}

	p_flow.currentRight = tile.x;

	// Right from last fall
	while(tile.x < p_flow.growRight.x)
	{
		setFlowType(tile, FluidFlowType_Right);
		++tile.x;
	}

	// Make sure we are at the last tile
	if(p_flow.stateRight >= FlowState_Done)
	{
		--tile.x;
	}

	// Single Overflow
	if( p_flow.stateRight == FlowState_DoneOverflowSingle ||
		p_flow.stateRight == FlowState_OverflowSingle ||
		p_flow.stateRight == FlowState_OverflowDoubleFirstTile)
	{
		setFlowType(tile, FluidFlowType_RightOverFlow);
	}
	// Double overflow
	else if(p_flow.stateRight == FlowState_DoneOverflowDouble ||
		    p_flow.stateRight == FlowState_OverflowDoubleSecondTile)
	{
		setFlowType(tile, FluidFlowType_RightOverFlow);
		--tile.x;
		setFlowType(tile, FluidFlowType_RightOverFlow);
	}

	// Fill grow tile
	if(p_flow.stateRight == FlowState_Expand || p_flow.stateRight == FlowState_EnterWarp)
	{
		if(tile.x == p_flow.growRight.x)
		{
			setFlowType(tile, FluidFlowType_Right);
		}
	}

	const tt::math::Point2 lastTile(std::min(tile.x, p_flow.growRight.x), tile.y);

	// Set fluid type for all tiles
	for(tt::math::Point2 flowTile = firstTile; flowTile.x <= lastTile.x; ++flowTile.x)
	{
		m_activeLayer->setFluidType(flowTile, p_flow.type);
	}
}


void FluidMgr::setFlowType(const tt::math::Point2& p_position, FluidFlowType p_flowType)
{
	const FluidFlowType oldFlowType = m_activeLayer->getFluidFlowType(p_position);

	m_activeLayer->setFluidFlowType(p_position, p_flowType);

	if(oldFlowType != p_flowType)
	{
		notifyTileChange(p_position);
	}
}


void FluidMgr::handleSolidPlaced(const tt::math::Point2& p_position)
{
	handleFallObstructed(p_position, false);

	handleFlowObstructed(p_position);

	
	// Handle solids added below flow
	if((p_position.y + 1) < static_cast<s32>(m_flows.size()))
	{
		const tt::math::Point2 abovePosition(p_position.x, p_position.y + 1);

		FluidFlows& flowsAbove = m_flows[p_position.y + 1];
		for(FluidFlows::iterator it = flowsAbove.begin(); it != flowsAbove.end(); ++it)
		{
			if(it->stateLeft == FlowState_Drain) continue;

			if(it->stateRight == FlowState_DoneOverflowDouble)
			{
				if(it->growRight.x == p_position.x + 1)
				{
					it->growRight.x--;
					it->stateRight = FlowState_Expand;
					setFlowTile(p_position + tt::math::Point2(-1, 1), it->type, FluidFlowType_Right);
					setFlowTile(p_position + tt::math::Point2( 0, 1), it->type, FluidFlowType_Right);
				}
				else if(it->growRight.x == p_position.x + 2)
				{
					it->stateRight = FlowState_Expand;
					it->growRight.x--;
					removeFlowTile(it->growRight, it->type);
					it->growRight.x--;
					setFlowTile(abovePosition, it->type, FluidFlowType_Right);
				}
			}
			else if(it->stateRight == FlowState_DoneOverflowSingle)
			{
				if(it->growRight.x == p_position.x + 1)
				{
					it->growRight.x--;
					it->stateRight = FlowState_Expand;
					setFlowTile(abovePosition, it->type, FluidFlowType_Right);
				}
			}

			if(it->stateLeft == FlowState_DoneOverflowDouble)
			{
				if(it->growLeft.x == p_position.x - 1)
				{
					it->growLeft.x++;
					it->stateLeft = FlowState_Expand;
					setFlowTile(p_position + tt::math::Point2( 1, 1), it->type, FluidFlowType_Left);
					setFlowTile(abovePosition, it->type, FluidFlowType_Left);
				}
				else if(it->growLeft.x == p_position.x - 2)
				{
					it->stateLeft = FlowState_Expand;
					it->growLeft.x++;
					removeFlowTile(it->growLeft, it->type);
					it->growLeft.x++;
					setFlowTile(abovePosition, it->type, FluidFlowType_Left);
				}
			}
			else if(it->stateLeft == FlowState_DoneOverflowSingle)
			{
				if(it->growLeft.x == p_position.x - 1)
				{
					it->growLeft.x++;
					it->stateLeft = FlowState_Expand;
					setFlowTile(abovePosition, it->type, FluidFlowType_Left);
				}
			}

			// Added solid under overflow situation
			if(it->growLeft.x == p_position.x &&
				(it->stateLeft == FlowState_OverflowDoubleFirstTile ||
				 it->stateLeft == FlowState_OverflowDoubleSecondTile ||
				 it->stateLeft == FlowState_OverflowSingle))
			{
				if(it->stateLeft == FlowState_OverflowDoubleSecondTile)
				{
					setFlowTile(p_position + tt::math::Point2(1, 1), it->type, FluidFlowType_Left);
				}
				removeFlowTile(it->growLeft, it->type);
				it->growLeft.x++;
				it->stateLeft = FlowState_Expand;
			}
			else if(it->growLeft.x == (p_position.x - 1) && it->stateLeft == FlowState_OverflowDoubleSecondTile)
			{
				removeFlowTile(it->growLeft, it->type);
				it->growLeft.x++;
				removeFlowTile(it->growLeft, it->type);
				it->growLeft.x++;

				it->stateLeft = FlowState_Expand;
			}
			else if(it->growRight.x == p_position.x &&
				(it->stateRight == FlowState_OverflowDoubleFirstTile ||
				 it->stateRight == FlowState_OverflowDoubleSecondTile ||
				 it->stateRight == FlowState_OverflowSingle))
			{
				if(it->stateRight == FlowState_OverflowDoubleSecondTile)
				{
					setFlowTile(p_position + tt::math::Point2(-1, 1), it->type, FluidFlowType_Right);
				}
				removeFlowTile(it->growRight, it->type);
				it->growRight.x--;
				it->stateRight = FlowState_Expand;
			}
			else if(it->growRight.x == (p_position.x + 1) && it->stateRight == FlowState_OverflowDoubleSecondTile)
			{
				removeFlowTile(it->growRight, it->type);
				it->growRight.x--;
				removeFlowTile(it->growRight, it->type);
				it->growRight.x--;
				it->stateRight = FlowState_Expand;
			}
		}
	}

	// Clear flow type
	m_activeLayer->setFluidFlowType(p_position, FluidFlowType_None);
	m_activeLayer->setFluidType(p_position, FluidType_Water);
}


void FluidMgr::handleSolidRemoved(const tt::math::Point2& p_position)
{
	for(u32 i = 0; i < FluidType_Count; ++i)
	{
		FluidType fluidType = static_cast<FluidType>(i);
		if(hasFluidSource(p_position, fluidType))
		{
			addFall(p_position, fluidType, FallType_FromSource);
		}
		
		if(m_stillTiles.find(p_position) != m_stillTiles.end())
		{
			const FluidType stillFluidType = getFluidType(m_levelLayer->getCollisionType(p_position));
			m_activeLayer->setFluidType(    p_position, stillFluidType);
			m_activeLayer->setFluidFlowType(p_position, FluidFlowType_Still);
		}
	}
	
	// Handle falls first
	
	FluidFalls& falls = m_falls[p_position.x];
	for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
	{
		if(it->growTile.y == p_position.y)
		{
			it->state = FlowState_Expand;
			if(it->growTile.y < it->startTile.y)
			{
				it->growTile.y++;
			}

			handleFeedPointRemoved(it->growTile, it->type);
		}
	}

	// Handle flow expansion

	FluidFlows& flows = m_flows[p_position.y];
	for(FluidFlows::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		if(it->stateLeft == FlowState_Drain) continue;

		if(it->growLeft.x == p_position.x)
		{
			if(it->stateLeft == FlowState_BlockedBySolid)
			{
				it->growLeft.x++;
				it->stateLeft = FlowState_Expand;
			}
			else if(it->stateLeft == FlowState_OverflowSingle)
			{
				it->growLeft.x++;
				it->stateLeft = FlowState_Expand;
			}
			else if(it->stateLeft == FlowState_DoneOverflowSingle)
			{
				it->growLeft.x += 2;
				it->stateLeft = FlowState_Expand;
			}
		}
		if(it->growRight.x == p_position.x)
		{
			if(it->stateRight == FlowState_BlockedBySolid)
			{
				it->growRight.x--;
				it->stateRight = FlowState_Expand;
			}
			else if(it->stateRight == FlowState_OverflowSingle)
			{
				it->growRight.x--;
				it->stateRight = FlowState_Expand;
			}
			else if(it->stateRight == FlowState_DoneOverflowSingle)
			{
				it->growRight.x -= 2;
				it->stateRight = FlowState_Expand;
			}
		}

		// Tile next to grow tile has become free => turn single into double fall
		const tt::math::Point2 belowPosition(p_position.x, p_position.y - 2);
		if(it->growLeft.x - 1 == p_position.x)
		{
			if(it->stateLeft == FlowState_OverflowSingle && m_tileRegMgr.isSolid(belowPosition) == false)
			{
				it->stateLeft = FlowState_OverflowDoubleFirstTile;
			}
		}
		else if(it->growRight.x + 1 == p_position.x && m_tileRegMgr.isSolid(belowPosition) == false)
		{
			if(it->stateRight == FlowState_OverflowSingle)
			{
				it->stateRight = FlowState_OverflowDoubleFirstTile;
			}
		}
	}

	// Handle solids removed below flow
	using tt::math::Point2;
	const Point2 abovePosition(p_position.x, p_position.y+1);
	
	if(abovePosition.y < static_cast<s32>(m_flows.size()))
	{
		FluidFlows& flowsAbove = m_flows[abovePosition.y];
		for(FluidFlows::iterator it = flowsAbove.begin(); it != flowsAbove.end(); ++it)
		{
			if(it->feedPointCount <= 0) continue;

			if(p_position.x == it->growLeft.x)
			{
				// Tile below grow area
				if(it->stateLeft == FlowState_Expand)
				{
					// Trigger recheck to generate overflow
					it->growLeft.x++;
				}
				
				// For all other states => ignore
				break;
			}
			else if(p_position.x == it->growRight.x)
			{
				// Tile below grow area
				if(it->stateRight == FlowState_Expand)
				{
					// Trigger recheck to generate overflow
					it->growRight.x--;
				}
				
				// For all other states => ignore, but still check adjacent
				continue;
			}
			else if (it->growLeft.x + 1 == p_position.x)
			{
				if (it->stateLeft == FlowState_BlockedBySolid ||
				    it->stateLeft == FlowState_BlockedByKill  ||
				    it->stateLeft == FlowState_BlockedByLava)
				{
					it->stateLeft = FlowState_OverflowSingle;
					it->growLeft.x++;
					setFlowTile(abovePosition, it->type, FluidFlowType_LeftOverFlow);
					break;
				}
				else if(it->stateLeft == FlowState_OverflowDoubleFirstTile ||
					    it->stateLeft == FlowState_OverflowSingle)
				{
					it->growLeft.x += 2;
					it->stateLeft = FlowState_Expand;
					break;
				}
			}
			else if(it->growRight.x - 1 == p_position.x)
			{
				if (it->stateRight == FlowState_BlockedBySolid ||
				    it->stateRight == FlowState_BlockedByKill  ||
				    it->stateRight == FlowState_BlockedByLava)
				{
					it->stateRight = FlowState_OverflowSingle;
					it->growRight.x--;
					setFlowTile(abovePosition, it->type, FluidFlowType_RightOverFlow);
					break;
				}
				else if(it->stateRight == FlowState_OverflowDoubleFirstTile ||
					    it->stateRight == FlowState_OverflowSingle)
				{
					it->growRight.x -= 2;
					it->stateRight = FlowState_Expand;
					break;
				}
			}
			else if(it->growLeft.x + 2 == p_position.x)
			{
				if(it->stateLeft == FlowState_DoneOverflowSingle)
				{
					it->stateLeft = FlowState_DoneOverflowDouble;
					setFlowTile(abovePosition, it->type, FluidFlowType_LeftOverFlow);
					addFall(abovePosition, it->type, FallType_FromOverflowDoubleFirst);
					break;
				}
				else if(it->stateLeft == FlowState_DoneOverflowDouble)
				{
					// Already handled this removed tile, prevent flow split by breaking out of loop
					break;
				}
				else if(it->stateLeft == FlowState_OverflowDoubleSecondTile)
				{
					removeFlowTile(it->growLeft, it->type);
					it->growLeft.x++;
					setFlowTile(abovePosition, it->type, FluidFlowType_LeftOverFlow);
					break;
				}
			}
			else if(it->growRight.x - 2 == p_position.x)
			{
				if(it->stateRight == FlowState_DoneOverflowSingle)
				{
					it->stateRight = FlowState_DoneOverflowDouble;
					setFlowTile(abovePosition, it->type, FluidFlowType_RightOverFlow);
					addFall(abovePosition, it->type, FallType_FromOverflowDoubleFirst);
					break;
				}
				else if(it->stateRight == FlowState_DoneOverflowDouble)
				{
					// Already handled this removed tile, prevent flow split by breaking out of loop
					break;
				}
				else if(it->stateRight == FlowState_OverflowDoubleSecondTile)
				{
					removeFlowTile(it->growRight, it->type);
					it->growRight.x--;
					setFlowTile(abovePosition, it->type, FluidFlowType_RightOverFlow);
					break;
				}
			}
			else if(it->growLeft.x + 3 == p_position.x)
			{
				if(it->stateLeft == FlowState_DoneOverflowDouble)
				{
					setFlowTile(abovePosition, it->type, FluidFlowType_LeftOverFlow);
					addFall(abovePosition, it->type, FallType_FromOverflowDoubleFirst);
					it->growLeft.x++;
					setLeft(it->area, static_cast<real>(it->growLeft.x + 1));
					removeFlowTile(it->growLeft, it->type);

					// Create dummy flow to remove the outer fall
					Flow dummy(it->area, it->type);
					dummy.growLeft.x = dummy.growRight.x = p_position.x - 2;
					removeFallsFromFlow(dummy);
					
					break;
				}

			}
			else if(it->growRight.x - 3 == p_position.x)
			{
				if(it->stateRight == FlowState_DoneOverflowDouble)
				{
					setFlowTile(abovePosition, it->type, FluidFlowType_RightOverFlow);
					addFall(abovePosition, it->type, FallType_FromOverflowDoubleFirst);
					it->growRight.x--;
					setRight(it->area, static_cast<real>(it->growRight.x));
					removeFlowTile(it->growRight, it->type);

					// Create dummy flow to remove the outer fall
					Flow dummy(it->area, it->type);
					dummy.growLeft.x = dummy.growRight.x = p_position.x + 2;
					removeFallsFromFlow(dummy);

					break;
				}

			}

			if(p_position.x  > it->growLeft.x && p_position.x < it->growRight.x)
			{
				if(m_tileRegMgr.isSolid(p_position + Point2::right) == false)
				{
					// Tile next to this is empty => causing flow split

					removeFlowTiles(*it);

					const Point2 splitPosition(abovePosition.x + 1, abovePosition.y);
					Flow leftPart = splitFlow(it, splitPosition);

					if(it->feedPointCount > 0)
					{
						it->stateLeft = FlowState_Expand;
						it->growLeft.x = splitPosition.x + 1;
						setLeft(it->area, static_cast<real>(splitPosition.x - 1));
						computeFlowDirection(*it);
					}
					else
					{
						it->growLeft.x++;
						setLeft(it->area, static_cast<real>(splitPosition.x + 1));
					}

					if(leftPart.feedPointCount > 0)
					{
						leftPart.stateRight = FlowState_Expand;
						leftPart.growRight.x = splitPosition.x - 2;
						setRight(leftPart.area, static_cast<real>(splitPosition.x + 1));
						computeFlowDirection(leftPart);
					}
					else
					{
						leftPart.growRight.x--;
						setRight(leftPart.area, static_cast<real>(splitPosition.x - 1));
					}

					flowsAbove.insert(it, leftPart);

					break;
				}
				else if(m_tileRegMgr.isSolid(p_position + Point2::left) == false)
				{
					// Tile next to this is empty => causing flow split

					removeFlowTiles(*it);

					const Point2 splitPosition(abovePosition);
					Flow leftPart = splitFlow(it, abovePosition);

					if(it->feedPointCount > 0)
					{
						it->stateLeft = FlowState_Expand;
						it->growLeft.x = splitPosition.x + 1;
						setLeft(it->area, static_cast<real>(splitPosition.x - 1));
						computeFlowDirection(*it);
					}
					else
					{
						it->growLeft.x++;
						setLeft(it->area, static_cast<real>(splitPosition.x + 1));
					}

					if(leftPart.feedPointCount > 0)
					{
						leftPart.stateRight = FlowState_Expand;
						leftPart.growRight.x = splitPosition.x - 2;
						setRight(leftPart.area, static_cast<real>(splitPosition.x + 1));
						computeFlowDirection(leftPart);
					}
					else
					{
						leftPart.growRight.x--;
						setRight(leftPart.area, static_cast<real>(splitPosition.x - 1));
					}

					flowsAbove.insert(it, leftPart);

					break;
				}
				else
				{
					addFall(p_position + Point2::down, it->type, FallType_FromFlowMiddle);
					break;
				}
			}
		}
	}

	const Point2 twoAbovePosition(p_position.x, p_position.y+2);
	
	if(twoAbovePosition.y < static_cast<s32>(m_flows.size()))
	{
		FluidFlows& flowsAbove = m_flows[twoAbovePosition.y];
		for(FluidFlows::iterator it = flowsAbove.begin(); it != flowsAbove.end(); ++it)
		{
			if(it->growLeft.x == p_position.x)
			{
				// Re-evaluate overflow size

				if(it->stateLeft == FlowState_OverflowSingle)
				{
					it->stateLeft = FlowState_Expand;
					it->growLeft.x++;
				}
				else if(it->stateLeft == FlowState_DoneOverflowSingle)
				{
					it->stateLeft = FlowState_Expand;
					it->growLeft.x+=2;
				}
			}
		}
	}
}


void FluidMgr::handleFeedPointAdded(const tt::math::Point2& p_position, FluidType p_type)
{
	TT_ASSERT(p_position.y >= 0 && p_position.y < m_levelLayer->getHeight());

	// Increase the incoming feed point count of the flow this feed point ends in

	FluidFlows& flows = m_flows[p_position.y];
	for(FluidFlows::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		if(it->area.contains(tt::math::Vector2(p_position.x + 0.5f, p_position.y + 0.5f)))
		{
			TT_ASSERT(it->type == p_type);
			it->feedPointCount++;
			break;
		}
	}

	insertFeedPoint(p_position);
}


void FluidMgr::handleFeedPointRemoved(const tt::math::Point2& p_position, FluidType p_type)
{
	if(p_position.y < 0 || p_position.y >= static_cast<s32>(m_feedPoints.size()))
	{
		return;
	}

	// Handle warps
	if(m_warpTiles.find(p_position) != m_warpTiles.end())
	{
		removeFallFromPairedWarp(p_position + tt::math::Point2(0,-1));
	}

	bool feedPointFound(false);
	FeedPoints& feedPoints = m_feedPoints[p_position.y];
	for(FeedPoints::iterator it = feedPoints.begin(); it != feedPoints.end(); ++it)
	{
		if(*it == p_position.x)
		{
			feedPoints.erase(it);
			feedPointFound = true;
			break;
		}
	}
	
	if(feedPointFound == false)
	{
		// Invalid feed point
		return;
	}

	FluidFlows& flows = m_flows[p_position.y];
	for(FluidFlows::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		if(it->area.contains(tt::math::Vector2(p_position.x + 0.5f, p_position.y + 0.5f)))
		{
			TT_ASSERT(it->type == p_type);
			it->feedPointCount--;

			if(it->feedPointCount > 0)
			{
				computeFlowDirection(*it);
			}

			break;
		}
	}
}


void FluidMgr::insertFeedPoint(const tt::math::Point2& p_position)
{
	bool inserted(false);
	FeedPoints& feedPoints = m_feedPoints[p_position.y];
	for(FeedPoints::iterator it = feedPoints.begin(); it != feedPoints.end(); ++it)
	{
		if(*it >= p_position.x)
		{
			feedPoints.insert(it, p_position.x);
			inserted = true;
			break;
		}
	}

	if(inserted == false)
	{
		feedPoints.push_back(p_position.x);
	}
}


s32 FluidMgr::countFeedPoints(s32 p_row, s32 p_startX, s32 p_endX)
{
	s32 count(0);
	FeedPoints& feedPoints = m_feedPoints[p_row];
	for(FeedPoints::iterator it = feedPoints.begin(); it != feedPoints.end(); ++it)
	{
		for(s32 x = p_startX; x < p_endX; ++x)
		{
			if(*it == x)
			{
				++count;
			}
		}
	}

	return count;
}


void FluidMgr::handleWarpPlaced(const tt::math::Point2& p_position)
{
	if(m_sourceTiles.find(p_position) != m_sourceTiles.end())
	{
		// Placing warp on top of fluid source
		if(hasFluidSource(p_position, FluidType_Water))
		{
			addFallFromWarp(p_position, FluidType_Water);
		}
		else
		{
			addFallFromWarp(p_position, FluidType_Lava);
		}
	}
	else
	{
		handleFallObstructed(p_position, true);

		handleFlowObstructed(p_position);
	}

	// Handle warp placed inside flow
}


void FluidMgr::handleWarpRemoved(const tt::math::Point2& p_position)
{
	// Process falls that cross this warp tile
	FluidFalls& falls = m_falls[p_position.x];
	for(FluidFalls::iterator it = falls.begin(); it != falls.end(); ++it)
	{
		if(it->startTile.y == p_position.y && it->fallType == FallType_FromWarp)
		{
			it->state = FlowState_Drain;
			removeFallTiles(it->startTile.x, it->startTile.y, it->growTile.y);
		}
		else if(it->growTile.y == p_position.y &&
			(it->state == FlowState_DoneEnterWarp || it->state == FlowState_EnterWarp))
		{
			it->growTile.y++;
			it->state = FlowState_Expand;
		}
	}

	// Process flows that flow into this warp tile
	FluidFlows& flows = m_flows[p_position.y];
	for(FluidFlows::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		if (it->growLeft.x == p_position.x &&
			(it->stateLeft == FlowState_DoneEnterWarp || it->stateLeft == FlowState_EnterWarp))
		{
			it->growLeft.x++;
			it->stateLeft = FlowState_Expand;
		}

		if (it->growRight.x == p_position.x &&
			(it->stateRight == FlowState_DoneEnterWarp || it->stateRight == FlowState_EnterWarp))
		{
			it->growRight.x--;
			it->stateRight = FlowState_Expand;
		}
	}
}


void FluidMgr::removeFallFromPairedWarp(const tt::math::Point2& p_position)
{
	for(WarpPairs::iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if(it->second.isValid)
		{
			if(it->second.warpRect1.contains(p_position))
			{
				// Drain fall at corresponding position in warpRect2
				tt::math::Point2 offset = p_position - it->second.warpRect1.getPosition();
				tt::math::Point2 fallStart = it->second.warpRect2.getPosition() + offset;
				
				FluidFalls& falls = m_falls[fallStart.x];
				for(FluidFalls::iterator fallIt = falls.begin(); fallIt != falls.end(); ++fallIt)
				{
					if (fallIt->startTile.y == fallStart.y &&
						fallIt->fallType == FallType_FromWarp)
					{
						TT_ASSERT(fallIt->warpID == it->first);

						drainFall(*fallIt);

						return;
					}
				}
			}
			else if(it->second.warpRect2.contains(p_position))
			{
				// Drain fall at corresponding position in warpRect1
				tt::math::Point2 offset = p_position - it->second.warpRect2.getPosition();
				tt::math::Point2 fallStart = it->second.warpRect1.getPosition() + offset;

				FluidFalls& falls = m_falls[fallStart.x];
				for(FluidFalls::iterator fallIt = falls.begin(); fallIt != falls.end(); ++fallIt)
				{
					if (fallIt->startTile.y == fallStart.y &&
						fallIt->fallType == FallType_FromWarp)
					{
						TT_ASSERT(fallIt->warpID == it->first);

						drainFall(*fallIt);

						return;
					}
				}
			}
		}
	}
}


bool FluidMgr::allowedToEnterWarp(const WarpID& p_id, const tt::math::Point2& p_position)
{
	for(WarpPairs::iterator it = m_warpPairs.begin(); it != m_warpPairs.end(); ++it)
	{
		if(it->second.warpRect1.contains(p_position) || it->second.warpRect2.contains(p_position))
		{
			// Found warp => allowed to enter if it is not the same ID as this warp
			return p_id != it->first;
		}
	}

	// Warp not found
	return false;
}


void FluidMgr::updateFalls(real p_elapsedTime)
{
	TT_ASSERTMSG(static_cast<s32>(m_falls.size()) == m_levelLayer->getWidth(),
	             "Fluid falls bookkeeping is incorrect for this level.\n"
	             "Falls container size %d, level width %d (should be the same).",
	             static_cast<s32>(m_falls.size()), m_levelLayer->getWidth());
	for(size_t x = 0; x < m_falls.size(); ++x)
	{
		for(FluidFalls::iterator it = m_falls[x].begin(); it != m_falls[x].end();)
		{
			if(expandFall(*it, p_elapsedTime))
			{
				++it;
			}
			else
			{
				// Fall is ready to clean up
				handleFeedPointRemoved(it->growTile + tt::math::Point2::down, it->type);
				it = m_falls[x].erase(it);
			}
		}
	}
}


void FluidMgr::updateFlows(real p_elapsedTime)
{
	TT_ASSERTMSG(static_cast<s32>(m_flows.size()) == m_levelLayer->getHeight(),
	             "Fluid flows bookkeeping is incorrect for this level.\n"
	             "Flows container size %d, level width %d (should be the same).",
	             static_cast<s32>(m_flows.size()), m_levelLayer->getHeight());
	for(size_t y = 0; y < m_flows.size(); ++y)
	{
		for(FluidFlows::iterator it = m_flows[y].begin(); it != m_flows[y].end();)
		{
			FluidFlows::iterator nextIt = it;
			++nextIt;

			Flow* adjacentFlow(0);
			if(nextIt != m_flows[y].end() && nextIt->stateLeft != FlowState_Drain)
			{
				adjacentFlow = &(*nextIt);
			}

			bool removeFlow(false);

			if(expandFlow(*it, adjacentFlow, p_elapsedTime))
			{
				if(adjacentFlow != 0 && 
					it->area.getLeft()  <= adjacentFlow->area.getRight() &&
					it->area.getRight() >= adjacentFlow->area.getLeft())
				{
					if (adjacentFlow->type      == it->type         &&
						it->stateRight          == FlowState_Expand &&
						adjacentFlow->stateLeft == FlowState_Expand)
					{
						// Merge flows
						
						adjacentFlow->area.setLeft(it->area.getLeft());
						adjacentFlow->feedPointCount += it->feedPointCount;
						adjacentFlow->growLeft = it->growLeft;
						adjacentFlow->stateLeft = it->stateLeft;
						adjacentFlow->currentLeft = it->currentLeft;

						computeFlowDirection(*adjacentFlow);

						removeFlow = true;
					}
					else if(it->type == FluidType_Water && adjacentFlow->type == FluidType_Lava)
					{
						// Shrink left flow
						shrinkFlowFromRight(*it, adjacentFlow->area.getLeft());
					}
					else if(it->type == FluidType_Lava && adjacentFlow->type == FluidType_Water)
					{
						// Shrink adjacent flow
						shrinkFlowFromLeft(*adjacentFlow, it->area.getRight());
					}
					else
					{
						// Warped flow meets itself again
						if(it->stateRight == FlowState_DoneEnterWarp)
						{
							adjacentFlow->stateLeft = FlowState_DoneEnterWarp;
						}
						else if(adjacentFlow->stateLeft == FlowState_DoneEnterWarp)
						{
							it->stateRight = FlowState_DoneEnterWarp;
						}
					}
				}
			}
			else
			{
				removeFallsFromFlow(*it);
				removeFlow = true;
			}

			if(removeFlow)
			{
				it = m_flows[y].erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}


void FluidMgr::updateFluids(real p_elapsedTime)
{
	const level::TileRegistrationMgr::TilePositions& changedPositions(m_tileRegMgr.getEntityTilesForFluids());
	
	// Handle changes from highest to lowest y value
	for(auto it = changedPositions.rbegin(); it != changedPositions.rend(); ++it)
	{
		handleTileChanged(*it);
	}
	
	m_tileRegMgr.clearEntityTilesForFluids();
	
	updateFalls(p_elapsedTime);
	updateFlows(p_elapsedTime);
}


static const char* getFlowStateAsString(FlowState p_state)
{
	switch(p_state)
	{
	case FlowState_Expand:                   return "Expand";
	case FlowState_OverflowSingle:           return "Overflow S";
	case FlowState_OverflowDoubleFirstTile:  return "Overflow D1";
	case FlowState_OverflowDoubleSecondTile: return "Overflow D2";
	case FlowState_EnterWarp:                return "Enter Warp";
	case FlowState_ExitWarp:                 return "Exit Warp";
	case FlowState_Done:                     return "Done";
	case FlowState_BlockedBySolid:           return "Block Solid";
	case FlowState_BlockedByKill:            return "Block Kill";
	case FlowState_BlockedByLava:            return "Block Lava";
	case FlowState_BlockedByStill:           return "Block Still";
	case FlowState_DoneOverflowSingle:       return "Done Overflow S";
	case FlowState_DoneOverflowDouble:       return "Done Overflow D";
	case FlowState_DoneEnterWarp:            return "Done Enter Warp";
	case FlowState_Drain:                    return "Draining";

	default: return "INVALID";
	}
}


static void renderFlow(const Flow& p_flow, s32 p_index)
{
	(void)p_index;
	using namespace tt::engine::renderer;
	tt::engine::debug::DebugRendererPtr dbg(Renderer::getInstance()->getDebug());

	static const ColorRGBA flowColor [FluidType_Count] = {ColorRGBA(0,0,255,50), ColorRGBA(255,0,0,50)};

	ColorRGBA color = flowColor[p_flow.type];

	dbg->renderSolidRect(color, p_flow.area);
	dbg->renderLine(color,
		tt::math::Vector3(p_flow.area.getLeft(), p_flow.area.getCenterPosition().y, 0),
		tt::math::Vector3(p_flow.area.getRight(), p_flow.area.getCenterPosition().y, 0));

	dbg->renderRect(ColorRGB::red, tt::math::PointRect(p_flow.growLeft,1,1));
	dbg->renderRect(ColorRGB::red, tt::math::PointRect(p_flow.growRight,1,1));

	dbg->renderLine(ColorRGB::red,
		tt::math::Vector3(static_cast<real>(p_flow.currentLeft), static_cast<real>(p_flow.growLeft.y), 0),
		tt::math::Vector3(static_cast<real>(p_flow.currentLeft), static_cast<real>(p_flow.growLeft.y + 1.5f), 0));

	dbg->renderLine(ColorRGB::red,
		tt::math::Vector3(static_cast<real>(p_flow.currentRight), static_cast<real>(p_flow.growLeft.y), 0),
		tt::math::Vector3(static_cast<real>(p_flow.currentRight), static_cast<real>(p_flow.growLeft.y + 1.5f), 0));

	tt::math::Point2 leftPos = AppGlobal::getGame()->getCamera().worldToScreen(
		p_flow.area.getPosition() + tt::math::Vector2(-1,1));

	tt::math::Point2 rightPos = AppGlobal::getGame()->getCamera().worldToScreen(
		tt::math::Vector2(p_flow.area.getRight(), p_flow.area.getBottom()));

	dbg->renderText(getFlowStateAsString(p_flow.stateLeft), leftPos.x, leftPos.y);
	dbg->renderText(getFlowStateAsString(p_flow.stateRight), rightPos.x, rightPos.y);

	//dbg->renderText(tt::str::toStr(p_flow.feedPointCount), pos.x, pos.y);
	//dbg->renderText(tt::str::toStr(p_index), leftPos.x + 10, leftPos.y + 10);

}


static void renderFall(const Fall& p_fall)
{
	using namespace tt::engine::renderer;
	tt::engine::debug::DebugRendererPtr dbg(Renderer::getInstance()->getDebug());

	static const ColorRGBA fallColor  [FluidType_Count] = {ColorRGBA(50,50,255,100), ColorRGBA(255,50,50,100)};

	// Draw fall
	dbg->renderSolidRect(fallColor[p_fall.type], p_fall.area);
	dbg->renderRect(fallColor[p_fall.type], tt::math::PointRect(p_fall.growTile, 1, 1));
	dbg->renderRect(ColorRGB::white, tt::math::PointRect(p_fall.startTile, 1, 1));

	tt::math::Point2 pos = AppGlobal::getGame()->getCamera().worldToScreen(
		p_fall.area.getPosition());

	dbg->renderText(getFlowStateAsString(p_fall.state), pos.x-8, pos.y);
}


void FluidMgr::renderFluids() const
{
	using namespace tt::engine::renderer;
	tt::engine::debug::DebugRendererPtr dbg(Renderer::getInstance()->getDebug());

	// Render falls
	for(s32 x = 0; x < m_levelLayer->getWidth(); ++x)
	{
		for(FluidFalls::const_iterator it = m_falls[x].begin(); it != m_falls[x].end(); ++it)
		{
			renderFall(*it);
		}
	}

	// Render flows
	for(s32 y = 0; y < m_levelLayer->getHeight(); ++y)
	{
		s32 index(0);
		for(FluidFlows::const_iterator it = m_flows[y].begin(); it != m_flows[y].end(); ++it)
		{
			renderFlow(*it, index);
			++index;
		}
	}

	// Render feed points
	for(s32 y = 0; y < m_levelLayer->getHeight(); ++y)
	{
		for(FeedPoints::const_iterator it = m_feedPoints[y].begin(); it != m_feedPoints[y].end(); ++it)
		{
			dbg->renderCircle(ColorRGB::white,
				tt::math::Vector2(static_cast<float>(*it) + 0.5f, static_cast<float>(y) + 0.5f), 0.25f);
		}
	}
}


void FluidMgr::resetFluidSimulation(bool p_forcePregeneration)
{
	const s32 newWidth  = m_levelLayer->getWidth();
	const s32 newHeight = m_levelLayer->getHeight();
	
	m_notifiedEntities.clear();
	
	// Remove all fluid tiles
	m_activeLayer->clear();
	
	// Because we clear the layer also clear the resources which we otherwise only clean up on tile removal.
	m_soundCues.clear();
	m_soundCuesForFalls.clear();
	m_particlesMgr.reset();
	
	m_falls.clear();
	m_falls.resize(newWidth);
	
	m_flows.clear();
	m_flows.resize(newHeight);
	
	m_feedPoints.clear();
	m_feedPoints.resize(newHeight);
	
	m_warpTiles.clear();
	
	m_tileRegMgr.clearEntityTilesForFluids();
	
	// Generate falls at all source locations
	tt::math::Point2 pos(0, 0);
	for (pos.y = 0; pos.y < newHeight; ++pos.y)
	{
		for (pos.x = 0; pos.x < newWidth; ++pos.x)
		{
			for(u32 i = 0; i < FluidType_Count; ++i)
			{
				FluidType fluidType(static_cast<FluidType>(i));
				if (hasFluidSource(pos, fluidType))
				{
					// Create fall
					addFall(pos, fluidType, FallType_FromSource);
				}
			}
		}
	}
	
	addStillWater();
	
	setAllWarpTiles(m_activeLayer);
	
	// Pregenerate
	if(m_preGenerate || p_forcePregeneration)
	{
		// FIXME: Need to determine pregeneration time
		for(u32 i = 0; i < 1000; ++i)
		{
			updateFluids(1.0f);
		}
		m_preGenerate = false;
	}
	
	//update(0.0f);
}



void FluidMgr::addStillWater()
{
	// Create still water objects.
	m_stillTilesSurface.clear();
	m_graphicsMgr.createStillFluidsQuads(m_stillTiles, m_levelLayer, &m_stillTilesSurface);
	
	for (Point2Set::iterator it = m_stillTiles.begin(); it != m_stillTiles.end(); ++it)
	{
		const tt::math::Point2& tilePos = (*it);
		FluidType fluidType = getFluidType(m_levelLayer->getCollisionType(tilePos));
		
		// place tile
		m_activeLayer->setFluidFlowType(tilePos, FluidFlowType_Still);
		m_activeLayer->setFluidType(tilePos, fluidType);
		notifyTileChange(tilePos);
	}
}


void FluidMgr::updateAudio(const tt::math::VectorRect& p_visibilityRect)
{
	// HACK: No fluid sfx in Ife area
	if (AppGlobal::getGame()->getStartInfo().getLevelName() == "area_ife")
	{
		return;
	}
	
	tt::math::PointRect visibilityTileRect = level::worldToTile(p_visibilityRect);
	
	const s32 minimumDiameter = 30 * 2; // I'm doing *2 so the radius is clear.
	const s32 maximumDiameter = 50 * 2; // We don't want to check the whole level if we're zoomed out a lot.
	
	tt::math::Point2 extraAudioDistance(0, 0);
	
	const s32 width  = visibilityTileRect.getWidth();
	if (width < minimumDiameter)
	{
		extraAudioDistance.x += ((minimumDiameter - width) + 1) / 2;
	}
	else if (width > maximumDiameter)
	{
		extraAudioDistance.x -= ((width - maximumDiameter) + 1) / 2;
	}
	const s32 height = visibilityTileRect.getHeight();
	if (height < minimumDiameter)
	{
		extraAudioDistance.y += ((minimumDiameter - height) + 1) / 2;
	}
	else if (height > maximumDiameter)
	{
		extraAudioDistance.y -= ((height - maximumDiameter) + 1) / 2;
	}
	
	tt::math::Point2 minTilePos = visibilityTileRect.getMin()     - extraAudioDistance;
	tt::math::Point2 maxTilePos = visibilityTileRect.getMaxEdge() + extraAudioDistance;
	
	if (minTilePos.x < 0) minTilePos.x = 0;
	if (minTilePos.y < 0) minTilePos.y = 0;
	TT_NULL_ASSERT(m_activeLayer);
	const s32 levelWidth  = m_activeLayer->getWidth();
	const s32 levelHeight = m_activeLayer->getHeight();
	if (maxTilePos.x >= levelWidth ) maxTilePos.x = levelWidth;
	if (maxTilePos.y >= levelHeight) maxTilePos.y = levelHeight;
	
	if (minTilePos.x > maxTilePos.x || minTilePos.y > maxTilePos.y)
	{
		return;
	}
	
	Point2s fallAudioPoints;
	
#if 0 // Use feed points. (Note: these didn't find falls falling into still fluids.)
	for (s32 yPos = minTilePos.y; yPos < maxTilePos.y; ++yPos)
	{
		TT_ASSERT(yPos >= 0 && static_cast<FeedPoints::size_type>(yPos) < m_feedPoints.size());
		const FeedPoints& feedPoints = m_feedPoints[yPos];
		
		// Find the first feedPoint within our rect.
		FeedPoints::const_iterator it = feedPoints.begin();
		while (it != feedPoints.end())
		{
			const s32 xPos = (*it);
			if (xPos > maxTilePos.x)
			{
				 // Outside rect, stop. Nothing found.
				it = feedPoints.end();
				break;
			}
			
			if (xPos > minTilePos.x)
			{
				// Inside rect.
				break;
			}
			++it;
		}
		
		s32 beginFeed = -1;
		s32 lastFeed  = -1;
		for (;it != feedPoints.end() && (*it) <= maxTilePos.x; ++it)
		{
			const s32 xPos = (*it);
			TT_ASSERT(xPos > minTilePos.x);
			
			if (beginFeed < 0) // Nothing started
			{
				beginFeed = xPos;
			}
			else if (xPos != lastFeed + 1) // Found gap
			{
				const s32 center = beginFeed + ((lastFeed - beginFeed) / 2);
				fallAudioPoints.push_back(tt::math::Point2(center, yPos));
				beginFeed = xPos;
			}
			lastFeed = xPos;
		}
		
		if (beginFeed > 0)
		{
			const s32 center = beginFeed + ((lastFeed - beginFeed) / 2);
			fallAudioPoints.push_back(tt::math::Point2(center, yPos));
		}
	}
#else // Use falls
	for (s32 xPos = minTilePos.x; xPos < maxTilePos.x; ++xPos)
	{
		TT_ASSERT(xPos >= 0 && static_cast<FluidFalls::size_type>(xPos) < m_falls.size());
		FluidFalls& falls = m_falls[xPos];
		
		// Find the lower tiles of all the falls that apply.
		for (FluidFalls::const_iterator it = falls.begin();
		     it != falls.end() && (*it).growTile.y <= maxTilePos.y; ++it)
		{
			const Fall& fall = (*it);
			
			if (fall.growTile.y >= minTilePos.y)
			{
				if(fall.state != FlowState_BlockedBySolid && fall.state != FlowState_BlockedByStill)
				{
					// Only start audio for falls that are hitting something.
					continue;
				}
				fallAudioPoints.push_back(fall.growTile);
			}
		}
	}
	
	// Sort so we can check which points we need to merge. (Connected points on y need to be merged.)
	std::sort(fallAudioPoints.begin(), fallAudioPoints.end(), tt::math::Point2LessYThenLessX());
	
	if (fallAudioPoints.empty() == false)
	{
		tt::math::Point2 startPoint(fallAudioPoints.front());
		tt::math::Point2 endPoint(startPoint.x - 1, startPoint.y); // -1 so we don't trigger gap dectection.
		Point2s copy;
		std::swap(copy, fallAudioPoints);
		for (Point2s::iterator it = copy.begin(); it != copy.end(); ++it)
		{
			const tt::math::Point2& tilePos = (*it);
			TT_ASSERT(startPoint.y <= tilePos.y);
			if (tilePos.x != endPoint.x + 1 || // Found gap
			    tilePos.y != startPoint.y    ) // On new row
			{
				const s32 center = startPoint.x + ((endPoint.x- startPoint.x) / 2);
				fallAudioPoints.push_back(tt::math::Point2(center, startPoint.y));
				TT_ASSERT(startPoint.y == endPoint.y);
				startPoint = tilePos;
			}
			endPoint = tilePos;
			TT_ASSERT(startPoint.x <= endPoint.x);
		}
		
		const s32 center = startPoint.x + ((endPoint.x- startPoint.x) / 2);
		fallAudioPoints.push_back(tt::math::Point2(center, startPoint.y));
		TT_ASSERT(startPoint.y == endPoint.y);
	}
#endif
	
	SoundCues prevSoundCuesForFalls;
	std::swap(prevSoundCuesForFalls, m_soundCuesForFalls);
	for (Point2s::iterator it = fallAudioPoints.begin(); it != fallAudioPoints.end(); ++it)
	{
		const tt::math::Point2 tilePos = (*it);
		SoundCues::iterator soundIt = prevSoundCuesForFalls.find(tilePos);
		if (soundIt != prevSoundCuesForFalls.end())
		{
			// Reuse previous sound
			m_soundCuesForFalls.insert(*soundIt);
			prevSoundCuesForFalls.erase(soundIt);
		}
		else
		{
			const tt::math::Vector2 pos(level::tileToWorld(tilePos));
			const tt::math::Vector3 worldPosition(pos.x, pos.y, 0.0f);
			using namespace tt::audio;
			
			// MARTIJN: Check one tile higher to get correct sound
			player::SoundCuePtr cue = audio::AudioPlayer::getInstance()->playPositionalEffectCue("Ambient",
					(m_activeLayer->getFluidType(tilePos + tt::math::Point2(0, 1)) == FluidType_Water) ? "ambience_waterfall" : "ambience_lavafall",
					worldPosition);
			
			if (cue != 0)
			{
				m_soundCuesForFalls.insert(std::make_pair(tilePos, cue));
			}
		}
	}
	
	// Stop all sound cues which are no longer in range.
	for (SoundCues::const_iterator it = prevSoundCuesForFalls.begin(); it != prevSoundCuesForFalls.end(); ++it)
	{
		(*it).second->stop();
	}
}


// Namespace end
}
}
}
