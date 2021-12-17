#if !defined(INC_TOKI_GAME_FLUID_GRAPHICSHELPERS_H)
#define INC_TOKI_GAME_FLUID_GRAPHICSHELPERS_H


#include <algorithm>
#include <list>
#include <vector>

#include <tt/math/Point2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_types.h>

#include <toki/game/fluid/types.h>


namespace toki {
namespace game {
namespace fluid {

// This file contains helper types and functions for the fluid graphics


//--------------------------------------------------------------------------------------------------
// QuadType

// NOTE: The order of the enum values is the order in which the quads will be rendered
enum QuadType
{
	// Background quads
	QuadType_Bg_WaterPoolLeft,
	QuadType_Bg_WaterPoolStill,
	QuadType_Bg_WaterPoolRight,
	
	QuadType_Bg_LavaPoolLeft,
	QuadType_Bg_LavaPoolStill,
	QuadType_Bg_LavaPoolRight,
	
	QuadType_Bg_WaterOverFlowLeft,
	QuadType_Bg_WaterOverFlowRight,
	
	QuadType_Bg_LavaOverFlowLeft,
	QuadType_Bg_LavaOverFlowRight,

	QuadType_Bg_WaterFall,
	QuadType_Bg_LavaFall,
	
	// Foreground quads
	QuadType_WaterFall,
	
	QuadType_WaterFallEdgeLeft,
	QuadType_WaterFallEdgeRight,
	
	QuadType_LavaFall,
	
	QuadType_LavaFallEdgeLeft,
	QuadType_LavaFallEdgeRight,
	
	QuadType_WaterPoolLeft,
	QuadType_WaterPoolStill,
	QuadType_WaterPoolRight,
	
	QuadType_LavaPoolLeft,
	QuadType_LavaPoolStill,
	QuadType_LavaPoolRight,
	
	QuadType_WaterOverFlowLeft,
	QuadType_WaterOverFlowRight,
	
	QuadType_LavaOverFlowLeft,
	QuadType_LavaOverFlowRight,
	
	QuadType_Count,
	
	QuadType_Invalid,
	
