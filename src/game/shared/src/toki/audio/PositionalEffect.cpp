#include <tt/audio/player/SoundCue.h>
#include <tt/pres/PresentationObject.h>

#include <toki/audio/PositionalEffect.h>
#include <toki/game/entity/Entity.h>

namespace toki {
namespace audio {

//--------------------------------------------------------------------------------------------------
// Public member functions

PositionalEffectPtr PositionalEffect::create(
	const AudioPlayer& p_player,
	const std::string& p_soundbank,
	const std::string& p_name,
	const tt::pres::PresentationObjectPtr& p_presentationObject)
{
	return createImpl(p_player, p_soundbank, p_name, p_presentationObject);
}


PositionalEffectPtr PositionalEffect::create(
	const AudioPlayer& p_player,
	const std::string& p_soundbank,
	const std::string& p_name,
	const game::entity::EntityHandle& p_entity)
{
	return createImpl(p_player, p_soundbank, p_name, p_entity);
}


PositionalEffectPtr PositionalEffect::create(
	const AudioPlayer& p_player,
	const std::string& p_soundbank,
	const std::string& p_name,
	const tt::math::Vector3& p_staticPosition)
{
	return createImpl(p_player, p_soundbank, p_name, p_staticPosition);
}


PositionalEffectPtr PositionalEffect::create(
	const AudioPlayer&               p_player,
	const std::string&               p_soundbank,
	const std::string&               p_name,
	const AudioPositionInterfacePtr& p_interface)
{
	return createImpl(p_player, p_soundbank, p_name, p_interface);
}


bool PositionalEffect::update(const AudioPlayer& p_player)
{
	TT_NULL_ASSERT(m_cue);
	tt::audio::player::SoundPlayer* soundPlayer(p_player.m_sound);
	if (soundPlayer == 0 || m_cue == 0)
	{
		return false;
	}
	
	// Check end conditions
	const tt::audio::player::SoundCue::State state = m_cue->getState();
	if (m_isLooping)
	{
		if (m_cue.use_count() <= 1)
		{
			stop();
			return false;
		}
	}
	else if (state != tt::audio::player::SoundCue::State_Playing &&
	         state != tt::audio::player::SoundCue::State_Paused)
	{
		return false;
	}
	
	updateLastKnownPosition();
	
	bool isInRange = true;
	if (m_isLooping)
	{
		real innerRadius = 0.0f;
		real outerRadius = 0.0f;
		if (m_cue->getRadius(&innerRadius, &outerRadius) == false)
		{
			TT_PANIC("Failed to get radius of cue. Has the radius been set?");
			return false;
		}
		
		isInRange = p_player.isInAudibleRange(m_lastKnownPosition, outerRadius);
		
		if (isInRange &&
		    state != tt::audio::player::SoundCue::State_Playing &&
		    state != tt::audio::player::SoundCue::State_Paused)
		{
			m_cue->play();
		}
		else if (isInRange == false && 
		         (state == tt::audio::player::SoundCue::State_Playing ||
		          state == tt::audio::player::SoundCue::State_Paused))
		{
			m_cue->stop();
		}
	}
	
	if (isInRange)
	{
		// Only update position if position has changed. Unfortunately this doesn't work with Xact3Player()
		// as that implementation also requires an update if the listener position has changed. Therefore, always
		// set position when in range
		m_cue->setPosition(m_lastKnownPosition);
	}
	return true;
}


void PositionalEffect::stop()
{
	TT_NULL_ASSERT(m_cue);
	if (m_cue != 0)
	{
		m_cue->stop();
		m_cue.reset();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PositionalEffect::updateLastKnownPosition()
{
	switch (m_sourceToUse)
	{
	case SourceType_Presentation:
		{
			tt::pres::PresentationObjectPtr presObj(m_presentationObject.lock());
			if (presObj != 0)
			{
				m_lastKnownPosition = tt::math::Vector3::zero * presObj->getCombinedMatrix();
			}
		}
		break;
		
	case SourceType_Entity:
		{
			const game::entity::Entity* entity = m_entity.getPtr();
			if (entity != 0)
			{
				m_lastKnownPosition = entity->getPositionForParticles(tt::math::Vector2::zero);
			}
		}
		break;
		
	case SourceType_Static:
		// No need to do anything: m_lastKnownPosition contains the sound position, and will not change
		break;
		
	case SourceType_PosInterface:
		{
			AudioPositionInterfacePtr interfacePtr = m_interface.lock();
			if (interfacePtr != 0)
			{
				m_lastKnownPosition = interfacePtr->getAudioPosition();
			}
		}
		break;
		
	default:
		TT_PANIC("Unsupported positional sound effect source type: %d", m_sourceToUse);
	}
}


// Namespace end
}
}
