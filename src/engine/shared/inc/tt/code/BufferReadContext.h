#if !defined(INC_TT_CODE_BUFFERREADCONTEXT_H)
#define INC_TT_CODE_BUFFERREADCONTEXT_H


#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

struct BufferReadContext
{
	typedef s32        StatusCode;
	typedef StatusCode (*RefillFunc)(BufferReadContext*);
	
	
	const u8*  start;       //!< Start of buffer.
	const u8*  end;         //!< (One past) end of buffer (i.e. bufferSize = end - start).
	const u8*  cursor;      //!< Cursor in buffer.
	StatusCode statusCode;  //!< The status of the buffer (e.g. whether an error occurred; status 0 is "no error").
	RefillFunc refillFunc;  //!< Function to call for refilling the buffer
	void*      userContext; //!< Implementation-defined context.
	
	
	inline BufferReadContext()
	:
	start(0),
	end(0),
	cursor(0),
	statusCode(0),
	refillFunc(0),
	userContext(0)
	{ }
	
	inline bool needsRefill() const { return cursor == end; }
	
	inline StatusCode refill()
	{
		TT_NULL_ASSERT(refillFunc);
		return (refillFunc == 0) ? 1 : refillFunc(this);
	}
	
	
	/*! \brief Creates a BufferReadContext to read from a fixed-size raw byte buffer.
	           The returned context does not support refilling (will panic if refill is requested). */
	static inline BufferReadContext createForRawBuffer(const u8* p_buffer, size_t p_bufferSizeInBytes)
	{
		BufferReadContext context;
		
		context.start       = p_buffer;
		context.end         = p_buffer + p_bufferSizeInBytes;
		context.cursor      = context.start;
		context.statusCode  = 0;
		context.refillFunc  = dummyRefillFunc;
		context.userContext = 0;
		
		return context;
	}
	
private:
	static inline StatusCode dummyRefillFunc(BufferReadContext* p_context)
	{
		TT_PANIC("Reached end of fixed-size read buffer (at address %p, size is %d bytes). "
		         "Cannot read any more data.", p_context->start, s32(p_context->end - p_context->start));
		return 1;
	}
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_BUFFERREADCONTEXT_H)
