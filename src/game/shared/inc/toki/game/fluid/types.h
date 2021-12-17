#if !defined(INC_TOKI_GAME_FLUID_TYPES_H)
#define INC_TOKI_GAME_FLUID_TYPES_H


#include <list>
#include <string>
#include <set>
#include <vector>

#include <tt/code/BitMask.h>
#include <tt/math/Rect.h>
#include <tt/math/hash/Hash.h>
#include <tt/platform/tt_error.h>

#include <toki/level/types.h>


namespace toki  /*! */ {
namespace game  /*! */ {
namespace fluid /*! */ {

typedef std::set<tt::math::Point2, tt::math::Point2Less> Point2Set;

enum FluidType
{
	FluidType_Water,//!< Water.
	FluidType_Lava, //!< Lava.
	
	FluidType_Count,
	FluidType_Invalid
};

typedef tt::code::BitMask<FluidType, FluidType_Count> FluidTypes;


enum FluidFlowType
{
	FluidFlowType_None,
	
	FluidFlowType_Fall,
	FluidFlowType_Left,
	FluidFlowType_LeftLvl2,
	FluidFlowType_LeftOverFlow,
	FluidFlowType_Right,
	FluidFlowType_RightLvl2,
	FluidFlowType_RightOverFlow,
	FluidFlowType_Still,
	FluidFlowType_StillUnderFall,
	
	FluidFlowType_Count,
	FluidFlowType_Invalid
};

enum NodeType
{
	NodeType_Fall,
	NodeType_Left,
	NodeType_Right,
	NodeType_Lvl2,
	
	NodeType_Count,
	NodeType_Invalid
};


enum FluidConstants
{
	FluidBits_Flow = 4,  // Number of bits reserved for fluid 'flow'
	FluidBits_Type = 2,  // Number of bits reserved for fluid 'type'
	FluidBits_Warp = 1,  // Number of bits reserved for fluid 'warp'
	
	FluidShift_Flow = 0,                                 // Where in the byte to place the fluid 'flow' bits
	FluidShift_Type = FluidShift_Flow + FluidBits_Flow,  // Where in the byte to place the fluid 'type' bits
	FluidShift_Warp = FluidShift_Type + FluidBits_Type,  // Where in the byte to place the fluid 'warp' bits
	
	FluidMask_Flow = ((1 << FluidBits_Flow) - 1) << FluidShift_Flow, // Mask for the direction this fluid tile is flowing (00001111)
	FluidMask_Type = ((1 << FluidBits_Type) - 1) << FluidShift_Type, // Mask for the type of fluid (water/lava)           (00110000)
	FluidMask_Warp = ((1 << FluidBits_Warp) - 1) << FluidShift_Warp  // Mask whether this tile is a warp or not           (01000000)
};

TT_STATIC_ASSERT(((FluidFlowType_Count - 1) << FluidShift_Flow) <= FluidMask_Flow);
TT_STATIC_ASSERT(((FluidType_Count     - 1) << FluidShift_Type) <= FluidMask_Type);


inline bool isValidFluidFlowType(FluidFlowType p_type)
{
	return p_type >= 0 && p_type < FluidFlowType_Count;
}


/*! \brief Indicates whether the specified type is a valid FluidType value. */
inline bool isValidFluidType(FluidType p_type)
{
	return p_type >= 0 && p_type < FluidType_Count;
}


inline const char* getFluidTypeName(FluidType p_type)
{
	switch (p_type)
	{
	case FluidType_Water: return "water";
	case FluidType_Lava:  return "lava";
		
	default:
		TT_PANIC("Invalid FluidType value: %d", p_type);
		return "";
	}
}


inline FluidType getFluidTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < FluidType_Count; ++i)
	{
		const FluidType fluidType = static_cast<FluidType>(i);
		if (p_name == getFluidTypeName(fluidType))
		{
			return fluidType;
		}
	}
	
