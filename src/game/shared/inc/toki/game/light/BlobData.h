#if !defined(INC_TOKI_GAME_LIGHT_BLOBDATA_H)
#define INC_TOKI_GAME_LIGHT_BLOBDATA_H

#include <vector>
#include <list>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>
#include <tt/math/Point2.h>


namespace toki {
namespace game {
namespace light {


struct SubQuad
{
	tt::math::Point2 min;
	tt::math::Point2 max;
	
	inline SubQuad()
	:
	min(-1, -1),
	max(-1, -1)
	{
	}
	
	inline SubQuad(const tt::math::Point2& p_pos)
	:
	min(p_pos),
	max(p_pos)
	{
	}
	
	inline SubQuad(const tt::math::Point2& p_min, const tt::math::Point2& p_max)
	:
	min(p_min),
	max(p_max)
	{
	}
};


typedef std::vector<SubQuad> SubQuads;


class TileBlob
{
public:
	inline TileBlob(const tt::math::Point2& p_pos)
	:
	m_minPoint(p_pos),
	m_maxPoint(p_pos),
	m_subQuads()
	{
	}
	
	inline TileBlob()
	:
	m_minPoint(),
	m_maxPoint(),
	m_subQuads()
	{
	}
	
	inline void add(const TileBlob& p_other)
	{
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
	
	inline const tt::math::Point2& getMin()  const { return m_minPoint;      }
	inline const tt::math::Point2& getMax()  const { return m_maxPoint;      }
	
	inline void addSubQuad(const SubQuad& p_quad)
	{
		m_subQuads.push_back(p_quad);
	}
	
	inline const SubQuads& getSubQuads() const { return m_subQuads; }
	
	inline void print() const
	{
		TT_Printf("blob min: (%d, %d). max: (%d, %d).",
		          m_minPoint.x, m_minPoint.y,
		          m_maxPoint.x, m_maxPoint.y);
	}
	
private:
	tt::math::Point2 m_minPoint;
	tt::math::Point2 m_maxPoint;
	SubQuads         m_subQuads;
};


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
	
	SubQuad activeMergeQuad;               // Once a column can no longer be filled heigh we'll try merge to with right.
	s32     activeMergeQuadBlobIndex; // If >= 0 the merge quad is active.
	
	inline BlobData()
	:
	allBlobs(),
	prevBlobIndex(),
	prevColumnDepth(),
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
		const s32 width = static_cast<s32>(prevBlobIndex.size());
		for (s32 i = 0; i < width; ++i)
		{
			if (prevBlobIndex[i] >= oldPointIdx) // The index >= have to be corrected
			{
				TT_ASSERT(prevBlobIndex[i] >= 0); // Only update valid index
				if (prevBlobIndex[i] == oldPointIdx) // old should get the new idx
				{
					prevBlobIndex[i] = newPointIdx;
				}
				else // prevBlobIndex[i] > oldPointIdx needs to be moved 1 index back because oldIt is gone
				{
					--prevBlobIndex[i];
				}
				TT_ASSERT(prevBlobIndex[i] >= 0); // Make sure the index is still valid.
			}
		}
	}
	
	inline void invalidateMergeQuad()
	{
		activeMergeQuadBlobIndex = -1;
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
			// Check if the current column can be merged to quad.
			
			if (p_index > 0 && // We're on the same Row as the merge quad. (We have a tile to the left)
			    activeMergeQuad.min.y == prevColumnDepth[p_index])  // Same size. (may merge).
			{
				// We can merge.
				TT_ASSERT(activeMergeQuad.min.x <= p_maxX);
				
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
		
		activeMergeQuadBlobIndex = prevBlobIndex[p_columnIndex];
		TT_ASSERT(activeMergeQuadBlobIndex >= 0);
		
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
	}
	
	inline void setPrevValues(s32 p_index, s32 p_blobIndex, s32 p_columnDepth)
	{
		assertIsValidIndex(p_index);
		
		TT_ASSERT((p_blobIndex <  0 && p_columnDepth <  0) || // *Both* values are invalid,
		          (p_blobIndex >= 0 && p_columnDepth >= 0));  // or valid.
		
		// Reset index
		prevBlobIndex[  p_index] = p_blobIndex;
		prevColumnDepth[p_index] = p_columnDepth;
	}
	
	inline void resetAllPrev(s32 p_index)
	{
		setPrevValues(p_index, -1, -1);
	}
	
	inline void assertAllPrevAreReset(s32 p_index) const
	{
		assertIsValidIndex(p_index);
		
		TT_ASSERT(prevBlobIndex[  p_index] == -1);
		TT_ASSERT(prevColumnDepth[p_index] == -1);
	}
};


inline void doGrowBlobData(BlobData& p_blobData, const tt::math::Point2& p_pos, bool p_includeTile)
{
	const s32 index     = p_pos.x; // Index to the prev* arrays
	const s32 leftIndex = index - 1;
	
	p_blobData.checkForQuadMerge(index, p_pos.x, p_pos.y);
	
	if (p_includeTile == false)
	{
		// Because current tile is non block, the one below can't be glued. So clear it.
		if (p_blobData.prevColumnDepth[index] >= 0) // We have an column below which is now ended.
		{
			if (p_blobData.hasActiveMergeQuad()   == false) // No merge already active.
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
	
	// --------------------------------------------
	// We have a light blocking tile at this point.
	
	if (p_blobData.prevBlobIndex[index] < 0) // No tile below
	{
		// Set own values
		p_blobData.allBlobs.push_back(TileBlob(p_pos)); // Create new blob for this point.
		p_blobData.setPrevValues(index, static_cast<s32>(p_blobData.allBlobs.size() - 1), p_pos.y);
	}
	else
	{
		TileBlob& blob = p_blobData.getBlob(p_blobData.prevBlobIndex[index]);
		blob.add(p_pos);
		
		// Sanity check - Make sure the active column depth is below the curreent pos.
		TT_ASSERT(p_blobData.prevColumnDepth[index] <= p_pos.y);
	}
	
	// ------------------------------------------------------
	// Check for merge with left tiles.
	// (More work because we could have gotten all the tiles from below.)
	if (leftIndex < 0) // No tile to the left.
	{
		return;
	}
	
	// Check if current tile matches the one on the left side.
	if (p_blobData.prevColumnDepth[leftIndex] >= 0 &&
	    p_blobData.prevBlobIndex[  leftIndex] != p_blobData.prevBlobIndex[index]) // But not the same blobs
	{
		// Need to merge the blobs because they touch.
		p_blobData.mergeBlobs(p_blobData.prevBlobIndex[index], p_blobData.prevBlobIndex[leftIndex]);
	}
}


inline void doGrowBlobDataFlush(BlobData& p_blobData, s32 p_index, s32 p_height)
{
	p_blobData.checkForQuadMerge(p_index, p_index, p_height);
	
	// Because current tile is non block, the one below can't be glued. So clear it.
	if (p_blobData.prevColumnDepth[p_index] >= 0) // We have an column below which is now ended.
	{
		if (p_blobData.hasActiveMergeQuad()   == false) // No merge already active.
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


#endif  // !defined(INC_TOKI_GAME_LIGHT_BLOBDATA_H)
