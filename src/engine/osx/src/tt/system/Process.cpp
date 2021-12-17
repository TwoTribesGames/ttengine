#include <tt/platform/tt_error.h>
#include <tt/system/Process.h>


namespace tt {
namespace system {

//--------------------------------------------------------------------------------------------------
// Public member functions

ProcessPtr Process::createProcess(const std::string& p_application,
                                  const std::string& p_arguments,
                                  const std::string& p_cwd)
{
	Process* process = new Process;
	if (process == 0)
	{
		TT_PANIC("Unable to create new Process object.");
		return ProcessPtr();
	}
	
	if (process->create(p_application, p_arguments, p_cwd) == false)
	{
		return ProcessPtr();
	}
	
	return ProcessPtr(process, deleteProcess);
}


bool Process::isRunning()
{
	// FIXME: Implement this stub!
	TT_PANIC("STUB");
	return false;
	/*
	DWORD exitcode;
	if (GetExitCodeProcess(m_pi.hProcess, &exitcode) == 0)
	{
		TT_PANIC("Unable to get process status (err: %d).", GetLastError());
		return false;
	}
	
	return exitcode == STILL_ACTIVE;
	*/
}


bool Process::wait(time_type /*p_milliSeconds*/)
{
	// FIXME: Implement this stub!
	TT_PANIC("STUB");
	return false;
	/*
	DWORD ms = p_milliSeconds == 0 ? INFINITE : static_cast<DWORD>(p_milliSeconds);
	DWORD ret = WaitForSingleObject(m_pi.hProcess, ms);
	
	if (ret == WAIT_FAILED)
	{
		TT_PANIC("Error waiting for process (err: %d).", GetLastError());
		return false;
	}
	
	if (ret == WAIT_TIMEOUT)
	{
		if (ms == INFINITE)
		{
			return true;
		}
		else
		{
			TT_PANIC("Error waiting for process (err: %d).", GetLastError());
			return false;
		}
	}
	if (ret != WAIT_OBJECT_0)
	{
		TT_PANIC("Error waiting for process (err: %d).", GetLastError());
		return false;
	}
	
	return true;
	*/
}


bool Process::terminate(exitcode_type /*p_exit*/)
{
	// FIXME: Implement this stub!
	TT_PANIC("STUB");
	return false;
	/*
	if (p_exit == STILL_ACTIVE)
	{
		TT_PANIC("Do not use STILL_ACTIVE (%d) as exit code!", STILL_ACTIVE);
		return false;
	}
	
	if (TerminateProcess(m_pi.hProcess, static_cast<UINT>(p_exit)) == 0)
	{
		TT_PANIC("Error terminating process (err: %d).", GetLastError());
		return false;
	}
	
	return true;
	*/
}


Process::exitcode_type Process::getExitCode()
{
	// FIXME: Implement this stub!
	TT_PANIC("STUB");
	return 0;
	/*
	DWORD exitcode;
	if (GetExitCodeProcess(m_pi.hProcess, &exitcode) == 0)
	{
		TT_PANIC("Unable to get process status (err: %d).", GetLastError());
		return 0;
	}
	
	if (exitcode == STILL_ACTIVE)
	{
		return 0;
	}
	return static_cast<exitcode_type>(exitcode);
	*/
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Process::Process()
{
	//ZeroMemory(&m_pi, sizeof(m_pi));
}


Process::~Process()
{
	//CloseHandle(m_pi.hThread);
	//CloseHandle(m_pi.hProcess);
}


void Process::deleteProcess(Process* p_process)
{
	delete p_process;
}


bool Process::create(const std::string& /*p_application*/,
                     const std::string& /*p_arguments*/,
                     const std::string& /*p_cwd*/)
{
	// FIXME: Implement this stub!
	TT_PANIC("STUB");
	return false;
	/*
	if (p_application.length() > MAX_PATH)
	{
		TT_PANIC("Application (%s) length is larger than MAX_PATH (%d).",
		         p_application.c_str(), MAX_PATH);
		return false;
	}
	
	std::string command = p_application + " " + p_arguments;
	
	char* cmd = new char[command.length() + 1];
	if (cmd == 0)
	{
		TT_PANIC("Failed to allocate buffer of %d bytes.", command.length() + 1);
		return false;
	}
	
	std::strcpy(cmd, command.c_str());
	
	// reroute std in out err
	STARTUPINFOA        si;
	
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
	si.dwFlags    = STARTF_USESTDHANDLES;
	
	// start application
	if (CreateProcessA(0, cmd, 0, 0, TRUE, 0, 0, p_cwd.empty() ? 0 : p_cwd.c_str(), &si, &m_pi) == 0)
	{
		// error
		TT_PANIC("Unable to create process '%s' (err: %d).", cmd, GetLastError());
		delete[] cmd;
		return false;
	}
	
	delete[] cmd;
	return true;
	*/
}

// Namespace end
}
}
