#include <tt/platform/tt_error.h>
#include <tt/thread/ConditionVariable.h>
#include <tt/thread/MessageQueue.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace thread {

struct MessageQueue::InternalData
{
	ConditionVariable notEmpty; //!< To hold threads calling receive
	ConditionVariable notFull;  //!< To hold threads calling send and jam
	Mutex             mutex;    //!< Protects internal queue
	size_type         size;     //!< Size of queue
	size_type         first;    //!< Index of first message in queue
	size_type         used;     //!< Number of messages in queue
	
	inline InternalData()
	:
	notEmpty(),
	notFull(),
	mutex(),
	size(0),
	first(0),
	used(0)
	{ }
};


//--------------------------------------------------------------------------------------------------
// Public member functions

MessageQueue::MessageQueue(size_type p_size)
:
m_queue(new InternalData),
m_messages(new Message[p_size])
{
	TT_ASSERTMSG(m_queue != 0, "Failed to create internal data.");
	TT_ASSERTMSG(m_messages != 0, "Failed to create internal message list.");
	
	m_queue->size  = p_size;
	m_queue->first = 0;
	m_queue->used  = 0;
}


MessageQueue::~MessageQueue()
{
	TT_ASSERTMSG(m_queue->used == 0, "Message queue not empty!");
	
	delete[] m_messages;
	delete   m_queue;
}


bool MessageQueue::send(Message p_message, bool p_block)
{
	m_queue->mutex.lock();
	while (m_queue->used == m_queue->size)
	{
		// queue full
		if (p_block)
		{
			m_queue->notFull.sleep(&m_queue->mutex);
		}
		else
		{
			m_queue->mutex.unlock();
			return false;
		}
	}
	
	// insert message at end of queue
	size_type index = (m_queue->first + m_queue->used) % m_queue->size;
	m_messages[index] = p_message;
	++m_queue->used;
	
	m_queue->mutex.unlock();
	
	m_queue->notEmpty.wake();
	
	return true;
}


bool MessageQueue::jam(Message p_message, bool p_block)
{
	m_queue->mutex.lock();
	while (m_queue->used == m_queue->size)
	{
		// queue full
		if (p_block)
		{
			m_queue->notFull.sleep(&m_queue->mutex);
		}
		else
		{
			m_queue->mutex.unlock();
			return false;
		}
	}
	
	// insert message before beginning of queue
	size_type index = m_queue->first;
	if (index == 0)
	{
		index = m_queue->size - 1;
	}
	else
	{
		--index;
	}
	m_messages[index] = p_message;
	++m_queue->used;
	m_queue->first = index;
	
	m_queue->mutex.unlock();
	
	m_queue->notEmpty.wake();
	
	return true;
}


bool MessageQueue::receive(Message& p_message, bool p_block)
{
	m_queue->mutex.lock();
	while (m_queue->used == 0)
	{
		// queue empty
		if (p_block)
		{
			m_queue->notEmpty.sleep(&m_queue->mutex);
		}
		else
		{
			m_queue->mutex.unlock();
			return false;
		}
	}
	
	// remove message from front of queue
	size_type index = m_queue->first;
	p_message = m_messages[index];
	
	++index;
	if (index == m_queue->size)
	{
		index = 0;
	}
	--m_queue->used;
	m_queue->first = index;
	
	m_queue->mutex.unlock();
	
	m_queue->notFull.wake();
	
	return true;
}


// ------------------------------------------------------------
// Private functions


// namespace end
}
}
