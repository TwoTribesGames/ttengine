#if !defined(INC_TT_SYSTEM_PROCESS_H)
#define INC_TT_SYSTEM_PROCESS_H

#if !defined(_XBOX)  // FIXME: It might be possible to implement this class on Xbox 360, with some restrictions


#include <string>
#include <sstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <tt/platform/tt_types.h>
#include <tt/thread/CriticalSection.h>
#include <tt/thread/BackgroundThread.h>


namespace tt {
namespace system {

class Process;
typedef tt_ptr<Process>::shared ProcessPtr;

class Process
{
public:
	typedef int exitcode_type;
	typedef int time_type;
	
	/*! \brief Creates a new process.
	    \param p_application The path to the application to start.
	    \param p_arguments The arguments to pass to the application.
	    \param p_cwd The working directory for the application.
	    \return Process object on success, null on failure.*/
	static ProcessPtr createProcess(const std::string& p_application,
	                                const std::string& p_arguments,
	                                const std::string& p_cwd = std::string(),
	                                bool               p_hideWindow = false,
	                                bool               p_captureOutput = false);
	
	/*! \brief Retrieves whether the process is running.
	    \return true when the process is running, false when it has terminated.*/
	bool isRunning();
	
	/*! \brief Waits for the process.
	    \param p_milliSeconds The amount of time to wait, 0 means wait until process has terminated.
	    \return true when the wait has completed or the process has terminated, false on failure.*/
	bool wait(time_type p_milliSeconds = 0);
	
	/*! \brief Terminates the process.
	    \param p_exit The forced exitcode for the process.
	    \return true when the process has terminated successfully, false on failure.*/
	bool terminate(exitcode_type p_exit);
	
	/*! \brief Retrieves the process' exit code.
	    \return 0 when the application is still running, the exit code when it has terminated.*/
	exitcode_type getExitCode();
	
	inline HANDLE getOutputReadHandle() const { return m_outputReadHandle; }
	inline HANDLE getOutputWriteHandle() const { return m_outputWriteHandle; }
	inline HANDLE getErrorWriteHandle() const { return m_errorWriteHandle; }
	
	void writeToOutputStream(const std::string& p_text);
	std::string getOutput();
	
private:
	class OutputReaderThread : public thread::BackgroundThread
	{
	public:
		OutputReaderThread();
		virtual ~OutputReaderThread() {};
		virtual int run(void* p_process);
		virtual void stop();
	private:
		bool m_running;
	};
	typedef tt_ptr<OutputReaderThread>::shared OutputReaderThreadPtr;
	
	Process();
	~Process();
	
	Process(const Process&);
	const Process& operator=(const Process&);
	
	static void deleteProcess(Process* p_process);
	
	bool create(const std::string& p_application,
	            const std::string& p_arguments,
	            const std::string& p_cwd,
	            bool               p_hideWindow,
	            bool               p_captureOutput);
	
	void closePipe(HANDLE p_handle);
	
	HANDLE m_outputReadHandle;
	HANDLE m_outputWriteHandle;
	HANDLE m_errorWriteHandle;
	
	OutputReaderThreadPtr m_readerThread;
	tt::thread::Mutex     m_mutex;
	std::ostringstream    m_outputStream;
	
	PROCESS_INFORMATION m_pi;
};

// Namespace end
}
}


#endif  // !defined(_XBOX)

#endif  // !defined(INC_TT_SYSTEM_PROCESS_H)
