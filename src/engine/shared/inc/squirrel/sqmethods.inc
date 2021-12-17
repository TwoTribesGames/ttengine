inline void validate_nargs(HSQUIRRELVM v, int p_nargs)
{
#ifndef TT_BUILD_FINAL
	if (sq_gettop(v) != p_nargs + 2)
	{
		std::string stack = tt::script::getCallStack(v);
		TT_PANIC("Invalid number of arguments. Expected '%d', got '%d'\n\nCallstack:\n%s",
			p_nargs, sq_gettop(v)-2, stack.c_str());
	}
#else
	(void)v;
	(void)p_nargs;
#endif
}

/////////////////////////////////
// SqBind custom squirrel method
////////////////////////////////

template<class T >
SQInteger _sqbind_sqmethod_VM(HSQUIRRELVM v) {

	T* self = &SqBind<T>::get(v, 1);
	typedef int (T::*M)(HSQUIRRELVM);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	sq_poptop(v);		// remove userpointer
	M method = *(reinterpret_cast<M*>(usrPtr));
	
	return (self->*method)(v);
}

template<class T>
void sqbind_sqmethod(HSQUIRRELVM v, const SQChar *p_name,  int (T::*method)(HSQUIRRELVM) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_VM<T>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class
}

inline void sqbind_sqfunction(HSQUIRRELVM v, const SQChar *p_name, SQFUNCTION p_function) 
{
	sq_pushroottable(v);
	sq_pushstring(v, p_name, -1);
	sq_newclosure(v, p_function, 0);
	sq_createslot(v, -3);
	sq_pop(v, 1);
}

template<class T>
void sqbind_static_sqmethod(HSQUIRRELVM v, const SQChar *p_name, SQFUNCTION p_function) 
{
	sq_pushobject(v, SqBind<T>::get_id()); // push class
	sq_pushstring(v, p_name, -1);
	sq_newclosure(v, p_function, 0);
	sq_newslot(v,-3, true);
	sq_pop(v, 1); // pop class
}

/////////////////////
// SqBind 0 arguments 
/////////////////////

template<class T >
SQInteger _sqbind_sqmethod_0(HSQUIRRELVM v)
{
	validate_nargs(v, 0);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)();
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	
	(self->*method)();
	
	return 0;
}

template<class T >
SQInteger _sqbind_sqmethod_0c(HSQUIRRELVM v) 
{
	validate_nargs(v, 0);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)() const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	
	(self->*method)();
	
	return 0;
}

template<class T, class R>
SQInteger _sqbind_sqmethod_0r(HSQUIRRELVM v) 
{
	validate_nargs(v, 0);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)();
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
		
	R r = (self->*method)();		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T, class R>
SQInteger _sqbind_sqmethod_0rc(HSQUIRRELVM v) 
{
	validate_nargs(v, 0);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)() const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
		
	R r = (self->*method)();		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)() )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0<T>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)() const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0c<T>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)() )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0r<T,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)() const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_0rc<T,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM>
SQInteger _sqbind_sqfunction_0(VM v)
{
	validate_nargs(v, 0);
	
	typedef void (*F)();
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	
	(function)();
	
	return 0;
}



template<class VM, class R>
SQInteger _sqbind_sqfunction_0r(VM v)
{
	validate_nargs(v, 0);
	
	typedef R (*F)();
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	
	R r = (function)();
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_0<VM>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(), const HSQOBJECT *p_class_id=NULL ) {

	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_0r<VM,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 1 arguments 
/////////////////////

template<class T,class P1 >
SQInteger _sqbind_sqmethod_1(HSQUIRRELVM v)
{
	validate_nargs(v, 1);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1);	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	
	(self->*method)(p1.getter.get(v,2));
	
	return 0;
}

template<class T,class P1 >
SQInteger _sqbind_sqmethod_1c(HSQUIRRELVM v)
{
	validate_nargs(v, 1);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1) const;	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;

	(self->*method)(p1.getter.get(v,2));
	
	return 0;
}

