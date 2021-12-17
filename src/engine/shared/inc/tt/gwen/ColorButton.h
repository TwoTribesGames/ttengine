#if !defined(INC_TT_GWEN_COLORBUTTON_H)
#define INC_TT_GWEN_COLORBUTTON_H


#include <Gwen/Controls/Button.h>
#include <Gwen/Gwen.h>


namespace tt {
namespace gwen {

/*! \brief A button displaying a single color (useful in combination with a color picker). */
class ColorButton : public Gwen::Controls::Button 
{
	GWEN_CONTROL_INLINE(ColorButton, Gwen::Controls::Button)
	{
		m_color = Gwen::Colors::White;
		SetText("");
	}
	
	inline void Render(Gwen::Skin::Base* p_skin)
	{
		p_skin->DrawColorDisplay(this, m_color);
	}
	
	inline void               SetColor(const Gwen::Color& p_color) { m_color = p_color; }
	inline const Gwen::Color& GetColor() const                     { return m_color;    }
	
private:
	Gwen::Color m_color;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_COLORBUTTON_H)
