#if !defined(INC_TT_THREAD_MESSAGEQUEUE_H)
#define INC_TT_THREAD_MESSAGEQUEUE_H

/*
	MessageQueue class, provides messaging system between threads.
	Sample code below class declaration.
*/

#include <tt/thread/types.h>


namespace tt {
namespace thread {

typedef void* Message;

class MessageQueue
{
public:
	/*! \brief Initializes a message queue object.
	    \param p_size Amount of messages that can be stored in the queue.*/
	MessageQueue(size_type p_size = 32);
	
	/*! \brief Destroys a message queue object.*/
	~MessageQueue();
	
	/*! \brief Inserts a message at the end of the queue.
	    \param p_message The message to insert in the queue.
	    \param p_block Whether to block when the queue is full.
	    \return True when the message was inserted,
	            false when block was false and the queue is full*/
	bool send(Message p_message, bool p_block);
	
	/*! \brief Inserts a message at the beginning of the queue.
	    \param p_message The message to insert in the queue.
	    \param p_block Whether to block when the queue is full.
	    \return True when the message was inserted,
	            false when block was false and the queue is full*/
	bool jam(Message p_message, bool p_block);
	
	/*! \brief Removed a message from the queue.
	    \param p_messageOUT Variable to hold the received message.
	    \param p_block Whether to block when the queue is empty.
	    \return True when a message was received,
	            false when block was false and the queue is empty*/
	bool receive(Message& p_messageOUT, bool p_block);
	
private:
	MessageQueue(const MessageQueue&);
	const MessageQueue& operator=(const MessageQueue&);
	
	struct InternalData;
	
	InternalData* m_queue;
	Message*      m_messages;
};

// namespace end
}
}

/*
	Sample.
	Consumer/Producer problem
	
	static MessageQueue s_queue;
	
	void producerThread()
	{
		for (;;)
		{
			s_queue.send(Message(), true); // blocks when queue is full
		}
	}
	
	void consumerThread()
	{
		for (;;)
		{
			Message msg
			s_queue.get(msg, true); // blocks when queue is empty
		}
	}
*/

#endif // !defined(INC_TT_THREAD_MESSAGEQUEUE_H)
