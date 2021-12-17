#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/TextBox.h>

#include <tt/gwen/ColorRGBAPicker.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace gwen {

//--------------------------------------------------------------------------------------------------
// Alpha Slider

GWEN_CONTROL_CONSTRUCTOR(AlphaSlider)
,
m_selectedDistance(64),
m_depressed(false)
{
	SetSize(32, 128);
	SetMouseInputEnabled(true);
}


void AlphaSlider::Render(Gwen::Skin::Base* p_skin)
{
	for (int y = 0; y < Height(); ++y)
	{
		const float yPercent = (float)y / (float)Height();
		const unsigned char shade(255 - (yPercent * 255));
		p_skin->GetRender()->SetDrawColor( Gwen::Color(shade, shade, shade, 255) );
		p_skin->GetRender()->DrawFilledRect( Gwen::Rect( 5, y, Width() - 10, 1 ) );
	}
	p_skin->GetRender()->DrawLinedRect( Gwen::Rect(5, 0, Width() - 10, Height()) );
	
	const int drawHeight = m_selectedDistance - 3;
	
	// Draw our selectors
	p_skin->GetRender()->SetDrawColor( Gwen::Color( 0, 0, 0, 255 ));
	p_skin->GetRender()->DrawFilledRect( Gwen::Rect( 0, drawHeight + 2, Width(), 1));
	p_skin->GetRender()->DrawFilledRect( Gwen::Rect( 0, drawHeight, 5, 5) );
	p_skin->GetRender()->DrawFilledRect( Gwen::Rect( Width() - 5, drawHeight, 5, 5) );
	p_skin->GetRender()->SetDrawColor( Gwen::Color( 255, 255, 255, 255 ) );
	p_skin->GetRender()->DrawFilledRect( Gwen::Rect( 1, drawHeight + 1, 3, 3 ) );
	p_skin->GetRender()->DrawFilledRect( Gwen::Rect( Width() - 4, drawHeight + 1, 3, 3 ) );
}


void AlphaSlider::OnMouseClickLeft(int p_x, int p_y, bool p_down)
{
	m_depressed = p_down;
	
	Gwen::MouseFocus = m_depressed ? this : 0;
	
	OnMouseMoved(p_x, p_y, 0, 0);
}


unsigned char AlphaSlider::GetAlphaAtHeight(int p_y)
{
	const float percentage = p_y / static_cast<float>(Height());
	return static_cast<unsigned char>(255.0f - (255 * percentage));
}


void AlphaSlider::OnMouseMoved(int p_x, int p_y, int /*p_deltaX*/, int /*p_deltaY*/)
{
	if (m_depressed)
	{
		Gwen::Point cursorPos = CanvasPosToLocal(Gwen::Point(p_x, p_y));
		if (cursorPos.y < 0)        cursorPos.y = 0;
		if (cursorPos.y > Height()) cursorPos.y = Height();
		
		m_selectedDistance = cursorPos.y;
		onSelectionChanged.Call(this);
	}
}

void AlphaSlider::SetAlpha(unsigned char p_alpha, bool p_doEvents)
{
	m_selectedDistance = static_cast<int>(((255 - p_alpha) / 255.0f) * Height());
	if (p_doEvents) onSelectionChanged.Call(this);
}


unsigned char AlphaSlider::GetSelectedAlpha()
{
	return GetAlphaAtHeight(m_selectedDistance);
}


