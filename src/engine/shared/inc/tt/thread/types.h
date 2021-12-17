#if !defined(INC_TT_THREAD_TYPES_H)
#define INC_TT_THREAD_TYPES_H

#include <tt/platform/tt_types.h>


namespace tt {
namespace thread {

enum Affinity
{
	Affinity_None  = 0x0, // No affinity, run on all cores
	Affinity_Core0 = 0x1,
	Affinity_Core1 = 0x2,
	Affinity_Core2 = 0x4,
	Affinity_Core3 = 0x8,
	Affinity_Core4 = 0x10,
	Affinity_Core5 = 0x20,
	Affinity_Core6 = 0x40,
	Affinity_Core7 = 0x80,
};


struct Thread;                         //!< Internal data for thread handle
typedef tt_ptr<Thread>::shared handle; //!< Thread handle type

typedef int (*ThreadProc)(void*);      //!< Thread procedure type

typedef u32 size_type;

typedef int priority_type;             //!< Type used for thread priority

// Priority types
extern const priority_type priority_idle;
extern const priority_type priority_lowest;
extern const priority_type priority_below_normal;
extern const priority_type priority_normal;
extern const priority_type priority_above_normal;
extern const priority_type priority_highest;
extern const priority_type priority_realtime;


// namespace end
}
}

#endif // !defined(INC_TT_THREAD_TYPES_H)
