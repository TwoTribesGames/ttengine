#include <tt/code/bufferutils.h>
#include <tt/engine/renderer/Texture.h>

#include <toki/game/entity/effect/ColorGradingEffectMgr.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/serialization/utils.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {


//--------------------------------------------------------------------------------------------------
// Public member functions

ColorGradingEffectMgr::ColorGradingEffectMgr()
:
m_defaultColorTarget(),
m_defaultColorCurrent(),
m_defaultColorSecond(),
m_effectsEnabled(true)
{
	reset();
}


const tt::engine::renderer::TexturePtr& ColorGradingEffectMgr::getDefaultColorGrading() const
{
	return m_defaultColorTarget;
}


void ColorGradingEffectMgr::setDefaultColorGrading(const tt::engine::renderer::TexturePtr& p_effectTexture,
                                                   real p_duration)
{
	m_defaultColorTarget = (validateTextureForColorGrading(p_effectTexture)) ?
	                       p_effectTexture : tt::engine::renderer::TexturePtr();
	
	if (p_duration <= 0.0f)
	{
		// Instant change.
		m_defaultColorCurrent.startFade(m_defaultColorTarget, p_duration);
		m_defaultColorSecond.reset();
		return;
	}
	
	// New target is nothing, fade both colors out.
	if (m_defaultColorTarget == 0)
	{
		m_defaultColorCurrent.startFadeOut(p_duration);
		m_defaultColorSecond .startFadeOut(p_duration);
		return;
	}
	
	// Are we moving back to second color?
	if (m_defaultColorSecond.getCurrentTexture() == m_defaultColorTarget)
	{
		std::swap(m_defaultColorCurrent, m_defaultColorSecond);
	}
	
	// Can we reuse current?
	if (m_defaultColorCurrent.getCurrentTexture() == m_defaultColorTarget)
	{
		m_defaultColorCurrent.startFade(m_defaultColorTarget, p_duration);
		m_defaultColorSecond .startFadeOut(p_duration);
		return;
	}
	
	real totalEffectStrength = 0.0f;
	for (s32 e = 0; e < Constants_MaxEffectCount; ++e)
	{
		totalEffectStrength += m_effectStrength[e];
	}
	
	// Can we do a cross fade? Or should we fade out/in?
	if (m_defaultColorSecond.isValid() ||      // The second effect is already in use.
	    totalEffectStrength > 0.0f) // There are effects active.
	{
		// Start fade out/in
		m_defaultColorCurrent.startFade(m_defaultColorTarget, p_duration);
		m_defaultColorSecond .startFadeOut(p_duration);
	}
	else
	{
		// We can and may cross fade.
		std::swap(m_defaultColorCurrent, m_defaultColorSecond);
		m_defaultColorCurrent.startFade(m_defaultColorTarget, p_duration);
		m_defaultColorSecond .startFadeOut(p_duration);
	}
}