//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(ColorRGBAPicker)
, 
m_lerpBox(0),
m_colorSlider(0),
m_alphaSlider(0),
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
	m_lerpBox->onSelectionChanged.Add(this, &ColorRGBAPicker::ColorBoxChanged);
	m_lerpBox->Dock(Gwen::Pos::Left);
	
	m_colorSlider = new Gwen::Controls::ColorSlider(this);
	m_colorSlider->SetPos(m_lerpBox->Right() + 2, 0);
	m_colorSlider->onSelectionChanged.Add(this, &ColorRGBAPicker::ColorSliderChanged);
	m_colorSlider->SetHeight(Height());
	
	m_alphaSlider = new AlphaSlider(this);
	m_alphaSlider->SetPos(m_colorSlider->Right() + 2, 0);
	m_alphaSlider->onSelectionChanged.Add(this, &ColorRGBAPicker::AlphaSliderChanged);
	m_alphaSlider->SetHeight(Height());
	
	m_after = new Gwen::ControlsInternal::ColorDisplay(this);
	m_after->SetSize(colorDisplaySize.x, colorDisplaySize.y);
	m_after->SetPos(m_alphaSlider->Right() + 2, 2);
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
		numeric->onLostKeyboardFocus.Add(this,    &ColorRGBAPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &ColorRGBAPicker::NumericTyped);
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
		numeric->onLostKeyboardFocus.Add(this,    &ColorRGBAPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &ColorRGBAPicker::NumericTyped);
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
		numeric->onLostKeyboardFocus.Add(this,    &ColorRGBAPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &ColorRGBAPicker::NumericTyped);
	}
	
	y += 20;
	
	{
		Label* label = new Label( this );
		label->SetText(L"A:");
		label->SizeToContents();
		label->SetPos( x, y );
		
		TextBoxNumeric* numeric = new TextBoxNumeric( this );
		numeric->SetName( "AlphaBox" );
		numeric->SetPos( x + 15, y -1  );
		numeric->SetSize( 26, 16 );
		numeric->onKeyboardFocus    .Add(numeric, &TextBoxNumeric::OnSelectAll);
		numeric->onLostKeyboardFocus.Add(this,    &ColorRGBAPicker::NumericTyped);
		numeric->onReturnPressed    .Add(this,    &ColorRGBAPicker::NumericTyped);
	}
}


void ColorRGBAPicker::SetColor(const Gwen::Color& p_color, bool p_onlyHue, bool p_reset)
{
	if (p_reset) m_ignoreCallbacks = true;
	
	m_color = p_color;
	UpdateControls();
	
	if (p_reset) m_before->SetColor(m_color);
	
	m_colorSlider->SetColor(m_color);
	m_alphaSlider->SetAlpha(m_color.a, false);
	m_lerpBox    ->SetColor(m_color,   p_onlyHue);
	
	m_after->SetColor(m_color);
	
	if (p_reset) m_ignoreCallbacks = false;
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

void ColorRGBAPicker::OnBoundsChanged(Gwen::Rect p_oldBounds)
{
	BaseClass::OnBoundsChanged(p_oldBounds);
	if (m_colorSlider != 0) m_colorSlider->SetHeight(Height());
	if (m_alphaSlider != 0) m_alphaSlider->SetHeight(Height());
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void ColorRGBAPicker::ColorBoxChanged(Gwen::Controls::Base* /*p_control*/)
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


void ColorRGBAPicker::ColorSliderChanged(Gwen::Controls::Base* /*p_control*/)
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


void ColorRGBAPicker::AlphaSliderChanged( Gwen::Controls::Base* /*p_control*/ )
{
	if (m_ignoreCallbacks) return;
	
	m_color.a = m_alphaSlider->GetSelectedAlpha();
	
	onColorChanged.Call(this);
	
	UpdateControls();
	Invalidate();
}


void ColorRGBAPicker::NumericTyped(Gwen::Controls::Base* p_control)
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
	else if (box->GetName().find("Alpha") != Gwen::String::npos)
	{
		newColor.a = textValue;
	}
	
	m_ignoreCallbacks = true;
	SetColor(newColor);
	m_ignoreCallbacks = false;
	
	// This is not called automatically anymore, because callbacks are ignored
	onColorChanged.Call(this);
}


void ColorRGBAPicker::UpdateControls()
{
	using Gwen::Controls::TextBoxNumeric;
	TextBoxNumeric* redBox   = gwen_cast<TextBoxNumeric>(FindChildByName("RedBox",   false));
	TextBoxNumeric* greenBox = gwen_cast<TextBoxNumeric>(FindChildByName("GreenBox", false));
	TextBoxNumeric* blueBox  = gwen_cast<TextBoxNumeric>(FindChildByName("BlueBox",  false));
	TextBoxNumeric* alphaBox = gwen_cast<TextBoxNumeric>(FindChildByName("AlphaBox", false));
	
	if (redBox   != 0) redBox  ->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.r)), false);
	if (greenBox != 0) greenBox->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.g)), false);
	if (blueBox  != 0) blueBox ->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.b)), false);
	if (alphaBox != 0) alphaBox->SetText(Gwen::Utility::ToString(static_cast<int>(m_color.a)), false);
	
	//TT_Printf("Setting color to <%u, %u, %u, %u>\n",  m_color.r, m_color.g, m_color.b, m_color.a);
	
	m_after->SetColor(m_color);
}

// Namespace end
}
}
