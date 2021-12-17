/******

Copyright (c) 2009 Juan Linietsky

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

******/

#ifndef SQBIND_H
#define SQBIND_H

#include <vector>
#include <sstream>
#include <string>
#include <stdarg.h>
#include <stdio.h>

#include <tt/platform/tt_error.h>
#include <tt/script/utils.h>

#include "squirrel.h"


/**
	@author Juan Linietsky <reduzio@gmail.com>
*/


#ifdef SQBIND_CUSTOMIZE

#include SQBIND_CUSTOMIZE

#else

#define SQBIND_NEW( T ) new T
#define SQBIND_DELETE( m_instance) delete m_instance
#define SQBIND_INLINE inline
#define SQBIND_DEBUG
#endif

#ifdef SQBIND_NAMESPACE

namespace SQBIND_NAMESPACE {

#endif

/*
	Global functions to register and uninitialized the bindings
*/
void SqBind_RegisterInit(bool* p_init);
void SqBind_UnInitAll();

/*

   Templates for obtaining the "simple type" of a type.
   if T*, const T, const T& or T& is passed, , SqBindSimpleType<T>::type_t will always be T
   
*/
template<class T>
struct SqBindSimpleType {
	
	typedef T type_t;
};

template<class T>
struct SqBindSimpleType<T&> {
	
	typedef T type_t;
};

template<class T>
struct SqBindSimpleType<const T> {
	
	typedef T type_t;
};

template<class T>
struct SqBindSimpleType<const T&> {
	
	typedef T type_t;
};

template<class T>
struct SqBindSimpleType<T*> {
	
	typedef T type_t;
};

template<class T>
struct SqBindSimpleType<const T*> {
	
	typedef T type_t;
};


static SQInteger sqbind_throwerror(HSQUIRRELVM v, const char* p_message, ... )
{
#if !defined(TT_BUILD_FINAL)
	// Display callstack info
	std::ostringstream oss;
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
	
	// Process actual error message
	va_list argp;
	char errbuff[256];
	va_start(argp, p_message);
	vsnprintf(errbuff,255,p_message, argp );
	va_end(argp);
	TT_PANIC("sqbind_throwerror: '%s'\nCallstack:\n\n%s", errbuff, oss.str().c_str());
	
	return sq_throwerror(v,errbuff);
#else
	(void)v;
	(void)p_message;
	return SQ_ERROR;
#endif	// !defined(TT_BUILD_FINAL)
}

/*

  default userdata release hook template for bound types,
  deletes the type correctly
  
 */

template<class T>
struct SqBindAllocator {

	static SQBIND_INLINE T *construct() {
	
		return SQBIND_NEW(T);
	}
	static SQBIND_INLINE T *copy_construct(const T* p_from) {
	
		return SQBIND_NEW(T(*p_from));
	}
	static SQBIND_INLINE bool assign(T* p_val, const T* p_from) {
	
		TT_NULL_ASSERT(p_val);
		TT_NULL_ASSERT(p_from);
		*p_val=*p_from;
		return true;
	}
	static SQBIND_INLINE void destruct(T* p_instance) {
	
		SQBIND_DELETE(p_instance);
	}
	
	static SQBIND_INLINE T& get_empty() {
	
		static T empty;
		return empty;
	}
};



template<class T>
struct SqBindAllocator_AssignOnly {
	static SQBIND_INLINE T *construct() {
		TT_PANIC("No construction allowed");
		return 0;
	}
	static SQBIND_INLINE T *copy_construct(const T*) {
		
		TT_PANIC("No construction allowed");
		return 0;
	}
	static SQBIND_INLINE bool assign(T* p_val, const T* p_from) {
		
		*p_val=*p_from;
		return true;
	}
	static SQBIND_INLINE void destruct(T*) {
		
		TT_PANIC("No destruct allowed");
	}
	
	static SQBIND_INLINE T& get_empty() {
		
		TT_PANIC("No get empty allowed");
		
		static T* empty = 0;
		return *empty;
	}
};



template <typename T>
class TypeToAllocatorTrait
{
public:
	typedef T type;
	typedef SqBindAllocator<T> allocator;
};


template<class T>
class SqBind {
public:
	typedef T Type;
	typedef T* (*Constructor)(HSQUIRRELVM v);
	typedef typename TypeToAllocatorTrait<T>::allocator A;

private:
	static HSQOBJECT class_id;
	static bool initialized;
	static const char *name;
	static bool instantiable;
	static HSQOBJECT string_constructor;
	static HSQOBJECT custom_member_var_table_set;
	static HSQOBJECT custom_member_var_table_get;
	static HSQOBJECT custom_member_var_table_offset;
	static Constructor constructor;
	
	

