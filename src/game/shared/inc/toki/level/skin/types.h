#if !defined(INC_TOKI_LEVEL_SKIN_TYPES_H)
#define INC_TOKI_LEVEL_SKIN_TYPES_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

//#include <toki/level/skin/BlobData.h>
#include <toki/level/skin/fwd.h>
#include <toki/level/types.h>
#include <toki/level/fwd.h>


namespace toki {
namespace level {
namespace skin {


enum Constants
{
	Constants_LevelExtensionSize = 1000 // How many tiles we extent edges and planes at the level border
};


enum SkinConfigType
{
	SkinConfigType_Solid,
	
	SkinConfigType_Count,
	SkinConfigType_Invalid
};

inline bool isValidSkinConfigType(SkinConfigType p_type)
{
	return p_type >= 0 && p_type < SkinConfigType_Count;
}

const char* const getSkinConfigTypeName(SkinConfigType p_skinConfigType);
SkinConfigType    getSkinConfigTypeFromName(const std::string& p_name);


enum MaterialType
{
	MaterialType_None,
	MaterialType_Solid,
	
	MaterialType_Count,
	MaterialType_Invalid
};

inline bool isValidMaterialType(MaterialType p_type)
{
	return p_type >= 0 && p_type < MaterialType_Count;
}

inline bool isSolid(MaterialType p_type)
{
	return p_type == MaterialType_Solid;
}

const char* const getMaterialTypeName(MaterialType p_materialType);
MaterialType      getMaterialTypeFromName(const std::string& p_name);


enum MaterialTheme
{
	MaterialTheme_None, // Only 'legal' for MaterialType_None.
	MaterialTheme_Sand,
	MaterialTheme_Rocks,
	MaterialTheme_Crystal,
	MaterialTheme_Beach,
	MaterialTheme_DarkRocks,
	
	MaterialTheme_Count,
	MaterialTheme_Invalid
};

inline bool isValidMaterialTheme(MaterialTheme p_type)
{
	return p_type >= 0 && p_type < MaterialTheme_Count;
}

const char* const getMaterialThemeName(MaterialTheme p_materialTheme);
MaterialTheme     getMaterialThemeFromName(const std::string& p_name);


enum EdgeType
{
	EdgeType_None      = 0,
	EdgeType_SolidBit  = 1 << 0,
	
	EdgeType_Count
};

inline bool isValidEdgeType(EdgeType p_type)
{
	return p_type >= 0 && p_type < EdgeType_Count;
}


EdgeType getEdgeForTiles(const TileMaterial& p_a, const TileMaterial& p_b);


enum TileSide
{
	TileSide_Top,
	TileSide_Bottom,
	TileSide_Left,
	TileSide_Right,
	
	TileSide_Count
};

enum TileSideBit
{
	TileSideBit_None    = 0x0,
	TileSideBit_Top     = 0x1 << TileSide_Top,
	TileSideBit_Bottom  = 0x1 << TileSide_Bottom,
	TileSideBit_Left    = 0x1 << TileSide_Left,
	TileSideBit_Right   = 0x1 << TileSide_Right,
	
	TileSideBit_Outside = 0x1 << TileSide_Count, // Used for checks which are done on tiles with no collision.
	
	TileSideBit_HorizontalMask = TileSideBit_Top  | TileSideBit_Bottom,
	TileSideBit_VerticalMask   = TileSideBit_Left | TileSideBit_Right,
	
	// The following to masks take into account the direction in which we're scanning the tiles.
	// We go from left to right and from bottom to top.
	TileSideBit_CornerStartMask = TileSideBit_Left | TileSideBit_Bottom,
	TileSideBit_CornerStopMask  = TileSideBit_Top  | TileSideBit_Right,
	
	TileSideBit_AllMask = TileSideBit_Top  | TileSideBit_Bottom |
	                      TileSideBit_Left | TileSideBit_Right  |
	                      TileSideBit_Outside
};


/* \brief This describes the shape of the outside edge of a group of tiles.
   \note  The Shape is based on the four sides of a square/tile. */
enum Shape
{
	Shape_None        = TileSideBit_None,
	
	// Edge (The (single) side of a tile)
	Shape_EdgeTop     = TileSideBit_Top,
	Shape_EdgeBottom  = TileSideBit_Bottom,
	Shape_EdgeLeft    = TileSideBit_Left,
	Shape_EdgeRight   = TileSideBit_Right,
	
	// Line (The parallel sides of a tile.)
	Shape_LineHorizontal = TileSideBit_Top  | TileSideBit_Bottom,
	Shape_LineVertical   = TileSideBit_Left | TileSideBit_Right,
	
	// Tip (Three sides for a tip, the 'open' side is opposite of name.)
	//      So TipTop has an open tile at it's bottom.
	Shape_TipTop    = TileSideBit_Top |                      TileSideBit_Left | TileSideBit_Right,
	Shape_TipBottom =                   TileSideBit_Bottom | TileSideBit_Left | TileSideBit_Right,
	Shape_TipLeft   = TileSideBit_Top | TileSideBit_Bottom | TileSideBit_Left                    ,
	Shape_TipRight  = TileSideBit_Top | TileSideBit_Bottom |                    TileSideBit_Right,
	