template<class T,class P1, class R>
SQInteger _sqbind_sqmethod_1r(HSQUIRRELVM v)
{
	validate_nargs(v, 1);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	
	R r = (self->*method)(p1.getter.get(v,2));
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class R>
SQInteger _sqbind_sqmethod_1rc(HSQUIRRELVM v)
{
	validate_nargs(v, 1);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	
	R r = (self->*method)(p1.getter.get(v,2));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1<T,P1>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1c<T,P1>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1r<T,P1,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_1rc<T,P1,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1>
SQInteger _sqbind_sqfunction_1(VM v)
{
	validate_nargs(v, 1);
	
	typedef void (*F)(P1);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;

	(function)(p1.getter.get(v,2));		
	
	return 0;
}



template<class VM,class P1, class R>
SQInteger _sqbind_sqfunction_1r(VM v)
{
	validate_nargs(v, 1);
	
	typedef R (*F)(P1);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	
	R r = (function)(p1.getter.get(v,2));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_1<VM,P1>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_1r<VM,P1,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 2 arguments 
/////////////////////

template<class T,class P1, class P2 >
SQInteger _sqbind_sqmethod_2(HSQUIRRELVM v)
{
	validate_nargs(v, 2);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2);	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	
	return 0;
}

template<class T,class P1, class P2 >
SQInteger _sqbind_sqmethod_2c(HSQUIRRELVM v)
{
	validate_nargs(v, 2);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2) const;	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	
	return 0;
}

template<class T,class P1, class P2, class R>
SQInteger _sqbind_sqmethod_2r(HSQUIRRELVM v)
{
	validate_nargs(v, 2);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class R>
SQInteger _sqbind_sqmethod_2rc(HSQUIRRELVM v)
{
	validate_nargs(v, 2);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2<T,P1, P2>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2c<T,P1, P2>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2r<T,P1, P2,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_2rc<T,P1, P2,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2>
SQInteger _sqbind_sqfunction_2(VM v)
{
	validate_nargs(v, 2);
	
	typedef void (*F)(P1, P2);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	(function)(p1.getter.get(v,2), p2.getter.get(v,3));
	
	return 0;
}



template<class VM,class P1, class P2, class R>
SQInteger _sqbind_sqfunction_2r(VM v)
{
	validate_nargs(v, 2);
	
	typedef R (*F)(P1, P2);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3));		
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_2<VM,P1, P2>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_2r<VM,P1, P2,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 3 arguments 
/////////////////////

template<class T,class P1, class P2, class P3 >
SQInteger _sqbind_sqmethod_3(HSQUIRRELVM v)
{
	validate_nargs(v, 3);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3);	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	
	return 0;
}

template<class T,class P1, class P2, class P3 >
SQInteger _sqbind_sqmethod_3c(HSQUIRRELVM v)
{
	validate_nargs(v, 3);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3) const;	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class R>
SQInteger _sqbind_sqmethod_3r(HSQUIRRELVM v)
{
	validate_nargs(v, 3);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class R>
SQInteger _sqbind_sqmethod_3rc(HSQUIRRELVM v)
{
	validate_nargs(v, 3);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3<T,P1, P2, P3>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3c<T,P1, P2, P3>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3r<T,P1, P2, P3,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_3rc<T,P1, P2, P3,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3>
SQInteger _sqbind_sqfunction_3(VM v)
{
	validate_nargs(v, 3);
	
	typedef void (*F)(P1, P2, P3);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class R>
SQInteger _sqbind_sqfunction_3r(VM v)
{
	validate_nargs(v, 3);
	
	typedef R (*F)(P1, P2, P3);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4));
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_3<VM,P1, P2, P3>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_3r<VM,P1, P2, P3,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 4 arguments 
/////////////////////

template<class T,class P1, class P2, class P3, class P4 >
SQInteger _sqbind_sqmethod_4(HSQUIRRELVM v)
{
	validate_nargs(v, 4);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4);	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4 >
SQInteger _sqbind_sqmethod_4c(HSQUIRRELVM v)
{
	validate_nargs(v, 4);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4) const;	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class R>
SQInteger _sqbind_sqmethod_4r(HSQUIRRELVM v)
{
	validate_nargs(v, 4);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class R>
SQInteger _sqbind_sqmethod_4rc(HSQUIRRELVM v)
{
	validate_nargs(v, 4);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4<T,P1, P2, P3, P4>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4c<T,P1, P2, P3, P4>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4) ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4r<T,P1, P2, P3, P4,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4) const ) {

	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_4rc<T,P1, P2, P3, P4,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3, class P4>
SQInteger _sqbind_sqfunction_4(VM v)
{
	validate_nargs(v, 4);
	
	typedef void (*F)(P1, P2, P3, P4);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class P4, class R>
SQInteger _sqbind_sqfunction_4r(VM v)
{
	validate_nargs(v, 4);
	
	typedef R (*F)(P1, P2, P3, P4);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5));
	_SQBC( R )::push(v, r );
	return 1;
}


//call for functions
template<class VM,class P1, class P2, class P3, class P4>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_4<VM,P1, P2, P3, P4>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_4r<VM,P1, P2, P3, P4,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 5 arguments 
/////////////////////

template<class T,class P1, class P2, class P3, class P4, class P5 >
SQInteger _sqbind_sqmethod_5(HSQUIRRELVM v)
{
	validate_nargs(v, 5);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4, P5);	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5 >
SQInteger _sqbind_sqmethod_5c(HSQUIRRELVM v)
{
	validate_nargs(v, 5);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4, P5) const;	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class R>
SQInteger _sqbind_sqmethod_5r(HSQUIRRELVM v)
{
	validate_nargs(v, 5);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4, P5);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class R>
SQInteger _sqbind_sqmethod_5rc(HSQUIRRELVM v)
{
	validate_nargs(v, 5);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4, P5) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5<T,P1, P2, P3, P4, P5>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5c<T,P1, P2, P3, P4, P5>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5r<T,P1, P2, P3, P4, P5,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_5rc<T,P1, P2, P3, P4, P5,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3, class P4, class P5>
SQInteger _sqbind_sqfunction_5(VM v)
{
	validate_nargs(v, 5);
	
	typedef void (*F)(P1, P2, P3, P4, P5);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class P4, class P5, class R>
SQInteger _sqbind_sqfunction_5r(VM v)
{
	validate_nargs(v, 5);
	
	typedef R (*F)(P1, P2, P3, P4, P5);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6));
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3, class P4, class P5>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4, P5), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_5<VM,P1, P2, P3, P4, P5>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4, class P5,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4, P5), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_5r<VM,P1, P2, P3, P4, P5,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 6 arguments 
/////////////////////

