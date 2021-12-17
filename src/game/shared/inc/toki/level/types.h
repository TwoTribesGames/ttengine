#if !defined(INC_TOKI_LEVEL_TYPES_H)
#define INC_TOKI_LEVEL_TYPES_H


#include <map>
#include <string>

#include <tt/code/BitMask.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_error.h>


namespace toki  /*! */ {
namespace level /*! */ {

/*! \brief The type of collision that a tile in the tile layer can have (only one type is supported per tile). */
enum CollisionType
{
	CollisionType_Air,             //!< Air (no collision).
	CollisionType_Solid,           //!< Solid: cannot be passed through.
	CollisionType_Water_Source,    //!< Water source (where flows start).
	CollisionType_Water_Still,     //!< Non-flowing water.
	CollisionType_Lava_Source,     //!< Lava source (where flows start).
	CollisionType_Lava_Still,      //!< Non-flowing lava.
	CollisionType_Solid_FluidKill, //!< Solid that stops fluids.
	
	CollisionType_Crystal_Solid,   //!< Solid crystal: lets light pass through, but is otherwise Solid.
	
	CollisionType_FluidKill, //!< Tile that stops fluids, but is not solid.
	
	CollisionType_AirPrefer,                //!< Air (no collision), but path finding entities prefer this tile over normal air.
	CollisionType_AirAvoid,                 //!< Air (no collision), but path finding entities try to avoid this tile.
	CollisionType_Solid_Allow_Pathfinding,  //!< Solid; cannot be passed through. But it is invisible to pathfinding.
	
	CollisionType_Count,
	CollisionType_Invalid
};

typedef tt::code::BitMask<CollisionType, CollisionType_Count> CollisionTypes;
typedef std::map<tt::math::Point2, level::CollisionType, tt::math::Point2Less> CollisionTiles;

// Bit masks that define which CollisionType values represent a particular category (solid, etc)
// These variables are defined in types_level.cpp (and need to be updated there when adding new collision types)
extern const CollisionTypes g_collisionTypesAir;
extern const CollisionTypes g_collisionTypesSolid;
extern const CollisionTypes g_collisionTypesSolidForPathfinding;
extern const CollisionTypes g_collisionTypesLightBlocking;
extern const CollisionTypes g_collisionTypesSoundBlocking;
extern const CollisionTypes g_collisionTypesFluids;
extern const CollisionTypes g_collisionTypesFluidsStill;
extern const CollisionTypes g_collisionTypesFluidsSource;
extern const CollisionTypes g_collisionTypesCrystal;


/*! \brief The type of theme that a tile in the tile layer can have (only one type is supported per tile). */
enum ThemeType
{
	ThemeType_UseLevelDefault, //!< UseLevelDefault
	ThemeType_DoNotTheme,      //!< DoNotTheme
	ThemeType_Sand,            //!< Sand
	ThemeType_Rocks,           //!< Rocks
	ThemeType_Beach,           //!< Beach
	ThemeType_DarkRocks,       //!< DarkRocks
	
	ThemeType_Count,
	ThemeType_Invalid
};
typedef std::map<tt::math::Point2, level::ThemeType, tt::math::Point2Less> ThemeTiles;


/*! \brief Controls who can place a specific entity in the level editor (whether it shows up in the entity library). */
enum Placeable
{
	Placeable_Hidden,           //!< Entity will not show up in the entity library.
	Placeable_Developer,        //!< Entity only shows up in the entity library for internal developer builds.
	Placeable_Everyone,         //!< Entity available in the entity library for everyone (including public builds).
	Placeable_UserLevelEditor,  //!< Entity only shows up when the application is started as a user level editor.
	
	Placeable_Count,
	Placeable_Invalid
};
typedef tt::code::BitMask<Placeable, Placeable_Count> PlaceableMask;

inline bool isValidPlaceable(Placeable p_placeable)
{
	return p_placeable >= 0 && p_placeable < Placeable_Count;
}


enum CollisionTypeConstants
{
	// For internal use in helper functions
	CollisionTypeBits  = 5,  // Number of bits reserved for CollisionType
	CollisionTypeShift = 0,  // Where in the byte to place the CollisionType bits
	CollisionTypeMask  = ((1 << CollisionTypeBits) - 1) << CollisionTypeShift, // Mask for the CollisionType (00011111)
	