	static SQInteger default_release_hook(SQUserPointer p_ptr,SQInteger /*p_size*/) {
		
		T *instance = reinterpret_cast<T*>(p_ptr);
		A::destruct(instance);
		return 0; // is it right?
	}	
	
	static SQInteger default_constructor(HSQUIRRELVM v) 
	{
		if (!instantiable) {
			return sqbind_throwerror(v,"Type '%s' is not instantiable.",name);
		}

		T *instance = 0;
		
		SQInteger params=sq_gettop(v);
		
		if (params==0) {
			return sqbind_throwerror(v,"Type '%s' got a strange amount of params.",name);
	
		}
		if (params==1) {
		
			instance=A::construct();
			if (!instance)
				return sqbind_throwerror(v,"Type '%s' does not support default constructor.\n",name);
						
		} else if (params>=2) {
		
			bool call_constructor=true;

			if (params==2) {
				// handle copyconstructor
				SQUserPointer tt=NULL;
				sq_gettypetag(v,2,&tt);
				if (tt && tt == get_typetag()) {
					
					instance=A::copy_construct(&get(v,2));	
					if (!instance)
						return sqbind_throwerror(v,"Type '%s' does not support copy constructor.\n",name);
					call_constructor=false;
				} 
			}
		
			if (call_constructor) {
			
				if (constructor) {
				
					instance=constructor(v);
					if (!instance) {
					
						return sqbind_throwerror(v,"Wrong parameters for type '%s' constructor.",name);	
					}
				} else {
				
					return sqbind_throwerror(v,"Wrong parameter count for type '%s' constructor.",name);			
				}
			}
		}
				
		
		sq_setinstanceup(v, 1, instance);
		sq_setreleasehook(v,1, default_release_hook);
						
		return 0;
	}
		
	static SQInteger default_cloned(HSQUIRRELVM v) {
	
		if (!instantiable) {
			return sqbind_throwerror(v,"Type '%s' is not instantiable/clonable.",name);
		}

		T *self = &SqBind<T>::get(v,1);
		T *from = &SqBind<T>::get(v,2);
		A::assign(self,from);
								
		return 0;
	}

	/* HANDLERS FOR DIRECT TO MEMORY MEMBER VARIABLE SET AND GET */
		
	typedef void (*MemberVarGetFunction)(HSQUIRRELVM,int,int);
	
	template<class I>
	static void sqbind_member_var_get(HSQUIRRELVM v,int p_idx,int p_offset) {
	
		unsigned char *data = (unsigned char*)&get(v,p_idx);
		I *i=(I*)&data[p_offset];
		SqBind<I>::push(v,i);
	}
		
		
	static SQInteger default_member_variable_get(HSQUIRRELVM v) {
	
		sq_pushobject(v,custom_member_var_table_get);
		
		sq_push(v,2); // copy the key
		if (SQ_FAILED( sq_get(v,-2) ))
		{
			sq_pushnull(v);
			return sq_throwobject(v); // key not found (not an errror)
		}
		SQUserPointer usrPtr;
		sq_getuserdata(v,-1, &usrPtr,NULL);
		MemberVarGetFunction* get_func = reinterpret_cast<MemberVarGetFunction*>(usrPtr);
		sq_pop(v,2); // pop userdata and get table
		
		sq_pushobject(v,custom_member_var_table_offset);
		sq_push(v,2); // copy the key
		if (SQ_FAILED( sq_get(v,-2) ))
		{
			return sqbind_throwerror(v,"default_member_variable_get() 'custom_member_var_table_offset' not found");
		}
		SQInteger offset;
		if (SQ_FAILED(sq_getinteger(v,-1,&offset)))
		{
			return sqbind_throwerror(v,"default_member_variable_get() getting offset as integer failed");
		}
		
		(*get_func)(v,1,static_cast<int>(offset));
		return 1;
	}

	typedef void (*MemberVarSetFunction)(HSQUIRRELVM,int,int,int);

	template<class I>
	static void sqbind_member_var_set(HSQUIRRELVM v,int p_idx,int p_from,int p_offset) {
		
		unsigned char *data = (unsigned char*)&SqBind<T>::get(v,p_idx);
		I *i=(I*)&data[p_offset];
		*i=SqBind<I>::get(v,p_from);
	}

