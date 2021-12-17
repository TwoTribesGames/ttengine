#include <tt/math/math.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/toStr.h>

#include <toki/game/fluid/WaveGenerator.h>
#include <toki/cfg.h>


namespace toki {
namespace game {
namespace fluid {

const tt::math::Point2 WaveGenerator::ms_invalidPos(-1,-1);

//--------------------------------------------------------------------------------------------------
// Public member functions

WaveGenerator::WaveGenerator()
:
m_surfaceHeights(),
m_startPos(),
m_prevPos(),
m_accumulator(0),
m_angle(0),
m_simulationPointsPerTile(8),
m_waveSettings(),
m_maxHeight(cfg()->getRealDirect("toki.fluids.waves.maxheight")),
m_minHeight(cfg()->getRealDirect("toki.fluids.waves.minheight"))
{
	std::string baseCfg("toki.fluids.waves.");
	
	m_velocitydiffusion[FluidType_Water] = cfg()->getRealDirect(baseCfg + "water.velocitydiffusion");
	m_velocitydiffusion[FluidType_Lava] = cfg()->getRealDirect(baseCfg + "lava.velocitydiffusion");
	
	m_diffusion[FluidType_Water] = cfg()->getRealDirect(baseCfg + "water.diffusion");
	m_diffusion[FluidType_Lava] = cfg()->getRealDirect(baseCfg + "lava.diffusion");
	
	for (s32 fluidType = 0; fluidType < FluidType_Count; ++fluidType)
	{
		for (s32 surfaceType = 0; surfaceType < PoolSurfaceType_Count; ++surfaceType)
		{
			std::string cfgOpt(baseCfg);
			switch(fluidType)
			{
			case FluidType_Water: cfgOpt += "water."; break;
			case FluidType_Lava:  cfgOpt += "lava.";break;
			default: break;
			}
			switch(surfaceType)
			{
			case PoolSurfaceType_Left:  cfgOpt += "default_wave_left"; break;
			case PoolSurfaceType_Still: cfgOpt += "default_wave_still"; break;
			case PoolSurfaceType_Right: cfgOpt += "default_wave_right"; break;
			default: break;
			}
			
			PersistentWaveSettings settings(cfg()->getRealDirect(cfgOpt + tt::str::toStr(1) + ".movespeed"),
			                                cfg()->getRealDirect(cfgOpt + tt::str::toStr(1) + ".wavewidth"),
			                                cfg()->getRealDirect(cfgOpt + tt::str::toStr(1) + ".waveheight"));
			
			m_waveSettings[fluidType][surfaceType ].push_back(settings);
			
			settings.moveSpeed  = cfg()->getRealDirect(cfgOpt + tt::str::toStr(2) + ".movespeed");
			settings.waveWidth  = cfg()->getRealDirect(cfgOpt + tt::str::toStr(2) + ".wavewidth");
			settings.waveHeight = cfg()->getRealDirect(cfgOpt + tt::str::toStr(2) + ".waveheight");
			
			m_waveSettings[fluidType][surfaceType ].push_back(settings);
			
		}
	}
}


void WaveGenerator::update(real p_delta)
{
	const real fixedUpdateTime = 1/60.0f;
	
	m_accumulator += p_delta;
	while (m_accumulator > fixedUpdateTime)
	{
		m_accumulator -= fixedUpdateTime;
		
		m_angle += fixedUpdateTime;
		m_angle = tt::math::fmod(m_angle, 2 * tt::math::pi);
		
		for (SurfaceHeights::iterator it = m_surfaceHeights.begin(); it != m_surfaceHeights.end();)
		{
			if (it->second.isDead)
			{
				it = m_surfaceHeights.erase(it);
				continue;
			}
			
			
			it->second.prevDensity.resize(it->second.simulationSize);
			it->second.currDensity.resize(it->second.simulationSize);
			
			it->second.prevDensity.swap(it->second.currDensity);
			
			for (Reals::size_type i = 1; i < it->second.prevDensity.size() - 1; ++i)
			{
				// SIMULATION LOGIC (Shallow water equation)
				//----------------------
				const real velocity = it->second.prevDensity[i] - it->second.currDensity[i];
				const real smooth =  (it->second.prevDensity[i - 1] + it->second.prevDensity[i + 1]) / 2.0f;
				
				
				it->second.currDensity[i] = smooth + velocity * m_velocitydiffusion[it->second.fluidType];
				it->second.currDensity[i] *= m_diffusion[it->second.fluidType];
				
				//----------------------
				
				PoolSurfaceType surfaceType = PoolSurfaceType_Invalid;
				
				if (i < it->second.leftToStillIndex)
				{
					surfaceType = PoolSurfaceType_Left;
				}
				else if (i > it->second.stillToRightIndex)
				{
					surfaceType = PoolSurfaceType_Right;
				}
				else surfaceType = PoolSurfaceType_Still;
				
				// Generate waves from the wave settings
				for (PersistentWaves::const_iterator settsIt = m_waveSettings[it->second.fluidType][surfaceType].begin();
					settsIt != m_waveSettings[it->second.fluidType][surfaceType].end(); ++settsIt)
				{
					const real phase = tt::math::pi * static_cast<real>(i) * settsIt->waveWidth;
					const real phaseOffset = m_angle * settsIt->moveSpeed;
					const real heightFactor = settsIt->waveHeight;
					const real change       = tt::math::sin(phase + phaseOffset) * heightFactor;
					it->second.currDensity[i] += change;
				}
				
				// clamp the heights
				tt::math::clamp(it->second.currDensity[i], m_minHeight, m_maxHeight);
				tt::math::clamp(it->second.prevDensity[i], m_minHeight, m_maxHeight);
				
				const s32 stepsPerTile = m_simulationPointsPerTile;
				
				// fade the sides that need fading
				if (it->second.leftEdge == EdgeOption_LeftFadeout)
				{
					real fadeFactor;
					if (i < ExtraSimulationHeights)
					{
						fadeFactor = 0.0f;
					}
					else if (i > static_cast<Reals::size_type>(stepsPerTile + ExtraSimulationHeights))
					{
						fadeFactor = 1.0f;
					}
					else
					{
						fadeFactor = (i - ExtraSimulationHeights) / static_cast<real>(stepsPerTile);
					}
					it->second.currDensity[i] *= fadeFactor;
					it->second.prevDensity[i] *= fadeFactor;
				}
				if (it->second.rightEdge == EdgeOption_RightFadeout)
				{
					real fadeFactor;
					Reals::size_type startSlope = it->second.prevDensity.size() - static_cast<Reals::size_type>(stepsPerTile + ExtraSimulationHeights);
					if (i > startSlope + stepsPerTile)
					{
						fadeFactor = 0.0f;
					}
					else if (i < startSlope)
					{
						fadeFactor = 1.0f;
					}
					else
					{
						fadeFactor = 1 - ((i - startSlope) / static_cast<real>(stepsPerTile));
					}
					it->second.currDensity[i] *= fadeFactor;
					it->second.prevDensity[i] *= fadeFactor;
				}
			}
			
			++it;
		}
	}
	
	for (DissipatingWaves::iterator it = m_dissipatingWaves.begin();
	     it != m_dissipatingWaves.end(); )
	{
		it->timeLeft -= p_delta;
		if (it->timeLeft < 0)
		{
			it = m_dissipatingWaves.erase(it);
		}
		else
		{
			real time = 1 - (it->timeLeft / it->duration);
			real height = tt::math::sin((time + 0.5f) * tt::math::twoPi);
			SurfaceHeights::iterator simulationIt = m_surfaceHeights.find(it->startPosition);
			
			if (simulationIt != m_surfaceHeights.end())
			{
				for (u32 index = it->startIndex; index <= it->endIndex; ++index)
				{
					if (index >= simulationIt->second.prevDensity.size() - 1 - ExtraSimulationHeights * 2 ||
					    index <= 1 + ExtraSimulationHeights) continue;
					
					simulationIt->second.prevDensity[index] = it->strength * height;
					simulationIt->second.currDensity[index] = it->strength * height;
				}
				++it;
			}
			else
			{
				it = m_dissipatingWaves.erase(it);
			}
		}
	}
}


void WaveGenerator::updateForRender()
{
	for (SurfaceHeights::iterator it = m_surfaceHeights.begin(); it != m_surfaceHeights.end(); ++it)
	{
		it->second.leftToStillIndex  = 0;
		it->second.stillToRightIndex = 0;
		
		it->second.isDead         = true;
		it->second.simulationSize = 0;
	}
}


void WaveGenerator::startWave(const tt::math::Point2& p_position, real p_tilePosition,
                              real p_width, real p_strength, real p_duration)
{
	const tt::math::Point2& startPos = getStartPos(p_position);
	if (startPos == ms_invalidPos) return;
	
	s32 startIndex = static_cast<s32>((p_tilePosition - p_width * 0.5f) * getWaveQuadsPerTile());
	s32 indexCount = static_cast<s32>(p_width * getWaveQuadsPerTile());
	
	u32 startSimulationIndex = getSimulationIndex(p_position, startIndex, startPos);
	u32 endSimulationIndex   = getSimulationIndex(p_position, startIndex + indexCount, startPos);
	
	m_dissipatingWaves.push_back(DissipatingWave(p_strength, p_duration, startPos,
	                                             startSimulationIndex, endSimulationIndex));
}


void WaveGenerator::setHeight(real p_height, const tt::math::Point2& p_position, s32 p_waveQuadIndex)
{
	const tt::math::Point2& startPos = getStartPos(p_position);
	
	SurfaceHeights::iterator it = m_surfaceHeights.find(startPos);
	if (it == m_surfaceHeights.end()) return;
	
	u32 simulationIndex = getSimulationIndex(p_position, p_waveQuadIndex, startPos);
	
	if (simulationIndex >= it->second.prevDensity.size() - 1 - ExtraSimulationHeights * 2 ||
	    simulationIndex <= 1 + ExtraSimulationHeights) return;
	
	it->second.prevDensity[simulationIndex] = p_height;
}


real WaveGenerator::getHeight(const tt::math::Point2& p_position, s32 p_waveQuadIndex) const
{
	const tt::math::Point2& startPos = getStartPos(p_position);
	
	SurfaceHeights::const_iterator it = m_surfaceHeights.find(startPos);
	if (it == m_surfaceHeights.end())
	{
		return -1.0f;
	}
	
	const u32 simulationIndex = getSimulationIndex(p_position, p_waveQuadIndex, startPos);
	
	if (it->second.prevDensity.size() <= simulationIndex)
	{
		return -1.0f;
	}
	
	// get the Average of this position and the 2 adjacent
	real out = it->second.currDensity[simulationIndex];
	
	if (simulationIndex + 1 > it->second.prevDensity.size())
	{
		out += it->second.currDensity[simulationIndex + 1];
	}
	
	if (simulationIndex > 0)
	{
		out += it->second.currDensity[simulationIndex - 1];
	}
	
	return out / 3.0f;
}


void WaveGenerator::addSimulationPosition(const tt::math::Point2& p_position, 
                                          EdgeOption p_edgeOption, PoolSurfaceType p_poolType,
                                          FluidType p_fluidType)
{
	const real defaultHeight = 0.0f;
	tt::math::Point2 startPos = getStartPos(p_position);
	
	SurfaceHeights::iterator it = m_surfaceHeights.find(startPos);
	// new position or a left edge, Start a new simulation.
	if (it == m_surfaceHeights.end() || p_edgeOption == EdgeOption_LeftEdge || p_edgeOption == EdgeOption_LeftFadeout)
	{
		SimulationData newWaveData;
		
		// remove the simulation to the right
		SurfaceHeights::iterator rightIt = m_surfaceHeights.find(startPos + tt::math::Point2::right);
		if (rightIt != m_surfaceHeights.end() && 
		    (rightIt->second.leftEdge == EdgeOption_LeftEdge ||
		     rightIt->second.leftEdge == EdgeOption_LeftFadeout))
		{
			// swap the old data with the new data, and add space for the new tile in the front
			newWaveData.prevDensity.swap(rightIt->second.prevDensity);
			newWaveData.prevDensity.insert(newWaveData.prevDensity.begin(), 
			                               convertQuadIndexToSimulationIndex(getWaveQuadsPerTile() * 2), 
			                               defaultHeight);
			newWaveData.currDensity.swap(rightIt->second.currDensity);
			newWaveData.currDensity.insert(newWaveData.currDensity.begin(), 
			                               convertQuadIndexToSimulationIndex(getWaveQuadsPerTile() * 2), 
			                               defaultHeight);
			
			m_surfaceHeights.erase(rightIt);
		}
		
		// if this startpos does not exist create a new WaveData with the position (not with the startPos)
		it = m_surfaceHeights.insert(std::make_pair(p_position, newWaveData)).first;
		it->second.leftEdge = p_edgeOption;
	}
	// position is start position but not a left edge, Remove this simulation and append to simulation on the left.
	else if (p_position == startPos && p_edgeOption != EdgeOption_LeftEdge && p_edgeOption != EdgeOption_LeftFadeout)
	{
		const tt::math::Point2& leftStartPos = getStartPos(p_position + tt::math::Point2::left);
		SurfaceHeights::iterator leftIt = m_surfaceHeights.find(leftStartPos);
		
		if (leftIt == m_surfaceHeights.end()) return;
		
		leftIt->second.currDensity.insert(leftIt->second.currDensity.end(), it->second.currDensity.begin(),  it->second.currDensity.end());
		leftIt->second.prevDensity.insert(leftIt->second.prevDensity.end(), it->second.prevDensity.begin(),  it->second.prevDensity.end());
		
		leftIt->second.rightEdge = it->second.rightEdge;
		m_surfaceHeights.erase(it);
		it = leftIt;
		startPos = leftStartPos;
	}
	
	it->second.rightEdge = p_edgeOption;
	
	TT_ASSERT(it->second.prevDensity.size() == it->second.currDensity.size());
	
	const u32 maxsimulationIndex = getSimulationIndex(p_position, getWaveQuadsPerTile() * 2 + ExtraSimulationHeights, startPos);
	
	switch (p_poolType)
	{
	case PoolSurfaceType_Left:  it->second.leftToStillIndex  = maxsimulationIndex; break;
	case PoolSurfaceType_Still: it->second.stillToRightIndex = maxsimulationIndex; break;
	case PoolSurfaceType_Right: break;
	default: TT_PANIC("Invalid Quad Type for surface");
	}
	
	it->second.fluidType = p_fluidType;
	
	// make sure the vector is big enough for the max index and the ExtraSimulationHeights margins
	if (it->second.prevDensity.size() < maxsimulationIndex + 1)
	{
		it->second.prevDensity.resize(maxsimulationIndex + 1, defaultHeight);
		it->second.currDensity.resize(maxsimulationIndex + 1, defaultHeight);
	}
	
	it->second.isDead = false;
	it->second.simulationSize = maxsimulationIndex + 1;
}


void WaveGenerator::handleLevelResized()
{
	m_surfaceHeights.clear();
}


//--------------------------------------------------------------------------------------------------
// Private member functions


const tt::math::Point2& WaveGenerator::getStartPos(const tt::math::Point2& p_pos) const
{
	for (SurfaceHeights::const_iterator it = m_surfaceHeights.begin(); 
	     it != m_surfaceHeights.end(); ++it)
	{
		if (it->first.y < p_pos.y) continue;
		if (it->first.y > p_pos.y || it->first.x > p_pos.x)
		{
			return ms_invalidPos;
		}
		
		if (it->first.x == p_pos.x) return it->first;
		
		SurfaceHeights::const_iterator nextIt = it; ++nextIt;
		if (nextIt == m_surfaceHeights.end() || nextIt->first.x > p_pos.x || 
		    nextIt->first.y > p_pos.y)
		{
			return it->first;
		}
		
		if (nextIt->first.x == p_pos.x) return nextIt->first;
	}
	return ms_invalidPos;
}


u32 WaveGenerator::getSimulationIndex(const tt::math::Point2& p_position, s32 p_waveQuadIndex,
                                      const tt::math::Point2& p_startPosition) const
{
	// get the offset between this position and the start position
	s32 offset = p_position.x - p_startPosition.x;
	// get the quadindex from the offset
	u32 index = (getWaveQuadsPerTile()*2) * offset + p_waveQuadIndex + ExtraSimulationHeights;
	
	// convert quadindex to simulationIndex
	return convertQuadIndexToSimulationIndex(index);
}


u32 WaveGenerator::convertQuadIndexToSimulationIndex(u32 p_waveQuadIndex) const
{
	return static_cast<u32>(tt::math::round(m_simulationPointsPerTile / (getWaveQuadsPerTile()*2.0f) * p_waveQuadIndex));
}


// Namespace end
}
}
}
