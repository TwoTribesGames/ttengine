#if !defined(INC_TT_COM_MESSAGE_H)
#define INC_TT_COM_MESSAGE_H

#include <tt/com/types.h>


namespace tt {
namespace com {


enum MessageResult
{
	MessageResult_False,
	MessageResult_True,
	MessageResult_Unknown,
	MessageResult_Error
};


class MessageNoData{};


inline const MessageNoData* getMessageNoDataPtr()
{
	static MessageNoData msgNoData;
	return &msgNoData;
}


class Message
{
public:
	template <class MessageTrait>
	explicit Message(MessageTrait p_MessageTrait,
	                 typename MessageTrait::DataPtr p_data = getMessageNoDataPtr())
	: m_id(p_MessageTrait.id), m_data(reinterpret_cast<const void*>(p_data)) { }
	
	Message() : m_id(MessageID::invalid), m_data(0) { }
	
	inline MessageID getID()   const { return m_id;   }
	
private:
	inline const void* getData() const
	{
		return m_data;
	}
	
	MessageID   m_id;
	const void* m_data;
	
	friend class MessageTraitBase;
};


class MessageTraitBase
{
protected:
	MessageTraitBase() {}
	~MessageTraitBase() {}
	
	inline const void* getPrivateData(Message p_message) const
	{
		return p_message.getData();
	}
};


template <typename MessageData = MessageNoData>
class MessageTrait : private MessageTraitBase
{
public:
	typedef MessageData DataType;
	typedef const MessageData* DataPtr;
	
	MessageID id;
	
	inline DataPtr getData(Message p_message) const
	{
		TT_ASSERT(p_message.getID() == id);
		return reinterpret_cast<DataPtr>(getPrivateData(p_message));
	}
	
	inline DataType getDataByValue(Message p_message) const
	{
		TT_ASSERT(getData(p_message) != 0);
		return *getData(p_message);
	}
	
	// Provide a default constructor, so that static const objects of this class can be instantiated
	MessageTrait() : MessageTraitBase() { }
};


// Namespace end
}
}


#endif  // !defined(INC_TT_COM_MESSAGE_H)
