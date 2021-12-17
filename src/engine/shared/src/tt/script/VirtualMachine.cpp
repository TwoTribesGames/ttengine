#if defined(TT_PLATFORM_WIN)
#define NOMINMAX
#include <windows.h>
#endif

#include <squirrel/squirrel.h>
#include <squirrel/sqbind.h>
#include <squirrel/sqstdio.h>
#include <squirrel/sqstdmath.h>
#include <squirrel/sqstdstring.h>

#include <tt/code/AutoGrowBuffer.h>
#include <tt/code/bufferutils.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/common.h>
#include <tt/str/toStr.h>
#include <tt/platform/tt_error.h>
#include <tt/system/Time.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>

#include <tt/script/VirtualMachine.h>
#include <tt/script/ScriptEngine.h>
#include <tt/script/ScriptObject.h>
#include <tt/script/SqTopRestorerHelper.h>
#include <tt/script/utils.h>


// SQ_PANIC implementation
#if !defined(TT_BUILD_FINAL)

//==================================================================================================
// Real (non-final) implementations of assert macros

#define SQ_PANIC(p_file, p_line, ...) do { tt::platform::error::TTPanic((p_file), (p_line), TT_FUNC_SIG, __VA_ARGS__); } while(0)

#else  // defined(TT_BUILD_FINAL)

//==================================================================================================
// Dummy no-op (final) implementations of assert macros

#if (defined(_MSC_VER) && _MSC_VER >= 1600)

// Special-cased dummy implementations for Visual Studio 2010,
// where sizeof(x) does not actually use x (still triggers warnings)

#define SQ_PANIC(...)                   do { ((void)(true ? 0 : ((void)(__VA_ARGS__), void(), 0))); } while(0)

#else

// Dummy implementations for all other compilers

// Dummy helper to get rid of __VA_ARGS__
// Note, this function's body is NEVER defined because it is never called.
namespace tt { namespace platform { namespace error {
	extern int DummyHelper(...);
} } }

#define SQ_PANIC(...)                   do { (void)sizeof(tt::platform::error::DummyHelper(__VA_ARGS__)); } while(0)

#endif  // defined(_MSC_VER) && _MSC_VER >= 1600

#endif  // defined(TT_BUILD_FINAL)


