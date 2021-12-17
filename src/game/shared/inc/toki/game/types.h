#if !defined(INC_TOKI_GAME_TYPES_H)
#define INC_TOKI_GAME_TYPES_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/platform/tt_types.h>


namespace toki /*! */ {
namespace game /*! */ {

/*! \brief Identifies a render layer for particle effects. */
enum ParticleLayer
{
	/*! \brief Name: "behind_shoebox_zero"<br/>
	           RenderGroup: 100<br/>
	           Rendered behind the shoebox "z = 0" layer. */
	ParticleLayer_BehindShoeboxZero,
	
	/*! \brief Name: "behind_entities"<br/>
	           RenderGroup: 200<br/>
	           Rendered behind the entities, in front of the back layer of fluids. */
	ParticleLayer_BehindEntities,
	
	/*! \brief Name: "behind_entities_sub_only"<br/>
	           RenderGroup: 240<br/>
	           Rendered behind the entities, in front of the back layer of fluids. (Sub cam only) */
	ParticleLayer_BehindEntitiesSubOnly,
	
	/*! \brief Name: "in_front_of_entities"<br/>
	           RenderGroup: 300<br/>
	           Rendered in front of the entities, behind the power beams. */
	ParticleLayer_InFrontOfEntities,
	
	/*! \brief Name: "in_front_of_entities_sub_only"<br/>
	           RenderGroup: 340<br/>
	           Rendered in front of the entities, behind the power beams. (Sub cam only) */
	ParticleLayer_InFrontOfEntitiesSubOnly,
	
	/*! \brief Name: "in_front_of_water"<br/>
	           RenderGroup: 325<br/>
	           Rendered in front of the front water layer. (Except still water front) */
	ParticleLayer_InFrontOfWater,
	
	/*! \brief Name: "in_front_of_still_water"<br/>
	           RenderGroup: 327<br/>
	           Rendered in front of the front still water layer. */
	ParticleLayer_InFrontOfStillWater,
	
	/*! \brief Name: "in_front_of_split"<br/>
	           RenderGroup: 350<br/>
	           Rendered in front of the split.*/
	ParticleLayer_InFrontOfSplit,
	
	/*! \brief Name: "in_front_of_shoebox_zero_one"<br/>
	           RenderGroup: 400<br/>
	           Rendered in front of "z = 0" front layer, before ParticleLayer_InFrontOfShoeboxZeroTwo.*/
	ParticleLayer_InFrontOfShoeboxZeroOne,
	
	/*! \brief Name: "in_front_of_shoebox_zero_two"<br/>
	           RenderGroup: 450<br/>
	           Rendered in front of the ParticleLayer_InFrontOfShoeboxZeroOne, before the shoebox foreground layer. */
	 ParticleLayer_InFrontOfShoeboxZeroTwo,
	
	/*! \brief Name: "behind_hud"<br/>
	           RenderGroup: 500<br/>
	           Rendered in front of all world layers, before rendering the HUD. */
	ParticleLayer_BehindHud,
	
	/*! \brief Name: "behind_hud_tv_only"<br/>
	           RenderGroup: 510<br/>
	           Rendered in front of all world layers, before rendering the HUD. */
	ParticleLayer_BehindHudTvOnly,
	
	/*! \brief Name: "behind_hud_drc_only"<br/>
	           RenderGroup: 520<br/>
	           Rendered in front of all world layers, before rendering the HUD. */
	ParticleLayer_BehindHudDrcOnly,
	
	/*! \brief Name: "hud"<br/>
	           RenderGroup: 600<br/>
	           Rendered in screen space. */
	ParticleLayer_Hud,
	
	/*! \brief Name: "hud_tv_only"<br/>
	           RenderGroup: 610<br/>
	           Rendered in screen space. */
	ParticleLayer_HudTvOnly,
	
	/*! \brief Name: "hud_drc_only"<br/>
	           RenderGroup: 620<br/>
	           Rendered in screen space. */
	ParticleLayer_HudDrcOnly,
	
