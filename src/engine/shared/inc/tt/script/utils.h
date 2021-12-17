#if !defined(INC_TT_SCRIPT_UTILS_H)
#define INC_TT_SCRIPT_UTILS_H


#include <string>

#include <squirrel/squirrel.h>


namespace tt {
namespace script {

const char* const sqObjectTypeName(SQObjectType p_type);

#if !defined(TT_BUILD_FINAL)
std::string getType(HSQUIRRELVM p_vm, SQObjectType p_type, const SQChar* p_name = 0, SQInteger p_index = -1);
#endif


std::string getCallStack(HSQUIRRELVM p_vm);
std::string getStack(HSQUIRRELVM p_vm);
void printCallStack(HSQUIRRELVM p_vm);
void printStack(HSQUIRRELVM p_vm);


// Namespace end
}
}


#endif  // !defined(INC_TT_SCRIPT_UTILS_H)
