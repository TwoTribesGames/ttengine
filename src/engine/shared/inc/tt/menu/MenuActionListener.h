#if !defined(INC_TT_MENU_MENUACTIONLISTENER_H)
#define INC_TT_MENU_MENUACTIONLISTENER_H


#include <string>

//#include <tt/platform/tt_types.h>
//#include <tt/memory/HeapMgr.h>


namespace tt {
namespace menu {

class MenuAction;


/*! \brief Menu action listener interface. */
class MenuActionListener
{
public:
	virtual ~MenuActionListener() { }
	
	/* \brief Handle the specified action.
	   \param p_action Action to be performed.
	   \return Wether action has been handled. */
	virtual bool doAction(const MenuAction& p_action) = 0;
	
	/*! \return The name of the listener. */
	virtual std::string getName() const = 0;
	
	
	// All action listeners must be on the safe heap
	/*
	static void* operator new(std::size_t p_blockSize)
	{
		using memory::HeapMgr;
#ifndef TT_BUILD_FINAL
		u32 foo = 0;
		asm	{    mov     foo, lr}
		return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize, 4, (void*)foo);
#else
		return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize);
#endif
	}
	
	static void operator delete(void* p_block)
	{
		memory::HeapMgr::freeToHeap(p_block);
	}
	//*/
	
protected:
	MenuActionListener() { }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUACTIONLISTENER_H)
