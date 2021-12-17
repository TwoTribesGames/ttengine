#include <tt/platform/tt_error.h>
#include <tt/system/Process.h>


namespace tt {
namespace system {

// ------------------------------------------------------------
// OutputReaderThread Implementation

Process::OutputReaderThread::OutputReaderThread()
:
BackgroundThread(),
m_running(true)
{
}


int Process::OutputReaderThread::run(void* p_process)
{
	if (p_process == 0)
	{
		TT_NULL_ASSERT(p_process);
		return -1;
	}
	
	tt::system::Process* process = static_cast<tt::system::Process*>(p_process);
	
	HANDLE outputHandle = process->getOutputReadHandle();
	if (outputHandle == 0)
	{
		TT_PANIC("Output read handle is 0");
		return -1;
	}
	
	const size_t maxBufSize = 1024 * 32;
	char buf[maxBufSize];
	
	while (m_running)
	{
		if (process->isRunning() == false)
		{
			break;
		}
			
		DWORD dataAvailable = 0;
		if (PeekNamedPipe(outputHandle, 0, 0, 0, &dataAvailable, 0) == FALSE)
		{
			break;
		}
			
		if (dataAvailable == 0)
		{
			continue;
		}
			
		if (dataAvailable > maxBufSize)
		{
			TT_PANIC("Reading from pipe with more data available (%d bytes) "
				"than the current buffer size (%d bytes)",
				dataAvailable, maxBufSize);
			dataAvailable = maxBufSize;
		}
			
		DWORD bytesRead = 0;
		if (ReadFile(outputHandle, buf, dataAvailable, &bytesRead, 0) == FALSE || bytesRead == 0)
		{
			break;
		}
		buf[bytesRead] = 0;
		process->writeToOutputStream(std::string(buf));
	}
	
	return 0;
}


void Process::OutputReaderThread::stop()
{
	m_running = false;
}


// ------------------------------------------------------------
// Public functions

ProcessPtr Process::createProcess(const std::string& p_application,
                                  const std::string& p_arguments,
                                  const std::string& p_cwd,
                                  bool               p_hideWindow,
                                  bool               p_captureOutput)
{
	Process* process = new Process;
	if (process == 0)
	{
		TT_PANIC("Unable to create new Process object.");
		return ProcessPtr();
	}
	
	if (process->create(p_application, p_arguments, p_cwd, p_hideWindow, p_captureOutput) == false)
	{
		deleteProcess(process);
		return ProcessPtr();
	}
	
	return ProcessPtr(process, deleteProcess);
}


bool Process::isRunning()
{
	DWORD exitcode;
	if (GetExitCodeProcess(m_pi.hProcess, &exitcode) == 0)
	{
		TT_PANIC("Unable to get process status (err: %d).", GetLastError());
		return false;
	}
	
	return exitcode == STILL_ACTIVE;
}


bool Process::wait(time_type p_milliSeconds)
{
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
}


bool Process::terminate(exitcode_type p_exit)
{
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
}


Process::exitcode_type Process::getExitCode()
{
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
}


void Process::writeToOutputStream(const std::string& p_text)
{
	tt::thread::CriticalSection section(&m_mutex);
	m_outputStream << p_text;
}


std::string Process::getOutput()
{
	tt::thread::CriticalSection section(&m_mutex);
	return m_outputStream.str();
}


// ------------------------------------------------------------
// Private functions

Process::Process()
:
m_outputReadHandle(0),
m_outputWriteHandle(0),
m_errorWriteHandle(0),
m_readerThread(new OutputReaderThread)
{
	ZeroMemory(&m_pi, sizeof(m_pi));
}


Process::~Process()
{
	if (m_outputReadHandle != 0) closePipe(m_outputReadHandle);
	if (m_outputWriteHandle != 0) closePipe(m_outputWriteHandle);
	if (m_errorWriteHandle != 0) closePipe(m_errorWriteHandle);
	
	CloseHandle(m_pi.hThread);
	CloseHandle(m_pi.hProcess);
}


void Process::deleteProcess(Process* p_process)
{
	// kill the reader thread before the actual object is deleted
	if (p_process->m_readerThread != 0)
	{
		p_process->m_readerThread->stopThread();
	}
	
	delete p_process;
}


bool Process::create(const std::string& p_application,
                     const std::string& p_arguments,
                     const std::string& p_cwd,
                     bool               p_hideWindow,
                     bool               p_captureOutput)
{
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
	if (p_captureOutput)
	{
		HANDLE outputReadTmpHandle;
		SECURITY_ATTRIBUTES sa;
		sa.nLength= sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		
		if (CreatePipe(&outputReadTmpHandle, &m_outputWriteHandle, &sa, 0) == FALSE)
		{
			TT_PANIC("Cannot create pipe");
			return false;
		}
		
		// Create a duplicate of the stdout write handle for the std
		// error write handle. This is necessary in case the child
		// application closes one of its std output handles.
		if (DuplicateHandle(GetCurrentProcess(), m_outputWriteHandle, GetCurrentProcess(),
				&m_errorWriteHandle, 0, TRUE, DUPLICATE_SAME_ACCESS) == FALSE)
		{
			TT_PANIC("Cannot duplicate pipe to create error pipe");
			return false;
		}
		
		// Create new stdout read handle and the stdin write handle.
		// Set the inheritance properties to FALSE. Otherwise, the child
		// inherits the these handles; resulting in non-closeable
		// handles to the pipes being created.
		if (DuplicateHandle(GetCurrentProcess(), outputReadTmpHandle, GetCurrentProcess(),
				&m_outputReadHandle, 0, FALSE,	// make it uninheritable.
				DUPLICATE_SAME_ACCESS) == FALSE)
		{
			TT_PANIC("Cannot create new stdout read handle and the stdin write handle.");
			return false;
		}
		
		closePipe(outputReadTmpHandle);
	}
	
	STARTUPINFOA        si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	
	si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = p_captureOutput ? m_outputWriteHandle : GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError  = p_captureOutput ? m_errorWriteHandle  : GetStdHandle(STD_ERROR_HANDLE);
	si.dwFlags    = STARTF_USESTDHANDLES;
	
	// start application
	if (CreateProcessA(0, cmd, 0, 0, TRUE, p_hideWindow ? CREATE_NO_WINDOW : 0, 0,
		p_cwd.empty() ? 0 : p_cwd.c_str(), &si, &m_pi) == 0)
	{
		// error
		TT_PANIC("Unable to create process '%s' (err: %d).", cmd, GetLastError());
		delete[] cmd;
		return false;
	}
	
	if (p_captureOutput)
	{
		m_readerThread->startThread(this);
	}
	
	delete[] cmd;
	return true;
}


void Process::closePipe(HANDLE p_handle)
{
	if (CloseHandle(p_handle) == FALSE)
	{
		TT_PANIC("Cannot close pipe");
	}
}


// Namespace end
}
}
