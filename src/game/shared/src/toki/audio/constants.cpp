#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#include <toki/audio/constants.h>


namespace toki {
namespace audio {

const char* getCategoryName(Category p_cat)
{
	switch (p_cat)
	{
	case Category_Ambient:   return "Ambient";
	case Category_Effects:   return "Effects";
	case Category_VoiceOver: return "VoiceOver";
	case Category_Music:     return "Music";
		
	default:
		TT_PANIC("Invalid audio category: %d", p_cat);
		return "";
	}
}


Category getCategoryFromName(const std::string& p_name)
{
	for (s32 i = 0; i < Category_Count; ++i)
	{
		Category cat = static_cast<Category>(i);
		if (getCategoryName(cat) == p_name)
		{
			return cat;
		}
	}
	
	return Category_Invalid;
}


const char* getReverbEffectName(ReverbEffect p_effect)
{
	switch (p_effect)
	{
	case ReverbEffect_None:   return "none";
	case ReverbEffect_Forest: return "forest";
	case ReverbEffect_Cave:   return "cave";
		
	default:
		TT_PANIC("Invalid reverb effect: %d", p_effect);
		return "";
	}
}


ReverbEffect getReverbEffectFromName(const std::string& p_name)
{
	for (s32 i = 0; i < ReverbEffect_Count; ++i)
	{
		ReverbEffect effect = static_cast<ReverbEffect>(i);
		if (getReverbEffectName(effect) == p_name)
		{
			return effect;
		}
	}
	
	return ReverbEffect_Invalid;
}

// Namespace end
}
}