void ColorGradingEffectMgr::update(real p_elapsedTime)
{
	m_defaultColorCurrent.update(p_elapsedTime);
	m_defaultColorSecond .update(p_elapsedTime);
	
	// --------------------------------------------------------------------------------------------
	// Find out what er the new active shader effects (for this frame).
	tt::engine::renderer::TexturePtr shaderEffectTextureThisFrame [EffectInShader_Count];
	real                             shaderEffectStrengthThisFrame[EffectInShader_Count] = {0};
	
	// Find the highest strength effects and put them in the *ThisFrame array see above.
	for (s32 e = 0; e < Constants_MaxEffectCount; ++e)
	{
		// Sanity check - Make sure unused effects have 0.0 strength and no texture.
		TT_ASSERT(m_effectUsed[e] || (m_effectStrength[e] == 0.0f && m_effectTexture[e] == 0));
		
		for (s32 s = 0; s < EffectInShader_Count; ++s)
		{
			if (m_effectStrength[e] > shaderEffectStrengthThisFrame[s] &&
			    m_effectTexture[e] != 0) // Strength doesn't matter if we don't have a texture.
			{
				real                             tempStrength = shaderEffectStrengthThisFrame[s];
				tt::engine::renderer::TexturePtr tempTexture  = shaderEffectTextureThisFrame [s];
				shaderEffectStrengthThisFrame[s] = m_effectStrength[e];
				shaderEffectTextureThisFrame [s] = m_effectTexture [e];
				
				// Check if the just overriden effect can inturn override another effect in the shader array.
				for (; s < EffectInShader_Count; ++s)
				{
					if (tempStrength > shaderEffectStrengthThisFrame[s])
					{
						std::swap(tempStrength, shaderEffectStrengthThisFrame[s]);
						std::swap(tempTexture , shaderEffectTextureThisFrame [s]);
					}
				}
			}
		}
	}
	
	// Decide what get's passed to shaders.
	// Effect can completly override default color.
	
	// Get total effect strength
	const real totalEffectStrength = m_effectsEnabled ? shaderEffectStrengthThisFrame[0] : 0.0f; // We only allow 1 effect active.
	TT_STATIC_ASSERT(1 <= EffectInShader_Count);
	shaderEffectStrengthThisFrame[1] = 0.0f; // Make sure there is no second effect color.
	const real defaultStrength = (1.0f - totalEffectStrength);
	
	s32 index = (totalEffectStrength <= 0.0f) ? 0 : 1;
	if (m_defaultColorCurrent.isValid() && index < EffectInShader_Count)
	{
		shaderEffectTextureThisFrame [index] = m_defaultColorCurrent.getCurrentTexture();
		shaderEffectStrengthThisFrame[index] = m_defaultColorCurrent.getCurrentStrength() * defaultStrength;
		++index;
	}
	if (m_defaultColorSecond.isValid() && index < EffectInShader_Count)
	{
		shaderEffectTextureThisFrame [index] = m_defaultColorSecond.getCurrentTexture();
		shaderEffectStrengthThisFrame[index] = m_defaultColorSecond.getCurrentStrength() * defaultStrength;
		++index;
	}
	
	// --------------------------------------------------------------------------------------------
	// No fade from inactive to active effects. (Just set the new effects as the active shader effects.)
	for (s32 s = 0; s < EffectInShader_Count; ++s)
	{
		m_shaderEffectTexture [s] = shaderEffectTextureThisFrame [s];
		m_shaderEffectStrength[s] = shaderEffectStrengthThisFrame[s];
		tt::math::clamp(m_shaderEffectStrength[s], 0.0f, 1.0f); // Final clamp to make sure we stay within range.
	}
	
	/* DEBUG Prints
	TT_Printf("ColorGradingEffectMgr %d shader effects: ", EffectInShader_Count);
	for (s32 s = 0; s < EffectInShader_Count; ++s)
	{
		const u32 crc1 = ((m_shaderEffectTexture[s] == 0) ? 0 : m_shaderEffectTexture[s]->getEngineID().crc1);
		const u32 crc2 = ((m_shaderEffectTexture[s] == 0) ? 0 : m_shaderEffectTexture[s]->getEngineID().crc2);
		TT_Printf("[%X:%X]: %f", crc1, crc2, m_shaderEffectStrength[s]);
		if (s < EffectInShader_Count - 1)
		{
			TT_Printf(", ");
		}
	}
	TT_Printf("\n");
	// */
}


void ColorGradingEffectMgr::reset()
{
	for (s32 i = 0; i < EffectInShader_Count; ++i)
	{
		m_shaderEffectTexture [i].reset();
		m_shaderEffectStrength[i] = 0.0f;
	}
	
	m_defaultColorTarget .reset();
	m_defaultColorCurrent.reset();
	m_defaultColorSecond .reset();
	
	for (s32 i = 0; i < Constants_MaxEffectCount; ++i)
	{
		m_effectUsed    [i] = false;
		m_effectStrength[i] = 0.0f;
		m_effectTexture [i].reset();
	}
}


void ColorGradingEffectMgr::onRequestReloadAssets()
{
	// Unload all textures but store their EngineIDs.
	m_defaultColorCurrent.reset();
	m_defaultColorSecond .reset();
	
	const tt::engine::EngineID targetColorId(   (m_defaultColorTarget    != 0) ? m_defaultColorTarget   ->getEngineID() : tt::engine::EngineID(0,0));
	m_defaultColorTarget.reset();
	
	u64 effectIdValues[Constants_MaxEffectCount] = { 0 };
	for (s32 i = 0; i < Constants_MaxEffectCount; ++i)
	{
		if (m_effectTexture[i] != 0)
		{
			const tt::engine::EngineID id = m_effectTexture[i]->getEngineID();
			effectIdValues[i] = id.getValue();
			m_effectTexture[i].reset(); // Only works if we're the only one who has a reference at this point!
		}
	}
	
	// ------------------------------------------
	// Reload textures
	for (s32 i = 0; i < Constants_MaxEffectCount; ++i)
	{
		if (effectIdValues[i] != 0)
		{
			const tt::engine::EngineID id(effectIdValues[i]);
			m_effectTexture[i] = tt::engine::renderer::TextureCache::get(id, false);
		}
	}
	
	if (targetColorId.valid())
	{
		m_defaultColorTarget = tt::engine::renderer::TextureCache::get(targetColorId, false);
		setDefaultColorGrading(m_defaultColorTarget, 0.0f);
	}
}