	// Helper constants for easier iteration
	QuadType_Background_First = QuadType_Bg_WaterPoolLeft,
	QuadType_Background_Last  = QuadType_Bg_LavaOverFlowRight,
	QuadType_Foreground_First = QuadType_WaterFall,
	QuadType_Foreground_Last  = QuadType_LavaOverFlowRight
};


inline bool isValidQuadType(QuadType p_type)
{
	return p_type >= 0 && p_type < QuadType_Count;
}


inline const char* const getQuadTypeName(QuadType p_quadType)
{
	switch (p_quadType)
	{
		// Water:
	case QuadType_Bg_WaterPoolLeft:      return "water_pool_moving_back";
	case QuadType_Bg_WaterPoolStill:     return "water_pool_still_back";
	case QuadType_Bg_WaterPoolRight:     return "water_pool_moving_back";
	case QuadType_Bg_WaterOverFlowLeft:  return "water_pool_moving_back";
	case QuadType_Bg_WaterOverFlowRight: return "water_pool_moving_back";
	case QuadType_Bg_WaterFall:          return "water_fall_back";
		
	case QuadType_WaterFall:          return "water_fall";
	case QuadType_WaterPoolLeft:      return "water_pool_moving";
	case QuadType_WaterPoolStill:     return "water_pool_still";
	case QuadType_WaterPoolRight:     return "water_pool_moving";
	case QuadType_WaterOverFlowLeft:  return "water_pool_moving";
	case QuadType_WaterOverFlowRight: return "water_pool_moving";
		
	case QuadType_WaterFallEdgeLeft:  return "water_fall_edge_left";
	case QuadType_WaterFallEdgeRight: return "water_fall_edge_right";
		
		// Lava:
	case QuadType_Bg_LavaPoolLeft:      return "lava_pool_moving_back";
	case QuadType_Bg_LavaPoolStill:     return "lava_pool_still_back";
	case QuadType_Bg_LavaPoolRight:     return "lava_pool_moving_back";
	case QuadType_Bg_LavaOverFlowLeft:  return "lava_pool_moving_back";
	case QuadType_Bg_LavaOverFlowRight: return "lava_pool_moving_back";
	case QuadType_Bg_LavaFall:          return "lava_fall_back";
		
	case QuadType_LavaFall:          return "lava_fall";
	case QuadType_LavaPoolLeft:      return "lava_pool_moving";
	case QuadType_LavaPoolStill:     return "lava_pool_still";
	case QuadType_LavaPoolRight:     return "lava_pool_moving";
	case QuadType_LavaOverFlowLeft:  return "lava_pool_moving";
	case QuadType_LavaOverFlowRight: return "lava_pool_moving";
		
	case QuadType_LavaFallEdgeLeft:  return "lava_fall_edge_left";
	case QuadType_LavaFallEdgeRight: return "lava_fall_edge_right";
		
	default:
		TT_PANIC("Unknown QuadType: %d", p_quadType);
		return "";
	}
}


inline QuadType getBackgroundQuadType(QuadType p_type)
{
	if (p_type >= QuadType_Background_First && p_type <= QuadType_Background_Last)
	{
		// Is already a background quad type
		return p_type;
	}
	
	switch (p_type)
	{
	case QuadType_WaterPoolLeft:      return QuadType_Bg_WaterPoolLeft;
	case QuadType_WaterPoolStill:     return QuadType_Bg_WaterPoolStill;
	case QuadType_WaterPoolRight:     return QuadType_Bg_WaterPoolRight;
	case QuadType_WaterOverFlowLeft:  return QuadType_Bg_WaterOverFlowLeft;
	case QuadType_WaterOverFlowRight: return QuadType_Bg_WaterOverFlowRight;
		
	case QuadType_LavaPoolLeft:      return QuadType_Bg_LavaPoolLeft;
	case QuadType_LavaPoolStill:     return QuadType_Bg_LavaPoolStill;
	case QuadType_LavaPoolRight:     return QuadType_Bg_LavaPoolRight;
	case QuadType_LavaOverFlowLeft:  return QuadType_Bg_LavaOverFlowLeft;
	case QuadType_LavaOverFlowRight: return QuadType_Bg_LavaOverFlowRight;
		
	default:
		TT_PANIC("No background quad available for type %d.", p_type);
		return p_type;
	}
}


inline QuadType getQuadType(FluidFlowType p_flowType, FluidType p_fluidType)
{
	if (isValidFluidType(p_fluidType) == false)
	{
		return QuadType_Invalid;
	}
	
	TT_ASSERT(p_fluidType == FluidType_Water || p_fluidType == FluidType_Lava);
	
	switch (p_flowType)
	{
	case FluidFlowType_Fall:
		return (p_fluidType == FluidType_Water) ? QuadType_WaterFall : QuadType_LavaFall;
		
	case FluidFlowType_Left:
	case FluidFlowType_LeftLvl2:
		return (p_fluidType == FluidType_Water) ? QuadType_WaterPoolLeft : QuadType_LavaPoolLeft;
		
	case FluidFlowType_Still:
	case FluidFlowType_StillUnderFall:
		return (p_fluidType == FluidType_Water) ? QuadType_WaterPoolStill : QuadType_LavaPoolStill;
		
	case FluidFlowType_Right:
	case FluidFlowType_RightLvl2:
		return (p_fluidType == FluidType_Water) ? QuadType_WaterPoolRight : QuadType_LavaPoolRight;
		
	case FluidFlowType_LeftOverFlow:
		return (p_fluidType == FluidType_Water) ? QuadType_WaterOverFlowLeft : QuadType_LavaOverFlowLeft;
		
	case FluidFlowType_RightOverFlow:
		return (p_fluidType == FluidType_Water) ? QuadType_WaterOverFlowRight : QuadType_LavaOverFlowRight;
		
	case FluidFlowType_None:
		return QuadType_Invalid;  // Do not render "no fluid"
		
	default:
		TT_PANIC("Unknown flowType: %d", p_flowType);
		return QuadType_Invalid;
	}
}


//--------------------------------------------------------------------------------------------------
// FluidPrimitive

enum FluidPrimitive
{
	FluidPrimitive_Pool,
	FluidPrimitive_Fall,
	FluidPrimitive_Overflow,
	
