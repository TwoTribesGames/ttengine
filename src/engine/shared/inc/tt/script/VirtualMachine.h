#if !defined(INC_TT_SCRIPT_VIRTUALMACHINE_H)
#define INC_TT_SCRIPT_VIRTUALMACHINE_H

#include <map>

#include <squirrel/squirrel.h>
#include <squirrel/sqdbg/sqrdbg.h>
#include <squirrel/sqbind.h>

#include <tt/fs/types.h>
#include <tt/script/fwd.h>
#include <tt/script/SqTopRestorerHelper.h>
#include <tt/script/ScriptObject.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#if !defined(TT_BUILD_FINAL)
#define TT_SCRIPT_SHOULD_RAISE_ERROR_ON
#endif

#if defined(TT_SCRIPT_SHOULD_RAISE_ERROR_ON)
#define TT_SCRIPT_RAISE_ERROR SQTrue
#else
#define TT_SCRIPT_RAISE_ERROR SQFalse
#endif

namespace tt {
namespace script {


class VirtualMachine
{
public:
	~VirtualMachine();
	
	void setCompileMode(VMCompileMode p_mode) { m_compileMode = p_mode; }
	VMCompileMode getCompileMode() const { return m_compileMode; }
	
	/*! \brief Disable caching forcing recompile and run each time. Some older projects need this. */
	void disableCaching_HACK() { m_disableCaching = true; }
	
	inline bool loadAndRunScript(const std::string& p_filename)
	{
		return loadAndRunScriptImpl(p_filename, true);
	}
	inline bool loadAndRunScriptNoRootPath(const std::string& p_filename)
	{
		return loadAndRunScriptImpl(p_filename, false);
	}
	
	bool runScriptBuf(const std::string& p_bufferedScript);
	
	ScriptObject loadScriptAsObject(const std::string& p_filename);
	bool runScriptObject(ScriptObject p_obj);
	
	bool compileFile(const std::string& p_inputFile, const std::string& p_outputFile, const std::string& p_name);
	inline bool saveClosure(const std::string& p_filename) { return writeClosure(m_vm, p_filename); }
	inline bool loadClosure(const std::string& p_filename) { return readClosure( m_vm, p_filename); }
	
	inline HSQUIRRELVM        getVM()               { return m_vm;       }
	inline const std::string& getRoot()       const { return m_rootPath; }
	inline bool               isInitialized() const { return m_vm != 0;  }
	void reset();
	void update();
	
#if !defined(TT_BUILD_FINAL)
	void printCallStack();
	void printStack();
#endif
	
	static SQInteger tt_include(HSQUIRRELVM v);
	
	bool hasSqFun(const std::string& p_functionName);
	bool hasSqMemberFun(const std::string& p_functionName, const HSQOBJECT& p_class);
	
	//----------------------------------------------------------------------------------------------
	// Calling global Squirrel functions
	// 0 Parameters
	bool callSqFun(const std::string& p_functionName);
	
	template <typename RetType>
	bool callSqFunWithReturn(RetType* p_return_OUT,
	                         const std::string& p_functionName);
	
	// 1 Parameters
	template <typename P1Type>
	bool callSqFun(const std::string& p_functionName, const P1Type& p_p1);
	
	template <typename RetType,
	          typename P1Type>
	inline bool callSqFunWithReturn(RetType* p_return_OUT,
	                                const std::string& p_functionName,
	                                const P1Type& p_p1);
	
	// 2 Parameters
	template <typename P1Type,
	          typename P2Type>
	bool callSqFun(const std::string& p_functionName,
	               const P1Type& p_p1,
	               const P2Type& p_p2);
	
	// 3 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type>
	bool callSqFun(const std::string& p_functionName,
	               const P1Type& p_p1,
	               const P2Type& p_p2,
	               const P3Type& p_p3);
	
	// 4 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type>
	bool callSqFun(const std::string& p_functionName,
	               const P1Type& p_p1,
	               const P2Type& p_p2,
	               const P3Type& p_p3,
	               const P4Type& p_p4);
	
