#if !defined(INC_TT_SCRIPT_HELPERS_H)
#define INC_TT_SCRIPT_HELPERS_H

#include <tt/script/VirtualMachine.h>

#define TT_SQBIND_SETVM(vm) \
	HSQUIRRELVM _vm = (vm)->getVM();
	
#define TT_SQBIND_FUNCTION(p_function)	\
	sqbind_function(_vm, _SC(#p_function), &p_function)

#define TT_SQBIND_FUNCTION_NAME(p_function, p_newName)	\
	sqbind_function(_vm, _SC(p_newName), &p_function)

#define TT_SQBIND_OFFSET_OF(st, m) \
	((size_t) ( (char*)&(tt::script::impl::nullPtr<st>()->m) - (char*)0 ))

#define TT_SQBIND_MEMBER(p_class, p_type, p_name)	\
	SqBind<p_class>::bind_member_variable<p_type>(_vm, _SC(#p_name), static_cast<int>(TT_SQBIND_OFFSET_OF( p_class, p_name)) )
	
#define TT_SQBIND_METHOD(p_class, p_method)	\
	sqbind_method<p_class>(_vm, _SC(#p_method), &p_class::p_method)
	
#define TT_SQBIND_METHOD_NAME(p_class, p_method, p_newName)	\
	sqbind_method<p_class>(_vm, _SC(p_newName), &p_class::p_method)
	
#define TT_SQBIND_STATIC_METHOD(p_class, p_method)	\
	sqbind_function(_vm, _SC(#p_method), &p_class::p_method, SqBind<p_class>::get_id_ptr() );
	
#define TT_SQBIND_STATIC_METHOD_NAME(p_class, p_method, p_newName)	\
	sqbind_function(_vm, _SC(p_newName), &p_class::p_method, SqBind<p_class>::get_id_ptr() );
	
#define TT_SQBIND_STATIC_SQUIRREL_METHOD(p_class, p_method) \
	sqbind_static_sqmethod<p_class>(_vm, _SC(#p_method), &p_class::p_method);
	
#define TT_SQBIND_STATIC_SQUIRREL_METHOD_NAME(p_class, p_method, p_newName) \
	sqbind_static_sqmethod<p_class>(_vm, _SC(p_newName), &p_class::p_method);
	
#define TT_SQBIND_INIT(p_class)	\
	SqBind<p_class>::init(_vm, _SC(#p_class))
	
#define TT_SQBIND_INIT_NAME(p_class, p_newName)	\
	SqBind<p_class>::init(_vm, _SC(p_newName))
	
#define TT_SQBIND_INIT_DERIVED(p_class, p_baseClass)	\
	SqBind<p_class>::init(_vm, _SC(#p_class), _SC(#p_baseClass))
	
#define TT_SQBIND_INIT_DERIVED_NAME(p_class, p_baseClass, p_newName)	\
	SqBind<p_class>::init(_vm, _SC(p_newName), _SC(p_baseClass))
	
#define TT_SQBIND_INIT_NO_INSTANCING(p_class)	\
	SqBind<p_class>::init(_vm, _SC(#p_class), static_cast<HSQOBJECT *>(0), false)
	
#define TT_SQBIND_INIT_NO_INSTANCING_NAME(p_class, p_newName)	\
	SqBind<p_class>::init(_vm, _SC(p_newName), static_cast<HSQOBJECT *>(0), false)
	
#define TT_SQBIND_INIT_DERIVED_NO_INSTANCING(p_class, p_baseClass)	\
	SqBind<p_class>::init(_vm, _SC(#p_class), _SC(#p_baseClass), false)
	
#define TT_SQBIND_SET_CONSTRUCTOR(p_class, p_constructor)	\
	SqBind<p_class>::set_custom_constructor( p_constructor )
	
#define TT_SQBIND_SQUIRREL_METHOD(p_class, p_method) \
	sqbind_sqmethod<p_class>(_vm, _SC(#p_method), &p_class::p_method)
	
#define TT_SQBIND_SQUIRREL_FUNCTION(p_function) \
	sqbind_sqfunction(_vm, (#p_function), &p_function);
	
#define TT_SQBIND_SET_ASSIGNONLY(p_class) \
	template <> \
	class TypeToAllocatorTrait<p_class> \
	{	\
		public: \
		typedef p_class type; \
		typedef SqBindAllocator_AssignOnly<p_class> allocator; \
	};
	
#define TT_SQBIND_CONSTANT(p_constant) \
	SQBIND_CONSTANT(_vm, p_constant)

#define TT_SQBIND_CONSTANT_NAME(p_constant, p_newName) \
	sqbind_bind_constant(_vm,_SC(p_newName), p_constant)
	
#define TT_SQBIND_CLASS_CONSTANT(p_class, p_constant) \
	SQBIND_CLASS_CONSTANT(_vm, p_class, p_constant)

#define TT_SQBIND_INTEGER(p_integer) \
	SQBIND_INTEGER( p_integer )

namespace tt {
namespace script {

namespace impl
{
	// 'Private' implementation details
	template <typename Type>
	inline Type* nullPtr() { return 0; }
}

// Namespace end
}
}
	
#endif // !defined(INC_TT_SCRIPT_HELPERS_H)
