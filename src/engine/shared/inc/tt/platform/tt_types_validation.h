#ifndef INC_TT_PLATFORM_TT_TYPES_VALIDATION_H
#define INC_TT_PLATFORM_TT_TYPES_VALIDATION_H

#include <tt/platform/tt_compile_time_error.h>

TT_STATIC_ASSERT(sizeof(u8    ) == 1);
TT_STATIC_ASSERT(sizeof(u16   ) == 2);
TT_STATIC_ASSERT(sizeof(u32   ) == 4);
TT_STATIC_ASSERT(sizeof(u64   ) == 8);

TT_STATIC_ASSERT(sizeof(s8    ) == 1);
TT_STATIC_ASSERT(sizeof(s16   ) == 2);
TT_STATIC_ASSERT(sizeof(s32   ) == 4);
TT_STATIC_ASSERT(sizeof(s64   ) == 8);

TT_STATIC_ASSERT(sizeof(vu8   ) == 1);
TT_STATIC_ASSERT(sizeof(vu16  ) == 2);
TT_STATIC_ASSERT(sizeof(vu32  ) == 4);
TT_STATIC_ASSERT(sizeof(vu64  ) == 8);

TT_STATIC_ASSERT(sizeof(vs8   ) == 1);
TT_STATIC_ASSERT(sizeof(vs16  ) == 2);
TT_STATIC_ASSERT(sizeof(vs32  ) == 4);
TT_STATIC_ASSERT(sizeof(vs64  ) == 8);

TT_STATIC_ASSERT(sizeof(f32   ) == 4);
TT_STATIC_ASSERT(sizeof(vf32  ) == 4);

TT_STATIC_ASSERT(sizeof(real  ) == 4);
TT_STATIC_ASSERT(sizeof(real64) == 8);

TT_STATIC_ASSERT(sizeof(intptr) == sizeof(void*));
TT_STATIC_ASSERT(sizeof(uintptr) == sizeof(void*));

#endif // INC_TT_PLATFORM_TT_STATIC_PLATFORM_TESTS_H
