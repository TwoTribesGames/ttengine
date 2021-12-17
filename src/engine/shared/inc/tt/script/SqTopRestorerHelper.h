#if !defined(INC_TT_SCRIPT_SQTOPRESTORERHELPER_H)
#define INC_TT_SCRIPT_SQTOPRESTORERHELPER_H

#include <squirrel/squirrel.h>

#include <tt/platform/tt_error.h>

namespace tt {
namespace script {

class SqTopRestorerHelper
{
public:
	inline SqTopRestorerHelper(HSQUIRRELVM p_vm,
	                           bool p_assertOnStackLeak = false,
	                           SQInteger p_allowedTopDifference = 0)
	:
	m_vm(p_vm),
	m_top(sq_gettop(p_vm) + p_allowedTopDifference),
	m_assertOnStackLeak(p_assertOnStackLeak)
	{
	}
	
	inline ~SqTopRestorerHelper()
	{
		restoreTop();
	}
	
	inline void restoreTop()
	{
		TT_ASSERTMSG(m_assertOnStackLeak == false || sq_gettop(m_vm) == m_top,
		             "Stack leak detected. Stack top is %d, but should be %d", sq_gettop(m_vm), m_top);
		sq_settop(m_vm, m_top);
	}
	
private:
	SqTopRestorerHelper(const SqTopRestorerHelper& p_helper);                  // Disable copy. Not implemented.
	const SqTopRestorerHelper& operator=(const SqTopRestorerHelper& p_helper); // Disable copy. Not implemented.
	
	HSQUIRRELVM m_vm;
	SQInteger m_top;
	bool m_assertOnStackLeak;
};


}
}
#endif //INC_TT_SCRIPT_SQTOPRESTORERHELPER_H
