#ifndef INC_TT_PROFILER_MEMORY_PROFILER_H
#define INC_TT_PROFILER_MEMORY_PROFILER_H

#include <tt/platform/tt_types.h>
#include <tt/math/math.h>
#include <tt/profiler/MemoryProfilerConstants.h>

#ifdef PROFILE_MEMORY

namespace tt {
namespace profiler {

#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
	#define LOWER_GUARD_PATTERN		(0xCAFEBABE)
	#define UPPER_GUARD_PATTERN		(0xDEADBEEF)
#endif

class MemoryProfiler 
{
public:
	struct DebugHeader
	{
		int m_index;
		u32 m_stamp;
		std::size_t m_headerSize;
		std::size_t m_allocatedSize;
#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
		u32 m_guardBytes;
#endif
	};

	struct HeapStats
	{
		std::size_t allocated;
		std::size_t largestAllocated;
		std::size_t debugOverhead;
		int nblocks;
	};

	static void init();
	static void startMeasure();
	static void stopMeasure( bool p_assert );
	static void stopMeasure( )
	{
		stopMeasure( false );
	}

	static void setBreakPoint( int p_breakpoint, int p_run = 1 );
	static void updateHeapStats();
	static void checkProfiler();

	static const HeapStats& getHeapStats(int p_heapid)
	{
		TT_ASSERT(p_heapid >=0 && p_heapid < Constants_NumberOfHeaps);

		return m_heapstats[p_heapid];
	}

	static void addMemBlock(void* p_block, void* p_creator, 
                            s32 p_type, std::size_t p_size, std::size_t p_alignment, 
		                    int p_heapid);

	static void delMemBlock(void* p_block, void* p_destructor);
	static void printMemBlock(void* p_block);

	enum DumpSettings
	{
		DumpSettings_AllAllocs,
		DumpSettings_NewAllocs,
		DumpSettings_ShowStatsOnly,
		DumpSettings_ShowStatsOnlyNoHeader
	};

	static void dump(DumpSettings p_settings);

	// helper function for malloc
	static inline std::size_t getCorrectedAllocSize(std::size_t p_size, 
		                                            std::size_t p_alignment)
    {
        // aligned size + reserve space to store debug info
#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
        return alignSize(sizeof(DebugHeader), p_alignment) +  
               p_size + sizeof(u32);	// upper guard bytes
#else
        return alignSize(sizeof(DebugHeader), p_alignment) +  
               p_size;
#endif
    }

	// helper function for malloc
	static inline void* getCorrectedAllocAddress(void *p_block,
                                                 std::size_t p_alignment)
	{
		return static_cast<u8 *>(p_block) + 
			   alignSize(sizeof(DebugHeader), p_alignment);
	}

	// helper function for free
	static inline void* getCorrectedFreeAddress(void *p_block)
	{
		// debug info is stored directly before p_block
		DebugHeader* info = getDebugHeader(p_block, -1);
		return static_cast<u8 *>(p_block) - info->m_headerSize;
	}

	// helper function 
	static inline std::size_t getDebugHeaderSize(void *p_block)
	{
		// debug info is stored directly before p_block
		DebugHeader* info = getDebugHeader(p_block, -1);
		return info->m_headerSize;
	}

private:
	static inline std::size_t alignSize(std::size_t p_size, std::size_t p_alignment)
	{
		return tt::math::roundUp( std::max(p_alignment, p_size),
			                      static_cast<int>(p_alignment) );
	}

	static inline DebugHeader* getDebugHeader(void* p_block, int p_offset)
	{
		return static_cast<DebugHeader*>(p_block) + p_offset;
	}

#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
	static inline u32* getUpperGuardBytes(void* p_block)
	{
		DebugHeader* info = getDebugHeader(p_block, -1);
		return reinterpret_cast<u32 *>(static_cast<u8 *>(p_block) + 
			                           alignSize(info->m_allocatedSize, 4));
	}
#endif

	class MemBlock
	{
	public:
		void* m_block;
		std::size_t m_size;
		std::size_t m_alignment;
		s32 m_type;
		u32 m_flags;
		u32 m_stamp;
		int m_heapid;
		void* m_creator;
		void* m_destructor;
	};

	enum MemBlockFlags
	{
		MemBlockFlags_None	= 0,
		MemBlockFlags_Used	= 1 << 0
	};

	class MemBlockComp
	{
	public:
		inline bool operator()(const MemBlock* p_a, const MemBlock* p_b) const
		{
			return p_a->m_stamp < p_b->m_stamp;
		}
	};
	static MemBlock m_mem_blocks[Constants_MaxMemBlocks];
	static MemBlock* m_mem_blocks_sorted[Constants_MaxMemBlocks];

	static int m_first_free;
	static int m_current_stamp;
	static int m_start_stamp;
	static int m_breakpoint_stamp;

	static int m_current_run;
	static int m_breakpoint_run;

	static HeapStats m_heapstats[Constants_NumberOfHeaps];
};

// namespace
}
}

#endif	// PROFILE_MEMORY

#endif	// INC_TT_PROFILER_MEMORY_PROFILER_H
