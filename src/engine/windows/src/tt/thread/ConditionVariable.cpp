#define NOMINMAX
#include <windows.h>

#include <tt/platform/tt_error.h>
#include <tt/thread/ConditionVariable.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace thread {

#ifdef USE_CONDITION_VARIABLE

	struct ConditionVariable::InternalData : public CONDITION_VARIABLE
	{
		
	};

#else

	struct ConditionVariable::InternalData
	{
		HANDLE handle; //!< Handle to an event
	};

	struct Mutex::InternalData
	{
		HANDLE handle;
	};

#endif

ConditionVariable::ConditionVariable()
:
m_variable(new InternalData)
{
#ifdef USE_CONDITION_VARIABLE
	InitializeConditionVariable(m_variable);
#else
	m_variable->handle = CreateEvent(NULL, TRUE, FALSE, NULL); //!< unnamed event
#endif
}


ConditionVariable::~ConditionVariable()
{
	wake(); // wake all threads waiting for this variable
#ifdef USE_CONDITION_VARIABLE
#else
	CloseHandle(m_variable->handle);
#endif
	delete m_variable;
}


void ConditionVariable::sleep(Mutex* p_mutex)
{
	TT_ASSERTMSG(p_mutex != 0, "No mutex specified!");
	
#ifdef USE_CONDITION_VARIABLE
	SleepConditionVariableCS(m_variable, reinterpret_cast<CRITICAL_SECTION*>(p_mutex->getData()), INFINITE);
#else
	ResetEvent(m_variable->handle); // ensure that we'll sleep here
	SignalObjectAndWait(p_mutex->getData()->handle, m_variable->handle, INFINITE, FALSE);
#endif
}


void ConditionVariable::wake()
{
#ifdef USE_CONDITION_VARIABLE
	WakeAllConditionVariable(m_variable);
#else
	SetEvent(m_variable->handle);
#endif
}


// namespace end
}
}