	ThemeTypeBits  = 3,  // Number of bits reserved for ThemeType
	ThemeTypeShift = 5,  // Where in the byte to place the ThemeType bits
	ThemeTypeMask  = ((1 << ThemeTypeBits) - 1) << ThemeTypeShift // Mask for the ThemeType (11100000)
};

TT_STATIC_ASSERT(((CollisionType_Count - 1) << CollisionTypeShift) <= CollisionTypeMask);
TT_STATIC_ASSERT(((ThemeType_Count     - 1) << ThemeTypeShift)     <= ThemeTypeMask);


/*! \brief Indicates whether the specified type is a valid CollisionType value. */
inline bool isValidCollisionType(CollisionType p_type)
{
	return p_type >= 0 && p_type < CollisionType_Count;
}

/*! \brief Retrieves a human-readable name for the CollisionType value. */
inline const char* getCollisionTypeName(CollisionType p_type)
{
	switch (p_type)
	{
	case CollisionType_Air:                      return "air";
	case CollisionType_Solid:                    return "solid";
	case CollisionType_Water_Source:             return "water_source";
	case CollisionType_Water_Still:              return "water_still";
	case CollisionType_Lava_Source:              return "lava_source";
	case CollisionType_Lava_Still:               return "lava_still";
	case CollisionType_Solid_FluidKill:          return "solid_fluidkill";
	case CollisionType_Crystal_Solid:            return "crystal_solid";
	case CollisionType_FluidKill:                return "fluidkill";
	case CollisionType_AirPrefer:                return "air_prefer";
	case CollisionType_AirAvoid:                 return "air_avoid";
	case CollisionType_Solid_Allow_Pathfinding:  return "solid_allow_pathfinding";
		
	default:
		TT_PANIC("Unknown/invalid CollisionType: %d", p_type);
		return "";
	}
}


inline std::string getCollisionTypeNameAsString(CollisionType p_type)
{
	return getCollisionTypeName(p_type);
}


/*! \brief Returns a CollisionType value for a given human-readable collision type name. */
inline CollisionType getCollisionTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < CollisionType_Count; ++i)
	{
		if (getCollisionTypeName(static_cast<CollisionType>(i)) == p_name)
		{
			return static_cast<CollisionType>(i);
		}
	}
	
	return CollisionType_Invalid;
}


inline char getCollisionTypeAsChar(CollisionType p_type)
{
	switch (p_type)
	{
	case CollisionType_Air:                      return ' ';
	case CollisionType_Solid:                    return 's';
	case CollisionType_Water_Source:             return 'w';
	case CollisionType_Water_Still:              return 'W';
	case CollisionType_Lava_Source:              return 'l';
	case CollisionType_Lava_Still:               return 'L';
	case CollisionType_Solid_FluidKill:          return 'z';
	case CollisionType_Crystal_Solid:            return 'c';
	case CollisionType_FluidKill:                return 'Z';
	case CollisionType_AirPrefer:                return '>';
	case CollisionType_AirAvoid:                 return '<';
	case CollisionType_Solid_Allow_Pathfinding:  return 'p';
	
	default:
		TT_PANIC("Unknown/invalid CollisionType: %d", p_type);
		return ' ';
	}
}


inline CollisionType getCollisionTypeFromChar(char p_char)
{
	for (s32 i = 0; i < CollisionType_Count; ++i)
	{
		const CollisionType type = static_cast<CollisionType>(i);
		if (getCollisionTypeAsChar(type) == p_char)
		{
			return type;
		}
	}
	
	return CollisionType_Invalid;
}


inline CollisionType getCollisionType(u8 p_attrib)
{
	return static_cast<CollisionType>((p_attrib & CollisionTypeMask) >> CollisionTypeShift);
}

inline void setCollisionType(u8& p_attrib, CollisionType p_type)
{
	p_attrib = static_cast<u8>((p_attrib & ~CollisionTypeMask) | (p_type << CollisionTypeShift));
}


inline bool isAir(CollisionType p_type)
{
	return g_collisionTypesAir.checkFlag(p_type);
}


