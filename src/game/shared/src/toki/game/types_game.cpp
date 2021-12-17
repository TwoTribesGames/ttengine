#include <tt/platform/tt_error.h>

#include <toki/game/types.h>


namespace toki {
namespace game {

const char* getParticleLayerName(ParticleLayer p_layer)
{
	switch (p_layer)
	{
	case ParticleLayer_BehindShoeboxZero:               return "behind_shoebox_zero";
	case ParticleLayer_BehindEntities:                  return "behind_entities";
	case ParticleLayer_BehindEntitiesSubOnly:           return "behind_entities_sub_only";
	case ParticleLayer_InFrontOfEntities:               return "in_front_of_entities";
	case ParticleLayer_InFrontOfEntitiesSubOnly:        return "in_front_of_entities_sub_only";
	case ParticleLayer_InFrontOfWater:                  return "in_front_of_water";
	case ParticleLayer_InFrontOfStillWater:             return "in_front_of_still_water";
	case ParticleLayer_InFrontOfSplit:                  return "in_front_of_split";
	case ParticleLayer_InFrontOfShoeboxZeroOne:         return "in_front_of_shoebox_zero_one";
	case ParticleLayer_InFrontOfShoeboxZeroTwo:         return "in_front_of_shoebox_zero_two";
	case ParticleLayer_BehindHud:                       return "behind_hud";
	case ParticleLayer_BehindHudTvOnly:                 return "behind_hud_tv_only";
	case ParticleLayer_BehindHudDrcOnly:                return "behind_hud_drc_only";
	case ParticleLayer_Hud:                             return "hud";
	case ParticleLayer_HudTvOnly:                       return "hud_tv_only";
	case ParticleLayer_HudDrcOnly:                      return "hud_drc_only";
	case ParticleLayer_Minimap:                         return "minimap";
	case ParticleLayer_InFrontOfHud:                    return "in_front_of_hud";
		
	default:
		TT_PANIC("Invalid particle layer value: %d", p_layer);
		return "";
	}
}


ParticleLayer getParticleLayerFromName(const std::string& p_layerName)
{
	for (s32 i = 0; i < ParticleLayer_Invalid; ++i)
	{
		const ParticleLayer layer = static_cast<ParticleLayer>(i);
		if (getParticleLayerName(layer) == p_layerName)
		{
			return layer;
		}
	}
	
	return ParticleLayer_Invalid;
}


s32 getParticleLayerRenderGroup(ParticleLayer p_layer)
{
	switch (p_layer)
	{
	case ParticleLayer_BehindShoeboxZero:               return ParticleRenderGroup_BehindShoeboxZero;
	case ParticleLayer_BehindEntities:                  return ParticleRenderGroup_BehindEntities;
	case ParticleLayer_BehindEntitiesSubOnly:           return ParticleRenderGroup_BehindEntitiesSubOnly;
	case ParticleLayer_InFrontOfEntities:               return ParticleRenderGroup_InFrontOfEntities;
	case ParticleLayer_InFrontOfEntitiesSubOnly:        return ParticleRenderGroup_InFrontOfEntitiesSubOnly;
	case ParticleLayer_InFrontOfWater:                  return ParticleRenderGroup_InFrontOfWater;
	case ParticleLayer_InFrontOfStillWater:             return ParticleRenderGroup_InFrontOfStillWater;
	case ParticleLayer_InFrontOfSplit:                  return ParticleRenderGroup_InFrontOfSplit;
	case ParticleLayer_InFrontOfShoeboxZeroOne:         return ParticleRenderGroup_InFrontOfShoeboxZeroOne;
	case ParticleLayer_InFrontOfShoeboxZeroTwo:         return ParticleRenderGroup_InFrontOfShoeboxZeroTwo;
	case ParticleLayer_BehindHud:                       return ParticleRenderGroup_BehindHud;
	case ParticleLayer_BehindHudTvOnly:                 return ParticleRenderGroup_BehindHudTvOnly;
	case ParticleLayer_BehindHudDrcOnly:                return ParticleRenderGroup_BehindHudDrcOnly;
	case ParticleLayer_Hud:                             return ParticleRenderGroup_Hud;
	case ParticleLayer_HudTvOnly:                       return ParticleRenderGroup_HudTvOnly;
	case ParticleLayer_HudDrcOnly:                      return ParticleRenderGroup_HudDrcOnly;
	case ParticleLayer_Minimap:                         return ParticleRenderGroup_Minimap;
	case ParticleLayer_InFrontOfHud:                    return ParticleRenderGroup_InFrontOfHud;
	case ParticleLayer_UseLayerFromParticleEffect:      return -1;  // Special case: passing a render group of -1 means "use value from trigger file"
		
	default:
		TT_PANIC("Invalid particle layer value: %d", p_layer);
		return -1;
	}
}


const char* getGameLayerName(GameLayer p_layer)
{
	switch (p_layer)
	{
	case GameLayer_ShoeboxBackground: return "shoebox_background";
	case GameLayer_Attributes:        return "attributes";
	case GameLayer_ShoeboxZero:       return "shoebox_zero";
	case GameLayer_ShoeboxForeground: return "shoebox_foreground";
	case GameLayer_Notes:             return "notes";
	case GameLayer_EditorWarnings:    return "editor_warnings";
		
	default:
		TT_PANIC("Invalid game layer value: %d", p_layer);
		return "";
	}
}


GameLayer getGameLayerFromName(const std::string& p_name)
{
	for (s32 i = 0; i < GameLayer_Count; ++i)
	{
		const GameLayer layer = static_cast<GameLayer>(i);
		if (getGameLayerName(layer) == p_name)
		{
			return layer;
		}
	}
	
	return GameLayer_Invalid;
}

// Namespace end
}
}