	// 5 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type,
	          typename P5Type>
	bool callSqFun(const std::string& p_functionName,
	               const P1Type& p_p1,
	               const P2Type& p_p2,
	               const P3Type& p_p3,
	               const P4Type& p_p4,
	               const P5Type& p_p5);
	
	// 6 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type,
	          typename P5Type,
	          typename P6Type>
	bool callSqFun(const std::string& p_functionName,
	               const P1Type& p_p1,
	               const P2Type& p_p2,
	               const P3Type& p_p3,
	               const P4Type& p_p4,
	               const P5Type& p_p5,
	               const P6Type& p_p6);
	
	// 7 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type,
	          typename P5Type,
	          typename P6Type,
	          typename P7Type>
	bool callSqFun(const std::string& p_functionName,
	               const P1Type& p_p1,
	               const P2Type& p_p2,
	               const P3Type& p_p3,
	               const P4Type& p_p4,
	               const P5Type& p_p5,
	               const P6Type& p_p6,
	               const P7Type& p_p7);
	
	
	//----------------------------------------------------------------------------------------------
	// Calling class specific Squirrel functions
	// 0 Parameters
	bool callSqMemberFun(const std::string& p_functionName, const HSQOBJECT& p_class, const HSQOBJECT& p_obj);
	
	template <typename RetType>
	inline bool callSqMemberFunWithReturn(RetType* p_return_OUT,
	                                      const std::string& p_functionName,
	                                      const HSQOBJECT& p_class,
	                                      const HSQOBJECT& p_obj);
	
	// 1 Parameters
	template <typename P1Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1);
	
	template <typename RetType,
	          typename P1Type>
	inline bool callSqMemberFunWithReturn(RetType* p_return_OUT,
	                                      const std::string& p_functionName,
	                                      const HSQOBJECT& p_class,
	                                      const HSQOBJECT& p_obj,
	                                      const P1Type* p_p1);
	
	template <typename RetType,
	          typename P1Type>
	inline bool callSqMemberFunWithReturn(RetType* p_return_OUT,
	                                      const std::string& p_functionName,
	                                      const HSQOBJECT& p_class,
	                                      const HSQOBJECT& p_obj,
	                                      const P1Type& p_p1);
	
	// 2 Parameters
	template <typename P1Type,
	          typename P2Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1,
	                     const P2Type& p_p2);
	
	template <typename P1Type,
	          typename P2Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type* p_p1,
	                     const P2Type& p_p2);
	
	// 3 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1,
	                     const P2Type& p_p2,
	                     const P3Type& p_p3);
	
	// 4 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1,
	                     const P2Type& p_p2,
	                     const P3Type& p_p3,
	                     const P4Type& p_p4);
	
	// 5 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type,
	          typename P5Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1,
	                     const P2Type& p_p2,
	                     const P3Type& p_p3,
	                     const P4Type& p_p4,
	                     const P5Type& p_p5);
	
	// 6 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type,
	          typename P5Type,
	          typename P6Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1,
	                     const P2Type& p_p2,
	                     const P3Type& p_p3,
	                     const P4Type& p_p4,
	                     const P5Type& p_p5,
	                     const P6Type& p_p6);
	
	// 7 Parameters
	template <typename P1Type,
	          typename P2Type,
	          typename P3Type,
	          typename P4Type,
	          typename P5Type,
	          typename P6Type,
	          typename P7Type>
	bool callSqMemberFun(const std::string& p_functionName,
	                     const HSQOBJECT& p_class,
	                     const HSQOBJECT& p_obj,
	                     const P1Type& p_p1,
	                     const P2Type& p_p2,
	                     const P3Type& p_p3,
	                     const P4Type& p_p4,
	                     const P5Type& p_p5,
	                     const P6Type& p_p6,
	                     const P7Type& p_p7);
	
	bool callFunction(const std::string& p_functionName, s32 p_nparams);
	bool callFunctionWithReturn(const std::string& p_functionName, s32 p_nparams);
	bool prepareFunctionOnStack(const std::string& p_functionName);
	bool prepareMethodOnStack(const std::string& p_methodName, const HSQOBJECT& p_class, const HSQOBJECT& p_object);
	