/*! \brief Returns true if this CollisionType is collision (isSolid). */
inline bool isSolid(CollisionType p_type)
{
	TT_ASSERTMSG(isValidCollisionType(p_type),
	             "Invalid CollisionType (%d) passed to isSolid from script.",
	             p_type);
	return g_collisionTypesSolid.checkFlag(p_type);
}


inline bool isSolidForPathfinding(CollisionType p_type)
{
	return g_collisionTypesSolidForPathfinding.checkFlag(p_type);
}


inline bool isLightBlocking(CollisionType p_type)
{
	return g_collisionTypesLightBlocking.checkFlag(p_type);
}


inline bool isSoundBlocking(CollisionType p_type)
{
	return g_collisionTypesSoundBlocking.checkFlag(p_type);
}


inline bool isFluidType(CollisionType p_type)
{
	return g_collisionTypesFluids.checkFlag(p_type);
}


inline bool isFluidStill(CollisionType p_type)
{
	return g_collisionTypesFluidsStill.checkFlag(p_type);
}

inline bool isFluidSource(CollisionType p_type)
{
	return g_collisionTypesFluidsSource.checkFlag(p_type);
}


inline bool isCrystal(CollisionType p_type)
{
	return level::isValidCollisionType(p_type) &&
	       g_collisionTypesCrystal.checkFlag(p_type);
}


inline bool isAttributeSolid(u8 p_attrib)
{
	return isSolid(getCollisionType(p_attrib));
}


//--------------------------------------------------------------------------------------------------
// ThemeType

inline bool isValidThemeType(ThemeType p_type)
{
	return p_type >= 0 && p_type < ThemeType_Count;
}


/*! \brief Retrieves a human-readable name for the ThemeType value. */
inline const char* getThemeTypeName(ThemeType p_type)
{
	switch (p_type)
	{
	case ThemeType_UseLevelDefault: return "default";
	case ThemeType_DoNotTheme:      return "none";
	case ThemeType_Sand:            return "sand";
	case ThemeType_Rocks:           return "rocks";
	case ThemeType_Beach:           return "beach";
	case ThemeType_DarkRocks:       return "dark_rocks";
		
	default:
		TT_PANIC("Unknown/invalid ThemeType: %d", p_type);
		return "";
	}
}


inline std::string getThemeTypeNameAsString(ThemeType p_type)
{
	return getThemeTypeName(p_type);
}


inline ThemeType getThemeTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < ThemeType_Count; ++i)
	{
		if (getThemeTypeName(static_cast<ThemeType>(i)) == p_name)
		{
			return static_cast<ThemeType>(i);
		}
	}
	
	// Dark_sand was renamed to beach.
	if (p_name == "dark_sand")
	{
		TT_WARN("Found 'dark_sand' changing to 'beach'.");
		return ThemeType_Beach;
	}
	
	return ThemeType_Invalid;
}


inline char getThemeTypeAsChar(ThemeType p_type)
{
	switch (p_type)
	{
	case ThemeType_UseLevelDefault: return '.';
	case ThemeType_DoNotTheme:      return 'x';
	case ThemeType_Sand:            return 's';
	case ThemeType_Rocks:           return 'r';
	case ThemeType_Beach:           return 'b';
	case ThemeType_DarkRocks:       return 'R';
		
	default:
		TT_PANIC("Unknown/invalid ThemeType: %d", p_type);
		return ' ';
	}
}


inline ThemeType getThemeTypeFromChar(char p_char)
{
	for (s32 i = 0; i < ThemeType_Count; ++i)
	{
		const ThemeType type = static_cast<ThemeType>(i);
		if (getThemeTypeAsChar(type) == p_char)
		{
			return type;
		}
	}
	
	return ThemeType_Invalid;
}


inline ThemeType getThemeType(u8 p_attrib)
{
	return static_cast<ThemeType>((p_attrib & ThemeTypeMask) >> ThemeTypeShift);
}


inline void setThemeType(u8& p_attrib, ThemeType p_type)
{
	p_attrib = static_cast<u8>((p_attrib & ~ThemeTypeMask) | (p_type << ThemeTypeShift));
}

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_TYPES_H)
