#ifndef MEMGZIO_H
#define MEMGZIO_H

/* gzio.c -- IO on .gz files
 * Copyright (C) 1995-2002 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Compile this file with -DNO_DEFLATE to avoid the compression code.
 */

/* memgzio.c - IO on .gz files in memory
 * Adapted from original gzio.c from zlib library by Forgotten
 */

#include <libz/zlib.h>

gzFile ZEXPORT memgzopen(char *memory, int available, const char *mode);
int ZEXPORT memgzread(gzFile file, voidp buf, unsigned len);
int ZEXPORT memgzwrite(gzFile file, const voidp buf, unsigned len);
int ZEXPORT memgzclose(gzFile file);
long ZEXPORT memtell(gzFile file);
z_off_t ZEXPORT memgzseek(gzFile file, z_off_t off, int whence);

// Newer version of zlib dropped gzio support
#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

#endif // MEMGZIO_H