	static SQInteger default_member_variable_set(HSQUIRRELVM v) {
	
		sq_pushobject(v,custom_member_var_table_set);
		sq_push(v,2); // copy the key
		
		if (SQ_FAILED( sq_get(v,-2) ))
		{
			sq_pushnull(v);
			return sq_throwobject(v); // key not found (not an errror)
		}
		
		SQUserPointer usrPtr;
		sq_getuserdata(v,-1, &usrPtr,NULL);
		MemberVarSetFunction* set_func = reinterpret_cast<MemberVarSetFunction*>(usrPtr);
		
		sq_pop(v,2); // pop userdata and get table
		sq_pushobject(v,custom_member_var_table_offset);
		sq_push(v,2); // copy the key
		if (SQ_FAILED( sq_get(v,-2) )) {
			return sqbind_throwerror(v,"default_member_variable_set() 'custom_member_var_table_offset' not found");
		}
		
		SQInteger offset;
		if (SQ_FAILED(sq_getinteger(v,-1,&offset)))
		{
			return sqbind_throwerror(v,"default_member_variable_set() getting offset as integer failed");
		}
		
		(*set_func)(v,1,3,static_cast<int>(offset));
		return 0;
	}
	
	static SQInteger default_full_comparator(HSQUIRRELVM v) {
	
		if (!is_type(v,3)) {
		
			return sqbind_throwerror(v,"_cmp: rvalue not of type %s\n",name);
		}
		
		T * a = &get(v,2);
		T * b = &get(v,3);
		
		if (*a < *b )
			return -1;
		else if (*a > *b )
			return 1;
		else return 0;		
	}
	
	static SQInteger default_boolean_comparator(HSQUIRRELVM v) {
	
		if (!is_type(v,3)) {
		
			return sqbind_throwerror(v,"_cmp: rvalue not of type %s\n",name);
		}
		
		T * a = &get(v,2);
		T * b = &get(v,3);
		
		if (*a == *b )
			return 0;
		else 
			return 1;
	}
	
	static void instance_type(HSQUIRRELVM v,HSQOBJECT /*p_type*/) {
#if 0				
		sq_pushobject(v,class_id);
		sq_pushobject(v,string_constructor);
		sq_get(v,-2);
		sq_createinstance(v,-1);
		sq_call(v,1,true,true);
		sq_remove(v,-2); // remove object
#else

		sq_pushobject(v,class_id);
		sq_pushroottable(v);
		sq_call(v,1,true,true);
		sq_remove(v,-2); // remove class
#endif
		
	}


public:	
	
// PUBLIC	
	


	static void push(HSQUIRRELVM v, const T& p_value) {
	
		if (!initialized) {
			sqbind_throwerror(v,"Type '%s', has not been initialized.",name);
			return;
		}
		// create the instance, then assign the value!
		
		instance_type(v,class_id);
		SQUserPointer usrPtr;
		sq_getinstanceup(v,-1, &usrPtr, get_typetag());
		A::assign(reinterpret_cast<T*>(usrPtr),&p_value);
	}
	
	static void push(HSQUIRRELVM v, T* p_value) {
	
		if (!initialized) {
			sqbind_throwerror(v,"Type '%s', has not been initialized.",name);
			return;
		}
		
		if (p_value == 0)
		{
			sq_pushnull(v);
			return;
		}
		
		// create the instance manually and asign the pointer to the exdisting value..
		// note: no release hook is created, so this is created unmanaged.
		sq_pushobject(v,class_id);
		sq_createinstance(v,sq_gettop(v));
		sq_remove(v,-2);
		sq_setinstanceup(v,-1,reinterpret_cast<SQUserPointer>(p_value));
		
	}
	
	static T& get(HSQUIRRELVM v, int p_idx) {
	
		if (!initialized) {
			// will crash anwyay, so who cares..
			sqbind_throwerror(v,"Type '%s', has not been initialized.",name);
			// not the best solution ever
			return A::get_empty();
		}
		
		SQUserPointer usrPtr;
		if (SQ_FAILED(sq_getinstanceup(v,p_idx,&usrPtr,&class_id))) {
		
			sqbind_throwerror(v,"Value at index is not of type '%s'",name);
			// not the best solution ever, again
			return A::get_empty();
		}
		T* up = reinterpret_cast<T*>(usrPtr);
		return *up;
	}
	
	static bool is_type(HSQUIRRELVM v,int p_idx) {
	
		if (!initialized) {
			// will crash anwyay, so who cares..
			sqbind_throwerror(v,"Type '%s', has not been initialized.",name);
			// not the best solution ever
			return false;
		}
		
		SQUserPointer tt=NULL;
		sq_gettypetag(v,p_idx,&tt);
		return (tt && tt == get_typetag());
	}
	
	struct Getter {
	
		SQBIND_INLINE T& get(HSQUIRRELVM v, int p_idx) {
		
			return SqBind<T>::get(v,p_idx);
		}	
	};
	
	struct GetterPtr {
	
		SQBIND_INLINE T* get(HSQUIRRELVM v, int p_idx)
		{
			if (sq_gettype(v, p_idx) == OT_NULL)
			{
				return 0;
			}
			
			return &SqBind<T>::get(v,p_idx);
		}
	};
	
