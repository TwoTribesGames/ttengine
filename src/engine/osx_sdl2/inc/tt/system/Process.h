#if !defined(INC_TT_SYSTEM_PROCESS_H)
#define INC_TT_SYSTEM_PROCESS_H

#include <string>

//#define NOMINMAX
//#include <windows.h>

#include <tt/platform/tt_types.h>


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
	                                const std::string& p_cwd = std::string());
	
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
	
private:
	Process();
	~Process();
	
	Process(const Process&);
	const Process& operator=(const Process&);
	
	static void deleteProcess(Process* p_process);
	
	bool create(const std::string& p_application,
	            const std::string& p_arguments,
	            const std::string& p_cwd);
	
	//PROCESS_INFORMATION m_pi;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_SYSTEM_PROCESS_H)
