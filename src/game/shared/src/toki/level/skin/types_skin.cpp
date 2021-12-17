#include <toki/level/skin/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/skin/TileMaterial.h>


namespace toki {
namespace level {
namespace skin {


const char* const getSkinConfigTypeName(SkinConfigType p_skinConfigType)
{
	switch (p_skinConfigType)
	{
	case SkinConfigType_Solid:  return "solid";
		
	default:
		TT_PANIC("Invalid skin config type: %d", p_skinConfigType);
		return "";
	}
}


SkinConfigType getSkinConfigTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < SkinConfigType_Count; ++i)
	{
		const SkinConfigType type = static_cast<SkinConfigType>(i);
		if (p_name == getSkinConfigTypeName(type))
		{
			return type;
		}
	}
	
	return SkinConfigType_Invalid;
}


const char* const getMaterialTypeName(MaterialType p_materialType)
{
	switch (p_materialType)
	{
	case MaterialType_None:        return "none";
	case MaterialType_Solid:       return "solid";
		
	default:
		TT_PANIC("Invalid material type: %d", p_materialType);
		return "";
	}
}


MaterialType getMaterialTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < MaterialType_Count; ++i)
	{
		const MaterialType type = static_cast<MaterialType>(i);
		if (p_name == getMaterialTypeName(type))
		{
			return type;
		}
	}
	
	return MaterialType_Invalid;
}


const char* const getMaterialThemeName(MaterialTheme p_materialTheme)
{
	switch (p_materialTheme)
	{
	case MaterialTheme_None:      return "none";
	case MaterialTheme_Sand:      return "sand";
	case MaterialTheme_Rocks:     return "rocks";
	case MaterialTheme_Beach:     return "beach";
	case MaterialTheme_DarkRocks: return "dark_rocks";
	case MaterialTheme_Crystal:   return "crystal";
		
	default:
		TT_PANIC("Invalid material theme: %d", p_materialTheme);
		return "";
	}
}


MaterialTheme getMaterialThemeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < MaterialTheme_Count; ++i)
	{
		const MaterialTheme theme = static_cast<MaterialTheme>(i);
		if (p_name == getMaterialThemeName(theme))
		{
			return theme;
		}
	}
	
	return MaterialTheme_Invalid;
}


EdgeType getEdgeForTiles(const TileMaterial& p_a, const TileMaterial& p_b)
{
	if (p_a == p_b)
	{
		// The same, no edge.
		return EdgeType_None;
	}
	
	u8 edgeBits = EdgeType_None;
	
	if (p_a.isSolid() != p_b.isSolid())
	{
		edgeBits |= EdgeType_SolidBit;
	}
	return static_cast<EdgeType>(edgeBits);
}


const char* const getShapeName(Shape p_shape)
{
	switch (p_shape)
	{
	case Shape_None:                     return "none";
	case Shape_EdgeTop:                  return "edge_top";
	case Shape_EdgeBottom:               return "edge_bottom";
	case Shape_EdgeLeft:                 return "edge_left";
	case Shape_EdgeRight:                return "edge_right";
	case Shape_LineHorizontal:           return "line_horizontal";
	case Shape_LineVertical:             return "line_vertical";
	case Shape_TipTop:                   return "tip_top";
	case Shape_TipBottom:                return "tip_bottom";
	case Shape_TipLeft:                  return "tip_left";
	case Shape_TipRight:                 return "tip_right";
	case Shape_Tile:                     return "tile";
	case Shape_CornerOutsideBottomLeft:  return "corner_outside_bottom_left";
	case Shape_CornerOutsideBottomRight: return "corner_outside_bottom_right";
	case Shape_CornerOutsideTopLeft:     return "corner_outside_top_left";
	case Shape_CornerOutsideTopRight:    return "corner_outside_top_right";
	case Shape_CornerInsideBottomLeft:   return "corner_inside_bottom_left";
	case Shape_CornerInsideBottomRight:  return "corner_inside_bottom_right";
	case Shape_CornerInsideTopLeft:      return "corner_inside_top_left";
	case Shape_CornerInsideTopRight:     return "corner_inside_top_right";
		
	default:
		TT_PANIC("Invalid shape: %d", p_shape);
		return "";
	}
}


Shape getShapeFromName(const std::string& p_name)
{
	// FIXME: Get rid of code duplication somehow
	if      (p_name == "none")                         return Shape_None;
	else if (p_name == "edge_top")                     return Shape_EdgeTop;
	else if (p_name == "edge_bottom")                  return Shape_EdgeBottom;
	else if (p_name == "edge_left")                    return Shape_EdgeLeft;
	else if (p_name == "edge_right")                   return Shape_EdgeRight;
	else if (p_name == "line_horizontal")              return Shape_LineHorizontal;
	else if (p_name == "line_vertical")                return Shape_LineVertical;
	else if (p_name == "tip_top")                      return Shape_TipTop;
	else if (p_name == "tip_bottom")                   return Shape_TipBottom;
	else if (p_name == "tip_left")                     return Shape_TipLeft;
	else if (p_name == "tip_right")                    return Shape_TipRight;
	else if (p_name == "tile")                         return Shape_Tile;
	else if (p_name == "corner_outside_bottom_left")   return Shape_CornerOutsideBottomLeft;
	else if (p_name == "corner_outside_bottom_right")  return Shape_CornerOutsideBottomRight;
	else if (p_name == "corner_outside_top_left")      return Shape_CornerOutsideTopLeft;
	else if (p_name == "corner_outside_top_right")     return Shape_CornerOutsideTopRight;
	else if (p_name == "corner_inside_bottom_left")    return Shape_CornerInsideBottomLeft;
	else if (p_name == "corner_inside_bottom_right")   return Shape_CornerInsideBottomRight;
	else if (p_name == "corner_inside_top_left")       return Shape_CornerInsideTopLeft;
	else if (p_name == "corner_inside_top_right")      return Shape_CornerInsideTopRight;
	else                                               return Shape_Invalid;
}

// Namespace end
}
}
}
