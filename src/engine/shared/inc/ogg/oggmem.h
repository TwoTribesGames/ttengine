/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os_types.h 7524 2004-08-11 04:20:36Z conrad $

 ********************************************************************/
#ifndef _OGG_MEM_H
#define _OGG_MEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


extern void* oggMalloc(size_t p_size);
extern void* oggCalloc(size_t p_num, size_t p_size);
extern void* oggRealloc(void* p_block, size_t p_size);
extern void  oggFree(void* p_block);

#ifdef __cplusplus
}
#endif

#endif  /* _OGG_MEM_H */
