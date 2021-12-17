
namespace tt {
namespace script {

inline bool VirtualMachine::hasSqFun(const std::string& p_functionName)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	sq_pushroottable(m_vm);
	sq_pushstring(m_vm, p_functionName.c_str(), -1);
	sq_get(m_vm, -2); //get the function from the root table
	
	const SQObjectType type(sq_gettype(m_vm, sq_gettop(m_vm)));
	return type == OT_CLOSURE || type == OT_NATIVECLOSURE;
}


inline bool VirtualMachine::hasSqMemberFun(const std::string& p_functionName, const HSQOBJECT& p_class)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	// Get function
	sq_pushobject(m_vm, p_class); // Push class here.
	sq_pushstring(m_vm, p_functionName.c_str(), -1);
	sq_get(m_vm, -2); //get the function from the root table
	
	const SQObjectType type(sq_gettype(m_vm, sq_gettop(m_vm)));
	return type == OT_CLOSURE || type == OT_NATIVECLOSURE;
}

inline bool VirtualMachine::prepareFunctionOnStack(const std::string& p_functionName)
{
	sq_pushroottable(m_vm);
	sq_pushstring(m_vm, p_functionName.c_str(), -1);
	sq_get(m_vm, -2); //get the function from the root table
	
	const SQObjectType type(sq_gettype(m_vm, sq_gettop(m_vm)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return false;
	}
	
	sq_pushroottable(m_vm); // this (function environment object)
	return true;
}

inline bool VirtualMachine::prepareMethodOnStack(const std::string& p_methodName,
                                                 const HSQOBJECT& p_class, const HSQOBJECT& p_object)
{
	sq_pushobject(m_vm, p_class); // Push class here.
	sq_pushstring(m_vm, p_methodName.c_str(), -1);
	sq_get(m_vm, -2); //get the function from the class
	
	const SQObjectType type(sq_gettype(m_vm, sq_gettop(m_vm)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return false;
	}
	
	sq_pushobject(m_vm, p_object); // Push this instance
	return true;
}


inline bool VirtualMachine::getValueFromStack(bool* p_value, const SQInteger p_stackIdx)
{
	TT_NULL_ASSERT(p_value);
	SQBool sqType = (*p_value);
	const bool result = SQ_SUCCEEDED(sq_getbool(m_vm, p_stackIdx, &sqType));
	(*p_value) = (sqType != SQFalse);
	return result;
}


template <typename type>
inline bool VirtualMachine::getValueFromStack(type* p_value, const SQInteger p_stackIdx)
{
	TT_NULL_ASSERT(p_value);
	*p_value = SqBind<type>::get(m_vm, p_stackIdx);
	return true;
}


inline bool VirtualMachine::callFunction(const std::string& p_functionName, s32 p_nparams)
{
	if (SQ_FAILED(sq_call(m_vm, p_nparams, SQFalse, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_WARN("Calling squirrel function '%s' failed", p_functionName.c_str());
		return false;
	}
	return true;
}


inline bool VirtualMachine::callFunctionWithReturn(const std::string& p_functionName, s32 p_nparams)
{
	if (SQ_FAILED(sq_call(m_vm, p_nparams, SQTrue, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_WARN("Calling squirrel function '%s' failed", p_functionName.c_str());
		return false;
	}
	return true;
}


//First calling the global squirrel functions

//0 parameters
inline bool VirtualMachine::callSqFun(const std::string& p_functionName)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	return callFunction(p_functionName, 1);
}


template <typename RetType>
inline bool VirtualMachine::callSqFunWithReturn(RetType* p_return_OUT,
                                                const std::string& p_functionName)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	if (callFunctionWithReturn(p_functionName, 1) == false)
	{
		return false;
	}
	
	TT_NULL_ASSERT(p_return_OUT);
	return getValueFromStack(p_return_OUT);
}


//1 parameters
template <typename P1Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                                      const P1Type& p_p1)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	
	return callFunction(p_functionName, 2);
}


template <typename RetType,
          typename P1Type>
inline bool VirtualMachine::callSqFunWithReturn(RetType* p_return_OUT,
                                                const std::string& p_functionName,
                                                const P1Type& p_p1)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	
	if (callFunctionWithReturn(p_functionName, 2) == false)
	{
		return false;
	}
	
	TT_NULL_ASSERT(p_return_OUT);
	return getValueFromStack(p_return_OUT);
}


//2 parameters
template <typename P1Type, typename P2Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                             const P1Type& p_p1,
                             const P2Type& p_p2)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	
	return callFunction(p_functionName, 3);
}


