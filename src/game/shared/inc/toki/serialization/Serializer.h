#if !defined(INC_TOKI_SERIALIZATION_SERIALIZER_H)
#define INC_TOKI_SERIALIZATION_SERIALIZER_H


#include <tt/code/AutoGrowBuffer.h>
#include <tt/fs/types.h>

#include <toki/serialization/fwd.h>


namespace toki {
namespace serialization {

class Serializer
{
public:
	// FIXME: This shouldn't be a constructor, but some form of factory function.
	// Allow creation for writing (optionally with buffer sizes),
	// or creation from existing data to load from.
	Serializer();
	
	tt::code::BufferWriteContext getAppendContext();
	tt::code::BufferReadContext  getReadContext() const;
	
	u32 getSize() const;
	
	SerializerPtr clone() const;
	
	inline void clear() { m_buffer->clear(); }
	
	static SerializerPtr createFromBuffer(tt::code::BufferReadContext* p_context);
	bool saveToBuffer(tt::code::BufferWriteContext* p_context) const;
	
	// FIXME: Where should the data be kept? Is it enough to keep it internally,
	// so that lifetime of this object dictates lifetime of the serialized data?
	// Support for writing data to file is a good idea. Should we also allow access
	// to the internal buffer so that the data may be stored elsewhere in memory?
	
private:
	explicit Serializer(const tt::code::AutoGrowBufferPtr& p_buffer);
	Serializer(const Serializer& p_rhs);
	
	// No assignment
	Serializer& operator=(const Serializer&);
	
	
	static const tt::code::Buffer::size_type ms_defaultInitialSize;
	static const tt::code::Buffer::size_type ms_defaultGrowBy;
	
	tt::code::AutoGrowBufferPtr m_buffer;
	
	/*
	"Named sections" are one possible solution for the problem of needing to reference data
	that is not yet available or stored elsewhere. This data needs to be available to loading
	code before loading starts, but during saving it only becomes known/available after the
	main data has been serialized.
	It's not certain at which scope this data is meant to be stored: is it data for a specific
	system, all kept internally? Or does it cross system boundaries? E.g. Timer serialization
	can reference an Entity, which is stored in a completely separate serialization area.
	
	map<name, ByteBuffer> m_namedSections;
	*/
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_SERIALIZATION_SERIALIZER_H)