template<class T,class P1, class P2, class P3, class P4, class P5, class P6 >
SQInteger _sqbind_sqmethod_6(HSQUIRRELVM v)
{
	validate_nargs(v, 6);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4, P5, P6);	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6 >
SQInteger _sqbind_sqmethod_6c(HSQUIRRELVM v)
{
	validate_nargs(v, 6);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4, P5, P6) const;	
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	
	return 0;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6, class R>
SQInteger _sqbind_sqmethod_6r(HSQUIRRELVM v)
{
	validate_nargs(v, 6);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4, P5, P6);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6, class R>
SQInteger _sqbind_sqmethod_6rc(HSQUIRRELVM v)
{
	validate_nargs(v, 6);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4, P5, P6) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));		
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5, P6) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6<T,P1, P2, P3, P4, P5, P6>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5, P6) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6c<T,P1, P2, P3, P4, P5, P6>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6 ,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5, P6) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6r<T,P1, P2, P3, P4, P5, P6,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T,class P1, class P2, class P3, class P4, class P5, class P6,class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5, P6) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_6rc<T,P1, P2, P3, P4, P5, P6,R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM,class P1, class P2, class P3, class P4, class P5, class P6>
SQInteger _sqbind_sqfunction_6(VM v)
{
	validate_nargs(v, 6);
	
	typedef void (*F)(P1, P2, P3, P4, P5, P6);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));
	
	return 0;
}



template<class VM,class P1, class P2, class P3, class P4, class P5, class P6, class R>
SQInteger _sqbind_sqfunction_6r(VM v)
{
	validate_nargs(v, 6);
	
	typedef R (*F)(P1, P2, P3, P4, P5, P6);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7));
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM,class P1, class P2, class P3, class P4, class P5, class P6>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4, P5, P6), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_6<VM,P1, P2, P3, P4, P5, P6>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4, class P5, class P6,class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4, P5, P6), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_6r<VM,P1, P2, P3, P4, P5, P6,R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


