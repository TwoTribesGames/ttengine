#if !defined(INC_TOKI_AUDIO_POSITIONALEFFECT_H)
#define INC_TOKI_AUDIO_POSITIONALEFFECT_H

#include <tt/audio/player/SoundPlayer.h>
#include <tt/math/Vector3.h>
#include <tt/pres/fwd.h>


#include <toki/audio/AudioPlayer.h>
#include <toki/audio/fwd.h>
#include <toki/game/entity/fwd.h>


namespace toki {
namespace audio {

class PositionalEffect
{
public:
	static PositionalEffectPtr create(
		const AudioPlayer& p_player,
		const std::string& p_soundbank,
		const std::string& p_name,
		const tt::pres::PresentationObjectPtr& p_presentationObject);
	
	static PositionalEffectPtr create(
		const AudioPlayer& p_player,
		const std::string& p_soundbank,
		const std::string& p_name,
		const game::entity::EntityHandle& p_entity);
	
	static PositionalEffectPtr create(
		const AudioPlayer& p_player,
		const std::string& p_soundbank,
		const std::string& p_name,
		const tt::math::Vector3& p_staticPosition);
	
	static PositionalEffectPtr create(
		const AudioPlayer&               p_player,
		const std::string&               p_soundbank,
		const std::string&               p_name,
		const AudioPositionInterfacePtr& p_interface);
	
	void stop();
	
	bool update(const AudioPlayer& p_playerr);
	
	inline const tt::audio::player::SoundCuePtr&  getCue() const { return m_cue; }
	inline const std::string& getName() const { return m_effectName; }
	inline const std::string& getSoundBank() const { return m_effectSoundbank; }
	
private:
	enum SourceType
	{
		SourceType_Presentation,
		SourceType_Entity,
		SourceType_Static,  // sound effect does not follow anything: remains at a static position
		SourceType_PosInterface,
		
		SourceType_Invalid
	};
	
	// Default constructor to allow this type to be used in std::vector
	inline PositionalEffect()
	:
	m_effectSoundbank(),
	m_effectName(),
	m_sourceToUse(SourceType_Invalid),
	m_presentationObject(),
	m_entity(),
	m_interface(),
	m_cue(),
	m_lastKnownPosition(tt::math::Vector3::zero),
	m_isLooping(false)
	{
	}
	
	inline PositionalEffect(const std::string&                     p_effectSoundbank,
	                        const std::string&                     p_effectName,
	                        const tt::pres::PresentationObjectPtr& p_presentationObject,
	                        bool                                   p_isLooping)
	:
	m_effectSoundbank(p_effectSoundbank),
	m_effectName(p_effectName),
	m_sourceToUse(SourceType_Presentation),
	m_presentationObject(p_presentationObject),
	m_entity(),
	m_interface(),
	m_cue(),
	m_lastKnownPosition(tt::math::Vector3::zero),
	m_isLooping(p_isLooping)
	{
	}
	
	inline PositionalEffect(const std::string&                p_effectSoundbank,
	                        const std::string&                p_effectName,
	                        const game::entity::EntityHandle& p_entity,
	                        bool                              p_isLooping)
	:
	m_effectSoundbank(p_effectSoundbank),
	m_effectName(p_effectName),
	m_sourceToUse(SourceType_Entity),
	m_presentationObject(),
	m_entity(p_entity),
	m_interface(),
	m_cue(),
	m_lastKnownPosition(tt::math::Vector3::zero),
	m_isLooping(p_isLooping)
	{
	}
	
	inline PositionalEffect(const std::string&       p_effectSoundbank,
	                        const std::string&       p_effectName,
	                        const tt::math::Vector3& p_staticPosition,
	                        bool                     p_isLooping)
	:
	m_effectSoundbank(p_effectSoundbank),
	m_effectName(p_effectName),
	m_sourceToUse(SourceType_Static),
	m_presentationObject(),
	m_entity(),
	m_interface(),
	m_cue(),
	m_lastKnownPosition(p_staticPosition),
	m_isLooping(p_isLooping)
	{
	}
	
	inline PositionalEffect(const std::string&               p_effectSoundbank,
	                        const std::string&               p_effectName,
	                        const AudioPositionInterfacePtr& p_interface,
	                        bool                             p_isLooping)
	:
	m_effectSoundbank(p_effectSoundbank),
	m_effectName(p_effectName),
	m_sourceToUse(SourceType_PosInterface),
	m_presentationObject(),
	m_entity(),
	m_interface(p_interface),
	m_cue(),
	m_lastKnownPosition(tt::math::Vector3::zero),
	m_isLooping(p_isLooping)
	{
	}
	
	void updateLastKnownPosition();
	
	template<typename Type>
	static inline PositionalEffectPtr createImpl(const AudioPlayer& p_player,
	                                             const std::string& p_soundbank,
	                                             const std::string& p_name,
	                                             Type p_param)
	{
		tt::audio::player::SoundPlayer* soundPlayer(p_player.m_sound);
		if (soundPlayer == 0)
		{
			return PositionalEffectPtr();
		}
		
		const bool isLooping(p_player.isCueLooping(p_soundbank, p_name));
		
		static const real defaultInnerRadius = 30.0f;
		static const real defaultOuterRadius = 60.0f;
		
		PositionalEffectPtr effect(new PositionalEffect(p_soundbank, p_name, p_param, isLooping));
		effect->updateLastKnownPosition();
		
		const bool isInRange(p_player.isInAudibleRange(effect->m_lastKnownPosition, defaultOuterRadius));
		
		if (isInRange == false && isLooping == false)
		{
			return PositionalEffectPtr();
		}
		
		// All good, now create and set the cue
		effect->m_cue = soundPlayer->createCue(p_soundbank, p_name, true);
		if (effect->m_cue == 0)
		{
			return PositionalEffectPtr();
		}
		
		// FIXME: Set correct defaults
		effect->m_cue->setRadius(defaultInnerRadius, defaultOuterRadius);
		
		// Ensure an initial effect position is set
		effect->m_cue->setPosition(effect->m_lastKnownPosition);
		
		// Play if in range
		if (isInRange)
		{
			effect->m_cue->play();
		}
		
		return effect;
	}
	
	std::string                         m_effectSoundbank;
	std::string                         m_effectName;
	SourceType                          m_sourceToUse;
	tt::pres::PresentationObjectWeakPtr m_presentationObject;
	game::entity::EntityHandle          m_entity;
	AudioPositionInterfaceWeakPtr       m_interface;
	tt::audio::player::SoundCuePtr      m_cue;
	tt::math::Vector3                   m_lastKnownPosition;
	bool                                m_isLooping;
};
	

// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_POSITIONALEFFECT_H)