void ColorGradingEffectMgr::serialize  (tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	for (s32 i = 0; i < EffectInShader_Count; ++i)
	{
		serialization::serializeTexturePtr(m_shaderEffectTexture [i], p_context);
		bu::put(                           m_shaderEffectStrength[i], p_context);
	}
	
	serialization::serializeTexturePtr(m_defaultColorTarget   , p_context);
	m_defaultColorCurrent.serialize(p_context);
	m_defaultColorSecond .serialize(p_context);
	bu::put(m_effectsEnabled, p_context);
	
	for (s32 i = 0; i < Constants_MaxEffectCount; ++i)
	{
		bu::put(                           m_effectUsed    [i], p_context);
		bu::put(                           m_effectStrength[i], p_context);
		serialization::serializeTexturePtr(m_effectTexture [i], p_context);
	}
}


void ColorGradingEffectMgr::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	for (s32 i = 0; i < EffectInShader_Count; ++i)
	{
		m_shaderEffectTexture [i] = serialization::unserializeTexturePtr(p_context);
		m_shaderEffectStrength[i] = bu::get<real>(                       p_context);
	}
	
	m_defaultColorTarget = serialization::unserializeTexturePtr(p_context);
	m_defaultColorCurrent.unserialize(p_context);
	m_defaultColorSecond .unserialize(p_context);
	m_effectsEnabled = bu::get<bool>(p_context);
	
	for (s32 i = 0; i < Constants_MaxEffectCount; ++i)
	{
		m_effectUsed    [i] = bu::get<bool>(                       p_context);
		m_effectStrength[i] = bu::get<real>(                       p_context);
		m_effectTexture [i] = serialization::unserializeTexturePtr(p_context);
	}
}


bool ColorGradingEffectMgr::validateTextureForColorGrading(const tt::engine::renderer::TexturePtr& p_texture)
{
	if (p_texture != 0)
	{
		const char* textureName = "";
#if !defined(TT_BUILD_FINAL)
		tt::engine::EngineID id = p_texture->getEngineID();
		textureName = id.getName().c_str();
#else
		(void) textureName;
#endif
		if (p_texture->getInfo().type != tt::engine::renderer::Type_VolumeTexture)
		{
			TT_PANIC("Can't use this texture ('%s') for color grading. Not a volume texture",
			         textureName);
			return false;
		}
		// TODO What else do we want to verify?
	}
	return true;
}


s32 ColorGradingEffectMgr::createEffect(const tt::engine::renderer::TexturePtr& p_texture, real p_strength)
{
	TT_ASSERT(validateTextureForColorGrading(p_texture));
	
	for (s32 i = 0; i < Constants_MaxEffectCount; ++i)
	{
		if (m_effectUsed    [i] == false)
		{
			m_effectUsed    [i] = true;
			m_effectTexture [i] = p_texture;
			m_effectStrength[i] = p_strength;
			return i;
		}
	}
	// No more place!
	TT_PANIC("Can't add any more Color Grading Effects!");
	return -1;
}


const tt::engine::renderer::TexturePtr& ColorGradingEffectMgr::getEffectTexture(s32 p_index) const
{
	if (isValidEffectIndex(p_index) == false)
	{
		TT_PANIC("Invalid effect index: %d", p_index);
		static tt::engine::renderer::TexturePtr empty;
		return empty;
	}
	TT_ASSERT(m_effectUsed[p_index]);
	return m_effectTexture[p_index];
}


real ColorGradingEffectMgr::getEffectStrength(s32 p_index) const
{
	if (isValidEffectIndex(p_index) == false)
	{
		TT_PANIC("Invalid effect index: %d", p_index);
		return 0.0f;
	}
	TT_ASSERT(m_effectUsed[p_index]);
	return m_effectStrength[p_index];
}


