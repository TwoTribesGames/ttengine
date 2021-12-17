#include <errno.h>
#include <semaphore.h>
#include <unistd.h>

#include <tt/platform/tt_error.h>
#include <tt/str/str.h>
#include <tt/thread/Semaphore.h>


namespace tt {
namespace thread {

static s32 g_semaphoreCounter = 0;
static std::string g_semaphoreNamePrefix;

// ------------------------------------------------------------------------------------------------


struct Semaphore::InternalData
{
    sem_t* sem;
	
	inline InternalData()
	:
	sem(0)
	{ }
};


Semaphore::Semaphore(s32 p_initialCount)
:
m_data(new InternalData)
{
	const s32 maxCount = 0x7FFFFFFF;
	TT_MINMAX_ASSERT(p_initialCount, 0, maxCount);
	TT_MAX_ASSERT(p_initialCount, SEM_VALUE_MAX);
	
	const mode_t       openMode = 0664;
	const unsigned int initialValue = static_cast<unsigned int>(p_initialCount);
	
	bool nameIsTaken = false;
	do
	{
		++g_semaphoreCounter;
		std::string name(g_semaphoreNamePrefix + "/" + str::toStr(getpid()) + "_" + str::toStr(g_semaphoreCounter));
		TT_ASSERTMSG(name.length() <= 31, "Generated semaphore name '%s' is too long.", name.c_str());
		
		sem_t* sem = sem_open(name.c_str(), O_CREAT | O_EXCL, openMode, initialValue);
		if (sem == SEM_FAILED)
		{
			if (errno == EEXIST)
			{
				// Try again with a new name
				nameIsTaken = true;
			}
			else
			{
				// Some other creation error
				TT_PANIC("Could not create POSIX semaphore.\nError (code %d): '%s'",
				         errno, strerror(errno));
				break;
			}
		}
		else
		{
			// Successfully created a named semaphore
			m_data->sem = sem;
			break;
		}
	} while (nameIsTaken);
}


Semaphore::~Semaphore()
{
	if (m_data->sem != 0)
	{
		const int result = sem_close(m_data->sem);
		TT_ASSERTMSG(result == 0, "Could not close POSIX semaphore.\nError (code %d): '%s'",
		             errno, strerror(errno));
	}
	delete m_data;
}


void Semaphore::wait()
{
	const int result = sem_wait(m_data->sem);
	TT_ASSERTMSG(result == 0, "sem_wait failed with error (code %d): '%s'",
				 errno, strerror(errno));
}


bool Semaphore::tryWait()
{
	const int result = sem_trywait(m_data->sem);
	if (result == EAGAIN)
	{
		return false;
	}
	TT_ASSERTMSG(result == 0, "sem_trywait failed with error (code %d): '%s'",
				 errno, strerror(errno));
	return true;
}


void Semaphore::signal()
{
	const int result = sem_post(m_data->sem);
	TT_ASSERTMSG(result == 0, "sem_post failed with error (code %d): '%s'",
				 errno, strerror(errno));
}


void Semaphore::setNamePrefix(const std::string& p_prefix)
{
	g_semaphoreNamePrefix = p_prefix;
}

// Namespace end
}
}