	FluidPrimitive_Count,
	FluidPrimitive_Invalid
};


inline bool isValidFluidPrimitive(FluidPrimitive p_primitiveType)
{
	return p_primitiveType >= 0 && p_primitiveType < FluidPrimitive_Count;
}


inline FluidPrimitive getPrimitiveFromFlowType(FluidFlowType p_flowType)
{
	switch (p_flowType)
	{
	case FluidFlowType_Fall:
		return FluidPrimitive_Fall;
		
	case FluidFlowType_Left:
	case FluidFlowType_Right:
	case FluidFlowType_LeftLvl2:
	case FluidFlowType_RightLvl2:
	case FluidFlowType_Still:
	case FluidFlowType_StillUnderFall:
		return FluidPrimitive_Pool;
		
	case FluidFlowType_LeftOverFlow:
	case FluidFlowType_RightOverFlow:
		return FluidPrimitive_Overflow;
		
	case FluidFlowType_None:
		return FluidPrimitive_Invalid;
		
	default:
		TT_PANIC("Unknown FluidFlowType: %d", p_flowType);
		return FluidPrimitive_Invalid;
	}
}


//--------------------------------------------------------------------------------------------------
// SubQuad

struct SubQuad
{
	tt::math::Point2 min;
	tt::math::Point2 max;
	QuadType         type;
	
	
	inline SubQuad()
	:
	min(-1, -1),
	max(-1, -1),
	type(QuadType_Invalid)
	{
	}
	
	inline SubQuad(const tt::math::Point2& p_pos, QuadType p_type)
	:
	min(p_pos),
	max(p_pos),
	type(p_type)
	{
		TT_ASSERT(isValidQuadType(type));
	}
	
	inline SubQuad(const tt::math::Point2& p_min, const tt::math::Point2& p_max, QuadType p_type)
	:
	min(p_min),
	max(p_max),
	type(p_type)
	{
		TT_ASSERT(isValidQuadType(type));
	}
};


typedef std::vector<SubQuad> SubQuads;


//--------------------------------------------------------------------------------------------------
// TileBlob

class TileBlob
{
public:
	inline TileBlob(FluidType p_fluid, FluidPrimitive p_primitiveType, const tt::math::Point2& p_pos)
	:
	m_fluid(p_fluid),
	m_primitiveType(p_primitiveType),
	m_minPoint(p_pos),
	m_maxPoint(p_pos),
	m_subQuads()
	{
	}
	
	inline TileBlob()
	:
	m_fluid(FluidType_Invalid),
	m_primitiveType(FluidPrimitive_Invalid),
	m_minPoint(),
	m_maxPoint(),
	m_subQuads()
	{
	}
	
	inline void add(const TileBlob& p_other)
	{
		TT_ASSERT(isValidFluidPrimitive(p_other.getPrimitiveType()));
		TT_ASSERT(m_fluid         == p_other.m_fluid);
		TT_ASSERT(m_primitiveType == p_other.m_primitiveType);
		
		m_minPoint.x = std::min(m_minPoint.x, p_other.m_minPoint.x);
		m_minPoint.y = std::min(m_minPoint.y, p_other.m_minPoint.y);
		
		m_maxPoint.x = std::max(m_maxPoint.x, p_other.m_maxPoint.x);
		m_maxPoint.y = std::max(m_maxPoint.y, p_other.m_maxPoint.y);
		
		const SubQuads& otherQuads = p_other.m_subQuads;
		m_subQuads.insert(m_subQuads.end(), otherQuads.begin(), otherQuads.end());
	}
	