void ColorGradingEffectMgr::setEffectTexture(s32 p_index, const tt::engine::renderer::TexturePtr& p_texture)
{
	TT_ASSERT(validateTextureForColorGrading(p_texture));
	
	if (isValidEffectIndex(p_index) == false)
	{
		TT_PANIC("Invalid effect index: %d", p_index);
		return;
	}
	TT_ASSERT(m_effectUsed[p_index]);
	m_effectTexture[p_index] = p_texture;
}


void ColorGradingEffectMgr::setEffectStrength(s32 p_index, real p_strength)
{
	if (isValidEffectIndex(p_index) == false)
	{
		TT_PANIC("Invalid effect index: %d", p_index);
		return;
	}
	TT_ASSERT(m_effectUsed[p_index]);
	m_effectStrength[p_index] = p_strength;
}


void ColorGradingEffectMgr::removeEffect(s32 p_index)
{
	if (isValidEffectIndex(p_index) == false)
	{
		TT_PANIC("Invalid effect index: %d", p_index);
		return;
	}
	
	TT_ASSERT(m_effectUsed[p_index]);
	m_effectUsed     [p_index] = false;
	m_effectTexture  [p_index].reset();
	m_effectStrength [p_index] = 0.0f;
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void ColorGradingEffectMgr::FadingColor::startFade(const tt::engine::renderer::TexturePtr& p_texture, real p_duration)
{
	m_textureTarget = p_texture;
	m_startStrength = m_currentStrength;
	m_duration      = p_duration;
	m_time          = 0.0f;
	if (p_duration <= 0.0f)
	{
		m_texture         = p_texture;
		m_currentStrength = 1.0f;
		TT_ASSERT(isDone());
	}
	else if (needFadeInOut())
	{
		m_duration *= 0.5f;
	}
	else if (m_texture == 0) // We have no texture
	{
		// Fade the new texture in, but start with 0 strength.
		m_startStrength   = 0.0f;
		m_currentStrength = 0.0f;
		m_texture         = m_textureTarget;
	}
}


void ColorGradingEffectMgr::FadingColor::startFadeOut(real p_duration)
{
	startFade(tt::engine::renderer::TexturePtr(), p_duration);
}


void ColorGradingEffectMgr::FadingColor::update(real p_deltaTime)
{
	if (m_time >= m_duration)
	{
		TT_ASSERT(isDone());
		return;
	}
	
	m_time += p_deltaTime;
	
	const real targetStrength = (needFadeInOut() || m_textureTarget == 0) ? 0.0f : 1.0f;
	
	m_currentStrength = tt::math::interpolation::Easing<real>::getValue(
					m_startStrength, (targetStrength - m_startStrength), m_time, m_duration,
					tt::math::interpolation::EasingType_Linear);
	tt::math::clamp(m_currentStrength, 0.0f, 1.0f);
	
	// Check if we're done with the fade.
	if (m_time >= m_duration)
	{
		m_currentStrength = targetStrength;
		if (needFadeInOut())
		{
			// Now start the fade in. (As normal fade.)
			m_texture       = m_textureTarget;
			m_time          = 0.0f;
			m_startStrength = 0.0f;
		}
		else
		{
			// We're done.
			m_texture = m_textureTarget;
			m_time    = m_duration;
			TT_ASSERT(isDone());
		}
	}
}


void ColorGradingEffectMgr::FadingColor::reset()
{
	*this = ColorGradingEffectMgr::FadingColor();
}


void ColorGradingEffectMgr::FadingColor::serialize  (tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	serialization::serializeTexturePtr(m_texture        , p_context);
	serialization::serializeTexturePtr(m_textureTarget  , p_context);
	bu::put(                           m_currentStrength, p_context);
	bu::put(                           m_time           , p_context);
	bu::put(                           m_duration       , p_context);
	bu::put(                           m_startStrength  , p_context);
}


void ColorGradingEffectMgr::FadingColor::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_texture         = serialization::unserializeTexturePtr(p_context);
	m_textureTarget   = serialization::unserializeTexturePtr(p_context);
	m_currentStrength = bu::get<real>(p_context);
	m_time            = bu::get<real>(p_context);
	m_duration        = bu::get<real>(p_context);
	m_startStrength   = bu::get<real>(p_context);
}


ColorGradingEffectMgr::FadingColor::FadingColor()
:
m_texture(),
m_textureTarget(),
m_currentStrength(0.0f),
m_time           (0.0f),
m_duration       (0.0f),
m_startStrength  (0.0f)
{
}

// Namespace end
}
}
}
}
