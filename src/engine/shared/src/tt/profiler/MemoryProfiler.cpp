#include <tt/profiler/MemoryProfiler.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/mem/Heap.h>
#include <tt/math/math.h>

// please enable profiler in platform specific MemoryProfilerConstants.h
#ifdef PROFILE_MEMORY

#ifndef TT_MEMORY_PROFILER_PRINTF
	#define TT_MEMORY_PROFILER_PRINTF(...)
#endif

namespace tt {
namespace profiler {

// FIXME: Fix the interrupts (platform specific) and code-breakpoint (platform specific)
// FIXME: Check all the signed/unsigned types
// FIXME: Cleanup a bit
// FIXME: perhaps add mechanism to support address alignment

int MemoryProfiler::m_breakpoint_run;
int MemoryProfiler::m_breakpoint_stamp;

MemoryProfiler::MemBlock MemoryProfiler::m_mem_blocks[Constants_MaxMemBlocks];
MemoryProfiler::MemBlock* MemoryProfiler::m_mem_blocks_sorted[Constants_MaxMemBlocks];

int MemoryProfiler::m_first_free;
int MemoryProfiler::m_current_stamp;
int MemoryProfiler::m_start_stamp;
int MemoryProfiler::m_current_run;
MemoryProfiler::HeapStats MemoryProfiler::m_heapstats[Constants_NumberOfHeaps];

void MemoryProfiler::init()
{
	m_first_free = 0;
	m_start_stamp = -1;

	TT_MEMORY_PROFILER_PRINTF("\n************* MEMORY PROFILER: INIT                     ************\n\n");
	dump(DumpSettings_ShowStatsOnlyNoHeader);

	m_current_stamp = 0;
	m_breakpoint_run = -1;
	m_breakpoint_stamp = -1;
	m_current_run = 0;

	for (int i=0; i < Constants_MaxMemBlocks; ++i)
	{
		m_mem_blocks[i].m_block = 0;
		m_mem_blocks[i].m_size = 0;
		m_mem_blocks[i].m_type = 255;
		m_mem_blocks[i].m_flags = MemBlockFlags_None;
		m_mem_blocks[i].m_stamp = 0;
		m_mem_blocks[i].m_creator = 0;
		m_mem_blocks[i].m_destructor = 0;
	}
}

void MemoryProfiler::updateHeapStats()
{
	// FIXME: platform independent mechanism to disable interrupts
	//	OSIntrMode mode = OS_DisableInterrupts();
	//BOOL mode = OSDisableInterrupts();

	// clear the heap stats
	for (int i = 0; i < Constants_NumberOfHeaps; i++)
	{
		m_heapstats[i].allocated = 0;
		m_heapstats[i].debugOverhead = 0;
		m_heapstats[i].largestAllocated = 0;
		m_heapstats[i].nblocks = 0;
	}

	// fetch heap stats
	for(int i = 0; i < Constants_MaxMemBlocks; i++)
	{
		if ( (m_mem_blocks[i].m_flags & MemBlockFlags_Used) )
		{
			int heapid = m_mem_blocks[i].m_heapid;
			m_heapstats[heapid].allocated += m_mem_blocks[i].m_size;

			if (m_mem_blocks[i].m_size > m_heapstats[heapid].largestAllocated)
			{
				m_heapstats[heapid].largestAllocated = m_mem_blocks[i].m_size;
			}
			m_heapstats[heapid].nblocks++;
#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
			m_heapstats[heapid].debugOverhead += sizeof(DebugHeader) + 
				                                 sizeof(u32);
#else
			m_heapstats[heapid].debugOverhead += sizeof(DebugHeader);
#endif
		}
	}

	//OS_RestoreInterrupts(mode);
	//OSRestoreInterrupts(mode);
}

void MemoryProfiler::addMemBlock(void* p_block, void* p_creator, 
                                 s32 p_type, size_t p_size, size_t p_alignment, 
                                 int p_heapid)
{
	// FIXME: platform independent mechanism to disable interrupts
	//	OSIntrMode mode = OS_DisableInterrupts();
	//BOOL mode = OSDisableInterrupts();

	TT_ASSERT(p_heapid >= 0 && p_heapid < Constants_NumberOfHeaps);
	
	if (m_start_stamp > 0)
	{
		// arrived at breakpoint (read below!!)
		if ( (m_current_stamp - m_start_stamp) == m_breakpoint_stamp && m_current_run == m_breakpoint_run)
		{
			// !!! In debugger: set PC to next line and soft run from there !!!
			__debugbreak();

			TT_MEMORY_PROFILER_PRINTF("\n************* MEMORY PROFILER: AT BREAKPOINT (RUN: %3d) ************\n\n", m_current_run);
			TT_MEMORY_PROFILER_PRINTF("Block: 0x%X (Creator: 0x%X), Type: %d, Size: %6d, Stamp: %6d\n",
				p_block,
				p_creator,
				p_type,
				p_size,
				m_breakpoint_stamp, 
				m_current_stamp);
		}
	}

	// 	debug info is stored directly before p_block
	DebugHeader* info = getDebugHeader(p_block, -1);

	// set debug info
	info->m_index = m_first_free;
	info->m_stamp = static_cast<u32>(m_current_stamp);
	info->m_headerSize = alignSize(sizeof(DebugHeader), p_alignment);
	info->m_allocatedSize = p_size;

	// fill mem block
	m_mem_blocks[m_first_free].m_type = p_type;
	m_mem_blocks[m_first_free].m_block = p_block;
	m_mem_blocks[m_first_free].m_size = p_size;
	m_mem_blocks[m_first_free].m_creator = p_creator;
	m_mem_blocks[m_first_free].m_alignment = p_alignment;
	m_mem_blocks[m_first_free].m_destructor = 0;
	m_mem_blocks[m_first_free].m_heapid = p_heapid;
	m_mem_blocks[m_first_free].m_stamp = static_cast<u32>(m_current_stamp);
	m_mem_blocks[m_first_free].m_flags |= MemBlockFlags_Used;

	// do guard bytes stuff
#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
	// set lower guard bytes
	info->m_guardBytes = LOWER_GUARD_PATTERN;

	// set upper guard bytes
	u32* upperGuardBytes = getUpperGuardBytes(p_block);
	*upperGuardBytes = UPPER_GUARD_PATTERN;
#endif

	m_current_stamp++;

	// search top 
	for(int i=(m_first_free+1); i < Constants_MaxMemBlocks; ++i)
	{
		if ((m_mem_blocks[i].m_flags & MemBlockFlags_Used) != MemBlockFlags_Used)
		{
			m_first_free = i;
			//OSRestoreInterrupts(mode);
			//FIXME: OS_RestoreInterrupts(mode);
			return;
		}
	}

	// search bottom
	for(int i=0; i<m_first_free; ++i)
	{
		if ((m_mem_blocks[i].m_flags & MemBlockFlags_Used) != MemBlockFlags_Used)
		{
			m_first_free = i;
			//OSRestoreInterrupts(mode);
			//FIXME: OS_RestoreInterrupts(mode);
			return;
		}
	}

	//OS_RestoreInterrupts(mode);
	//OSRestoreInterrupts(mode);

	TT_PANIC("Out of memblocks!\n"
			 "Solution: Increase MAX_MEMBLOCKS in MemoryProfiler.h");
}

void MemoryProfiler::setBreakPoint(int p_breakpoint_stamp, int p_breakpoint_run)
{
	m_breakpoint_stamp = p_breakpoint_stamp;
	m_breakpoint_run = p_breakpoint_run;
}

void MemoryProfiler::delMemBlock(void* p_block, void* p_destructor)
{
	// FIXME: platform independent mechanism to disable interrupts
	//	OSIntrMode mode = OS_DisableInterrupts();
	//BOOL mode = OSDisableInterrupts();

	// 	debug info is stored directly before p_block
	DebugHeader* info = getDebugHeader(p_block, -1);

#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
	if (info->m_guardBytes != LOWER_GUARD_PATTERN)
	{
		TT_WARN("Lower Guard Bytes Failed! Block 0x%X Destructor 0x%X\n", 
			p_block, p_destructor);

		printMemBlock(p_block);

		TT_PANIC("Lower Guard Bytes Failed!\n");
	}

	if ((*getUpperGuardBytes(p_block)) != UPPER_GUARD_PATTERN)
	{
		TT_WARN("Upper Guard Bytes Failed! Block 0x%X Destructor 0x%X\n", 
			p_block, p_destructor);

		printMemBlock(p_block);

		TT_PANIC("Upper Guard Bytes Failed!\n");
	}
#endif

	// make unsigned to save some OOB checks
	unsigned int index = static_cast<unsigned int>(info->m_index);
	unsigned int stamp = static_cast<unsigned int>(info->m_stamp);

	// check some debug stuff
	if ( index >= Constants_MaxMemBlocks ||
		 (m_mem_blocks[index].m_flags & MemBlockFlags_Used) != MemBlockFlags_Used || 
		 m_mem_blocks[index].m_block != p_block ||
		 m_mem_blocks[index].m_stamp != stamp || stamp >= static_cast<unsigned int>(m_current_stamp) )
	{
		static char buf[1024];
		char* buf_ptr = buf;

		std::sprintf(buf_ptr, "DESTRUCTOR\n"
						 "Block       : 0x%08X\n" 
						 "Destructor  : 0x%08X\n" 
						 "Index       : %d\n"
						 "Stamp       : %d\n"
						 "Cur. Stamp  : %d\n"
						 "Header Size : %d\n"
						 "Alloc Size  : %d\n",
						p_block, p_destructor,
						index, stamp, info->m_headerSize, 
						info->m_allocatedSize, m_current_stamp);

		buf_ptr = buf + std::strlen(buf);

		// OOB check to make sure this is within limits
		if (index < Constants_MaxMemBlocks)
		{
			std::sprintf(buf_ptr, "FAILED BLOCK\n"
							 "Block     : 0x%08X\n" 
							 "Stamp     : %d\n",
							 m_mem_blocks[index].m_block,
							 m_mem_blocks[index].m_stamp);
			buf_ptr = buf + std::strlen(buf);
		}

		int matchingBlocks = 0;
		// first find number of matching blocks
		for (int i = 0; i < Constants_MaxMemBlocks; ++i)
		{
			if (m_mem_blocks[i].m_block == p_block)
			{
				matchingBlocks++;
			}
		}

		bool found_block = false;

		// find correct block
		for (int i = 0; i < Constants_MaxMemBlocks; ++i)
		{
			if (m_mem_blocks[i].m_block == p_block && m_mem_blocks[i].m_stamp == stamp)
			{
				std::sprintf(buf_ptr, "FOUND BLOCK (%d)\n"
								 "Index     : %d\n" 
								 "Creator   : 0x%08X\n"
								 "Destructor: 0x%08X\n"
								 "Flags     : 0x%08X\n"
								 "Stamp     : %d\n"
								 "Size      : %d\n"
								 "Type      : %d\n",
								 matchingBlocks,
							     i,  
								 m_mem_blocks[i].m_creator,
								 m_mem_blocks[i].m_destructor,
								 m_mem_blocks[i].m_flags, m_mem_blocks[i].m_stamp, 
								 m_mem_blocks[i].m_size, m_mem_blocks[i].m_type);
				buf_ptr = buf + std::strlen(buf);

				found_block = true;
				break;
			}
		}

		// still not found? find matching block pointer
		if (found_block == false)
		{
			for (int i = 0; i < Constants_MaxMemBlocks; ++i)
			{
				if (m_mem_blocks[i].m_block == p_block)
				{
					std::sprintf(buf_ptr, "FOUND BLOCK (%d)\n"
									 "Index     : %d\n" 
									 "Creator   : 0x%08X\n"
									 "Destructor: 0x%08X\n"
									 "Flags     : 0x%08X\n"
									 "STAMP     : %d\n"
									 "Size      : %d\n"
									 "Type      : %d\n",
									 matchingBlocks,
									 i,  
									 m_mem_blocks[i].m_creator,
									 m_mem_blocks[i].m_destructor,
									 m_mem_blocks[i].m_flags, m_mem_blocks[i].m_stamp, 
									 m_mem_blocks[i].m_size, m_mem_blocks[i].m_type);
					buf_ptr = buf + std::strlen(buf);

					found_block = true;
					break;
				}
			}
		}

		// still not found? find matching stamp
		if (found_block == false)
		{
			for (int i = 0; i < Constants_MaxMemBlocks; ++i)
			{
				if (m_mem_blocks[i].m_stamp == stamp)
				{
					std::sprintf(buf_ptr, "FOUND MATCHING STAMP\n"
									 "BLOCK     : 0x%08X\n"
									 "Index     : %d\n" 
									 "Creator   : 0x%08X\n"
									 "Destructor: 0x%08X\n"
									 "Flags     : 0x%08X\n"
									 "Size      : %d\n"
									 "Type      : %d\n",
									 m_mem_blocks[i].m_block,
									 i,  
									 m_mem_blocks[i].m_creator,
									 m_mem_blocks[i].m_destructor,
									 m_mem_blocks[i].m_flags, 
									 m_mem_blocks[i].m_size, 
									 m_mem_blocks[i].m_type);
					buf_ptr = buf + std::strlen(buf);

					found_block = true;
					break;
				}
			}
		}

		// still not found? report search failure
		if (found_block == false)
		{
			std::sprintf(buf_ptr, "\nNo matching block/stamp found in array. Try increasing Constants_MaxMemBlocks.");
			buf_ptr = buf + std::strlen(buf);
		}
		
		// should be first to prevent array OOB
		TT_ASSERTMSG(index < Constants_MaxMemBlocks, 
			"Index out of bounds.\nProbably cause of writing outside array boundary.\n"
			"Additional info:\n\n%s", buf);

		TT_ASSERTMSG((m_mem_blocks[index].m_flags & MemBlockFlags_Used) == MemBlockFlags_Used,
			"Block at index not in use.\nProbably cause of writing outside array or double deletion.\n"
			"Additional info:\n\n%s", buf);

		TT_ASSERTMSG(m_mem_blocks[index].m_block == p_block,
			"Invalid block at index.\nProbably cause of writing outside array or double deletion.\n"
			"Additional info:\n\n%s", buf);

		TT_ASSERTMSG(m_mem_blocks[index].m_stamp == stamp,
			"Invalid stamp at index.\nProbably cause of writing outside array or double deletion.\n"
			"Additional info:\n\n%s", buf);

		TT_ASSERTMSG(stamp < static_cast<unsigned int>(m_current_stamp),
			"Stamp out of bounds.\nProbably cause of writing outside array boundary..\n"
			"Additional info:\n\n%s", buf);

		TT_PANIC("Should not get here\n");
	}

	// 'clear' block
	m_mem_blocks[index].m_flags &= ~(MemBlockFlags_Used);

	// set destroyer (can be useful to trace double deletion)
	m_mem_blocks[index].m_destructor = p_destructor;

	//OS_RestoreInterrupts(mode);
	//OSRestoreInterrupts(mode);
}

void  MemoryProfiler::checkProfiler()
{
	TT_MEMORY_PROFILER_PRINTF("*** MemoryProfiler::checkProfiler ***\n");
	for(int i=0; i<Constants_MaxMemBlocks; ++i)
	{
		if ((m_mem_blocks[i].m_flags & MemBlockFlags_Used) == MemBlockFlags_Used)
		{
			void* block = m_mem_blocks[i].m_block;
			// verify whether info in block matches the info stored in this array
			DebugHeader* header = getDebugHeader(block, -1);

#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
			if (header->m_guardBytes != LOWER_GUARD_PATTERN)
			{
				TT_WARN("Lower Guard Bytes Failed! Block 0x%X\n", 
					block);

				printMemBlock(block);

				TT_PANIC("Lower Guard Bytes Failed!\n");
			}

			if ((*getUpperGuardBytes(block)) != UPPER_GUARD_PATTERN)
			{
				TT_WARN("Upper Guard Bytes Failed! Block 0x%X\n", 
					block);

				printMemBlock(block);

				TT_PANIC("Upper Guard Bytes Failed!\n");
			}
#endif

			if (header->m_index != i || 
				header->m_stamp != m_mem_blocks[i].m_stamp ||
				header->m_allocatedSize != m_mem_blocks[i].m_size)
			{
				TT_WARN("FAILURE IN BLOCK\n");
				printMemBlock(block);
			}
		}
	}
}

void MemoryProfiler::printMemBlock(void* p_block)
{
	// FIXME: platform independent mechanism to disable interrupts
	//	OSIntrMode mode = OS_DisableInterrupts();
	//BOOL mode = OSDisableInterrupts();

	DebugHeader* info = getDebugHeader(p_block, -1);

	TT_MEMORY_PROFILER_PRINTF("== DEBUG INFO ==\n"
			 "Block       : 0x%08X\n"
			 "Index       : %d\n"
			 "Stamp       : %d\n"
#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
			 "Lower Guard : 0x%X\n"
			 "Upper Guard : 0x%X\n"
#endif
			 "Alloc Size  : %d\n"
			 "Header Size : %d\n",
			p_block,
			info->m_index,
			info->m_stamp,
#ifdef MEMORY_PROFILER_ENABLE_GUARD_BYTES
			info->m_guardBytes,
			*getUpperGuardBytes(p_block),
#endif
			info->m_allocatedSize,
			info->m_headerSize);

	TT_ASSERT( m_mem_blocks[info->m_index].m_block == p_block );

	TT_MEMORY_PROFILER_PRINTF("== MEMBLOCK INFO ==\n"
			 "Size        : %d\n"
			 "Type        : %d\n"
			 "Stamp       : %d\n"
			 "Creator     : 0x%08X\n" 
			 "Destructor  : 0x%08X\n"
			 "Alignment   : %d\n"
			 "Flags       : 0x%08X\n",
			m_mem_blocks[info->m_index].m_size,
			m_mem_blocks[info->m_index].m_type,
			m_mem_blocks[info->m_index].m_stamp,
			m_mem_blocks[info->m_index].m_creator,
			m_mem_blocks[info->m_index].m_destructor,
			m_mem_blocks[info->m_index].m_alignment,
			m_mem_blocks[info->m_index].m_flags);

	//OS_RestoreInterrupts(mode);
	//OSRestoreInterrupts(mode);
}


void MemoryProfiler::startMeasure()
{
	m_start_stamp = m_current_stamp;

	m_current_run++;
	TT_MEMORY_PROFILER_PRINTF("\n************* MEMORY PROFILER: START MEASURE (RUN: %3d) ************\n\n", m_current_run);
}


void MemoryProfiler::stopMeasure( bool p_assert )
{
	// make sure we got a real snapshot of the memory at that time
	// FIXME: platform independent mechanism to disable interrupts
	//	OSIntrMode mode = OS_DisableInterrupts();
	//BOOL mode = OSDisableInterrupts();

	int failed_blocks = 0;
	size_t mem_leaked = 0;

	TT_MEMORY_PROFILER_PRINTF("\n************* MEMORY PROFILER: STOP MEASURE  (RUN: %3d) ************\n", m_current_run);

	TT_MEMORY_PROFILER_PRINTF("Start stamp   : %d\n", m_start_stamp);
	TT_MEMORY_PROFILER_PRINTF("Current stamp : %d (+ %d new calls)\n\n", m_current_stamp, 
		m_current_stamp-m_start_stamp);

	TT_ASSERTMSG(m_start_stamp != -1, "startMeasure was not called.");

	// copy into temporary array as we can't sort the original
	for(int i=0; i < Constants_MaxMemBlocks; ++i)
	{
		m_mem_blocks_sorted[i] = &m_mem_blocks[i];
	}

	// sort the temporary array
	std::sort(m_mem_blocks_sorted, m_mem_blocks_sorted + Constants_MaxMemBlocks, MemBlockComp());

	// all memblocks with count >= p_reference_point should be NULL
	for(int i=0; i < Constants_MaxMemBlocks; ++i)
	{
		if (m_mem_blocks_sorted[i]->m_stamp >= static_cast<u32>(m_start_stamp) && 
			m_mem_blocks_sorted[i]->m_flags & MemBlockFlags_Used)
		{
			TT_MEMORY_PROFILER_PRINTF("--> FAILED: 0x%08X (Creator: 0x%08X Destructor: 0x%08X), Type: %d, Size: %6d, Stamp: %6d (%d)\n",
				m_mem_blocks_sorted[i]->m_block,
				m_mem_blocks_sorted[i]->m_creator,
				m_mem_blocks_sorted[i]->m_destructor,
				m_mem_blocks_sorted[i]->m_type,
				m_mem_blocks_sorted[i]->m_size,
				m_mem_blocks_sorted[i]->m_stamp-m_start_stamp,
				m_mem_blocks_sorted[i]->m_stamp);
			failed_blocks++;
			mem_leaked += m_mem_blocks_sorted[i]->m_size;
		}
	}
	
	TT_MEMORY_PROFILER_PRINTF("\nBlocks Failed : %d\n", failed_blocks);
	TT_MEMORY_PROFILER_PRINTF("Memory Leaked : %d bytes\n", mem_leaked);
	size_t totalFree = mem::Heap::getTotalFreeSize();
	size_t largestFree = mem::Heap::getLargestFreeSize();

	TT_MEMORY_PROFILER_PRINTF("\nMemory Statistics\n");
	TT_MEMORY_PROFILER_PRINTF("Memory free   : %d bytes\n", totalFree);
	TT_MEMORY_PROFILER_PRINTF("Largest block : %d bytes\n", largestFree);

	// multiply by 1000 for percentage computation ( will error if > 4 MB )
	if (totalFree > 0)
	{
		size_t percentage = ((totalFree-largestFree)*1000) / totalFree;

		TT_MEMORY_PROFILER_PRINTF("Fragmentation : %d.%d%%\n\n", percentage/10, percentage%10) ;
	}

	if ( failed_blocks > 0 && p_assert )
	{
		TT_PANIC("Memory Profiler Detected %d Leaks", failed_blocks);
	}

	// reinit
	m_start_stamp = -1;

	//OS_RestoreInterrupts( mode );
	//OSRestoreInterrupts(mode);
}

// dump all
void MemoryProfiler::dump(DumpSettings p_settings)
{
	// make sure we got a real snapshot of the memory at that time
	// FIXME: platform independent mechanism to disable interrupts
	//	OSIntrMode mode = OS_DisableInterrupts();
	//BOOL mode = OSDisableInterrupts();

	if (p_settings != DumpSettings_ShowStatsOnlyNoHeader)
	{
		TT_MEMORY_PROFILER_PRINTF("\n************* MEMORY PROFILER: DUMP MEMORY   (RUN: %3d) ************\n", m_current_run);
	}

	updateHeapStats();


	HeapStats totalStats = {0};

	// add static class size to debugOverhead
	totalStats.debugOverhead += sizeof(m_mem_blocks) + sizeof(m_mem_blocks_sorted);
	size_t totalHeapSize = 0;
	size_t totalFreeSize = 0;
	size_t totalLargestFreeSize = 0;

	for (int i = 0; i < Constants_NumberOfHeaps; i++)
	{
		const HeapStats& stats = getHeapStats(i);
		size_t heapSize = mem::Heap::getSize(i);
		size_t freeSize = mem::Heap::getTotalFreeSize(i);
		size_t largestFreeSize = mem::Heap::getLargestFreeSize(i);

		TT_MEMORY_PROFILER_PRINTF("*********** HEAP %d ***********\n", i);
		TT_MEMORY_PROFILER_PRINTF("Heap Size           : %d (%dk)\n", heapSize, heapSize / 1024);
		TT_MEMORY_PROFILER_PRINTF("Heap Free           : %d (%dk)\n", freeSize, freeSize / 1024);
		TT_MEMORY_PROFILER_PRINTF("Largest Free        : %d (%dk)\n\n", largestFreeSize, largestFreeSize / 1024);

		TT_MEMORY_PROFILER_PRINTF("-- PROFILER STATS --\n");
		TT_MEMORY_PROFILER_PRINTF("- Allocated         : %d (%dk)\n", stats.allocated, stats.allocated / 1024);
		TT_MEMORY_PROFILER_PRINTF("- Largest Allocated : %d (%dk)\n", stats.largestAllocated, stats.largestAllocated / 1024);
		TT_MEMORY_PROFILER_PRINTF("- Blocks In Use     : %d\n", stats.nblocks);
		TT_MEMORY_PROFILER_PRINTF("- Debug Overhead    : %d", stats.debugOverhead);

		// output MemoryProfiler overhead percentage
		if (stats.allocated > 0)
		{
			size_t overhead = (stats.debugOverhead*1000) / stats.allocated;
			TT_MEMORY_PROFILER_PRINTF(" (%d.%d%%)", overhead / 10, overhead % 10);
		}
		TT_MEMORY_PROFILER_PRINTF("\n");

		// update totals
		totalStats.allocated += stats.allocated;
		if (stats.largestAllocated > totalStats.largestAllocated)
		{
			totalStats.largestAllocated = stats.largestAllocated;
		}
		totalStats.debugOverhead += stats.debugOverhead;
		totalStats.nblocks += stats.nblocks;

		totalHeapSize += heapSize;
		totalFreeSize += freeSize;
		if (freeSize > totalLargestFreeSize)
		{
			totalLargestFreeSize = freeSize;
		}
	}

	TT_MEMORY_PROFILER_PRINTF("********** ALL HEAPS **********\n");
	TT_MEMORY_PROFILER_PRINTF("Heap Size           : %d (%dk)\n", totalHeapSize, totalHeapSize / 1024);
	TT_MEMORY_PROFILER_PRINTF("Heap Free           : %d (%dk)\n", totalFreeSize, totalFreeSize / 1024);
	TT_MEMORY_PROFILER_PRINTF("Largest Free        : %d (%dk)\n\n", totalLargestFreeSize, totalLargestFreeSize / 1024);

	TT_MEMORY_PROFILER_PRINTF("-- PROFILER STATS --\n");
	TT_MEMORY_PROFILER_PRINTF("- Allocated         : %d (%dk)\n", totalStats.allocated, totalStats.allocated / 1024);
	TT_MEMORY_PROFILER_PRINTF("- Largest Allocated : %d (%dk)\n", totalStats.largestAllocated, totalStats.largestAllocated / 1024);
	TT_MEMORY_PROFILER_PRINTF("- Blocks In Use     : %d\n", totalStats.nblocks);
	TT_MEMORY_PROFILER_PRINTF("- Debug Overhead    : %d", totalStats.debugOverhead);

	if (totalStats.allocated > 0)
	{
		size_t overhead = (totalStats.debugOverhead*1000) / totalStats.allocated;
		TT_MEMORY_PROFILER_PRINTF(" (%d.%d%%)", overhead / 10, overhead % 10);
	}
	TT_MEMORY_PROFILER_PRINTF("\n");

	if (p_settings != DumpSettings_ShowStatsOnlyNoHeader)
	{
		TT_MEMORY_PROFILER_PRINTF("\n************* MEMORY PROFILER: END DUMP MEMORY           ************\n\n", m_current_run);
	}

	if (p_settings != DumpSettings_ShowStatsOnly && 
		p_settings != DumpSettings_ShowStatsOnlyNoHeader)
	{
		for(int i=0; i < Constants_MaxMemBlocks; ++i)
		{
			if (m_start_stamp < 0)
			{
				TT_MEMORY_PROFILER_PRINTF("%4d. 0x%08X (Creator: 0x%08X Destructor: 0x%08X), Type: %d, Size: %6d, Stamp: %s (%d)\n",
					i,
					m_mem_blocks[i].m_block,
					m_mem_blocks[i].m_creator,
					m_mem_blocks[i].m_destructor,
					m_mem_blocks[i].m_type,
					m_mem_blocks[i].m_size,
					" **** ",
					m_mem_blocks[i].m_stamp);
			}
			else
			{
				// check if we should show only the new ones
				if (static_cast<int>(m_mem_blocks[i].m_stamp)-m_start_stamp >= 0 || 
					p_settings == DumpSettings_AllAllocs)
				{
					TT_MEMORY_PROFILER_PRINTF("%4d. 0x%08X (Creator: 0x%08X), Type: %d, Size: %6d, Stamp: %6d (%d)\n",
						i,
						m_mem_blocks[i].m_block,
						m_mem_blocks[i].m_creator,
						m_mem_blocks[i].m_type,
						m_mem_blocks[i].m_size,
						m_mem_blocks[i].m_stamp,
						m_mem_blocks[i].m_stamp-m_start_stamp);
				}
			}
		}
	}

//	OS_RestoreInterrupts( mode );
	//OSRestoreInterrupts(mode);
}


// namespace
}
}

#endif // #ifdef PROFILE_MEMORY
