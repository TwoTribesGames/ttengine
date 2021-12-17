#if !defined(INC_TT_SCRIPT_FWD_H)
#define INC_TT_SCRIPT_FWD_H



namespace tt {
namespace script {

class ScriptEngine;
class ScriptObject;
class ScriptValue;
class SqTopRestorerHelper;

class VirtualMachine;
typedef tt_ptr<VirtualMachine>::shared VirtualMachinePtr;
typedef tt_ptr<VirtualMachine>::weak   VirtualMachineWeakPtr;

enum VMCompileMode
{
	VMCompileMode_NutOnly,   // The original behavior. Only look for .nut files. (The default)
	VMCompileMode_NutToBnut, // Look for .nut files, compile to bnut if found and delete nut, otherwise load bnuts.
	VMCompileMode_BnutOnly   // Only look for bnuts and load those.
};


// Namespace end
}
}


#endif  // !defined(INC_TT_SCRIPT_FWD_H)
