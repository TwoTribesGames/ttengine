#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/script/utils.h>
#include <tt/str/str.h>



namespace tt {
namespace script {

const char* const sqObjectTypeName(SQObjectType p_type)
{
	switch (p_type)
	{
	case OT_ARRAY:         return "OT_ARRAY";
	case OT_BOOL:          return "OT_BOOL";
	case OT_CLASS:         return "OT_CLASS";
	case OT_CLOSURE:       return "OT_CLOSURE";
	case OT_FLOAT:         return "OT_FLOAT";
	case OT_FUNCPROTO:     return "OT_FUNCPROTO";
	case OT_GENERATOR:     return "OT_GENERATOR";
	case OT_INSTANCE:      return "OT_INSTANCE";
	case OT_INTEGER:       return "OT_INTEGER";
	case OT_NATIVECLOSURE: return "OT_NATIVECLOSURE";
	case OT_NULL:          return "OT_NULL";
	case OT_OUTER:         return "OT_OUTER";
	case OT_STRING:        return "OT_STRING";
	case OT_TABLE:         return "OT_TABLE";
	case OT_THREAD:        return "OT_THREAD";
	case OT_USERDATA:      return "OT_USERDATA";
	case OT_USERPOINTER:   return "OT_USERPOINTER";
	case OT_WEAKREF:       return "OT_WEAKREF";
	default:
		TT_PANIC("Unknown type '%d'", p_type);
		break;
	}
	return "<Invalid SQObjectType>";
}


#if !defined(TT_BUILD_FINAL)
std::string getType(HSQUIRRELVM p_vm, SQObjectType p_type, const SQChar* p_name, SQInteger p_index)
{
	std::stringstream ss;
	if(p_name != 0)
	{
		ss << "[" << std::string(p_name) << "] ";
	}
	switch(p_type)
	{
		case OT_NULL:
		{
			ss << "NULL" << std::endl;
			
			break;
		}
		case OT_INTEGER:
		{
			SQInteger i;
			if (SQ_SUCCEEDED(sq_getinteger(p_vm, p_index, &i)))
			{
				ss << tt::str::toStr(i) << std::endl;
			}
			else
			{
				ss << "Failed to get integer." << std::endl;
			}
			break;
		}
		case OT_FLOAT:
		{
			SQFloat f;
			if (SQ_SUCCEEDED(sq_getfloat(p_vm, p_index, &f)))
			{
				ss << tt::str::toStr(f) << std::endl;
			}
			else
			{
				ss << "Failed to get float." << std::endl;
			}
			break;
		}
		case OT_USERPOINTER:
		{
			SQUserPointer o = NULL;
			sq_getuserpointer(p_vm, p_index, &o);
			ss << "USERPOINTER; " << tt::str::toStr(o) << std::endl;
			break;
		}
		case OT_STRING:
		{
			const SQChar* s;
			if(SQ_FAILED(sq_getstring(p_vm, p_index, &s)))
			{
				ss << "Failed to get string at index " << tt::str::toStr(p_index) << std::endl;
			}
			else
			{
				ss << "\"" << std::string(s) << "\"" << std::endl;
			}
			break;
		}
		case OT_TABLE:
		{
			ss << "TABLE" << std::endl;
			break;
		}
		case OT_ARRAY:
		{
			ss << "ARRAY" << std::endl;
			break;
		}
		case OT_CLOSURE:
		{
			ss << "CLOSURE" << std::endl;
			break;
		}
		case OT_NATIVECLOSURE:
		{
			ss << "NATIVECLOSURE" << std::endl;
			break;
		}
		case OT_GENERATOR:
		{
			ss << "GENERATOR" << std::endl;
			break;
		}
		case OT_USERDATA:
		{
			SQUserPointer o = NULL;
			SQUserPointer p = NULL;
			if (SQ_SUCCEEDED(sq_getuserdata(p_vm, p_index, &o, &p)))
			{
				ss << "USERDATA: Payload ;" << tt::str::toStr(o) << 
				      " , Userdata tag; " << tt::str::toStr(p) << std::endl;
			}
			else
			{
				ss << "failed to get userdata" << std::endl;
			}
			break;
		}
		case OT_THREAD:
		{
			ss << "THREAD" << std::endl;
			break;
		}
		case OT_CLASS:
		{
			ss << "CLASS" << std::endl;
			break;
		}
		case OT_INSTANCE:
		{
			SQUserPointer usrPtr;
			sq_getinstanceup(p_vm, p_index, &usrPtr, 0);
			HSQOBJECT obj;
			sq_getstackobj(p_vm, p_index, &obj);
			SQUserPointer typeTag;
			sq_getobjtypetag(&obj, &typeTag);
			ss << "INSTANCE [" << obj._unVal.pInstance << "] up [" << usrPtr << "] tag [" << typeTag << "]" << std::endl;
			break;
		}
		case OT_WEAKREF:
		{
			ss << "WEAKREF" << std::endl;
			break;
		}
		case OT_BOOL:
		{
			SQBool sqBool = false;
			if (SQ_SUCCEEDED(sq_getbool(p_vm, p_index, &sqBool)))
			{
				ss << (sqBool ? "true" : "false") << std::endl;
			}
			else
			{
				ss << "sq_getbool failed" << std::endl;
			}
			break;
		}
		default:
		{
			ss << "UNKNOWN" << std::endl;
			TT_PANIC("Unknown Type: %d", p_type);
			break;
		}
	}
	
	return ss.str();
}
#endif // #if !defined(TT_BUILD_FINAL)


std::string getCallStack(HSQUIRRELVM p_vm)
{
#if !defined(TT_BUILD_FINAL)
	std::stringstream ss;
	
	SQInteger    sLevel = 1;  //1 is to skip this function that is level 0
	SQStackInfos si;
	while (SQ_SUCCEEDED(sq_stackinfos(p_vm, sLevel, &si)))
	{
		const SQChar* fn  = _SC("unknown");
		const SQChar* src = _SC("unknown");
		if (si.funcname != 0) fn  = si.funcname;
		if (si.source   != 0) src = si.source;
		ss << "*FUNCTION [" << std::string(fn) << "()] " << std::string(src) << 
			" line [" << tt::str::toStr(si.line) << "]" << std::endl;
		++sLevel;
	}
	
	ss << std::endl << "LOCALS" << std::endl;
	for (SQUnsignedInteger uLevel = 0; uLevel < 10; ++uLevel)
	{
		SQUnsignedInteger seq = 0;
		for (const SQChar* name = sq_getlocal(p_vm, uLevel, seq);
		     name != 0;
		     name = sq_getlocal(p_vm, uLevel, seq))
		{
			++seq;
			ss << getType(p_vm, sq_gettype(p_vm, -1), name);
			sq_pop(p_vm, 1);
		}
	}
	
	return ss.str();
#else
	
	(void)p_vm;
	return std::string();
	
#endif // #if !defined(TT_BUILD_FINAL)
}


std::string getStack(HSQUIRRELVM p_vm)
{
#if !defined(TT_BUILD_FINAL)
	std::stringstream ss;
	
	SQInteger top = sq_gettop(p_vm);
	
	for (SQInteger stackIdx = 1; stackIdx <= top; ++stackIdx)
	{
		std::string indexStr(tt::str::toStr(stackIdx) + " " + tt::str::toStr(stackIdx - (top + 1)));
		const char*  name = indexStr.c_str();
		SQObjectType type = sq_gettype(p_vm, stackIdx);
		ss << getType(p_vm, type, name, stackIdx);
	}
	
	return ss.str();
#else
	
	(void)p_vm;
	return std::string();
	
#endif //#if !defined(TT_BUILD_FINAL)
}


void printCallStack(HSQUIRRELVM p_vm)
{
#if !defined(TT_BUILD_FINAL)
	TT_Printf("%s", getCallStack(p_vm).c_str());
#else
	(void)p_vm;
#endif //#if !defined(TT_BUILD_FINAL)
}


void printStack(HSQUIRRELVM p_vm)
{
#if !defined(TT_BUILD_FINAL)
	TT_Printf("%s", getStack(p_vm).c_str());
#else
	(void)p_vm;
#endif //#if !defined(TT_BUILD_FINAL)
}


// Namespace end
}
}