/////////////////////
// SqBind 7 arguments 
/////////////////////

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7 >
SQInteger _sqbind_sqmethod_7(HSQUIRRELVM v)
{
	validate_nargs(v, 7);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4, P5, P6, P7);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	SqCParam<P7> p7;
	
	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), 
		p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7), p7.getter.get(v,8));
	
	return 0;
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
SQInteger _sqbind_sqmethod_7c(HSQUIRRELVM v)
{
	validate_nargs(v, 7);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef void (T::*M)(P1, P2, P3, P4, P5, P6, P7) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	SqCParam<P6> p7;

	(self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), 
		p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7), p7.getter.get(v,8));
	
	return 0;
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class R>
SQInteger _sqbind_sqmethod_7r(HSQUIRRELVM v)
{
	validate_nargs(v, 7);
	
	T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4, P5, P6, P7);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	SqCParam<P7> p7;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), 
		p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7), p7.getter.get(v,8));
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class R>
SQInteger _sqbind_sqmethod_7rc(HSQUIRRELVM v)
{
	validate_nargs(v, 7);
	
	const T* self = &SqBind<T>::get(v, 1 );
	if (self == 0) return 0;
	typedef R (T::*M)(P1, P2, P3, P4, P5, P6, P7) const;
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1, &usrPtr,NULL);
	M method = *(reinterpret_cast<M*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	SqCParam<P6> p7;
	
	R r = (self->*method)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), 
		p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7), p7.getter.get(v,8));
	_SQBC( R )::push(v, r );
	return 1;
}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5, P6, P7) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_7<T,P1, P2, P3, P4, P5, P6, P7>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  void (T::*method)(P1, P2, P3, P4, P5, P6, P7) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_7c<T,P1, P2, P3, P4, P5, P6, P7>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5, P6, P7) )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_7r<T, P1, P2, P3, P4, P5, P6, P7, R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

template<class T, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class R>
void sqbind_method( HSQUIRRELVM v, const SQChar *p_name,  R (T::*method)(P1, P2, P3, P4, P5, P6, P7) const )
{
	sq_pushobject(v,SqBind<T>::get_id()); // push class
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,method);
	sq_newclosure(v,_sqbind_sqmethod_7rc<T, P1, P2, P3, P4, P5, P6, P7, R>,1);
	sq_newslot(v,-3,false);
	sq_pop(v,1); // pop class

}

// STATIC METHODS / FUNCTIONS

// using vm as template parameter.... 
// otherwise overload won't work

template<class VM, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
SQInteger _sqbind_sqfunction_7(VM v)
{
	validate_nargs(v, 7);
	
	typedef void (*F)(P1, P2, P3, P4, P5, P6, P7);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	SqCParam<P7> p7;
	
	(function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), 
		p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7), p7.getter.get(v,8));
	
	return 0;
}


template<class VM,class P1, class P2, class P3, class P4, class P5, class P6, class P7, class R>
SQInteger _sqbind_sqfunction_7r(VM v)
{
	validate_nargs(v, 7);
	
	typedef R (*F)(P1, P2, P3, P4, P5, P6, P7);
	SQUserPointer usrPtr;
	sq_getuserdata(v,-1,&usrPtr,NULL);
	F function = *(reinterpret_cast<F*>(usrPtr));
	SqCParam<P1> p1;
	SqCParam<P2> p2;
	SqCParam<P3> p3;
	SqCParam<P4> p4;
	SqCParam<P5> p5;
	SqCParam<P6> p6;
	SqCParam<P7> p7;
	
	R r = (function)(p1.getter.get(v,2), p2.getter.get(v,3), p3.getter.get(v,4), 
		p4.getter.get(v,5), p5.getter.get(v,6), p6.getter.get(v,7), p7.getter.get(v,8));
	_SQBC( R )::push(v, r );
	return 1;
}

//call for functions
template<class VM, class P1, class P2, class P3, class P4, class P5, class P6, class P7>
void sqbind_function( VM v, const SQChar *p_name,  void (*function)(P1, P2, P3, P4, P5, P6, P7), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
		
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_7<VM,P1, P2, P3, P4, P5, P6, P7>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class

}


template<class VM, class P1, class P2, class P3, class P4, class P5, class P6, class P7, class R>
void sqbind_function( VM v, const SQChar *p_name,  R (*function)(P1, P2, P3, P4, P5, P6, P7), const HSQOBJECT *p_class_id=NULL )
{
	if (p_class_id)
		sq_pushobject(v,*p_class_id); // push class
	else
		sq_pushroottable(v);
	sq_pushstring(v,p_name,-1);
	sqbind_push_method_userdata(v,function);
	sq_newclosure(v,_sqbind_sqfunction_7r<VM, P1, P2, P3, P4, P5, P6, P7, R>,1);
	sq_newslot(v,-3,p_class_id!=NULL);
	sq_pop(v,1); // pop class
}