	inline void add(const tt::math::Point2& p_pos)
	{
		TT_ASSERT(m_minPoint.x <= p_pos.x);
		TT_ASSERT(m_minPoint.y <= p_pos.y);
		
		m_maxPoint.x = std::max(m_maxPoint.x, p_pos.x);
		m_maxPoint.y = std::max(m_maxPoint.y, p_pos.y);
	}
	
	inline FluidType               getFluidType()     const { return m_fluid;         }
	inline FluidPrimitive          getPrimitiveType() const { return m_primitiveType; }
	inline const tt::math::Point2& getMin()           const { return m_minPoint;      }
	inline const tt::math::Point2& getMax()           const { return m_maxPoint;      }
	
	inline void addSubQuad(const SubQuad& p_quad)
	{
		m_subQuads.push_back(p_quad);
	}
	
	inline const SubQuads& getSubQuads() const { return m_subQuads; }
	
	inline void print() const
	{
		TT_Printf("type: %d, min: (%d, %d). max: (%d, %d).",
		          m_primitiveType,
		          m_minPoint.x, m_minPoint.y,
		          m_maxPoint.x, m_maxPoint.y);
	}
	
private:
	FluidType        m_fluid;
	FluidPrimitive   m_primitiveType;
	tt::math::Point2 m_minPoint;
	tt::math::Point2 m_maxPoint;
	SubQuads         m_subQuads;
};


//--------------------------------------------------------------------------------------------------
// BlobData

struct BlobData
{
	typedef std::list<TileBlob> Blobs;
	
	Blobs allBlobs; // All the tile blobs that are found are stored here.
	
	// The following prev* vectors holds the tile info previously found, 
	// these tiles might get extended with new tiles.
	// 
	// While iterating over a row prev*[i] points to:
	// * The found blobs to the left of the current row (y) if (i < x).
	// * The found blobs in the row below (y - 1)           if (i >= x).
	std::vector<s32>            prevBlobIndex;   // Points to index of allBlobs
	std::vector<s32>            prevColumnDepth; // Deepest point in this column which could form a quad.
	std::vector<FluidType>      prevFluid;
	std::vector<FluidFlowType>  prevFlow;
	std::vector<FluidPrimitive> prevPrimitive;
	
	SubQuad activeMergeQuad;               // Once a column can no longer be filled heigh we'll try merge to with right.
	s32     activeMergeQuadBlobIndex; // If >= 0 the merge quad is active.
	
	
	inline BlobData()
	:
	allBlobs(),
	prevBlobIndex(),
	prevColumnDepth(),
	prevFluid(),
	prevFlow(),
	prevPrimitive(),
	activeMergeQuad(),
	activeMergeQuadBlobIndex(-1)
	{ }
	
	inline void reset(s32 width)
	{
		allBlobs.clear();
		
		prevBlobIndex.clear();
		prevBlobIndex.resize(  width, -1);
		prevColumnDepth.clear();
		prevColumnDepth.resize(width, -1);
		prevFluid.clear();
		prevFluid.resize(      width, FluidType_Invalid);
		prevFlow.clear();
		prevFlow.resize(       width, FluidFlowType_None);
		prevPrimitive.clear();
		prevPrimitive.resize(  width, FluidPrimitive_Invalid);
		
		activeMergeQuad.type     = QuadType_Invalid;
		activeMergeQuadBlobIndex = -1;
	}
	
	inline bool hasActiveMergeQuad() const { return activeMergeQuadBlobIndex >= 0; }
	
	inline TileBlob& getBlob(s32 p_index)
	{
		TT_ASSERT(p_index >= 0 && static_cast<size_t>(p_index) < allBlobs.size());
		Blobs::iterator blobIt = allBlobs.begin();
		std::advance(blobIt, p_index);
		TT_ASSERT(blobIt != allBlobs.end());
		return (*blobIt);
	}
	
