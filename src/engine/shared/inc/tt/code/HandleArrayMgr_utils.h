#if !defined(INC_TT_CODE_HANDLEARRAYMGRUTILS_H)
#define INC_TT_CODE_HANDLEARRAYMGRUTILS_H


#include <tt/code/bufferutils_get.h>
#include <tt/code/bufferutils_put.h>
#include <tt/code/HandleArrayMgr.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace code {

template<typename T>
void serializeHandleArrayMgr(const HandleArrayMgr<T>& p_mgr, BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	const T*  object      = p_mgr.getFirst();
	const s32 activeCount = p_mgr.getActiveCount();
	bufferutils::put(activeCount, p_context);
	
	for (s32 i = 0; i < activeCount; ++i, ++object)
	{
		bufferutils::putHandle(object->getHandle(), p_context);
		object->serializeCreationParams(p_context);
		object->serialize(p_context);
	}
}


template<typename T>
void unserializeHandleArrayMgr(HandleArrayMgr<T>* p_mgr_OUT, BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_mgr_OUT);
	TT_NULL_ASSERT(p_context);
	
	p_mgr_OUT->reset();
	
	const s32 objectCount = bufferutils::get<s32>(p_context);
	
	for (s32 i = 0; i < objectCount; ++i)
	{
		Handle<T> handle = bufferutils::getHandle<T>(p_context);
		typename T::CreationParams createParams(T::unserializeCreationParams(p_context));
		
		T* object = p_mgr_OUT->createWithSpecificHandle(createParams, handle);
		if (object == 0)
		{
			TT_PANIC("Could not create HandleArrayMgr object for handle 0x%08X. "
			         "Unserialization cannot continue.", handle.getValue());
			break;
		}
		object->unserialize(p_context);
	}
	p_mgr_OUT->doInternalCleanup();
}

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_HANDLEARRAYMGRUTILS_H)