	/*! \brief Name: "minimap"<br/>
	           RenderGroup 700<br/>
	           Rendered in minimap. (screenspace) */
	ParticleLayer_Minimap,
	
	/*! \brief Name: "in_front_of_hud"<br/>
	           RenderGroup 800<br/>
	           Rendered in front of hud. (Only for fades!) (screenspace). */
	ParticleLayer_InFrontOfHud,
	
	ParticleLayer_Count,
	ParticleLayer_Invalid,
	
	/*! \brief Special layer value: do not overwrite the render group, but use the one specified in the particle effect file.
	           This value is for use in scripts. Presentation files should just not specify a layer to get this behavior.
	           This value therefore does not have a corresponding layer name or render group. */
	ParticleLayer_UseLayerFromParticleEffect
};


/*! \brief Indicates whether the parameter is a valid ParticleLayer value. */
inline bool isValidParticleLayer(ParticleLayer p_layer)
{ return p_layer >= 0 && p_layer < ParticleLayer_Count; }

/*! \brief Retrieves the name of a particle layer (which can be used in presentation files, for example). */
const char* getParticleLayerName(ParticleLayer p_layer);

inline std::string getParticleLayerNameAsString(ParticleLayer p_layer)
{ return getParticleLayerName(p_layer); }

/*! \brief Returns a ParticleLayer value for a given particle layer name. */
ParticleLayer getParticleLayerFromName(const std::string& p_layerName);

// Returns the render group number for the specified particle layer.
// This is intentionally kept separate from the enum values, so that existing particle effects
// continue to work and so that the numbers aren't easily confused with other numbers in script.
// This function is also not exposed to script, because script should use the ParticleLayer values.
s32 getParticleLayerRenderGroup(ParticleLayer p_layer);



// The render groups associated with the particle layers (these aren't exposed to script, meant for internal use only)
// IMPORTANT: Do not change existing values: these values are used in the particle effect files as RenderGroup numbers.
enum ParticleRenderGroup
{
	ParticleRenderGroup_BehindShoeboxZero              = 100,
	ParticleRenderGroup_BehindEntities                 = 200,
	ParticleRenderGroup_BehindEntitiesSubOnly          = 240,
	ParticleRenderGroup_InFrontOfEntities              = 300,
	ParticleRenderGroup_InFrontOfEntitiesSubOnly       = 340,
	ParticleRenderGroup_InFrontOfWater                 = 325,
	ParticleRenderGroup_InFrontOfStillWater            = 327,
	ParticleRenderGroup_InFrontOfSplit                 = 350,
	ParticleRenderGroup_InFrontOfShoeboxZeroOne        = 400,
	ParticleRenderGroup_InFrontOfShoeboxZeroTwo        = 450,
	ParticleRenderGroup_BehindHud                      = 500,
	ParticleRenderGroup_BehindHudTvOnly                = 510,
	ParticleRenderGroup_BehindHudDrcOnly               = 520,
	ParticleRenderGroup_Hud                            = 600,
	ParticleRenderGroup_HudTvOnly                      = 610,
	ParticleRenderGroup_HudDrcOnly                     = 620,
	ParticleRenderGroup_Minimap                        = 700,
	ParticleRenderGroup_InFrontOfHud                   = 800
};



// For internal use (not for script, hence not documented):

enum GameLayer
{
	GameLayer_ShoeboxBackground,
	GameLayer_Attributes,
	GameLayer_ShoeboxZero,
	GameLayer_ShoeboxForeground,
	GameLayer_Notes,
	GameLayer_EditorWarnings,
	
	GameLayer_Count,
	GameLayer_Invalid
};

typedef tt::code::BitMask<GameLayer, GameLayer_Count> GameLayers;


inline bool isValidGameLayer(GameLayer p_layer)
{ return p_layer >= 0 && p_layer < GameLayer_Count; }

const char* getGameLayerName(GameLayer p_layer);
GameLayer getGameLayerFromName(const std::string& p_name);

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_TYPES_H)
