#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/particles/ParticleEffect.h>
#include <tt/math/math.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/fluid/FluidParticlesMgr.h>
#include <toki/level/helpers.h>


namespace toki {
namespace game {
namespace fluid {

static const u32 g_framesBeforeKill = 10;

//--------------------------------------------------------------------------------------------------
// Public member functions


FluidParticlesMgr::FluidParticlesMgr()
:
m_particles(),
m_retriggeredParticles(),
m_dirty(true)
{
}


void FluidParticlesMgr::update()
{
	for (TileParticles::iterator it = m_particles.begin(); it != m_particles.end(); )
	{
		const ParticlePosition&                 particlePos    = (*it).first;
		
		bool remove = false;
		// FIXME: Don't manage particle lifetime by retriggers.
		if (m_retriggeredParticles.find(particlePos) == m_retriggeredParticles.end()) // not retriggered
		{
			const tt::engine::particles::ParticleEffectPtr& particleEffect = (*it).second;
			
			if (particleEffect->isActive())
			{
				particleEffect->stop();
			}
			else // not active anymore
			{
				remove = true;
			}
			
			// the fluids are dirty (don't use dirty flag for new tiles)
			if (m_dirty && isBurstParticleType(particlePos.particleType) == false) 
			// New Tile particles are burst and should stop when they are done. And not check the dirty flag.
			{
				remove = true;
			}
		}
		
		if (remove)
		{
			it = m_particles.erase(it);
		}
		else
		{
			++it;
		}
	}
	m_retriggeredParticles.clear();
	m_dirty = false;

	checkExpandEffect(m_fallExpandParticles);
	checkExpandEffect(m_flowExpandLeftParticles);
	checkExpandEffect(m_flowExpandRightParticles);
}


void FluidParticlesMgr::triggerParticle(const tt::math::Point2& p_tilePos,
                                        TileOrientation         p_tileOrientation,
                                        ParticleType            p_particleType,
                                        FluidType               p_fluidType,
                                        bool                    p_flipX,
                                        bool                    p_flipY)
{
	TT_ASSERT(isTileOrientationValid(p_tileOrientation));
	TT_ASSERT(isValidFluidType(p_fluidType));
	
	const ParticlePosition pp(
		convertToParticlePosition(p_tilePos, p_tileOrientation, p_particleType, p_fluidType));
	
	TileParticles::iterator it = m_particles.find(pp);
	if (it == m_particles.end())
	{
		tt::engine::particles::ParticleEffectPtr particleEffect = getParticleEffect(p_fluidType, p_particleType);
		if (particleEffect == 0)
		{
			return;
		}
		
		particleEffect->getTrigger()->setOrigin(pp.worldPosition);
		
		// Potentional flipping of the particle effect
		u32 mask = 0;
		if (p_flipX) mask ^= tt::engine::particles::FlipAxis_X;
		if (p_flipY) mask ^= tt::engine::particles::FlipAxis_Y;
		particleEffect->flip(mask);
		
		particleEffect->spawn();
		
		m_particles.insert(std::make_pair(pp, particleEffect));
		
		// New trigger was added so something changed. raise dirty flag
		m_dirty = true;
	}
	
	m_retriggeredParticles.insert(pp);
}


void FluidParticlesMgr::updateExpansion(const FluidShape* p_shape, ParticleType p_type)
{
	switch(p_type)
	{
	case ParticleType_ExpandFall:
		updateExpandEffect(m_fallExpandParticles, p_shape, p_type);
		break;

	case ParticleType_ExpandFlowLeft:
		updateExpandEffect(m_flowExpandLeftParticles, p_shape, p_type);
		break;

	case ParticleType_ExpandFlowRight:
		updateExpandEffect(m_flowExpandRightParticles, p_shape, p_type);
		break;

	default:
		TT_PANIC("Not an expansion particle type: %d", p_type);
	}
}


void FluidParticlesMgr::setDirty()
{
	m_dirty = true;
}


void FluidParticlesMgr::clearParticleCache()
{
	for(u32 i = 0; i < FluidType_Count; ++i)
	{
		for(u32 j = 0; j < ParticleType_Count; ++j)
		{
			m_cache[i][j].reset();
		}
	}
}



//--------------------------------------------------------------------------------------------------
// Private member functions

bool FluidParticlesMgr::ParticlePositionLess::operator()(const ParticlePosition& p_a,
                                                         const ParticlePosition& p_b) const
{
	if (p_a.tilePosition != p_b.tilePosition)
	{
		tt::math::Point2Less less;
		return less(p_a.tilePosition, p_b.tilePosition);
	}
	else if (p_a.horizontalOrientation != p_b.horizontalOrientation)
	{
		return p_a.horizontalOrientation < p_b.horizontalOrientation;
	}
	else if (p_a.verticalOrientation != p_b.verticalOrientation)
	{
		return p_a.verticalOrientation < p_b.verticalOrientation;
	}
	else if (p_a.particleType != p_b.particleType)
	{
		return p_a.particleType < p_b.particleType;
	}
	return p_a.fluidType < p_b.fluidType;
}


FluidParticlesMgr::ParticlePosition FluidParticlesMgr::convertToParticlePosition(
		const tt::math::Point2& p_tilePos,
		TileOrientation         p_tileOrientation,
		ParticleType            p_particleType,
		FluidType               p_fluidType)
{
	TT_ASSERT(isTileOrientationValid(p_tileOrientation));
	
	tt::math::Point2          tilePosition(p_tilePos);
	HorizontalTileOrientation horizontalOrientation = HorizontalTileOrientation_Center;
	VerticalTileOrientation   verticalOrientation   = VerticalTileOrientation_Center;
	
	tt::math::Vector2         positionOffset;
	const real                tileSize = level::tileToWorld(1);
	const real                halfTile = tt::math::getHalf(tileSize);
	
	switch (p_tileOrientation)
	{
		case TileOrientation_TopLeft     :
		case TileOrientation_TopCenter   :
		case TileOrientation_TopRight    :
			verticalOrientation = VerticalTileOrientation_Top;
			positionOffset.y = tileSize;
			break;
		case TileOrientation_CenterLeft  :
		case TileOrientation_CenterCenter:
		case TileOrientation_CenterRight :
			verticalOrientation = VerticalTileOrientation_Center;
			positionOffset.y = halfTile;
			break;
		case TileOrientation_BottomLeft  :
		case TileOrientation_BottomCenter:
		case TileOrientation_BottomRight :
			verticalOrientation = VerticalTileOrientation_Top;
			--tilePosition.y;
			positionOffset.y = tileSize;
			break;
		default: break;
	}
	
	switch (p_tileOrientation)
	{
		case TileOrientation_TopLeft     :
		case TileOrientation_CenterLeft  :
		case TileOrientation_BottomLeft  :
			horizontalOrientation = HorizontalTileOrientation_Left;
			positionOffset.x = 0.0f;
			break;
		case TileOrientation_TopCenter   :
		case TileOrientation_CenterCenter:
		case TileOrientation_BottomCenter:
			horizontalOrientation = HorizontalTileOrientation_Center;
			positionOffset.x = halfTile;
			break;
		case TileOrientation_TopRight    :
		case TileOrientation_CenterRight :
		case TileOrientation_BottomRight :
			horizontalOrientation = HorizontalTileOrientation_Left;
			++tilePosition.x;
			positionOffset.x = 0.0f;
			break;
		default: break;
	}
	
	const tt::math::Vector2 worldPosition(level::tileToWorld(tilePosition) + positionOffset);
	
	return ParticlePosition(worldPosition, tilePosition, horizontalOrientation, verticalOrientation,
	                        p_particleType, p_fluidType);
}


void FluidParticlesMgr::updateExpandEffect(FluidExpandParticles& p_particles, const FluidShape* p_shape, ParticleType p_type)
{
	real xPos(0.0f);
	
	if(p_type == ParticleType_ExpandFall)
	{
		// Centered in fall tile
		xPos = p_shape->area.getLeft() + 0.5f;
	}
	else if(p_type == ParticleType_ExpandFlowLeft)
	{
		// Left of flow
		xPos = p_shape->area.getLeft();
	}
	else if(p_type == ParticleType_ExpandFlowRight)
	{
		// Right of flow
		xPos = p_shape->area.getRight();
	}
	
	const tt::math::Vector2 worldPosition2D(xPos, p_shape->area.getTop());
	const tt::math::Vector3 worldPosition3D(worldPosition2D, 0.0f);
	
	tt::engine::particles::ParticleEffectPtr particleEffect;
	
	FluidExpandParticles::iterator it = p_particles.find(p_shape);
	if (it == p_particles.end())
	{
		// No particle effect started yet

		particleEffect = getParticleEffect(p_shape->type, p_type);

		if (particleEffect == 0)
		{
			return;
		}

		particleEffect->spawn();

		FluidExpandEffect effect;
		effect.effect          = particleEffect;
		effect.soundCueWithPos = getSoundCueWithPosition(p_shape->type, p_type, worldPosition3D);
		effect.killCounter     = 0;
		
		p_particles.insert(std::make_pair(p_shape, effect));
	}
	else
	{
		FluidExpandEffect& expandEffect = it->second;
		
		particleEffect = expandEffect.effect;
		if (expandEffect.soundCueWithPos != 0)
		{
			expandEffect.soundCueWithPos->setAudioPosition(worldPosition3D);
		}
		expandEffect.killCounter = 0;
	}
	
	particleEffect->getTrigger()->setOrigin(worldPosition2D);
}


void FluidParticlesMgr::checkExpandEffect(FluidExpandParticles& p_particles)
{
	for (FluidExpandParticles::iterator it = p_particles.begin(); it != p_particles.end(); )
	{
		FluidExpandEffect& expandEffect = it->second;
		expandEffect.killCounter++;
		
		if (expandEffect.killCounter > g_framesBeforeKill)
		{
			// No updates received anymore, kill effect
			
			expandEffect.effect->stop();
			
			if (expandEffect.soundCueWithPos != 0)
			{
				audio::AudioPlayer::getInstance()->fadeOutAndKeepAliveUntilDone(
						expandEffect.soundCueWithPos->getCue(), 0.2f);
			}
			
			it = p_particles.erase(it);
		}
		else
		{
			++it;
		}
	}
}


tt::engine::particles::ParticleEffectPtr FluidParticlesMgr::getParticleEffect(FluidType    p_fluidType,
	                                                                  ParticleType p_particleType)
{
	if(m_cache[p_fluidType][p_particleType] == 0)
	{
		// Load particle effect into cache
		std::string triggerFile("particles/");
		
		switch (p_fluidType)
		{
		case FluidType_Water: triggerFile += "water_"; break;
		case FluidType_Lava:  triggerFile += "lava_";  break;
			
		default:
			TT_PANIC("Unsupported fluid type: %d", p_fluidType);
			return tt::engine::particles::ParticleEffectPtr();
		}
		
		switch (p_particleType)
		{
		case ParticleType_NewTile:              triggerFile += "new_tile";             break;
		case ParticleType_RemovedTile:          triggerFile += "removed_tile";         break;
		case ParticleType_SingleFall:           triggerFile += "single_fall";          break;
		case ParticleType_DoubleFall:           triggerFile += "double_fall";          break;
		case ParticleType_DoubleFall_Center:    triggerFile += "double_fall_center";   break;
		case ParticleType_WaterLavaCollision: 
			if (p_fluidType == FluidType_Water) triggerFile += "collision_lava";
			else                                triggerFile += "collision_water";
			break;
		case ParticleType_ExpandFall:           triggerFile += "fall_expand";          break;
		case ParticleType_ExpandFlowLeft:       triggerFile += "flow_expand_left";     break;
		case ParticleType_ExpandFlowRight:      triggerFile += "flow_expand_right";    break;
			
		default:
			TT_PANIC("Unsupported Particle Type '%d'", p_particleType);
			return tt::engine::particles::ParticleEffectPtr();
		}
		
		triggerFile += ".trigger";
		
		using namespace tt::engine::particles;
		ParticleEffectPtr particleEffect = ParticleMgr::getInstance()->createEffect(
			triggerFile, tt::math::Vector3::zero);

		if (particleEffect == 0)
		{
			return tt::engine::particles::ParticleEffectPtr();
		}

		m_cache[p_fluidType][p_particleType] = particleEffect;
	}

	return m_cache[p_fluidType][p_particleType]->clone();
}


toki::audio::SoundCueWithPositionPtr FluidParticlesMgr::getSoundCueWithPosition(
	FluidType p_fluidType, ParticleType p_particleType, const tt::math::Vector3& p_pos)
{
	if (p_fluidType == FluidType_Water &&
	   (p_particleType == ParticleType_ExpandFlowLeft || p_particleType == ParticleType_ExpandFlowRight))
	{
		return toki::audio::SoundCueWithPosition::create("Ambient", "ambience_waterstream", p_pos);
	}
	
	return toki::audio::SoundCueWithPositionPtr();
}


// Namespace end
}
}
}
