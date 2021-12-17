#if !defined(INC_TT_GWEN_EQUALSIZES_H)
#define INC_TT_GWEN_EQUALSIZES_H


#include <Gwen/Controls/Base.h>


namespace tt {
namespace gwen {

/*! \brief Layout class that fits all children within itself, making each child the same width or height
           (depending on whether the control is horizontal or vertical).
           The other dimension is set to the control's width or height. */
class EqualSizes : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL_INLINE(EqualSizes, Base)
	,
	m_horizontal(true),
	m_innerSpacing(0)
	{
	}
	
	inline void SetHorizontal(bool p_horizontal)
	{
		if (p_horizontal != m_horizontal)
		{
			m_horizontal = p_horizontal;
			Invalidate();
		}
	}
	
	inline bool IsHorizontal() const { return m_horizontal; }
	
	inline void SetInnerSpacing(int p_innerSpacing)
	{
		if (p_innerSpacing >= 0 && p_innerSpacing != m_innerSpacing)
		{
			m_innerSpacing = p_innerSpacing;
			Invalidate();
		}
	}
	
	inline int GetInnerSpacing() const { return m_innerSpacing; }
	
	
	virtual void PostLayout(Gwen::Skin::Base* /*p_skin*/)
	{
		int visibleChildCount = 0;
		for (Base::List::iterator it = Children.begin(); it != Children.end(); ++it)
		{
			if ((*it)->Hidden() == false)
			{
				++visibleChildCount;
			}
		}
		
		if (visibleChildCount == 0)
		{
			return;
		}
		
		const Gwen::Padding& padding(GetPadding());
		
		int innerWidth  = Width()  - padding.left - padding.right;
		int innerHeight = Height() - padding.top  - padding.bottom;
		
		if (m_horizontal)
		{
			innerWidth -= (visibleChildCount - 1) * m_innerSpacing;
			const double widthPerChild = innerWidth / static_cast<double>(visibleChildCount);
			
			double    x = static_cast<double>(padding.left);
			const int y = padding.top;
			
			for (Base::List::iterator it = Children.begin(); it != Children.end(); ++it)
			{
				Base* child = *it;
				if (child->Hidden()) continue;
				
				const Gwen::Margin& childMargin(child->GetMargin());
				
				Gwen::Rect childBounds(child->GetBounds());
				childBounds.x = static_cast<int>(x + childMargin.left);
				childBounds.y = y + childMargin.top;
				childBounds.w = static_cast<int>(widthPerChild - childMargin.left - childMargin.right);
				childBounds.h = innerHeight - childMargin.top - childMargin.bottom;
				child->SetBounds(childBounds);
				
				x += widthPerChild + m_innerSpacing;
			}
		}
		else
		{
			innerHeight -= (visibleChildCount - 1) * m_innerSpacing;
			const double heightPerChild = innerHeight / static_cast<double>(visibleChildCount);
			
			const int x = padding.left;
			double    y = static_cast<double>(padding.top);
			
			for (Base::List::iterator it = Children.begin(); it != Children.end(); ++it)
			{
				Base* child = *it;
				if (child->Hidden()) continue;
				
				const Gwen::Margin& childMargin(child->GetMargin());
				
				Gwen::Rect childBounds(child->GetBounds());
				childBounds.x = x + childMargin.left;
				childBounds.y = static_cast<int>(y + childMargin.top);
				childBounds.w = innerWidth - childMargin.left - childMargin.right;
				childBounds.h = static_cast<int>(heightPerChild - childMargin.top - childMargin.bottom);
				child->SetBounds(childBounds);
				
				y += heightPerChild + m_innerSpacing;
			}
		}
	}
	
private:
	bool m_horizontal;
	int  m_innerSpacing;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_EQUALSIZES_H)
