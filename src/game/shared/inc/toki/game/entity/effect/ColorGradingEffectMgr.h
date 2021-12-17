#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_COLORGRADINGEFFECTMGR_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_COLORGRADINGEFFECTMGR_H

#include <tt/engine/renderer/fwd.h>

#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {

class ColorGradingEffectMgr
{
public:
	enum EffectInShader // These are the effects that can be crossfaded in the shader.
	{
		EffectInShader_One,
		EffectInShader_Two,
		
		EffectInShader_Count
	};
	static inline bool isValidEffectInShader(EffectInShader p_effect) { return p_effect >= 0 && p_effect < EffectInShader_Count; }
	
	ColorGradingEffectMgr();
	
	// Default effect getters & setters
	const tt::engine::renderer::TexturePtr& getDefaultColorGrading() const;
	void setDefaultColorGrading(const tt::engine::renderer::TexturePtr& p_effectTexture, real p_duration);
	
	void update(real p_elapsedTime);
	void reset();
	void onRequestReloadAssets();
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	// Getters for shader
	inline const tt::engine::renderer::TexturePtr& getShaderEffectTexture(EffectInShader p_effect)  const
	{
		TT_ASSERT(isValidEffectInShader(p_effect));
		return m_shaderEffectTexture[p_effect];
	}
	inline real getShaderEffectStrength(EffectInShader p_effect) const
	{
		TT_ASSERT(isValidEffectInShader(p_effect));
		return m_shaderEffectStrength[p_effect];
	}
	inline real getShaderEffectTotalStrength() const
	{
		real totalStrength = 0.0f;
		for (s32 i = 0; i < EffectInShader_Count; ++i)
		{
			totalStrength += m_shaderEffectStrength[i];
		}
		if (totalStrength > 1.0f)
		{
			totalStrength = 1.0f;
		}
		TT_MINMAX_ASSERT(totalStrength, 0.0f, 1.0f);
		return totalStrength;
	}
	
	static bool validateTextureForColorGrading(const tt::engine::renderer::TexturePtr& p_texture);
	
	inline const bool areEffectsEnabled() { return m_effectsEnabled; }
	void setEffectsEnabled(bool p_enabled) { m_effectsEnabled = p_enabled; }
	s32 createEffect(const tt::engine::renderer::TexturePtr& p_texture, real p_strength);
	const tt::engine::renderer::TexturePtr& getEffectTexture (s32 p_index) const;
	real                                    getEffectStrength(s32 p_index) const;
	void setEffectTexture (s32 p_index, const tt::engine::renderer::TexturePtr& p_texture);
	void setEffectStrength(s32 p_index, real p_strength);
	void removeEffect(s32 p_index);
	static inline bool isValidEffectIndex(s32 p_index) { return p_index >= 0 && p_index < Constants_MaxEffectCount; }
	
private:
	enum Constants
	{
		Constants_MaxEffectCount = 64
	};
	struct FadingColor
	{
		inline bool isDone()   const { return m_time >= m_duration; }
		inline const tt::engine::renderer::TexturePtr& getTargetTexture()  const { return m_textureTarget;   }
		inline const tt::engine::renderer::TexturePtr& getCurrentTexture() const { return m_texture;         }
		inline real getCurrentStrength()                                   const { return m_currentStrength; }
		inline bool isValid() const { return m_texture != 0 || m_textureTarget != 0; }
		
		void startFade(const tt::engine::renderer::TexturePtr& p_texture, real p_duration);
		void startFadeOut(real p_duration);
		void update(real p_deltaTime);
		void reset();
		
		void serialize  (tt::code::BufferWriteContext* p_context) const;
		void unserialize(tt::code::BufferReadContext*  p_context);
		
		inline bool needFadeInOut() const { return m_texture != m_textureTarget &&           // We need fade in/out if we're changing textures.
		                                           m_texture != 0 && m_textureTarget != 0; } // But not if we're going to/from nothing.
		
		tt::engine::renderer::TexturePtr m_texture;
		tt::engine::renderer::TexturePtr m_textureTarget;
		real                             m_currentStrength;
		real                             m_time;
		real                             m_duration;
		real                             m_startStrength;
		
		FadingColor();
	};
	
	// These are bound to the shader. (SoA)
	tt::engine::renderer::TexturePtr m_shaderEffectTexture [EffectInShader_Count];
	real                             m_shaderEffectStrength[EffectInShader_Count];
	
	// Default effect.
	tt::engine::renderer::TexturePtr m_defaultColorTarget;  // The target default color.
	// Default transitions
	FadingColor                      m_defaultColorCurrent;
	FadingColor                      m_defaultColorSecond;  // The second color, this one is blended of faded.
	
	// The big list of effects. (SoA)
	// ------------------------------
	// Sorted on strength. (Don't really sort, just find the biggest strength.)
	// The highest effects (the number is EffectInShader_Count) are blended in shader.
	// 
	// How we blend/fade new/old shader effects in/out of active use:
	// * The 'in shader' effects can swap places with out any problem. (Doesn't matter for the final effect.)
	// * If we have a new effect which is replaces an active one we'll needs to fade the old one out and the new one is then faded in.
	// * Even if the effect that's faded out doesn't exit in the list anymore, we still fade it out. (Don't just turn the effect off.)
	// * We can start with the fade in directly if no effect was active or strength was already 0.
	bool                             m_effectsEnabled;
	bool                             m_effectUsed    [Constants_MaxEffectCount];
	real                             m_effectStrength[Constants_MaxEffectCount];
	tt::engine::renderer::TexturePtr m_effectTexture [Constants_MaxEffectCount];
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_COLORGRADINGEFFECTMGR_H)