private:
	class CharBuffer;
	typedef tt_ptr<CharBuffer>::shared CharBufferPtr;
	class CharBuffer
	{
	public:
		static CharBufferPtr create(const std::string& p_filename);
		~CharBuffer();
		
		inline bool readNext(char* p_char_OUT)
		{
			if (m_position >= m_size)
			{
				return false;
			}
			
			*p_char_OUT = m_buffer[m_position];
			++m_position;
			return true;
		}
		
	private:
		CharBuffer(const tt::fs::FilePtr& p_file);
		
		char*             m_buffer;
		tt::fs::size_type m_size;
		tt::fs::size_type m_position;
	};
	
	bool getValueFromStack(bool* p_value, SQInteger p_stackIdx = -1);
	
	template <typename type>
	bool getValueFromStack(type* p_value, SQInteger p_stackIdx = -1); // Uses SqBind for getter
	
	typedef std::map<std::string, ScriptObject> ScriptObjects;
	
	static VirtualMachine* create(const std::string& p_root, s32 p_debuggerPort, VMCompileMode p_mode);
	
	explicit VirtualMachine(const std::string& p_rootPath, s32 p_debuggerPort, VMCompileMode p_mode);
	VirtualMachine(const VirtualMachine& p_helper);                  // Disable copy. Not implemented.
	const VirtualMachine& operator=(const VirtualMachine& p_helper); // Disable copy. Not implemented.
	
	bool register_global_func(HSQUIRRELVM v, SQFUNCTION f, const char *fname);
	
	bool loadAndRunScriptImpl(const std::string& p_filename, bool p_useRootPath);
	bool loadOrCompileScript(const std::string& p_filename, bool p_useRootPath);
	bool callSQ(const std::string& p_source);
	
	static void printFunc(HSQUIRRELVM /*p_vm*/, const SQChar* p_format, ...);
	static void errorFunc(HSQUIRRELVM /*p_vm*/, const SQChar* p_format, ...);
	
	inline static SQInteger buffer_lexfeedASCII(SQUserPointer p_buffer)
	{
		CharBufferPtr buffer = (*static_cast<CharBufferPtr*>(p_buffer));
		char c = 0;
		if (buffer->readNext(&c))
		{
			return c;
		}
		return 0;
	}
	
	static bool compile_file(HSQUIRRELVM v, const std::string& p_filename, const std::string& p_name);
	static void compileErrorHandler(HSQUIRRELVM /*v*/, const SQChar* p_desc,const SQChar* p_source,
	                            SQInteger p_line, SQInteger column);
	static SQInteger tt_panic(HSQUIRRELVM v);
	static SQInteger tt_warning(HSQUIRRELVM v);
	static SQInteger printerror(HSQUIRRELVM v);
	
	static bool writeClosure(HSQUIRRELVM v, const std::string& p_filename);
	static bool readClosure( HSQUIRRELVM v, const std::string& p_filename);
	
	static SQInteger writeFunction(SQUserPointer p_userPtr, SQUserPointer p_data,SQInteger p_dataSize);
	static SQInteger readFunction( SQUserPointer p_userPtr, SQUserPointer p_data,SQInteger p_dataSize);
	
	HSQUIRRELVM   m_vm;
	std::string   m_rootPath;
	ScriptObjects m_cachedIncludes;
	VMCompileMode m_compileMode;
	bool          m_disableCaching; // Disable script caching for older projects were we get script problem if we caching. (A function will not get overriden.)
	
#if defined(TT_PLATFORM_WIN)
	HSQREMOTEDBG m_rdbg;
#endif
	
	friend class ScriptEngine;
};


}
}

#include "VirtualMachineMethods.inc"

#endif //INC_TT_SCRIPT_VIRTUALMACHINE_H

