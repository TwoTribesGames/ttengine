#ifndef TT_BUILD_FINAL

#include <tt/math/math.h>
#include <tt/mem/mem.h>
#include <tt/mem/profiler.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

namespace tt {
namespace mem {
namespace profiler {


// ------------------------------------------------------------
// Internal types

struct PreBlock
{
	size_type size;      //!< Requested size passed to alloc()
	size_type alignment; //!< Alignment
	void*     creator;   //!< Creator address
	s32       type;      //!< Allocation type
	
	size_type run;       //!< Run at which block was allocated
	size_type tag;       //!< Allocation tag
	size_type runtag;    //!< Allocation tag of specific run
	
	size_type guard;     //!< Guard bytes
};


struct PostBlock
{
	size_type guard; //!< Guard bytes
};


struct DebugBlock
{
	PreBlock* pre;    //!< Pre block
	size_type tag;    //!< Allocation tag
	size_type run;    //!< Run
	size_type flags;  //!< bit 0: allocated, bit 1: free
};


enum
{
	Magic_Free  = 0xFEEEFEEE, //!< Value of freed memory.
	Magic_Fence = 0xFDFDFDFD, //!< Value of guard bytes.
	Magic_Alloc = 0xCDCDCDCD  //!< Value of allocated memory.
};


enum
{
	Max_Runs = 16,         //!< Maximum number of nested runs
	Max_Blocks = 64 * 1024 //!< Maximum number of debug blocks
};

enum
{
	Flag_Allocated = 0x00000001, //!< Memory was allocated.
	Flag_Freed     = 0x00000002  //!< Memory was freed.
};


// ------------------------------------------------------------
// Internal variables

static bool s_leak;  //!< Whether leak detection is on or not.
static bool s_guard; //!< Whether guard bytes are enabled or not.
static bool s_clear; //!< Whether pre alloc and post free clearing is enabled or not.

static size_type s_run; //!< Current run.
static size_type s_tag; //!< Current tag.

static size_type s_startTags[Max_Runs]; //!< Start tag for each run
static size_type s_breakTags[Max_Runs]; //!< Tags to break at for each run

static DebugBlock s_blocks[Max_Blocks]; //!< Allocation information
static size_type  s_lastBlock;          //!< Last used block


// ------------------------------------------------------------
// Internal functions

static void dumpBlock(void* p_preBlock,  size_type p_preSize,
                      void* p_userBlock, size_type p_userSize,
                      void* p_postBlock, size_type p_postSize)
{
	TT_Printf("pre:  %p (%d bytes)\n", p_preBlock,  p_preSize);
	TT_Printf("user: %p (%d bytes)\n", p_userBlock, p_userSize);
	TT_Printf("post: %p (%d bytes)\n", p_postBlock, p_postSize);
	
	PreBlock* pre = reinterpret_cast<PreBlock*>(p_preBlock);
	TT_Printf("size: %u\n", pre->size);
	TT_Printf("alignment: %u\n", pre->alignment);
	TT_Printf("creator: %p\n", pre->creator);
	TT_Printf("type: %d\n", pre->type);
	TT_Printf("run: %u\n", pre->run);
	TT_Printf("tag: %u\n", pre->tag);
	TT_Printf("runtag: %u\n", pre->runtag);
	TT_Printf("lower guard: 0x%08X\n", pre->guard);
	
	PostBlock* post = reinterpret_cast<PostBlock*>(p_postBlock);
	TT_Printf("upper guard: 0x%08X\n\n", post->guard);
}


static void triggerBreakPoint()
{
#ifdef TT_PLATFORM_WIN
	asm { bkpt 0 }
#else
	TT_PANIC("No asm breakpoint supported on this platform");
#endif
}


static void allocCallback(size_type p_size,
                          size_type p_alignment,
                          void*     p_creator,
                          s32       p_type,
                          void*     p_preBlock,  size_type p_preSize,
                          void*     p_userBlock, size_type p_userSize,
                          void*     p_postBlock, size_type p_postSize)
{
	TT_ASSERT(reinterpret_cast<size_type>(p_userBlock) ==
	          (reinterpret_cast<size_type>(p_preBlock) + p_preSize));
	TT_ASSERT(reinterpret_cast<size_type>(p_postBlock) ==
	          (reinterpret_cast<size_type>(p_userBlock) + p_userSize));
	
	if (s_breakTags[s_run] != 0 &&
	    (s_breakTags[s_run] + s_startTags[s_run] == s_tag ||
	     s_breakTags[0] == s_tag))
	{
		triggerBreakPoint();
	}
	TT_ASSERT(p_preSize  >= sizeof(PreBlock));
	TT_ASSERT(p_postSize >= sizeof(PostBlock));
	TT_ASSERT(p_userSize >= p_size);
	
	PreBlock* pre = reinterpret_cast<PreBlock*>(p_preBlock);
	pre->size = p_size;
	pre->alignment = p_alignment;
	pre->creator = p_creator;
	pre->type = p_type;
	
	pre->run = s_run;
	pre->tag = s_tag;
	pre->runtag = s_tag - s_startTags[s_run];
	
	pre->guard = Magic_Fence;
	
	PostBlock* post = reinterpret_cast<PostBlock*>(p_postBlock);
	post->guard = Magic_Fence;
	
	if (s_clear)
	{
		mem::fill32(p_userBlock, Magic_Alloc, p_userSize);
	}
	
	// create an allocation block
	DebugBlock* block = 0;
	for (size_type index = s_lastBlock; block == 0 && index < Max_Blocks; ++index)
	{
		if (s_blocks[index].flags == 0)
		{
			block = &s_blocks[index];
			s_lastBlock = index;
		}
	}
	
	for (size_type index = 0; block == 0 && index < s_lastBlock; ++index)
	{
		if (s_blocks[index].flags == 0)
		{
			block = &s_blocks[index];
			s_lastBlock = index;
		}
	}
	
	TT_ASSERT(block != 0);
	block->pre   = pre;
	block->tag   = s_tag;
	block->run   = s_run;
	block->flags = Flag_Allocated;
	
	++s_tag;
}


static void freeCallback(void* p_destroyer,
                         void* p_preBlock,  size_type p_preSize,
                         void* p_userBlock, size_type p_userSize,
                         void* p_postBlock, size_type p_postSize)
{
	(void)p_destroyer;
	TT_ASSERT(p_preSize  >= sizeof(PreBlock));
	TT_ASSERT(p_postSize >= sizeof(PostBlock));
	
	PreBlock* pre = reinterpret_cast<PreBlock*>(p_preBlock);
	if (s_guard && pre->guard != Magic_Fence)
	{
		TT_WARN("Block %p lower guard bytes violated (0x%08X)",
			p_userBlock, pre->guard);
		dumpBlock(p_preBlock, p_preSize, p_userBlock, p_userSize, p_postBlock, p_postSize);
		triggerBreakPoint();
	}
	
	PostBlock* post = reinterpret_cast<PostBlock*>(p_postBlock);
	if (s_guard && post->guard != Magic_Fence)
	{
		TT_WARN("Block %p upper guard bytes violated (0x%08X)",
			p_userBlock, post->guard);
		dumpBlock(p_preBlock, p_preSize, p_userBlock, p_userSize, p_postBlock, p_postSize);
		triggerBreakPoint();
	}
	
	// find debug block
	DebugBlock* block = 0;
	for (size_type index = 0; block == 0 && index <= s_lastBlock; ++index)
	{
		if (s_blocks[s_lastBlock - index].pre == pre)
		{
			block = &s_blocks[s_lastBlock - index];
		}
	}
	for (size_type index = Max_Blocks - 1; block == 0 && index > s_lastBlock; --index)
	{
		if (s_blocks[index].pre == pre)
		{
			block = &s_blocks[s_lastBlock - index];
		}
	}
	TT_ASSERT(block != 0);
	
	TT_ASSERTMSG((block->flags & Flag_Freed) == 0, "Double deletion");
	block->flags = Flag_Freed;
	
	if (s_clear)
	{
		mem::fill32(p_userBlock, Magic_Free, p_userSize);
	}
}


// ------------------------------------------------------------
// Public functions

void init(bool p_leak, bool p_guard, bool p_clear)
{
	s_leak  = p_leak;
	s_guard = p_guard;
	s_clear = p_clear;
	
	s_run = 0;
	s_tag = 0;
	
	zero32(s_startTags, sizeof(s_startTags));
	zero32(s_breakTags, sizeof(s_breakTags));
	zero32(s_blocks, sizeof(s_blocks));
	s_lastBlock = 0;
	
	mem::setDebugCallbacks(sizeof(PreBlock), sizeof(PostBlock), allocCallback, freeCallback);
}


void start()
{
	TT_ASSERT(s_run < (Max_Runs - 1));
	++s_run;
	
	TT_Printf("tt::mem::profiler: Start run %u at tag %u\n", s_run, s_tag);
	
	s_startTags[s_run] = s_tag;
}


void stop(bool p_assert)
{
	TT_Printf("tt::mem::profiler: Stop run %u at tag %u (%u allocations)\n", s_run, s_tag, s_tag - s_startTags[s_run]);
	
	// see if there are any allocations left
	size_type blocks = 0;
	size_type bytes = 0;
	for (size_type i = 0; i < Max_Blocks; ++i)
	{
		if (s_blocks[i].run == s_run)
		{
			if ((s_blocks[i].flags & Flag_Allocated) != 0)
			{
				TT_Printf("0x%08X: %7u bytes by %p tag %6u run %2u runtag %5u\n",
					s_blocks[i].pre, s_blocks[i].pre->size, s_blocks[i].pre->creator, s_blocks[i].pre->tag,
					s_blocks[i].pre->run, s_blocks[i].pre->runtag);
				++blocks;
				bytes += s_blocks[i].pre->size;
			}
		}
	}
	if (blocks > 0)
	{
		TT_Printf("%u blocks leaked %u bytes\n", blocks, bytes);
		TT_ASSERT(p_assert == false);
	}
	
	TT_ASSERT(s_run > 0);
	--s_run;
}


void setBreakAlloc(size_type p_alloc, size_type p_run)
{
	TT_ASSERT(p_run < Max_Runs);
	s_breakTags[p_run] = p_alloc;
}


// namespace end
}
}
}

#endif // !defined (TT_BUILD_FINAL)