//3 parameters
template <typename P1Type, typename P2Type, typename P3Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                             const P1Type& p_p1,
                             const P2Type& p_p2,
                             const P3Type& p_p3)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	
	return callFunction(p_functionName, 4);
}


//4 parameters
template <typename P1Type, typename P2Type, typename P3Type, typename P4Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                             const P1Type& p_p1,
                             const P2Type& p_p2,
                             const P3Type& p_p3,
                             const P4Type& p_p4)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	
	return callFunction(p_functionName, 5);
}


//5 parameters
template <typename P1Type, typename P2Type, typename P3Type, typename P4Type, typename P5Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                             const P1Type& p_p1,
                             const P2Type& p_p2,
                             const P3Type& p_p3,
                             const P4Type& p_p4,
                             const P5Type& p_p5)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	SqBind<P5Type>::push(m_vm, p_p5);
	
	return callFunction(p_functionName, 6);
}


//6 parameters
template <typename P1Type, 
          typename P2Type, 
          typename P3Type, 
          typename P4Type, 
          typename P5Type, 
          typename P6Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                             const P1Type& p_p1,
                             const P2Type& p_p2,
                             const P3Type& p_p3,
                             const P4Type& p_p4,
                             const P5Type& p_p5,
                             const P6Type& p_p6)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	SqBind<P5Type>::push(m_vm, p_p5);
	SqBind<P6Type>::push(m_vm, p_p6);
	
	return callFunction(p_functionName, 7);
}


//7 parameters
template <typename P1Type,
          typename P2Type,
          typename P3Type,
          typename P4Type,
          typename P5Type,
          typename P6Type,
          typename P7Type>
inline bool VirtualMachine::callSqFun(const std::string& p_functionName,
                             const P1Type& p_p1,
                             const P2Type& p_p2,
                             const P3Type& p_p3,
                             const P4Type& p_p4,
                             const P5Type& p_p5,
                             const P6Type& p_p6,
                             const P7Type& p_p7)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareFunctionOnStack(p_functionName) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	SqBind<P5Type>::push(m_vm, p_p5);
	SqBind<P6Type>::push(m_vm, p_p6);
	SqBind<P7Type>::push(m_vm, p_p7);
	
	return callFunction(p_functionName, 8);
}


//Calling class specific squirrel functions
//0 parameters
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	return callFunction(p_functionName, 1);
}


template <typename RetType>
inline bool VirtualMachine::callSqMemberFunWithReturn(RetType* p_return_OUT,
                                                      const std::string& p_functionName,
                                                      const HSQOBJECT& p_class,
                                                      const HSQOBJECT& p_obj)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	if (callFunctionWithReturn(p_functionName, 1) == false)
	{
		return false;
	}
	
	TT_NULL_ASSERT(p_return_OUT);
	return getValueFromStack(p_return_OUT);
}


//1 parameters
template <typename P1Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	
	return callFunction(p_functionName, 2);
}


template <typename RetType,
          typename P1Type>
