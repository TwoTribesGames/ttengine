#if !defined(INC_TT_GWEN_LISTBOXEX_H)
#define INC_TT_GWEN_LISTBOXEX_H


#include <Gwen/Controls/ListBox.h>


namespace tt {
namespace gwen {

/*! \brief List box that also exposes double click and enter events. */
class ListBoxEx : public Gwen::Controls::ListBox
{
public:
	GWEN_CONTROL_INLINE(ListBoxEx, ListBox)
	{ }
	virtual ~ListBoxEx() { }
	
	virtual void OnMouseDoubleClickLeft(int p_x, int p_y)
	{
		BaseClass::OnMouseDoubleClickLeft(p_x, p_y);
		onMouseDoubleClickLeft.Call(this);
	}
	
	virtual bool OnKeyReturn(bool p_down)
	{
		const bool retval = BaseClass::OnKeyReturn(p_down);
		onKeyReturn.Call(this);
		return retval;
	}
	
	virtual bool OnKeyEscape(bool p_down)
	{
		const bool retval = BaseClass::OnKeyEscape(p_down);
		onKeyEscape.Call(this);
		return retval;
	}
	
	
	Gwen::Event::Caller onMouseDoubleClickLeft;
	Gwen::Event::Caller onKeyReturn;
	Gwen::Event::Caller onKeyEscape;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_LISTBOXEX_H)
