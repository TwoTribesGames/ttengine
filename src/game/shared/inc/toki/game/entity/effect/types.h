#if !defined(INC_TOKI_GAME_ENTITY_EFFECT_TYPES_H)
#define INC_TOKI_GAME_ENTITY_EFFECT_TYPES_H


#include <tt/platform/tt_error.h>


namespace toki     /*! */ {
namespace game     /*! */ {
namespace entity   /*! */ {
namespace effect /*! */ {


/*! \brief The type of a power beam (controls which settings and graphics it uses). */
enum EffectRectTarget
{
	EffectRectTarget_CameraPos,            //!< Effect strength is based on camera position.
	EffectRectTarget_ControllingEntityPos, //!< Effect strength is based on controlling entity position.
	
	EffectRectTarget_Count,
	EffectRectTarget_Invalid
};


inline bool isValidEffectRectTarget(EffectRectTarget p_type)
{
	return p_type >= 0 && p_type < EffectRectTarget_Count;
}


/*! \brief The type of a effect. */
enum EffectType
{
	EffectType_Fog,          //!< Fog Effect
	EffectType_VOF,          //!< VOF Effect
	EffectType_CameraEffect, //!< Camera Effect
	EffectType_LightAmbient, //!< Light ambient Effect
	EffectType_Blur,         //!< Blur Effect
	EffectType_Script,       //!< Script Effect.
	
	EffectType_Count,
	EffectType_Invalid
};


inline bool isValidEffectType(EffectType p_type)
{
	return p_type >= 0 && p_type < EffectType_Count;
}

inline const char* getEffectTypeName(EffectType p_type)
{
	switch (p_type)
	{
	case EffectType_Fog:          return "fog";
	case EffectType_VOF:          return "vog";
	case EffectType_CameraEffect: return "camera_effect";
	case EffectType_LightAmbient: return "light_ambient";
	case EffectType_Blur:         return "blur";
	case EffectType_Script:       return "script";
		
	default:
		TT_PANIC("Invalid effect type: %d. Does not have a corresponding name.", p_type);
		return "";
	}
}

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_EFFECT_TYPES_H)
