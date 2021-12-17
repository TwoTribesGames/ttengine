#if !defined(INC_TT_THREAD_THREAD_H)
#define INC_TT_THREAD_THREAD_H

#include <tt/thread/types.h>


namespace tt {
namespace thread {

/*! \brief Returns the number of CPUs (or cores) on the current machine. */
s32 getProcessorCount();

/*! \brief Creates a new thread.
    \param p_proc Thread procedure.
    \param p_param Parameter for thread procedure.
    \param p_suspended Whether the thread is in suspended state after being created.
    \param p_stackSize Stack size in bytes, use 0 for default.
    \param p_priority Priority for new thread.
    \param p_name The name for this thread, use 0 for no name
    \return Shared pointer to thread handle, do not close until thread has terminated. 0 on fail.*/
handle create(ThreadProc    p_proc,
              void*         p_param,
              bool          p_suspended = true,
              size_type     p_stackSize = 0,
              priority_type p_priority  = priority_normal,
              Affinity      p_affinity  = Affinity_None,
              const char*   p_name      = 0);

/*! \brief Resumes a suspended thread.
    \param p_thread Thread to resume.*/
void resume(const handle& p_thread);

/*! \brief Suspends a thread.
    \param p_thread Thread to suspend.*/
void suspend(const handle& p_thread);

/*! \brief Puts the current thread to sleep.
    \param p_milliSeconds The minimum amount of milliseconds the thread should sleep.*/
void sleep(int p_milliSeconds);

/*! \brief Performs rescheduling.
    Allows threads with the same priority to run and performs rescheduling.*/
void yield();

/*! \brief Terminates a thread.
    \param p_thread Thread to terminate.
    \param p_exitCode Thread's forced exit code.*/
void terminate(const handle& p_thread, int p_exitCode);

/*! \brief Exits the current thread.
    \param p_exitCode Exit code.*/
void exit(int p_exitCode);

/*! \brief Returns the current thread.
    \return The current thread.*/
handle getCurrent();

/*! \brief Returns a thread's priority.
    \param p_thread The thread to get the priority of.
    \return The priority of the thread.*/
priority_type getPriority(const handle& p_thread);

/*! \brief Sets a thread's priority.
    \param p_thread The thread to set the priority of.
    \param p_priority The priority to set.*/
void setPriority(const handle& p_thread, priority_type p_priority);

/*! \brief Puts the current thread to sleep until the specified thread has finished.
    \param p_thread The thread to wait for.*/
void wait(const handle& p_thread);

/*! \brief Checks whether a thread has ended.
    \param p_thread The thread to check.
    \return Whether the thread has ended.*/
bool hasEnded(const handle& p_thread);

/*! \brief Set display name for thread (for debugging)
    \param p_thread The thread to set the name on.
    \param p_name The name for this thread. */
void setName(const handle& p_thread, const char* p_name);

// namespace end
}
}

#endif // !defined(INC_TT_THREAD_THREAD_H)