namespace tt {
namespace script {

VirtualMachine::~VirtualMachine()
{
#if defined(TT_PLATFORM_WIN)
	if (m_rdbg != 0)
	{
		sq_rdbg_shutdown(m_rdbg);
	}
#endif
	
	m_cachedIncludes.clear();
	//sq_pop(m_vm, sq_gettop(m_vm));
	sq_close(m_vm);
	m_vm = 0;
}


void VirtualMachine::reset()
{
	//TT_Printf("VirtualMachine::reset - pre-reset top: %d\n", sq_gettop(m_vm));
	sq_pop(m_vm, sq_gettop(m_vm));
	
	m_cachedIncludes.clear();
	
	sq_pushroottable(m_vm); //push the root table (where the globals of the script will be stored)
	//TT_Printf("VirtualMachine::reset - post-reset top: %d\n", sq_gettop(m_vm));
}


void VirtualMachine::update()
{
#if defined(TT_PLATFORM_WIN)
	if (m_rdbg != 0)
	{
		sq_rdbg_update(m_rdbg);
	}
#endif
}

#if !defined(TT_BUILD_FINAL)
void VirtualMachine::printCallStack()
{
	tt::script::printCallStack(m_vm);
}


void VirtualMachine::printStack()
{
	tt::script::printStack(m_vm);
}
#endif // #if !defined(TT_BUILD_FINAL)


SQInteger VirtualMachine::tt_include(HSQUIRRELVM v)
{
	SQInteger nargs = sq_gettop(v);
	const SQChar* strPtr = 0;
	switch(sq_gettype(v,nargs))
	{
		case OT_STRING:
		{
			if (SQ_FAILED(sq_getstring(v, nargs, &strPtr)))
			{
				TT_PANIC("Failed to get string (sq_getString(v, top, &strPtr))");
			}
			break;
		}
		default:
		{
			TT_PANIC("Expected String, but the argument wasn't one");
			break;
		}
	}
	VirtualMachinePtr vmPtr = ScriptEngine::getVM(v);
	TT_NULL_ASSERT(vmPtr);
	if(strPtr != 0 && vmPtr != 0)
	{
		std::string file(strPtr);
		
		// Make sure file ends with .nut. This is so we always find it in the cache.
		// It doesn't matter if we include without extension or with .nut to with .bnut; it's the same thing.
		const std::string nutExtension(".nut");
		const std::string bnutExtension(".bnut");
		// Check if bnut needs removal.
		if (tt::str::endsWith(tt::str::toLower(file), bnutExtension))
		{
			file = file.substr(0, file.length() - bnutExtension.length());
		}
		// Check if nut needs to be added.
		if (tt::str::endsWith(tt::str::toLower(file), nutExtension) == false)
		{
			file += nutExtension;
		}
		
		ScriptObject scriptObject(vmPtr->getVM());
		ScriptObjects::iterator it = vmPtr->m_cachedIncludes.find(file);
		if (it == vmPtr->m_cachedIncludes.end())
		{
			scriptObject = vmPtr->loadScriptAsObject(file);
			if (vmPtr->m_disableCaching == false)
			{
				vmPtr->m_cachedIncludes.insert(std::make_pair(file, scriptObject));
			}
			vmPtr->runScriptObject(scriptObject); // Only run include the first time. (When created)
		}
	}
	return 1;
}


//-------------------------------------------------------------------------------------------------
// Private member functions

VirtualMachine::CharBufferPtr VirtualMachine::CharBuffer::create(const std::string& p_filename)
{
	if (tt::fs::fileExists(p_filename) == false)
	{
		TT_PANIC("File '%s' does not exist.", p_filename.c_str());
		return CharBufferPtr();
	}
	
	tt::fs::FilePtr file(tt::fs::open(p_filename, tt::fs::OpenMode_Read));
	if (file == 0)
	{
		TT_PANIC("Opening file '%s' failed.", p_filename.c_str());
		return CharBufferPtr();
	}
	
	return CharBufferPtr(new CharBuffer(file));
}


VirtualMachine::CharBuffer::CharBuffer(const tt::fs::FilePtr& p_file)
:
m_buffer(0),
m_size(0),
m_position(0)
{
	TT_NULL_ASSERT(p_file);
	m_size   = p_file->getLength();
	m_buffer = new char[m_size];
	tt::fs::size_type bytesRead = p_file->read(m_buffer, m_size);
	TT_ASSERT(bytesRead == m_size);
}


VirtualMachine::CharBuffer::~CharBuffer()
{
	delete [] m_buffer;
}


bool VirtualMachine::loadAndRunScriptImpl(const std::string& p_filename, bool p_useRootPath)
{
	if (loadOrCompileScript(p_filename, p_useRootPath) == false)
	{
		return false;
	}
	
	// and run it.
	sq_pushroottable(m_vm);
	
	if (callSQ(p_filename) == false)
	{
		return false;
	}
	
	sq_remove(m_vm, -1); // Removes the closure (of a loaded script.)
	
	return true;
}


bool VirtualMachine::loadOrCompileScript(const std::string& p_filename, bool p_useRootPath)
{
	std::string filename = (p_useRootPath) ? (m_rootPath + p_filename) : p_filename;
	
	// Check if .nut or .bnut extension is already present. (Remove is found.)
	const std::string nutExtension(".nut");
	const std::string bnutExtension(".bnut");
	if (tt::str::endsWith(tt::str::toLower(filename), nutExtension))
	{
		filename = filename.substr(0, filename.length() - nutExtension.length());
	}
	else if (tt::str::endsWith(tt::str::toLower(filename), bnutExtension))
	{
		filename = filename.substr(0, filename.length() - bnutExtension.length());
	}
	
	const std::string nutFilename = filename + nutExtension;
	const std::string binFilename = filename + bnutExtension;
	
	// Should we load the bnut?
	if (tt::fs::fileExists(nutFilename) == false && tt::fs::fileExists(binFilename))
	{
		return loadClosure(binFilename);
	}
	
	// Try to load the nut.
	if (compile_file(m_vm, nutFilename, binFilename) == false)
	{
		return false;
	}
	
	if (m_compileMode == VMCompileMode_NutToBnut)
	{
		// Save the nut result as bnut
		if (saveClosure(binFilename))
		{
			const bool destroyResult = tt::fs::destroyFile(nutFilename);
			TT_ASSERT(destroyResult);
		}
		else
		{
			TT_PANIC("Failed to save compiled nut as bnut file: '%s'", binFilename.c_str());
		}
	}
	
	return true;
}


bool VirtualMachine::runScriptBuf(const std::string& p_bufferedScript)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	sq_pushroottable(m_vm);
	if (SQ_FAILED(sq_compilebuffer(m_vm, p_bufferedScript.c_str(), static_cast<int>(p_bufferedScript.length()),
	              "VirtualMachine::runScriptBuf", TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Compiling buffer '%s' failed!", p_bufferedScript.c_str());
		return false;
	}
	
	// Got function.
	sq_pushroottable(m_vm); // push 'this'.
	if (callSQ("VirtualMachine::runScriptBuf") == false)
	{
		return false;
	}
	return true;
}


ScriptObject VirtualMachine::loadScriptAsObject(const std::string& p_filename)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	ScriptObject obj(getVM());
	
	if (loadOrCompileScript(p_filename, true) == false)
	{
		return obj;
	}
	
	obj.getFromStack(-1, "loadScriptAsObject from file: '" + p_filename + "'");
	return obj;
}


bool VirtualMachine::runScriptObject(ScriptObject p_obj)
{
	if (p_obj.checkVM(getVM()) == false)
	{
		TT_PANIC("Not the same VM between VirtualMachine and ScriptObject!");
		return false;
	}
	
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	p_obj.push();
	
	// Run it.
	sq_pushroottable(m_vm);
	
	
	if (callSQ("VirtualMachine::runScriptObject") == false)
	{
		return false;
	}
	
	sq_remove(m_vm, -1); // Removes the closure (of a loaded script.)
	
	return true;
}


bool VirtualMachine::compileFile(const std::string& p_inputFile,
                                 const std::string& p_outputFile,
                                 const std::string& p_name)
{
	tt::script::SqTopRestorerHelper(m_vm, true);
	
	if (compile_file(m_vm, p_inputFile, p_name) == false)
	{
		return false;
	}
	
	if (p_outputFile.empty() == false) // Should we save the closure?
	{
		if (saveClosure(p_outputFile))
		{
			return false; // save failed.
		}
	}
	
	sq_remove(m_vm, -1); // Removes the closure (of a loaded script.)
	return true;
}


bool VirtualMachine::callSQ(const std::string& p_source)
{
	if (SQ_FAILED(sq_call(m_vm, 1, SQFalse, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("sq_call for source '%s' failed!", p_source.c_str());
		return false;
	}
	return true;
}


void VirtualMachine::printFunc(HSQUIRRELVM /*p_vm*/, const SQChar* p_format, ...)
{
#if !defined(TT_BUILD_FINAL)
#if defined(TT_PLATFORM_WIN)
	HANDLE promptHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(promptHandle, &info);
	SetConsoleTextAttribute(promptHandle, FOREGROUND_GREEN);
#endif
	
	va_list args;
	va_start(args, p_format);
#ifdef SQUNICODE
	#error No unicode print function for squirrel!
#else
	TT_VPrintf(p_format, args);
#endif
	va_end(args);
	
#if defined(TT_PLATFORM_WIN)
	SetConsoleTextAttribute(promptHandle, info.wAttributes);
#endif
#else
	(void)p_format;
#endif // !defined(TT_BUILD_FINAL)
}


void VirtualMachine::errorFunc(HSQUIRRELVM /*p_vm*/, const SQChar* p_format, ...)
{
#if !defined(TT_BUILD_FINAL)
#if defined(TT_PLATFORM_WIN)
	HANDLE promptHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(promptHandle, &info);
	SetConsoleTextAttribute(promptHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
	
	va_list args;
	va_start(args, p_format);
#ifdef SQUNICODE
	#error No unicode print function for squirrel!
#else
	TT_VPrintf(p_format, args);
#endif
	va_end(args);
	
#if defined(TT_PLATFORM_WIN)
	SetConsoleTextAttribute(promptHandle, info.wAttributes);
#endif
#else
	(void)p_format;
#endif // !defined(TT_BUILD_FINAL)
}


bool VirtualMachine::compile_file(HSQUIRRELVM v, const std::string& p_filename, const std::string& p_name)
{
	CharBufferPtr buffer = CharBuffer::create(p_filename);
	if (buffer == 0)
	{
		return false;
	}
	return SQ_SUCCEEDED(sq_compile(v, buffer_lexfeedASCII, &buffer, p_name.c_str(), TT_SCRIPT_RAISE_ERROR));
}


void VirtualMachine::compileErrorHandler(HSQUIRRELVM /*v*/, const SQChar* p_desc,const SQChar* p_source,
                                SQInteger p_line, SQInteger column)
{
	SQ_PANIC(p_source, static_cast<s32>(p_line), "Squirrel compile error! source: '%s', line: %d, column: %d\ndesc: '%s'\n",
	         p_source, static_cast<s32>(p_line), static_cast<s32>(column), p_desc);
}


SQInteger VirtualMachine::tt_panic(HSQUIRRELVM v)
{
#if !defined(TT_BUILD_FINAL)
	std::ostringstream oss;
	
	oss << "Script Panic" << std::endl;
	
	SQInteger nargs = sq_gettop(v);
	const SQChar* strPtr = 0;
	switch(sq_gettype(v,nargs))
	{
		case OT_STRING:
		{
			if (SQ_FAILED(sq_getstring(v, nargs, &strPtr)))
			{
				oss << "Internal Error: failed to get string (sq_getString(v, top, &strPtr))";
			}
			else
			{
				oss << strPtr;
			}
			
			break;
		}
		default:
		{
			oss << "Internal Error: Expected String, but the argument wasn't one";
			break;
		}
	}
	
	oss << std::endl << std::endl << "Callstack:" << std::endl;
	
	// Display callstack info
	SQInteger sLevel = 1;  //1 is to skip this function that is level 0
	SQStackInfos si;
	while (SQ_SUCCEEDED(sq_stackinfos(v, sLevel, &si)))
	{
		const SQChar *fn  = _SC("unknown");
		const SQChar *src = _SC("unknown");
		if (si.funcname)fn = si.funcname;
		if (si.source)src  = si.source;
		oss << "* [" << fn << "()] file: " << src << " line: " << si.line << std::endl;
		sLevel++;
	}
	
	TT_PANIC(oss.str().c_str());
	
#else
	(void)v;
#endif
	return 1;
}


SQInteger VirtualMachine::tt_warning(HSQUIRRELVM v)
{
#if !defined(TT_BUILD_FINAL)
#if defined(TT_PLATFORM_WIN)
	HANDLE promptHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(promptHandle, &info);
	SetConsoleTextAttribute(promptHandle, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
	
	std::ostringstream oss;

	oss << "WARNING: ";
	
	// Fetch the error message
	SQInteger nargs = sq_gettop(v);
	const SQChar* strPtr = 0;
	switch(sq_gettype(v,nargs))
	{
		case OT_STRING:
		{
			if (SQ_FAILED(sq_getstring(v, nargs, &strPtr)))
			{
				oss << "Internal Error: failed to get string (sq_getString(v, top, &strPtr))";
			}
			else
			{
				oss << strPtr;
			}
			
			break;
		}
		default:
		{
			oss << "Internal Error: Expected String, but the argument wasn't one";
			break;
		}
	}
	
	// Display file info
	SQInteger sLevel = 1;  //1 is to skip this function that is level 0
	SQStackInfos si;
	if (SQ_SUCCEEDED(sq_stackinfos(v, sLevel, &si)))
	{
		const SQChar *fn  = _SC("unknown");
		const SQChar *src = _SC("unknown");
		if (si.funcname)fn = si.funcname;
		if (si.source)src  = si.source;
		oss << std::endl << "- " << fn << "() in " << src << ":" << si.line << std::endl;
	}
	TT_Printf("%s", oss.str().c_str());
#if defined(TT_PLATFORM_WIN)
	SetConsoleTextAttribute(promptHandle, info.wAttributes);
#endif
#else //#if !defined(TT_BUILD_FINAL)
	(void)v;
#endif
	return 1;
}


bool VirtualMachine::register_global_func(HSQUIRRELVM v, SQFUNCTION f, const char *fname)
{
	sq_pushroottable(v);
	sq_pushstring(v,fname,-1);
	sq_newclosure(v,f,0);
	if (SQ_FAILED(sq_createslot(v,-3)))
	{
		TT_PANIC("Creating function with name: '%s' failed!\n", fname);
		return false;
	}
	sq_pop(v,1);
	return true;
}


SQInteger VirtualMachine::printerror(HSQUIRRELVM v)
{
#ifndef TT_BUILD_FINAL
	std::string callStack(tt::script::getCallStack(v));
	if(sq_gettop(v) >= 1)
	{
		// Get calling source file and line
		SQInteger    sLevel = 1;  //1 is to skip this function that is level 0
		SQStackInfos si;
		if (SQ_SUCCEEDED(sq_stackinfos(v, sLevel, &si)))
		{
			const SQChar* src = _SC("unknown");
			if (si.source != 0) src = si.source;
			
			// Get error message
			const SQChar *sErrMessage = 0;
			if(SQ_SUCCEEDED(sq_getstring(v,2,&sErrMessage)))
			{
				SQ_PANIC(src, static_cast<s32>(si.line), _SC("Squirrel error:\n%s\nCallstack:\n%s"), sErrMessage, callStack.c_str());
			}
			else
			{
				SQ_PANIC(src, static_cast<s32>(si.line), _SC("Squirrel error:\n[unknown error]\nCallstack:\n%s"), callStack.c_str());
			}
		}
	}
#else
	(void)v;
#endif
	return 0;
}


bool VirtualMachine::writeClosure(HSQUIRRELVM v, const std::string& p_filename)
{
	// Open File and get WriteContext from autoGrowBuffer.
	tt::fs::FilePtr file(tt::fs::open(p_filename, tt::fs::OpenMode_Write));
	if (file == 0)
	{
		TT_PANIC("Opening file '%s' failed.", p_filename.c_str());
		return false;
	}
	
	tt::code::AutoGrowBufferPtr  buffer (tt::code::AutoGrowBuffer::create(1024, 512));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	// Serialize closure
	if (SQ_FAILED(sq_writeclosure(v, writeFunction, &context)))
	{
		return false;
	}
	
	// Write into the File.
	context.flush();
	
	// Save the data's total (used/written) size
	bool saveOk = tt::fs::writeInteger(file, static_cast<u32>(buffer->getUsedSize()));
	
	// Save all data blocks
	const s32 blockCount = buffer->getBlockCount();
	for (s32 i = 0; i < blockCount; ++i)
	{
		const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(buffer->getBlockSize(i));
		saveOk = saveOk && (file->write(buffer->getBlock(i), blockSize) == blockSize);
	}
	
	return saveOk;
}


bool VirtualMachine::readClosure( HSQUIRRELVM v, const std::string& p_filename)
{
	if (tt::fs::fileExists(p_filename) == false)
	{
		TT_PANIC("File '%s' does not exist.", p_filename.c_str());
		return false;
	}
	
	tt::fs::FilePtr file(tt::fs::open(p_filename, tt::fs::OpenMode_Read));
	if (file == 0)
	{
		TT_PANIC("Opening file '%s' failed.", p_filename.c_str());
		return false;
	}
	
	if (file->getLength() == 0)
	{
		TT_WARNING("Skipping empty file '%s'", p_filename.c_str());
		return false;
	}
	
	u32 dataSize = 0;
	if (tt::fs::readInteger(file, &dataSize) == false)
	{
		TT_PANIC("Loading size from data failed.");
		return false;
	}
	
	if (dataSize == 0)
	{
		TT_PANIC("dataSize == 0");
		return false;
	}
	
	u8* data = new u8[dataSize];
	
	const tt::fs::size_type readSize = static_cast<tt::fs::size_type>(dataSize);
	if (file->read(data, readSize) != readSize)
	{
		TT_PANIC("Loading data (%u bytes) failed.", dataSize);
		delete[] data;
		return false;
	}
	
	tt::code::AutoGrowBufferPtr buffer = tt::code::AutoGrowBuffer::createPrePopulated(
			data,
			static_cast<tt::code::Buffer::size_type>(dataSize),
			128);
	delete[] data;
	data = 0;
	
	if (buffer == 0)
	{
		TT_PANIC("Could not create new AutoGrowBuffer (of %u bytes) from file data.", dataSize);
		return false;
	}
	
	tt::code::BufferReadContext context(buffer->getReadContext());
	
	return SQ_SUCCEEDED(sq_readclosure(v, readFunction, &context));
}


SQInteger VirtualMachine::writeFunction(SQUserPointer p_userPtr, SQUserPointer p_data,SQInteger p_dataSize)
{
	tt::code::BufferWriteContext* context = reinterpret_cast<tt::code::BufferWriteContext*>(p_userPtr);
	TT_NULL_ASSERT(context);
	
	tt::code::bufferutils::put(reinterpret_cast<u8*>(p_data), p_dataSize, context);
	return (context->statusCode == 0) ? p_dataSize : 0;
}


SQInteger VirtualMachine::readFunction( SQUserPointer p_userPtr, SQUserPointer p_data,SQInteger p_dataSize)
{
	tt::code::BufferReadContext* context = reinterpret_cast<tt::code::BufferReadContext*>(p_userPtr);
	TT_NULL_ASSERT(context);
	
	tt::code::bufferutils::get(reinterpret_cast<u8*>(p_data), p_dataSize, context);
	return (context->statusCode == 0) ? p_dataSize : 0;
}


VirtualMachine* VirtualMachine::create(const std::string& p_root, s32 p_debuggerPort, VMCompileMode p_mode)
{
	VirtualMachine* vmPtr(new VirtualMachine(p_root, p_debuggerPort, p_mode));
	return vmPtr;
}


VirtualMachine::VirtualMachine(const std::string& p_rootPath, s32 p_debuggerPort, VMCompileMode p_mode)
:
m_vm(0),
m_rootPath(p_rootPath),
m_cachedIncludes(),
m_compileMode(p_mode),
m_disableCaching(false)
#if defined(TT_PLATFORM_WIN)
,m_rdbg(0)
#endif
{
	m_vm = sq_open(1024);
	
	sq_setprintfunc(m_vm, printFunc, errorFunc);
	sq_setcompilererrorhandler(m_vm, compileErrorHandler);
	
	// Add Squirrel function to the stack here, so it can be set as error handler.
	// The error handler is called with 2 parameters,
	//  an environment object (this) and a object. The object can be any squirrel type.
	sq_newclosure(m_vm, printerror, 0);
	sq_seterrorhandler(m_vm);
	
	sq_enabledebuginfo(m_vm, SQTrue);
	
	sq_pushroottable(m_vm); //push the root table(were the globals of the script will be stored)
	
	register_global_func(m_vm, VirtualMachine::tt_include, "tt_include");
	register_global_func(m_vm, VirtualMachine::tt_panic, "tt_panic");
	register_global_func(m_vm, VirtualMachine::tt_warning, "tt_warning");
	
	// register squirrel standard libraries
	sqstd_register_mathlib(m_vm);
	sqstd_register_stringlib(m_vm);
	
#if defined(TT_PLATFORM_WIN)
	if (p_debuggerPort > 0)
	{
		m_rdbg = sq_rdbg_init(m_vm, static_cast<unsigned short>(p_debuggerPort), SQFalse);
		TT_ASSERT(m_rdbg != 0);
		TT_Printf("\n\n*** DEBUG SERVER IS AWAITING CLIENT CONNECTION ***\n\n");
		sq_rdbg_waitforconnections(m_rdbg);
	}
#else
	TT_ASSERTMSG(p_debuggerPort <= 0,
	             "Squirrel remote debugging isn't supported on platforms other than Windows.");
#endif
}

// Namespace end
}
}