	static void bind_method(HSQUIRRELVM v, const SQChar *p_name, SQFUNCTION p_function,bool p_static=false) {
	
		sq_pushobject(v,class_id);
		sq_pushstring(v,p_name,-1);
		sq_newclosure(v,p_function,0);
		sq_newslot(v,-3,p_static);
		sq_pop(v,1);// pop class
	}
	

	template<class I>
	static void bind_member_variable(HSQUIRRELVM v,const SQChar *p_name,int p_offset) {
	
		// store the offset
		
		sq_pushobject(v,custom_member_var_table_offset);
		sq_pushstring(v,p_name,-1);		
		sq_pushinteger(v,p_offset);
		sq_newslot(v,-3,false); // set the get function
		sq_pop(v,1); // pop get table
		
		// store the get function
		
		sq_pushobject(v,custom_member_var_table_get);
		
		sq_pushstring(v,p_name,-1);
		unsigned char* _get_addr=(unsigned char*)sq_newuserdata(v,sizeof(MemberVarGetFunction));
		
		union {
			MemberVarGetFunction _getfunc;
			unsigned char _bytes[sizeof(MemberVarGetFunction)];
		
		} _get_union;
		
		_get_union._getfunc=sqbind_member_var_get<I>;
		
		for (size_t i=0;i<sizeof(MemberVarGetFunction);i++)
			_get_addr[i]=_get_union._bytes[i];
		
		sq_newslot(v,-3,false); // set the get function
		sq_pop(v,1); // pop get table
		
		//store the set function
				
		sq_pushobject(v,custom_member_var_table_set);
		
		sq_pushstring(v,p_name,-1);
		unsigned char* _set_addr=(unsigned char*)sq_newuserdata(v,sizeof(MemberVarSetFunction));
		
		union {
			MemberVarSetFunction _setfunc;
			unsigned char _bytes[sizeof(MemberVarSetFunction)];
		
		} _set_union;
		
		_set_union._setfunc=sqbind_member_var_set<I>;
		
		for (size_t i=0;i<sizeof(MemberVarSetFunction);i++)
			_set_addr[i]=_set_union._bytes[i];
		
		sq_newslot(v,-3,false); // set the get function
		sq_pop(v,1); // pop get table

	}		

	static HSQOBJECT get_id() {
	
		return class_id;
	}
	
	static HSQOBJECT* get_id_ptr() {
	
		return &class_id;
	}
	
	static SQUserPointer get_typetag() {
	
		// use the address of classid (which is static) as typetag
		return static_cast<SQUserPointer>(&class_id);
	}

	static void set_custom_constructor(Constructor p_constructor) {
	
		constructor=p_constructor;
	}

	static void bind_full_comparator(HSQUIRRELVM v) {
	
		bind_method(v,"_cmp",default_full_comparator,false);
	
	}

	static void bind_bool_comparator(HSQUIRRELVM v) {
	
		bind_method(v,"_cmp",default_boolean_comparator,false);
	
	}

	static void bind_constant(HSQUIRRELVM v,const SQChar *p_name,SQInteger p_value) {
	
		sq_pushobject(v,class_id);
		sq_pushstring(v,p_name,-1);
		sq_pushinteger(v,p_value);
		sq_newslot(v,-3,true);
	
	}

	static void init(HSQUIRRELVM v,const SQChar * p_name,const SQChar *p_base_class_name, bool p_instantiable=true) {

		sq_pushroottable(v);
		sq_pushstring(v,p_base_class_name,-1);
		if (SQ_FAILED( sq_get(v,-2) ) ) {
		
			TT_PANIC("SqBind Error -  Base Class '%s' not in root table (doesn't exist?)", name);		
			sq_pop(v,1);
			return;
		}
		
		HSQOBJECT base_class;
		sq_resetobject(&base_class);
		sq_getstackobj(v,-1,&base_class);
		init(v,p_name,&base_class,p_instantiable);
		sq_pop(v,2); // pop base class and root table
		
	}
	
