#if !defined(INC_TT_SCRIPT_SCRIPTOBJECT_H)
#define INC_TT_SCRIPT_SCRIPTOBJECT_H


#include <squirrel/squirrel.h>


namespace tt {
namespace script {

class ScriptObject
{
public:
	explicit inline ScriptObject(HSQUIRRELVM p_vm)
	:
	m_vm(p_vm),
	m_obj(),
	m_isValid(false)
#if !defined(TT_BUILD_FINAL)
	,
	m_debugName()
#endif
	{
		sq_resetobject(&m_obj);
	}
	
	explicit inline ScriptObject(HSQUIRRELVM p_vm, s32 p_stackIndex)
	:
	m_vm(p_vm),
	m_obj(),
	m_isValid(false)
#if !defined(TT_BUILD_FINAL)
	,
	m_debugName()
#endif
	{
		sq_resetobject(&m_obj);
		getFromStack(p_stackIndex, "");
	}
	
	inline ScriptObject(const ScriptObject& p_rhs)
	:
	m_vm(p_rhs.m_vm),
	m_obj(),
	m_isValid(p_rhs.m_isValid)
#if !defined(TT_BUILD_FINAL)
	,
	m_debugName(p_rhs.m_debugName)
#endif
	{
		sq_resetobject(&m_obj);
		m_obj = p_rhs.m_obj;
		
		sq_addref(m_vm, &m_obj);
	}
	
	inline ~ScriptObject()
	{
		const SQBool lostAllReferences = sq_release(m_vm, &m_obj);
		if (lostAllReferences)
		{
			//TT_Printf("ScriptObject was deleted");
		}
		
		sq_resetobject(&m_obj);
		m_isValid = false;
	}
	
	inline const ScriptObject& operator=(const ScriptObject& p_rhs)
	{
		if (this == &p_rhs)
		{
			return *this;
		}
		sq_release(m_vm, &m_obj);
		m_vm      = p_rhs.m_vm;
		m_obj     = p_rhs.m_obj;
		m_isValid = p_rhs.m_isValid;
#if !defined(TT_BUILD_FINAL)
		m_debugName = p_rhs.m_debugName;
#endif
		sq_addref(m_vm, &m_obj);
		return *this;
	}
	
	inline bool isValid()                 const { return m_isValid;    }
	inline bool checkVM(HSQUIRRELVM p_vm) const { return m_vm == p_vm; }
	inline const HSQOBJECT& getObject()   const { TT_ASSERT(isValid()); return m_obj; }
	
	inline void push()
	{
		if (isValid())
		{
			// Push this instance
			sq_pushobject(m_vm, m_obj);
		}
		else
		{
			TT_PANIC("Can't push an invalid ScriptObject!");
		}
	}
	
	inline void getFromStack(SQInteger p_stackIdx, const std::string& p_debugName)
	{
		const SQBool lostAllReferences  = sq_release(m_vm, &m_obj);
		if (lostAllReferences)
		{
			//TT_Printf("ScriptObject was deleted");
		}
		
		m_isValid = false;
		sq_resetobject(&m_obj);
		
		
#if !defined(TT_BUILD_FINAL)
		m_debugName = p_debugName;
#endif
		
		const SQRESULT result = sq_getstackobj(m_vm, p_stackIdx, &m_obj);
		if (SQ_SUCCEEDED(result))
		{
			m_isValid = true;
			sq_addref(m_vm, &m_obj);
		}
		else
		{
			TT_PANIC("sq_getstackobj failed for ScriptObject debugName: '%s'!",
			         p_debugName.c_str());
		}
	}
	
private:
	HSQUIRRELVM m_vm;
	HSQOBJECT   m_obj;
	bool        m_isValid;
#if !defined(TT_BUILD_FINAL)
	std::string m_debugName;
#endif
};

// Namespace end
}
}


#endif //INC_TT_SCRIPT_SCRIPTOBJECT_H
