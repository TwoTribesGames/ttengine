/*
	see copyright notice in squirrel.h
*/
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>

#include "sqpcheader.h"

void *sq_vm_malloc(SQUnsignedInteger size)
{
	return operator new (size);
}

void *sq_vm_realloc(void* p, SQUnsignedInteger oldsize, SQUnsignedInteger size)
{
	if(oldsize >= size)
	{
		return p;
	}

	void* a = (void*)sq_vm_malloc(size);
	if(p != 0)
	{
		tt::mem::copy8(a, p, (size < oldsize) ? (size) : (oldsize));
	}
	sq_vm_free(p, oldsize);
	return a;
}

void sq_vm_free(void* p, SQUnsignedInteger /*size*/){ operator delete (p); }