	static void init(HSQUIRRELVM v,const SQChar * p_name,HSQOBJECT *p_base_class=NULL, bool p_instantiable=true) {
		
 		// already bound
		if (initialized) {
			
			TT_PANIC("SqBind Error - Class '%s' already initialized", name);
			return;
		}
		
		/* CREATE CLASS TABLE */
		name=p_name;
		instantiable=p_instantiable;
		initialized=true;
		SqBind_RegisterInit(&initialized);
		
		// preparate for adding it to globals
		sq_pushroottable(v);
		sq_pushstring(v,p_name,-1);
		if (p_base_class) {
			sq_pushobject(v,*p_base_class);
		}
		sq_newclass(v,p_base_class!=NULL);
		sq_getstackobj(v, -1, &class_id);
		SQRESULT result = sq_settypetag(v, -1, get_typetag());
		TT_ASSERT(SQ_SUCCEEDED(result));
		
		// create the default constructor
		
		sq_pushstring(v,_SC("constructor"),-1);
		sq_resetobject(&string_constructor);
		sq_getstackobj(v,-1,&string_constructor);
		
		sq_newclosure(v,default_constructor,0);
		sq_newslot(v,-3,false); // add the default constructor
		
		sq_pushstring(v,_SC("_cloned"),-1);
		sq_newclosure(v,default_cloned,0);
		sq_newslot(v,-3,false); // add the default cloned
		
		// override _set/_get for the custom ones
		sq_pushstring(v,_SC("_set"),-1);
		sq_newclosure(v,default_member_variable_set,0);
		sq_newslot(v,-3,false);
		
		sq_pushstring(v,_SC("_get"),-1);
		sq_newclosure(v,default_member_variable_get,0);
		sq_newslot(v,-3,false);
		
		sq_resetobject(&custom_member_var_table_set);
		sq_resetobject(&custom_member_var_table_get);
		
		// Copy members
		if (p_base_class)
		{
			// copy set table
			sq_pushobject(v, *p_base_class);
			sq_pushstring(v,_SC("__member_variables_set"), -1);
			sq_get(v,-2);		// get the set table
			sq_remove(v, -2);	// remove base class from stack
			sq_pushstring(v,_SC("__member_variables_set"), -1);
			result = sq_clone(v, -2);	// clone the set table
			TT_ASSERT(SQ_SUCCEEDED(result));
			sq_remove(v, -3);	// remove the original set table from stack
			sq_getstackobj(v,-1,&custom_member_var_table_set);
			sq_newslot(v,-3,true);
			
			// copy get table
			sq_pushobject(v, *p_base_class);
			sq_pushstring(v,_SC("__member_variables_get"), -1);
			sq_get(v,-2);		// get the get table
			sq_remove(v, -2);	// remove base class from stack
			sq_pushstring(v,_SC("__member_variables_get"), -1);
			result = sq_clone(v, -2);	// clone the get table
			TT_ASSERT(SQ_SUCCEEDED(result));
			sq_remove(v, -3);	// remove the original get table from stack
			sq_getstackobj(v,-1,&custom_member_var_table_get);
			sq_newslot(v,-3,true);
			
			// copy offset table
			sq_pushobject(v, *p_base_class);
			sq_pushstring(v,_SC("__member_variables_offset"), -1);
			sq_get(v,-2);		// get the offset table
			sq_remove(v, -2);	// remove base class
			sq_pushstring(v,_SC("__member_variables_offset"), -1);
			result = sq_clone(v, -2);	// clone the offset table
			TT_ASSERT(SQ_SUCCEEDED(result));
			sq_remove(v, -3);	// remove the original offset table from stack
			sq_getstackobj(v,-1,&custom_member_var_table_offset);
			sq_newslot(v,-3,true);
		}
		else // no base class
		{
			// create new set table
			sq_pushstring(v,_SC("__member_variables_set"),-1);
			sq_newtable(v);
			sq_getstackobj(v,-1,&custom_member_var_table_set);
			sq_newslot(v,-3,true);
			
			// create new get table
			sq_pushstring(v,_SC("__member_variables_get"),-1);
			sq_newtable(v);
			sq_getstackobj(v,-1,&custom_member_var_table_get);
			sq_newslot(v,-3,true);
			
			// create new offset table
			sq_pushstring(v,_SC("__member_variables_offset"),-1);
			sq_newtable(v);
			sq_getstackobj(v,-1,&custom_member_var_table_offset);
			sq_newslot(v,-3,true);
		}
		
		sq_newslot(v,-3,false); // add class to the root table
		
		sq_pop(v,1); // pop root table
	}
	

};

// init static variables.

template<class T>
HSQOBJECT SqBind<T>::class_id;
template<class T>
bool SqBind<T>::initialized=false;
template<class T>
const char *SqBind<T>::name="<undefined>";
template<class T>
bool SqBind<T>::instantiable=false;
template<class T>
HSQOBJECT SqBind<T>::custom_member_var_table_set;
template<class T>
HSQOBJECT SqBind<T>::custom_member_var_table_get;
template<class T>
HSQOBJECT SqBind<T>::custom_member_var_table_offset;
template<class T>
HSQOBJECT SqBind<T>::string_constructor;
template<class T>
typename SqBind<T>::Constructor SqBind<T>::constructor=NULL;


// SqBind for std string

template<>
class SqBind<std::string> {
public:
	struct Getter {
		SQBIND_INLINE std::string get(HSQUIRRELVM v, int p_idx) {
			return SqBind<std::string>::get(v,p_idx);
		}
	};
	struct GetterPtr {
		std::string temp;
		SQBIND_INLINE std::string* get(HSQUIRRELVM v, int p_idx) {
			temp=SqBind<std::string>::get(v,p_idx);
			return &temp;
		}
	};
	static std::string get(HSQUIRRELVM v, int p_idx) {
		if (sq_gettype(v,p_idx)!=OT_STRING) {
			sqbind_throwerror(v,"Type is not string!");
			return std::string();
		}
		const SQChar * str;
		if (SQ_FAILED(sq_getstring(v,p_idx,&str)))
		{
			sqbind_throwerror(v, "sq_getstring failed!");
			return std::string();
		}
		return std::string(str);
	}
	
