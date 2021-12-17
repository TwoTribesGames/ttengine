#if !defined(INC_TOKI_GAME_EDITOR_HOTKEY_HANDLER_H)
#define INC_TOKI_GAME_EDITOR_HOTKEY_HANDLER_H


#include <Gwen/Controls/Base.h>

#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace editor {
namespace hotkey {

class HandlerBase
{
public:
	virtual ~HandlerBase() { }
	
	inline void* getTargetClass() const { return m_targetClass; }
	
	virtual void trigger() = 0;
	
protected:
	HandlerBase(void* p_targetClass)
	:
	m_targetClass(p_targetClass)
	{ }
	
	void* m_targetClass;
};

typedef tt_ptr<HandlerBase>::shared HandlerPtr;


template<typename T>
class Handler : public HandlerBase
{
public:
	typedef void (T::*HandlerFunc)();
	typedef void (T::*HandlerWithSourceFunc)(Gwen::Controls::Base*);
	
	
	inline static HandlerPtr create(T* p_class, HandlerFunc p_func)
	{
		return HandlerPtr(new Handler(p_class, p_func));
	}
	
	inline static HandlerPtr create(T*                    p_class,
	                                HandlerWithSourceFunc p_funcWithSource,
	                                Gwen::Controls::Base* p_source)
	{
		return HandlerPtr(new Handler(p_class, p_funcWithSource, p_source));
	}
	
	virtual ~Handler() { }
	
	virtual void trigger()
	{
		T* target = reinterpret_cast<T*>(m_targetClass);
		if (m_func != 0)
		{
			(target->*m_func)();
		}
		else
		{
			(target->*m_funcWithSource)(m_source);
		}
	}
	
private:
	Handler(T* p_class, HandlerFunc p_func)
	:
	HandlerBase(p_class),
	m_func(p_func),
	m_funcWithSource(0),
	m_source(0)
	{ }
	
	Handler(T* p_class, HandlerWithSourceFunc p_funcWithSource, Gwen::Controls::Base* p_source)
	:
	HandlerBase(p_class),
	m_func(0),
	m_funcWithSource(p_funcWithSource),
	m_source(p_source)
	{ }
	
	
	HandlerFunc           m_func;
	HandlerWithSourceFunc m_funcWithSource;
	Gwen::Controls::Base* m_source;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_HOTKEY_HANDLER_H)
