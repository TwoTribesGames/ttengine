#if !defined(INC_TT_GWEN_COLORRGBAPICKER_H)
#define INC_TT_GWEN_COLORRGBAPICKER_H


#include <Gwen/Controls/Base.h>
#include <Gwen/Controls/ColorControls.h>
#include <Gwen/Controls/ColorPicker.h>

#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace gwen {

/*! \brief Alpha (opacity) value slider. */
class GWEN_EXPORT AlphaSlider : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL(AlphaSlider, Gwen::Controls::Base);
	
	virtual void Render          (Gwen::Skin::Base* p_skin);
	virtual void OnMouseMoved    (int p_x, int p_y, int p_deltaX, int p_deltaY);
	virtual void OnMouseClickLeft(int p_x, int p_y, bool p_down);
	
	unsigned char GetSelectedAlpha();
	unsigned char GetAlphaAtHeight(int p_y);
	void          SetAlpha        (unsigned char p_alpha, bool p_doEvents = true);
	
	Gwen::Event::Caller onSelectionChanged;
	
protected:
	int  m_selectedDistance;
	bool m_depressed;
};


/*! \brief Color picker with alpha support. */
class ColorRGBAPicker : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL(ColorRGBAPicker, Gwen::Controls::Base);
	virtual ~ColorRGBAPicker() { }
	
	inline const Gwen::Color& GetColor() const { return m_color;              }
	inline Gwen::Color GetDefaultColor() const { return m_before->GetColor(); }
	void SetColor(const Gwen::Color& p_color, bool p_onlyHue = false, bool p_reset = false);
	
	Gwen::Event::Caller onColorChanged;
	
protected:
	virtual void OnBoundsChanged(Gwen::Rect p_oldBounds);
	
private:
	void ColorBoxChanged   (Gwen::Controls::Base* p_control);
	void ColorSliderChanged(Gwen::Controls::Base* p_control);
	void AlphaSliderChanged(Gwen::Controls::Base* p_control);
	void NumericTyped      (Gwen::Controls::Base* p_control);
	
	void UpdateControls();
	
	
	Gwen::Controls::ColorLerpBox*         m_lerpBox;
	Gwen::Controls::ColorSlider*          m_colorSlider;
	AlphaSlider*                          m_alphaSlider;
	Gwen::ControlsInternal::ColorDisplay* m_before;
	Gwen::ControlsInternal::ColorDisplay* m_after;
	Gwen::Color                           m_color;
	bool                                  m_ignoreCallbacks;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_COLORRGBAPICKER_H)
