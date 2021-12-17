#if !defined(INC_TT_SCRIPT_SCRIPTENGINE_H)
#define INC_TT_SCRIPT_SCRIPTENGINE_H

#include <map>

#include <squirrel/squirrel.h>

#include <tt/script/helpers.h>
#include <tt/script/VirtualMachine.h>
#include <tt/script/SqTopRestorerHelper.h>

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

class ScriptEngine
{
public:
	static VirtualMachinePtr createVM(const std::string& p_rootPath,
	                                  s32 p_debuggerPort = 0,
	                                  VMCompileMode p_mode = VMCompileMode_NutOnly);
	static VirtualMachinePtr getVM(HSQUIRRELVM p_vm);

	static void destroy();

	static inline bool isEmpty() { return ms_virtualMachines.empty(); }
private:
	ScriptEngine();  // Static class. Not implemented.
	~ScriptEngine(); // Static class. Not implemented.
	
	static void remove(VirtualMachine* p_virtualMachine);

	typedef std::map<HSQUIRRELVM, VirtualMachineWeakPtr> VirtualMachineContainer;
	static VirtualMachineContainer ms_virtualMachines;
};


// Namespace end
}
}


#endif // !defined(INC_TT_SCRIPT_SCRIPTENGINE_H)

