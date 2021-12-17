#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/math/Random.h>
#include <tt/math/math.h>

#include <toki/game/light/JitterEffect.h>


namespace toki {
namespace game {
namespace light {

//--------------------------------------------------------------------------------------------------
// Public member functions

JitterEffect::JitterEffect(real p_scaleFrequency,     real p_scaleAmplitude,
                           real p_positionXFrequency, real p_positionXAmplitude,
                           real p_positionYFrequency, real p_positionYAmplitude)
:
m_scaleJitterValue(0.0f),
m_positionJitterValue(),
m_scaleSettings(p_scaleFrequency, p_scaleAmplitude),
m_positionXSettings(p_positionXFrequency, p_positionXAmplitude),
m_positionYSettings(p_positionYFrequency, p_positionYAmplitude),
m_time(0.0f)
{
	update(0.0f);
}


void JitterEffect::update(real p_deltaTime)
{
	m_time += p_deltaTime;
	
	m_scaleJitterValue = m_scaleSettings.amplitude *
		tt::math::sin(m_time * m_scaleSettings.frequency + m_scaleSettings.phase);
	
	m_positionJitterValue.x = m_positionXSettings.amplitude *
		tt::math::sin(m_time * m_positionXSettings.frequency + m_positionXSettings.phase);
	
	m_positionJitterValue.y = m_positionYSettings.amplitude *
		tt::math::sin(m_time * m_positionYSettings.frequency + m_positionYSettings.phase);
}



void JitterEffect::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_scaleJitterValue,            p_context);
	bu::put(m_positionJitterValue,         p_context);
	bu::put(m_scaleSettings.amplitude,     p_context);
	bu::put(m_scaleSettings.frequency,     p_context);
	bu::put(m_scaleSettings.phase,         p_context);
	bu::put(m_positionXSettings.amplitude, p_context);
	bu::put(m_positionXSettings.frequency, p_context);
	bu::put(m_positionXSettings.phase,     p_context);
	bu::put(m_positionYSettings.amplitude, p_context);
	bu::put(m_positionYSettings.frequency, p_context);
	bu::put(m_positionYSettings.phase,     p_context);
	bu::put(m_time,                        p_context);
}


JitterEffectPtr JitterEffect::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	JitterEffectPtr jitterEffect(new JitterEffect());
	
	jitterEffect->m_scaleJitterValue            = bu::get<real             >(p_context);
	jitterEffect->m_positionJitterValue         = bu::get<tt::math::Vector2>(p_context);
	jitterEffect->m_scaleSettings.amplitude     = bu::get<real             >(p_context);
	jitterEffect->m_scaleSettings.frequency     = bu::get<real             >(p_context);
	jitterEffect->m_scaleSettings.phase         = bu::get<real             >(p_context);
	jitterEffect->m_positionXSettings.amplitude = bu::get<real             >(p_context);
	jitterEffect->m_positionXSettings.frequency = bu::get<real             >(p_context);
	jitterEffect->m_positionXSettings.phase     = bu::get<real             >(p_context);
	jitterEffect->m_positionYSettings.amplitude = bu::get<real             >(p_context);
	jitterEffect->m_positionYSettings.frequency = bu::get<real             >(p_context);
	jitterEffect->m_positionYSettings.phase     = bu::get<real             >(p_context);
	jitterEffect->m_time                        = bu::get<real             >(p_context);
	
	return jitterEffect;
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

JitterEffect::Settings::Settings(real p_frequency, real p_amplitude)
:
frequency(p_frequency),
amplitude(p_amplitude),
phase(0.0f)
{
	TT_ASSERT(frequency >= 0.0f);
	TT_ASSERT(amplitude >= 0.0f);
	if (p_frequency > 0.0f && p_amplitude > 0.0f)
	{
		// allow for 30% variance in frequency & amplitude
		const real maxVariance = 0.3f;
		tt::math::Random& random(tt::math::Random::getStatic());
		
		phase     = random.getNormalizedNext() * tt::math::twoPi;
		frequency = p_frequency * (1.0f - maxVariance) + 
			(p_frequency * random.getNormalizedNext() * maxVariance);
		
		amplitude = p_amplitude * (1.0f - maxVariance) + 
			(p_amplitude * random.getNormalizedNext() * maxVariance);
	}
}


JitterEffect::JitterEffect()
:
m_scaleJitterValue(0.0f),
m_positionJitterValue(),
m_scaleSettings(0.0f, 0.0f),
m_positionXSettings(0.0f, 0.0f),
m_positionYSettings(0.0f, 0.0f),
m_time(0.0f)
{
}

// Namespace end
}
}
}