	inline void mergeBlobs(s32 p_indexOne, s32 p_indexTwo)
	{
		const s32 oldPointIdx = p_indexOne;
		s32 newPointIdx       = p_indexTwo;
		
		TT_ASSERT(oldPointIdx >= 0 && oldPointIdx < static_cast<s32>(allBlobs.size()));
		TT_ASSERT(newPointIdx >= 0 && newPointIdx < static_cast<s32>(allBlobs.size()));
		
		// Merge blobs
		{
			Blobs::iterator oldIt = allBlobs.begin();
			std::advance(oldIt, oldPointIdx);
			
			// Add points from old blob to the new blob.
			getBlob(newPointIdx).add((*oldIt));
			allBlobs.erase(oldIt);
			
			if (oldPointIdx < newPointIdx)
			{
				--newPointIdx;
			}
		}
		
		if (activeMergeQuadBlobIndex >= 0) // If set
		{
			TT_ASSERT(isValidQuadType(activeMergeQuad.type)); // Need valid quad if index is valid.
			
			// Make sure it keeps point to the correct blob index.
			if (activeMergeQuadBlobIndex >= oldPointIdx)
			{
				if (activeMergeQuadBlobIndex == oldPointIdx)
				{
					activeMergeQuadBlobIndex = newPointIdx;
				}
				else
				{
					--activeMergeQuadBlobIndex;
				}
				TT_ASSERT(activeMergeQuadBlobIndex >= 0); // Make sure the index is still valid.
			}
		}
		
		// Update all prevBlobIndex's because oldIt is gone.
		const std::vector<s32>::size_type width = prevBlobIndex.size();
		for (std::vector<s32>::size_type i = 0; i < width; ++i)
		{
			if (prevBlobIndex[i] >= oldPointIdx) // The prevBlobIndex[i] >= have to be corrected
			{
				if (prevBlobIndex[i] == oldPointIdx) // old should get the new idx
				{
					prevBlobIndex[i] = newPointIdx;
				}
				else // prevBlobIndex[i] > oldPointIdx needs to be moved 1 prevBlobIndex[i] back because oldIt is gone
				{
					--prevBlobIndex[i];
				}
			}
		}
	}
	
	inline void invalidateMergeQuad()
	{
		activeMergeQuadBlobIndex = -1;
		activeMergeQuad.type     = QuadType_Invalid;
	}
	
	inline void saveAndResetMergeQuad()
	{
		// Done with quad merge.
		TileBlob& blob = getBlob(activeMergeQuadBlobIndex);
		blob.addSubQuad(activeMergeQuad);
		
		// Invalid quad
		invalidateMergeQuad();
	}
	
	inline void checkForQuadMerge(s32 p_index, s32 p_maxX, s32 p_newDepth)
	{
		if (hasActiveMergeQuad()) // We have a quad to merge to.
		{
			TT_ASSERT(isValidQuadType(activeMergeQuad.type)); // Need valid quad if index is valid.
			
			// Check if the current column can be merged to quad.
			
			if (p_index               > 0 && // We're on the same Row as the merge quad. (We have a tile to the left)
			    activeMergeQuad.type  == getQuadType(prevFlow[p_index], prevFluid[p_index]) &&
			    activeMergeQuad.min.y == prevColumnDepth[p_index] && // Same type and depth
			    activeMergeQuad.min.x <= p_maxX)
			{
				// We can merge.
				
				TT_ASSERT(p_maxX > activeMergeQuad.max.x); // Sanity check to make sure the override below is really needed.
				activeMergeQuad.max.x = p_maxX;
				
				// The bottom of this column goes to merge quad so reset depth to remaining top.
				prevColumnDepth[p_index] = p_newDepth;
			}
			else
			{
				// Done with quad merge.
				saveAndResetMergeQuad();
			}
		}
	}
	