	return FluidType_Invalid;
}


inline FluidType getFluidType(u8 p_attrib)
{
	return static_cast<FluidType>((p_attrib & FluidMask_Type) >> FluidShift_Type);
}


inline FluidType getFluidType(level::CollisionType p_collisionType)
{
	switch (p_collisionType)
	{
	case level::CollisionType_Water_Source:
	case level::CollisionType_Water_Still:
		return FluidType_Water;
		
	case level::CollisionType_Lava_Source:
	case level::CollisionType_Lava_Still:
		return FluidType_Lava;
		
	default:
		return FluidType_Invalid;
	}
}


inline void setFluidType(u8& p_attrib, FluidType p_type)
{
	p_attrib = static_cast<u8>((p_attrib & ~FluidMask_Type) | (p_type << FluidShift_Type));
}


inline FluidFlowType getFluidFlowType(u8 p_attrib)
{
	return static_cast<FluidFlowType>((p_attrib & FluidMask_Flow) >> FluidShift_Flow);
}

inline void setFluidFlowType(u8& p_attrib, FluidFlowType p_type)
{
	p_attrib = static_cast<u8>((p_attrib & ~FluidMask_Flow) | (p_type << FluidShift_Flow));
}


inline bool isWarpTile(u8 p_attrib)
{
	return (p_attrib & FluidMask_Warp) == FluidMask_Warp;
}

inline void setWarpTile(u8& p_attrib, bool p_isWarpTile)
{
	if (p_isWarpTile) p_attrib |=  FluidMask_Warp;
	else              p_attrib &= ~FluidMask_Warp;
}


inline NodeType getNodeType(FluidFlowType p_flowType)
{
	switch(p_flowType)
	{
	case FluidFlowType_Fall:          return NodeType_Fall;
	case FluidFlowType_Left:
	case FluidFlowType_LeftOverFlow:  return NodeType_Left;
	case FluidFlowType_Right: 
	case FluidFlowType_RightOverFlow: return NodeType_Right;
	case FluidFlowType_Still:
	case FluidFlowType_StillUnderFall:
	case FluidFlowType_LeftLvl2:
	case FluidFlowType_RightLvl2:     return NodeType_Lvl2;
	
	default:
		TT_PANIC("Invalid FluidFlowType [%d] to convert to NodeType",p_flowType);
		return NodeType_Invalid;
	}
}

inline FluidFlowType getFluidFlowType(NodeType p_nodeType)
{
	switch(p_nodeType)
	{
	case NodeType_Fall:    return FluidFlowType_Fall;
	case NodeType_Left:    return FluidFlowType_Left;
	case NodeType_Right:   return FluidFlowType_Right;
	
	default:
		TT_PANIC("Invalid NodeType [%d] to convert to FluidFlowType",p_nodeType);
		return FluidFlowType_Invalid;
	}
}


inline bool isFall(FluidFlowType p_type)
{
	return p_type == FluidFlowType_Fall         ||
	       p_type == FluidFlowType_LeftOverFlow ||
	       p_type == FluidFlowType_RightOverFlow;
}


inline bool isFalling(FluidFlowType p_type)
{
	return p_type == FluidFlowType_Fall         ||
	       p_type == FluidFlowType_LeftOverFlow ||
	       p_type == FluidFlowType_RightOverFlow;
}


inline bool isLvl2(FluidFlowType p_type)
{
	return p_type == FluidFlowType_Still          ||
	       p_type == FluidFlowType_StillUnderFall ||
	       p_type == FluidFlowType_LeftLvl2       ||
	       p_type == FluidFlowType_RightLvl2;
}


inline bool isStill(FluidFlowType p_type)
{
	return p_type == FluidFlowType_Still         ||
	       p_type == FluidFlowType_StillUnderFall;
}


inline bool isValidNodeType(NodeType p_type)
{
	return p_type >= 0 && p_type < NodeType_Count;
}


inline const char* getNodeTypeName(NodeType p_type)
{
	switch (p_type)
	{
	case NodeType_Fall:  return "fall";
	case NodeType_Left:  return "left";
	case NodeType_Right: return "right";
	case NodeType_Lvl2:  return "lvl2";
		
	default:
		TT_PANIC("Invalid NodeType value: %d", p_type);
		return "";
	}
}


inline NodeType getNodeTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < NodeType_Count; ++i)
	{
		const NodeType nodeType = static_cast<NodeType>(i);
		if (p_name == getNodeTypeName(nodeType))
		{
			return nodeType;
		}
	}
	
	return NodeType_Invalid;
}


