#include "Gwen/Controls/RGBColorPicker.h"
#include "Gwen/Controls/ColorControls.h"
#include "Gwen/Controls/ColorPicker.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Utility.h"

#include <tt/math/Point2.h>
#include <tt/platform/tt_printf.h>

using namespace Gwen;
using namespace Gwen::Controls;


GWEN_CONTROL_CONSTRUCTOR(RGBColorPicker)
, 
m_lerpBox(0),
m_colorSlider(0),
m_before(0),
m_after(0),
m_color(Gwen::Colors::White),
m_ignoreCallbacks(false)
{
	SetMouseInputEnabled(true);
	SetSize(256, 64);
	SetCacheToTexture();
	
	static const tt::math::Point2 colorDisplaySize(48, 24);
	
	m_lerpBox = new Gwen::Controls::ColorLerpBox(this);
	m_lerpBox->onSelectionChanged.Add(this, &RGBColorPicker::ColorBoxChanged);
	m_lerpBox->Dock(Gwen::Pos::Left);
	
	m_colorSlider = new Gwen::Controls::ColorSlider(this);
	m_colorSlider->SetPos(m_lerpBox->Right() + 2, 0);
	m_colorSlider->onSelectionChanged.Add(this, &RGBColorPicker::ColorSliderChanged);
	m_colorSlider->SetHeight(Height());
	
	m_after = new Gwen::ControlsInternal::ColorDisplay(this);
	m_after->SetSize(colorDisplaySize.x, colorDisplaySize.y);
	m_after->SetPos(m_colorSlider->Right() + 2, 2);
	m_after->SetDrawCheckers(true);
	
	m_before = new Gwen::ControlsInternal::ColorDisplay(this);
	m_before->SetSize(colorDisplaySize.x, colorDisplaySize.y);
	m_before->SetPos(m_after->X(), m_after->Bottom() + 2);
	m_before->SetDrawCheckers(true);
	
	const int x = m_before->X();
	int       y = m_before->Y() + 30;
	
	using Gwen::Controls::Label;
	using Gwen::Controls::TextBoxNumeric;
	
	{
		Label* label = new Label( this );
		label->SetText(L"R:");
		label->SizeToContents();
		label->SetPos( x, y );
		
		TextBoxNumeric* numeric = new TextBoxNumeric( this );
		numeric->SetName( "RedBox" );
		numeric->SetPos( x + 15, y -1  );
		numeric->SetSize( 26, 16 );
		numeric->onKeyboardFocus    .Add(numeric, &TextBoxNumeric::OnSelectAll);
		numeric->onLostKeyboardFocus.Add(this,    &RGBColorPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &RGBColorPicker::NumericTyped);
	}
	
	y += 20;
	
	{
		Label* label = new Label( this );
		label->SetText(L"G:");
		label->SizeToContents();
		label->SetPos( x, y );
		
		TextBoxNumeric* numeric = new TextBoxNumeric( this );
		numeric->SetName( "GreenBox" );
		numeric->SetPos( x + 15, y -1  );
		numeric->SetSize( 26, 16 );
		numeric->onKeyboardFocus    .Add(numeric, &TextBoxNumeric::OnSelectAll);
		numeric->onLostKeyboardFocus.Add(this,    &RGBColorPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &RGBColorPicker::NumericTyped);
	}
	
	y += 20;
	
	{
		Label* label = new Label( this );
		label->SetText(L"B:");
		label->SizeToContents();
		label->SetPos( x, y );
		
		TextBoxNumeric* numeric = new TextBoxNumeric( this );
		numeric->SetName( "BlueBox" );
		numeric->SetPos( x + 15, y -1  );
		numeric->SetSize( 26, 16 );
		numeric->onKeyboardFocus    .Add(numeric, &TextBoxNumeric::OnSelectAll);
		numeric->onLostKeyboardFocus.Add(this,    &RGBColorPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &RGBColorPicker::NumericTyped);
	}
}


void RGBColorPicker::SetColor(const Gwen::Color& p_color, bool p_onlyHue, bool p_reset)
{
	if (p_reset) m_ignoreCallbacks = true;
	
	m_color = p_color;
	UpdateControls();
	
	if (p_reset) m_before->SetColor(m_color);
	
	m_colorSlider->SetColor(m_color);
	m_lerpBox    ->SetColor(m_color,   p_onlyHue);
	
	m_after->SetColor(m_color);
	
	if (p_reset) m_ignoreCallbacks = false;
}


//--------------------------------------------------------------------------------------------------
// Protected member functions


void RGBColorPicker::OnBoundsChanged(Gwen::Rect p_oldBounds)
{
	BaseClass::OnBoundsChanged(p_oldBounds);
	if (m_colorSlider != 0) m_colorSlider->SetHeight(Height());
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void RGBColorPicker::ColorBoxChanged(Gwen::Controls::Base* /*p_control*/)
{
	if (m_ignoreCallbacks) return;
	
	if (m_lerpBox != 0)
	{
		Gwen::Color newColor = m_lerpBox->GetSelectedColor();
		m_color = Gwen::Color(newColor.r, newColor.g, newColor.b, m_color.a);
	}
	
	onColorChanged.Call(this);
	
	UpdateControls();
	Invalidate();
}


void RGBColorPicker::ColorSliderChanged(Gwen::Controls::Base* /*p_control*/)
{
	if (m_ignoreCallbacks) return;
	
	if (m_lerpBox != 0)
	{
		Gwen::Color newColor = m_colorSlider->GetSelectedColor();
		m_color = Gwen::Color(newColor.r, newColor.g, newColor.b, m_color.a);
		m_lerpBox->SetColor(newColor, true);
	}
	Invalidate();
}


void RGBColorPicker::NumericTyped(Gwen::Controls::Base* p_control)
{
	Gwen::Controls::TextBoxNumeric* box = gwen_cast<Gwen::Controls::TextBoxNumeric>(p_control);
	if (box == 0) return;
	
	if (box->GetText().Get().empty()) return;
	
	int textValue = atoi(box->GetText().c_str());
	if (textValue < 0)   textValue = 0;
	if (textValue > 255) textValue = 255;
	
	Gwen::Color newColor = GetColor();
	
	if (box->GetName().find("Red") != Gwen::String::npos)
	{
		newColor.r = textValue;
	}
	else if (box->GetName().find("Green") != Gwen::String::npos)
	{
		newColor.g = textValue;
	}
	else if (box->GetName().find("Blue") != Gwen::String::npos)
	{
		newColor.b = textValue;
	}
	
	m_ignoreCallbacks = true;
	SetColor(newColor);
	m_ignoreCallbacks = false;
	
	// This is not called automatically anymore, because callbacks are ignored
	onColorChanged.Call(this);
}


void RGBColorPicker::UpdateControls()
{
	using Gwen::Controls::TextBoxNumeric;
	TextBoxNumeric* redBox   = gwen_cast<TextBoxNumeric>(FindChildByName("RedBox",   false));
	TextBoxNumeric* greenBox = gwen_cast<TextBoxNumeric>(FindChildByName("GreenBox", false));
	TextBoxNumeric* blueBox  = gwen_cast<TextBoxNumeric>(FindChildByName("BlueBox",  false));
	
	if (redBox   != 0) redBox  ->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.r)), false);
	if (greenBox != 0) greenBox->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.g)), false);
	if (blueBox  != 0) blueBox ->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.b)), false);
	
	//TT_Printf("Setting color to <%u, %u, %u>\n",  m_color.r, m_color.g, m_color.b);
	
	m_after->SetColor(m_color);
}