	inline void createMergeQuad(s32 p_columnIndex, s32 p_maxY)
	{
		TT_ASSERT(hasActiveMergeQuad() == false); // Make sure the index is invalid.
		TT_ASSERT(isValidQuadType(activeMergeQuad.type) == false); // Expect invalid quad if index is invalid.
		
		activeMergeQuadBlobIndex = prevBlobIndex[p_columnIndex];
		activeMergeQuad.type     = getQuadType(prevFlow[p_columnIndex], prevFluid[p_columnIndex]);
		TT_ASSERT(activeMergeQuadBlobIndex >= 0);
		TT_ASSERT(isValidQuadType(activeMergeQuad.type));
		
		activeMergeQuad.min.x = p_columnIndex;
		activeMergeQuad.min.y = prevColumnDepth[p_columnIndex];
		activeMergeQuad.max.x = p_columnIndex;
		activeMergeQuad.max.y = p_maxY;
	}
	
	inline void assertIsValidIndex(s32 p_index) const
	{
		TT_ASSERT(                    p_index >= 0);
		TT_ASSERT(static_cast<size_t>(p_index) < prevBlobIndex.size());
		TT_ASSERT(static_cast<size_t>(p_index) < prevColumnDepth.size());
		TT_ASSERT(static_cast<size_t>(p_index) < prevFluid.size());
		TT_ASSERT(static_cast<size_t>(p_index) < prevFlow.size());
		TT_ASSERT(static_cast<size_t>(p_index) < prevPrimitive.size());
	}
	
	inline void setPrevValues(s32 p_index, s32 p_blobIndex, 
	                          FluidType p_fluidType, FluidFlowType p_flowType,
	                          FluidPrimitive p_primitveType, s32 p_columnDepth)
	{
		assertIsValidIndex(p_index);
		
		// Reset index
		prevBlobIndex[  p_index] = p_blobIndex;
		prevColumnDepth[p_index] = p_columnDepth;
		prevFluid[      p_index] = p_fluidType;
		prevFlow[       p_index] = p_flowType;
		prevPrimitive[  p_index] = p_primitveType;
	}
	
	inline void resetAllPrev(s32 p_index)
	{
		setPrevValues(p_index, -1, FluidType_Invalid, FluidFlowType_None, FluidPrimitive_Invalid, -1);
	}
	