inline bool VirtualMachine::callSqMemberFunWithReturn(RetType* p_return_OUT,
                                                      const std::string& p_functionName,
                                                      const HSQOBJECT& p_class,
                                                      const HSQOBJECT& p_obj,
                                                      const P1Type* p_p1)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	
	if (callFunctionWithReturn(p_functionName, 2) == false)
	{
		return false;
	}
	
	TT_NULL_ASSERT(p_return_OUT);
	return getValueFromStack(p_return_OUT);
}


template <typename RetType,
          typename P1Type>
inline bool VirtualMachine::callSqMemberFunWithReturn(RetType* p_return_OUT,
                                                      const std::string& p_functionName,
                                                      const HSQOBJECT& p_class,
                                                      const HSQOBJECT& p_obj,
                                                      const P1Type& p_p1)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	
	if (callFunctionWithReturn(p_functionName, 2) == false)
	{
		return false;
	}
	
	TT_NULL_ASSERT(p_return_OUT);
	return getValueFromStack(p_return_OUT);
}


//2 parameters
template <typename P1Type,
          typename P2Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1,
                                            const P2Type& p_p2)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	
	return callFunction(p_functionName, 3);
}


template <typename P1Type,
          typename P2Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type* p_p1,
                                            const P2Type& p_p2)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	
	return callFunction(p_functionName, 3);
}


//3 parameters
template <typename P1Type,
          typename P2Type,
          typename P3Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1,
                                            const P2Type& p_p2,
                                            const P3Type& p_p3)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	
	return callFunction(p_functionName, 4);
}


//4 parameters
template <typename P1Type,
          typename P2Type,
          typename P3Type,
          typename P4Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1,
                                            const P2Type& p_p2,
                                            const P3Type& p_p3,
                                            const P4Type& p_p4)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	
	return callFunction(p_functionName, 5);
}


//5 parameters
template <typename P1Type,
          typename P2Type,
          typename P3Type,
          typename P4Type,
          typename P5Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1,
                                            const P2Type& p_p2,
                                            const P3Type& p_p3,
                                            const P4Type& p_p4,
                                            const P5Type& p_p5)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	SqBind<P5Type>::push(m_vm, p_p5);
	
	return callFunction(p_functionName, 6);
}


//6 parameters
template <typename P1Type,
          typename P2Type,
          typename P3Type,
          typename P4Type,
          typename P5Type,
          typename P6Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1,
                                            const P2Type& p_p2,
                                            const P3Type& p_p3,
                                            const P4Type& p_p4,
                                            const P5Type& p_p5,
                                            const P6Type& p_p6)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	SqBind<P5Type>::push(m_vm, p_p5);
	SqBind<P6Type>::push(m_vm, p_p6);
	
	return callFunction(p_functionName, 7);
}


//7 parameters
template <typename P1Type,
          typename P2Type,
          typename P3Type,
          typename P4Type,
          typename P5Type,
          typename P6Type,
          typename P7Type>
inline bool VirtualMachine::callSqMemberFun(const std::string& p_functionName,
                                            const HSQOBJECT& p_class,
                                            const HSQOBJECT& p_obj,
                                            const P1Type& p_p1,
                                            const P2Type& p_p2,
                                            const P3Type& p_p3,
                                            const P4Type& p_p4,
                                            const P5Type& p_p5,
                                            const P6Type& p_p6,
                                            const P7Type& p_p7)
{
	tt::script::SqTopRestorerHelper helper(m_vm);
	
	if (prepareMethodOnStack(p_functionName, p_class, p_obj) == false)
	{
		return false;
	}
	
	// Parameters here.
	SqBind<P1Type>::push(m_vm, p_p1);
	SqBind<P2Type>::push(m_vm, p_p2);
	SqBind<P3Type>::push(m_vm, p_p3);
	SqBind<P4Type>::push(m_vm, p_p4);
	SqBind<P5Type>::push(m_vm, p_p5);
	SqBind<P6Type>::push(m_vm, p_p6);
	SqBind<P7Type>::push(m_vm, p_p7);
	
	return callFunction(p_functionName, 8);
}

}
}

