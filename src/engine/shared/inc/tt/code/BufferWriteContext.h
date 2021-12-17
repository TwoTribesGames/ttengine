#if !defined(INC_TT_CODE_BUFFERWRITECONTEXT_H)
#define INC_TT_CODE_BUFFERWRITECONTEXT_H


#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

struct BufferWriteContext
{
	typedef s32        StatusCode;
	typedef StatusCode (*RefillFunc)(BufferWriteContext*);
	
	
	u8*        start;       //!< Start of buffer.
	u8*        end;         //!< (One past) end of buffer (i.e. bufferSize = end - start).
	u8*        cursor;      //!< Cursor in buffer.
	StatusCode statusCode;  //!< The status of the buffer (e.g. whether an error occurred; status 0 is "no error").
	RefillFunc flushFunc;   //!< Call when done writing data.
	RefillFunc refillFunc;  //!< Function to call for refilling the buffer
	void*      userContext; //!< Implementation-defined context.
	
	
	inline BufferWriteContext()
	:
	start(0),
	end(0),
	cursor(0),
	statusCode(0),
	flushFunc(0),
	refillFunc(0),
	userContext(0)
	{ }
	
	/*! \brief Creates a BufferWriteContext to write to a fixed-size raw byte buffer.
	           The returned context does not support refilling (will panic if refill is requested). */
	static inline BufferWriteContext createForRawBuffer(u8* p_buffer, size_t p_bufferSizeInBytes)
	{
		BufferWriteContext context;
		
		context.start       = p_buffer;
		context.end         = p_buffer + p_bufferSizeInBytes;
		context.cursor      = context.start;
		context.statusCode  = 0;
		context.flushFunc   = dummyFlushFunc;
		context.refillFunc  = dummyRefillFunc;
		context.userContext = 0;
		
		return context;
	}
	
	inline bool needsRefill() const { return cursor == end; }
	
	inline StatusCode flush()
	{
		TT_NULL_ASSERT(flushFunc);
		return (flushFunc == 0) ? 1 : flushFunc(this);
	}
	
	inline StatusCode refill()
	{
		TT_NULL_ASSERT(refillFunc);
		return (refillFunc == 0) ? 1 : refillFunc(this);
	}
	
private:
	static inline StatusCode dummyFlushFunc(BufferWriteContext* /*p_context*/)
	{
		return 0;
	}
	
	static inline StatusCode dummyRefillFunc(BufferWriteContext* p_context)
	{
		TT_PANIC("Reached end of fixed-size write buffer (at address %p, size is %d bytes). "
		         "Cannot write any more data.", p_context->start, s32(p_context->end - p_context->start));
		return 1;
	}
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_BUFFERWRITECONTEXT_H)