	inline void assertAllPrevAreReset(s32 p_index) const
	{
		assertIsValidIndex(p_index);
		
		TT_ASSERT(prevBlobIndex[  p_index] == -1);
		TT_ASSERT(prevColumnDepth[p_index] == -1);
		TT_ASSERT(prevFluid[      p_index] == FluidType_Invalid);
		TT_ASSERT(prevFlow[       p_index] == FluidFlowType_None);
		TT_ASSERT(prevPrimitive[  p_index] == FluidPrimitive_Invalid);
	}
};


inline void doGrowBlobData(BlobData& p_blobData, const tt::math::Point2& p_pos, FluidFlowType p_flowType, u8 p_value)
{
	TT_ASSERT(isValidFluidFlowType(p_flowType));
	
	const s32 index     = p_pos.x;     // Index to the prev* arrays
	const s32 leftIndex = index - 1;
	
	const FluidPrimitive primitiveType = getPrimitiveFromFlowType(p_flowType);
	
	p_blobData.checkForQuadMerge(index, p_pos.x, p_pos.y);
	
	if (primitiveType == FluidPrimitive_Invalid)
	{
		// Because current tile has no fluid.
		// The one below can't be glued so clear it.
		if (p_blobData.prevPrimitive[index] != FluidPrimitive_Invalid) // Do we need to reset?
		{
			if (p_blobData.prevColumnDepth[index] != -1 &&  // We have an column below which is now ended.
			    p_blobData.hasActiveMergeQuad()   == false) // No merge already active.
			{
				// The current pos is no longer valid for the quad below. So -1.
				p_blobData.createMergeQuad(index, p_pos.y - 1);
			}
			
			p_blobData.resetAllPrev(index);
		}
#if defined(TT_BUILD_DEV)
		else
		{
			p_blobData.assertAllPrevAreReset(index);
		}
#endif
		return;
	}
	
	const FluidType fluidType = getFluidType(p_value);
	TT_ASSERT(isValidFluidType(fluidType));
	
	// ---------------------------------------------------------------------
	// Set array[index] with own values, and maybe merge with the tile below
	//
	// Check if the tile below doesn't match
	if (p_blobData.prevPrimitive[index] != primitiveType ||
	    p_blobData.prevFluid[    index] != fluidType)
	{
		if (p_blobData.prevColumnDepth[index] != -1 &&  // We have an column below which is now ended.
		    p_blobData.hasActiveMergeQuad()   == false) // No merge already active.
		{
			// The current pos is no longer valid for the quad below. So -1.
			p_blobData.createMergeQuad(index, p_pos.y - 1);
		}
		
		// No match, set own values
		p_blobData.allBlobs.push_back(TileBlob(fluidType, primitiveType, p_pos)); // Create new blob for this point.
		p_blobData.setPrevValues(index, static_cast<s32>(p_blobData.allBlobs.size() - 1), fluidType, p_flowType, primitiveType, p_pos.y);
	}
	else
	{
#if defined(TT_BUILD_DEV)
		// The exiting values can be reused, and the points are shared.
		TT_ASSERT(p_blobData.prevFluid[    index] == fluidType);
		TT_ASSERT(p_blobData.prevPrimitive[index] == primitiveType);
#endif
		TileBlob& blob = p_blobData.getBlob(p_blobData.prevBlobIndex[index]);
		blob.add(p_pos);
		
		// Sanity check - Make sure the active column depth is below the curreent pos.
		TT_ASSERT(p_blobData.prevColumnDepth[index] <= p_pos.y);
		
		// Different quad type, need to split. up here.
		if ( getQuadType(p_blobData.prevFlow[index], p_blobData.prevFluid[index]) != getQuadType(p_flowType, fluidType) )
		{
			if (p_blobData.prevColumnDepth[index] != -1 &&  // We have an column below which is now ended.
			    p_blobData.hasActiveMergeQuad()   == false) // No merge already active.
			{
				// The current pos is no longer valid for the quad below. So -1.
				p_blobData.createMergeQuad(index, p_pos.y - 1);
				p_blobData.prevColumnDepth[index] = p_pos.y; // Make sure these types continue.
			}
		}
	}
	
	// ------------------------------------------------------
	// Check for merge with left tiles.
	// (More work because we could have gotten all the tiles from below.)
	if (leftIndex < 0) // No tile to the left.
	{
		return;
	}
	
	// Check if current tile matches the one on the left side.
	if (p_blobData.prevPrimitive[leftIndex] == primitiveType &&
	    p_blobData.prevFluid[    leftIndex] == fluidType     &&      // Match
	    p_blobData.prevBlobIndex[leftIndex] != p_blobData.prevBlobIndex[index]) // But not the same blobs
	{
		// Need to merge the blobs because they touch.
		p_blobData.mergeBlobs(p_blobData.prevBlobIndex[index], p_blobData.prevBlobIndex[leftIndex]);
	}
}


inline void doGrowBlobDataFlush(BlobData& p_blobData, s32 p_index, s32 p_height)
{
	p_blobData.checkForQuadMerge(p_index, p_index, -1);
	
	if (p_blobData.prevPrimitive[p_index] != FluidPrimitive_Invalid) // Do we need to reset?
	{
		if (p_blobData.prevColumnDepth[p_index] != -1 &&  // We have an column below which is now ended.
		    p_blobData.hasActiveMergeQuad()   == false) // No merge already active.
		{
			// The current pos is no longer valid for the quad below. So -1.
			p_blobData.createMergeQuad(p_index, p_height - 1);
		}
		
		p_blobData.resetAllPrev(p_index);
	}
#if defined(TT_BUILD_DEV)
	else
	{
		p_blobData.assertAllPrevAreReset(p_index);
	}
#endif
}


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_GRAPHICSHELPERS_H)
