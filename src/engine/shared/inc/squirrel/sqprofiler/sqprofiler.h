/*	see copyright notice in squirrel.h */
#ifndef _SQPROFILER_H
#define _SQPROFILER_H

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API void sq_registerscriptprofiler(HSQUIRRELVM v);
SQUIRREL_API SQInteger sq_startprofiler(HSQUIRRELVM v);
SQUIRREL_API SQInteger sq_stopprofiler(HSQUIRRELVM v);
SQUIRREL_API SQInteger sq_updateprofiler(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _SQPROFILER_H