inline const char* getFluidFlowTypeName(FluidFlowType p_type)
{
	switch (p_type)
	{
	case FluidFlowType_None:           return "None";
	case FluidFlowType_Fall:           return "Fall";
	case FluidFlowType_Left:           return "Left";
	case FluidFlowType_LeftLvl2:       return "LeftLvl2";
	case FluidFlowType_LeftOverFlow:   return "LeftOverFlow";
	case FluidFlowType_Right:          return "Right";
	case FluidFlowType_RightLvl2:      return "RightLvl2";
	case FluidFlowType_RightOverFlow:  return "RightOverFlow";
	case FluidFlowType_Still:          return "Still";
	case FluidFlowType_StillUnderFall: return "StillUnderFall";
	
	default:
		TT_PANIC("Invalid getFluidFlowType value: %d", p_type);
		return "";
	}
}


//--------------------------------------------------------------------------------------------------
// NEW FLUIDS CODE

enum FlowState
{
	FlowState_Expand,
	FlowState_OverflowSingle,
	FlowState_OverflowDoubleFirstTile,
	FlowState_OverflowDoubleSecondTile,
	FlowState_EnterWarp,
	FlowState_ExitWarp,

	FlowState_Done,
	FlowState_BlockedBySolid,
	FlowState_BlockedByKill,
	FlowState_BlockedByLava,
	FlowState_BlockedByStill,
	FlowState_DoneOverflowSingle,
	FlowState_DoneOverflowDouble,
	FlowState_DoneEnterWarp,

	FlowState_Drain,

	FlowState_Count
};


enum FallType
{
	FallType_FromSource,
	FallType_FromFlowMiddle,
	FallType_FromOverflowSingle,
	FallType_FromOverflowDoubleFirst,
	FallType_FromOverflowDoubleSecond,
	FallType_FromWarp
};

typedef tt::math::hash::Hash<32> WarpID;

struct FluidShape
{
	tt::math::VectorRect area;
	FluidType            type;
	WarpID               warpID;

	FluidShape() : area(), type(FluidType_Water) { }

	FluidShape(const tt::math::VectorRect& p_area, FluidType p_type)
	:
	area(p_area),
	type(p_type),
	warpID()
	{ }
};


struct Flow : public FluidShape
{
	tt::math::Point2 growLeft;
	tt::math::Point2 growRight;
	FlowState        stateLeft;
	FlowState        stateRight;
	s32              feedPointCount;
	s32              currentLeft;
	s32              currentRight;

	Flow(const tt::math::VectorRect& p_area, FluidType p_type)
	:
	FluidShape(p_area, p_type),
	growLeft(area.getPosition()),
	growRight(area.getPosition()),
	stateLeft(FlowState_Expand),
	stateRight(FlowState_Expand),
	feedPointCount(1),
	currentLeft(growLeft.x),
	currentRight(growRight.x +1)
	{ }
};


struct Fall : public FluidShape
{
	tt::math::Point2    startTile;
	tt::math::Point2    growTile;
	FlowState           state;
	FallType            fallType;
};


typedef std::list<Fall> FluidFalls;
typedef std::list<Flow> FluidFlows;
typedef std::list<s32>  FeedPoints;
typedef std::vector<FluidFalls> LevelFalls;
typedef std::vector<FluidFlows> LevelFlows;


//--------------------------------------------------------------------------------------------------


enum EntityFluidEffectType // We have entity effects (particle, sounds) for these types.
{
	EntityFluidEffectType_Surface, // in pool
	EntityFluidEffectType_Fall,    // under fall
	
	EntityFluidEffectType_Count
};

inline bool isValidEntityFluidEffectType(EntityFluidEffectType p_EntityFluidEffectType)
{
	return p_EntityFluidEffectType >= 0 && p_EntityFluidEffectType < EntityFluidEffectType_Count;
}


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_TYPES_H)