	// Tile (All four sides.)
	Shape_Tile      = TileSideBit_Top | TileSideBit_Bottom | TileSideBit_Left | TileSideBit_Right,
	
	// Corner Outside (Only two sides are found and they touch creating a corner.)
	//  _
	// |X  (X is the solid part of the larger shape. | and _ are the edges.)
	// The shape above is top left.
	// (Outside corner is described as it relates to it's location in the larger group of tiles.)
	Shape_CornerOutsideBottomLeft  = TileSideBit_Bottom | TileSideBit_Left,
	Shape_CornerOutsideBottomRight = TileSideBit_Bottom | TileSideBit_Right,
	Shape_CornerOutsideTopLeft     = TileSideBit_Top    | TileSideBit_Left,
	Shape_CornerOutsideTopRight    = TileSideBit_Top    | TileSideBit_Right,
	
	// Corner Inside (This is the only shape described from a tile which itself has no collision.
	//                Just like the outside corner only two sides are found and they touch.)
	// X|_  <-- This open tile is the bottom left inside corner.
	// X X  (The Xs are the collision inside the larger shape.)
	Shape_CornerInsideBottomLeft  = TileSideBit_Outside | TileSideBit_Bottom | TileSideBit_Left,
	Shape_CornerInsideBottomRight = TileSideBit_Outside | TileSideBit_Bottom | TileSideBit_Right,
	Shape_CornerInsideTopLeft     = TileSideBit_Outside | TileSideBit_Top    | TileSideBit_Left,
	Shape_CornerInsideTopRight    = TileSideBit_Outside | TileSideBit_Top    | TileSideBit_Right,
	
	Shape_Count = TileSideBit_AllMask + 1,
	Shape_Invalid
};

const char* const getShapeName(Shape p_shape);
Shape             getShapeFromName(const std::string& p_name);

inline bool isValidShape(Shape p_shape)
{
	return p_shape >= 0 && p_shape < Shape_Count;
}

inline bool isEdgeShape(Shape p_shape)
{
	switch (p_shape)
	{
	case Shape_EdgeTop:
	case Shape_EdgeBottom:
	case Shape_EdgeLeft:
	case Shape_EdgeRight:
	case Shape_LineHorizontal:
	case Shape_LineVertical:
		return true;
		
	default:
		return false;
	}
}


inline bool isStartingWithOutside(Shape p_shape, TileSideBit p_toCheckMask)
{
	// Check if we have a corner start bit active for the other sides.
	const u8 cornerBits = (p_shape  & TileSideBit_CornerStartMask);
	const u8 otherBits  = (cornerBits & p_toCheckMask);
	return (cornerBits != otherBits) && cornerBits != 0;
}


inline bool isEndingWithOutside(Shape p_shape, TileSideBit p_toCheckMask)
{
	// Check if we have a corner start bit active for the other sides.
	const u8 cornerBits = (p_shape  & TileSideBit_CornerStopMask);
	const u8 otherBits  = (cornerBits & p_toCheckMask);
	return (cornerBits != otherBits) && cornerBits != 0;
}


/*

Legend:
A - Air Tile
C - (Solid) Collision Tile
Y - Crystal

* - Any Tile (Wildcard)

================ Corners ================

== Outside - Corners ==

AA*
ACC Top    - Left  Outside Corner
*C*

*AA
CCA Top    - Right Outside Corner
*C*

*C* Bottom - Right Outside Corner
CCA
*AA

*C* Bottom - Left  Outside Corner
ACC
AA*

== Inside - Corners ==

***
*CC Top    - Left   Inside Corner
*CA

***
CC* Top    - Right  Inside Corner
AC*

AC* Bottom - Right  Inside Corner
CC*
***

*CA Bottom - Left   Inside Corner
*CC
***

================ Edges ================

A  AA  A...A
C  CC  C...C   Top Edge
C  CC  C...C

C  CC  C...C
C  CC  C...C   Bottom Edge
A  AA  A...A

ACC ACC ACC    Left Edge
    ACC ...
        ...
        ACC

CCA CCA CCA    Right Edge
    CCA ...
        ...
        CCA

================ Center Plane ================


Fill inside of edges with quads.


================ Line ================

A AA A...A
C CC C...C  Horizontal Line
A AA A...A

ACA ACA ACA Vertical Line
    ACA ...
        ...
        ACA

================ Tip ================

*A*
ACA  Top Tip

*A
AC   Left Tip
*A

ACA  Bottom Tip
*A*

A*
CA   Right Tip
A*
 
================= Tile =================

*A*
ACA   Single Tile	
*A*

================= diagonal connection =================

This is placed between the tiles.

CA
AC backslash (\) diagonal connection

AC
CA slash (/)     diagonal connection

*/

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_TYPES_H)
