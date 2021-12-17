#if !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_TYPES_H)
#define INC_TOKI_GAME_ENTITY_GRAPHICS_TYPES_H


#include <tt/platform/tt_error.h>


namespace toki     /*! */ {
namespace game     /*! */ {
namespace entity   /*! */ {
namespace graphics /*! */ {

/*! \brief The type of a power beam (controls which settings and graphics it uses). */
enum PowerBeamType
{
	PowerBeamType_Hack,         //!< The hack beam
	PowerBeamType_PreviewLaser, //!< A preview laser beam.
	PowerBeamType_Laser,        //!< A laser beam.
	PowerBeamType_Electricity,  //!< An electricity beam.
	PowerBeamType_Sight,        //!< A sight indicator beam.
	
	PowerBeamType_Count,
	PowerBeamType_Invalid
};


inline bool isValidPowerBeamType(PowerBeamType p_type)
{
	return p_type >= 0 && p_type < PowerBeamType_Count;
}

inline const char* getPowerBeamTypeName(PowerBeamType p_type)
{
	switch (p_type)
	{
	case PowerBeamType_Hack:         return "hack_beam";
	case PowerBeamType_PreviewLaser: return "previewlaser_beam";
	case PowerBeamType_Laser:        return "laser_beam";
	case PowerBeamType_Electricity:  return "electricity_beam";
	case PowerBeamType_Sight:        return "sight_beam";
		
	default:
		TT_PANIC("Invalid power beam type: %d. Does not have a corresponding name.", p_type);
		return "";
	}
}


/*! \brief Horizontal Alignment. */
enum HorizontalAlignment
{
	HorizontalAlignment_Left,   //!< 
	HorizontalAlignment_Center, //!< 
	HorizontalAlignment_Right   //!< 
};


/*! \brief Vertical Alignment. */
enum VerticalAlignment
{
	VerticalAlignment_Top,    //!< 
	VerticalAlignment_Center, //!< 
	VerticalAlignment_Bottom  //!< 
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_TYPES_H)