	static void push(HSQUIRRELVM v, const std::string& p_value) {
		sq_pushstring(v,p_value.c_str(),-1);
	}
};

//


#define SQBIND_CLASS_CONSTANT(m_vm,m_class,m_constant) \
	SqBind<m_class>::bind_constant( m_vm, _SC( #m_constant ), m_class::m_constant)


static SQBIND_INLINE void sqbind_bind_constant(HSQUIRRELVM v, const SQChar *p_constant, SQInteger p_value) {

	sq_pushconsttable(v);
	sq_pushstring(v,p_constant,-1);
	sq_pushinteger(v,p_value);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop table

}

#define SQBIND_CONSTANT(m_vm,m_constant) \
	sqbind_bind_constant(m_vm,_SC( #m_constant ), m_constant)


/* C++ native type binding specializations */

// integer binding. This is also very useful to bind enumerations.

#define SQBIND_INTEGER( T )\
template<>\
class SqBind<T> {\
public:\
	struct Getter {\
		SQBIND_INLINE T get(HSQUIRRELVM v, int p_idx) {\
			return SqBind<T>::get(v,p_idx);\
		}\
	};\
	struct GetterPtr {\
		T temp;\
		SQBIND_INLINE T* get(HSQUIRRELVM v, int p_idx) {\
			temp=SqBind<T>::get(v,p_idx);\
			return &temp;\
		}\
	};\
	typedef T Type;\
	static bool is_type(HSQUIRRELVM v, int p_idx) {\
		return (sq_gettype(v,p_idx)&SQOBJECT_NUMERIC) != 0;\
	}\
	static T get(HSQUIRRELVM v, int p_idx) {\
		if (!(sq_gettype(v,p_idx)&SQOBJECT_NUMERIC)) {\
			sqbind_throwerror(v,"Type is not numeric!");\
			return (T)0;\
		}\
		SQInteger ret;\
		sq_getinteger(v,p_idx,&ret);\
		return (T)ret;\
	}\
	\
	static void push(HSQUIRRELVM v, const T& p_value) {\
		sq_pushinteger(v,static_cast<SQInteger>(p_value));\
	}\
	static void push(HSQUIRRELVM v, const T* p_value) {\
		if (!p_value)\
			sq_pushnull(v);\
		else\
			sq_pushinteger(v,static_cast<SQInteger>(*p_value));\
	}\
}

SQBIND_INTEGER(unsigned char);
SQBIND_INTEGER(signed char);
SQBIND_INTEGER(unsigned short);
SQBIND_INTEGER(signed short);
SQBIND_INTEGER(unsigned int);
SQBIND_INTEGER(signed int);
SQBIND_INTEGER(unsigned long);
SQBIND_INTEGER(signed long);


#if defined(_SQ64)

#ifdef _MSVC

SQBIND_INTEGER(__int64);
SQBIND_INTEGER(unsigned __int64);

#else
SQBIND_INTEGER(long long);
SQBIND_INTEGER(unsigned long long);

#endif

#endif // #if defined(_SQ64)

// floating point binding

#define SQBIND_FLOAT( T )\
template<>\
class SqBind<T> {\
public:\
	struct Getter {\
		SQBIND_INLINE T get(HSQUIRRELVM v, int p_idx) {\
			return SqBind<T>::get(v,p_idx);\
		}\
	};\
	struct GetterPtr {\
		T temp;\
		SQBIND_INLINE T* get(HSQUIRRELVM v, int p_idx) {\
			temp=SqBind<T>::get(v,p_idx);\
			return &temp;\
		}\
	};\
	typedef T Type;\
	static bool is_type(HSQUIRRELVM v, int p_idx) {\
		return (sq_gettype(v,p_idx)&SQOBJECT_NUMERIC) != 0;\
	}\
	static T get(HSQUIRRELVM v, int p_idx) {\
		if (!(sq_gettype(v,p_idx)&SQOBJECT_NUMERIC)) {\
			sqbind_throwerror(v,"Type is not numeric!");\
			return 0;\
		}\
		SQFloat ret;\
		sq_getfloat(v,p_idx,&ret);\
		return (T)ret;\
	}\
\
	static void push(HSQUIRRELVM v, const T& p_value) {\
		sq_pushfloat(v,p_value);\
	}\
	static void push(HSQUIRRELVM v, const T* p_value) {\
		if (!p_value)\
			sq_pushnull(v);\
		else\
			sq_pushfloat(v,*p_value);\
	}\
}

SQBIND_FLOAT(float);

#if defined(SQUSEDOUBLE)
SQBIND_FLOAT(double);
SQBIND_FLOAT(long double);
#endif //#if defined(SQUSEDOUBLE)

// bool binding

#define SQBIND_BOOL( T )\
template<>\
class SqBind<T> {\
public:\
	struct Getter {\
		SQBIND_INLINE T get(HSQUIRRELVM v, int p_idx) {\
			return SqBind<T>::get(v,p_idx);\
		}\
	};\
	struct GetterPtr {\
		T temp;\
		SQBIND_INLINE T* get(HSQUIRRELVM v, int p_idx) {\
			temp=SqBind<T>::get(v,p_idx);\
			return &temp;\
		}\
	};\
	typedef T Type;\
	static bool is_type(HSQUIRRELVM v, int p_idx) {\
		if ((sq_gettype(v,p_idx)&SQOBJECT_NUMERIC) != 0) {\
			return true;\
		} else if (sq_gettype(v,p_idx)==OT_BOOL) {\
			return true;\
		} \
		return false;\
	}\
	static T get(HSQUIRRELVM v, int p_idx) {\
		if ((sq_gettype(v,p_idx)&SQOBJECT_NUMERIC) != 0) {\
			SQInteger ret;\
			sq_getinteger(v,p_idx,&ret);\
			return (T)(ret != 0);\
		} else if (sq_gettype(v,p_idx)==OT_BOOL) {\
			SQBool ret;\
			sq_getbool(v,p_idx,&ret);\
			return (T)(ret != 0);\
		} else {		\
			sqbind_throwerror(v,"Type (0x%X) is not numeric!", sq_gettype(v,p_idx));\
			return false;\
		}\
	}\
\
	static void push(HSQUIRRELVM v, const T& p_value) {\
		sq_pushbool(v,p_value);\
	}\
	static void push(HSQUIRRELVM v, const T* p_value) {\
		if (!p_value)\
			sq_pushnull(v);\
		else\
			sq_pushbool(v,*p_value);\
	}\
}

SQBIND_BOOL(bool);

// SqBind for std vector (used for arrays)


template<typename Type>
class SqBind<std::vector<Type> >
{
public:
	
	struct Getter 
	{
		SQBIND_INLINE std::vector<Type> get(HSQUIRRELVM v, int p_idx)
		{
			return SqBind<std::vector<Type> >::get(v,p_idx);
		}
	};
	
	struct GetterPtr
	{
		std::vector<Type> temp;
		SQBIND_INLINE std::vector<Type>* get(HSQUIRRELVM v, int p_idx)
		{
			temp = SqBind<std::vector<Type> >::get(v,p_idx);
			return &temp;
		}
	};
	
	static std::vector<Type> get(HSQUIRRELVM v, int p_idx)
	{
		if (sq_gettype(v,p_idx) != OT_ARRAY )
		{
			sqbind_throwerror(v,"Type is not array!");
			return std::vector<Type>();
		}
		
		std::vector<Type> vec;
		
		// Make sure we have positive stack index 
		SQInteger stackIdx = (p_idx < 0) ? sq_gettop(v) + p_idx + 1 : p_idx;
		
		//push your table/array here
		sq_pushnull(v); //null iterator
		while(SQ_SUCCEEDED(sq_next(v, stackIdx)))
		{
			//here -1 is the value and -2 is the key
			vec.push_back(SqBind<Type>::get(v, -1));
			sq_pop(v, 2); //pops key and val before the next iteration
		}
		sq_pop(v, 1); //pops the null iterator
		
		return vec;
	}
	
	static void push(HSQUIRRELVM v, const std::vector<Type>& p_value)
	{
		sq_newarray(v, static_cast<SQInteger>(p_value.size()));
		SQInteger count = 0;
		for (typename std::vector<Type>::const_iterator it = p_value.begin(); it != p_value.end(); ++it, ++count)
		{
			sq_pushinteger(v, count);
			SqBind<Type>::push(v, (*it));
			sq_set(v, -3);
		}
	}
};


template<typename Type>
class SqBind<std::vector<Type*> >
{
public:
	
	struct Getter 
	{
		SQBIND_INLINE std::vector<Type*> get(HSQUIRRELVM v, int p_idx)
		{
			return SqBind<std::vector<Type*> >::get(v,p_idx);
		}
	};
	
	struct GetterPtr
	{
		std::vector<Type*> temp;
		SQBIND_INLINE std::vector<Type*>* get(HSQUIRRELVM v, int p_idx)
		{
			temp = SqBind<std::vector<Type*> >::get(v,p_idx);
			return &temp;
		}
	};
	
	static std::vector<Type*> get(HSQUIRRELVM v, int p_idx)
	{
		if (sq_gettype(v,p_idx) != OT_ARRAY )
		{
			sqbind_throwerror(v,"Type is not array!");
			return std::vector<Type*>();
		}
		
		std::vector<Type*> vec;
		
		// Make sure we have positive stack index 
		SQInteger stackIdx = (p_idx < 0) ? sq_gettop(v) + p_idx + 1 : p_idx;
		
		//push your table/array here
		sq_pushnull(v); //null iterator
		while(SQ_SUCCEEDED(sq_next(v, stackIdx)))
		{
			//here -1 is the value and -2 is the key
			vec.push_back(SqBind<Type>::get(v, -1));
			sq_pop(v, 2); //pops key and val before the next iteration
		}
		sq_pop(v, 1); //pops the null iterator
		
		return vec;
	}
	
	static void push(HSQUIRRELVM v, const std::vector<Type*>& p_value)
	{
		sq_newarray(v, static_cast<SQInteger>(p_value.size()));
		SQInteger count = 0;
		for (typename std::vector<Type*>::const_iterator it = p_value.begin(); it != p_value.end(); ++it, ++count)
		{
			sq_pushinteger(v, count);
			SqBind<Type>::push(v, (*it));
			sq_set(v, -3);
		}
	}
};


#if 0

// this is an example class on how to bind your own string type to native
// in this example, std::string is used.

template<>
class SqBind<std::string> {
public:
	struct Getter {
		SQBIND_INLINE std::string get(HSQUIRRELVM v, int p_idx) {
			return SqBind<std::string>::get(v,2);
		}
	};
	struct GetterPtr {
		std::string temp;
		SQBIND_INLINE std::string* get(HSQUIRRELVM v, int p_idx) {
			temp=SqBind<std::string>::get(v,2);
			return &temp;
		}
	};
	static std::string get(HSQUIRRELVM v, int p_idx) {
		if (sq_gettype(v,p_idx)!=OT_STRING) {
			sqbind_throwerror(v,"Type is not string!");
			return std::string();
		}
		const SQChar * str;
		if (SQ_FAILED(sq_getstring(v,p_idx,&str)))
		{
			sqbind_throwerror("sq_getstring failed!");
			return std::string();
		}
		return std::string(str);
	}

	static void push(HSQUIRRELVM v, const std::string& p_value) {
		sq_pushstring(v,p_value.c_str(),-1);
	}
};

#endif



/* HELPERS FOR sqbind_method */

template<class M>
void sqbind_push_method_userdata(HSQUIRRELVM v, M method ) {

	union {
	
		M m;
		unsigned char b[sizeof(M)];
	} _mu;
	
	_mu.m=method;
	unsigned char *ptr =(unsigned char*)sq_newuserdata(v, sizeof(M));
	for (size_t i=0;i<sizeof(M);i++)
		ptr[i]=_mu.b[i];			
}

template<class T>
struct SqCParam {
	typename SqBind<T>::Getter getter;
};

template<class T>
struct SqCParam<T&> {
	typename SqBind<T>::Getter getter;
};

template<class T>
struct SqCParam<const T&> {
	typename SqBind<T>::Getter getter;
};

template<class T>
struct SqCParam<const T> {
	typename SqBind<T>::Getter getter;
};

template<class T>
struct SqCParam<T*> {

	typename SqBind<T>::GetterPtr getter;
};

template<class T>
struct SqCParam<const T*> {

	typename SqBind<T>::GetterPtr getter;
};

template<class T, class M>
static void sqbind_push_method_userdata(HSQUIRRELVM v, M method ) {

	union {
	
		M m;
		unsigned char b[sizeof(M)];
	} _mu;
	
	_mu.m=method;
	unsigned char *ptr =(unsigned char*)sq_newuserdata(v,sizeof(M));
	for (size_t i=0;i<sizeof(M);i++)
		ptr[i]=_mu.b[i];			
		
	
}

// little helper



#define _SQBC( T ) SqBind<typename SqBindSimpleType<T>::type_t>

#include "sqmethods.inc"

#ifdef SQBIND_NAMESPACE

}; // end of namespace
#endif


#endif
