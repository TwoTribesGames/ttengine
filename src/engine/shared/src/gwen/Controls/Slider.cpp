/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include <math.h>
#include "Gwen/Controls/Slider.h"

using namespace Gwen;
using namespace Gwen::Controls;
using namespace Gwen::ControlsInternal;

GWEN_CONTROL_CONSTRUCTOR( SliderBar )
{
	SetTarget( this );
	RestrictToParent( true );
}

void SliderBar::Render( Skin::Base* skin )
{
	skin->DrawSlideButton( this, IsDepressed(), IsHorizontal() );
}


GWEN_CONTROL_CONSTRUCTOR( Slider )
{
	SetBounds( Gwen::Rect( 0, 0, 32, 128 ) );
	m_SliderBar = new SliderBar( this );
	m_SliderBar->onDragged.Add( this, &Slider::OnMoved );
	m_SliderBar->onDragStart.Add   ( this, &Slider::OnDragStart );    // Two Tribes addition for onValueChangeStart/Finished
	m_SliderBar->onDragComplete.Add( this, &Slider::OnDragComplete ); // Two Tribes addition for onValueChangeStart/Finished

	m_fMin = 0.0f;
	m_fMax = 1.0f;
	m_bClampToNotches = false;
	m_iNumNotches = 5;
	m_fValue = 0.0f;
	m_dragStarted = false;
	SetTabbable( true );
}

void Slider::OnMoved( Controls::Base* /*control*/ )
{
	SetValueInternal( CalculateValue() );
}

void Slider::Layout( Skin::Base* skin )
{
	BaseClass::Layout( skin );
}

float Slider::CalculateValue()
{
	return 0;
}

void Slider::SetFloatValue( float val, bool /*forceUpdate*/ )
{
	float newVal = val;
	if ( newVal < m_fMin ) { newVal = m_fMin; }

	if ( newVal > m_fMax ) { newVal = m_fMax; }

	// Normalize Value
	newVal = ( newVal - m_fMin ) / ( m_fMax - m_fMin );
	SetValueInternal( newVal, newVal != val);
	Redraw();
}

void Slider::SetValueInternal( float val, bool bForceValueChanged )
{
	if ( m_bClampToNotches )
	{
		val = floorf( ( val * ( float ) m_iNumNotches ) + 0.5f );
		val /= ( float ) m_iNumNotches;
	}
	
	if ( m_fValue != val || bForceValueChanged)
	{
		m_fValue = val;
		onValueChanged.Call( this );
	}

	UpdateBarFromValue();
}


// Two Tribes addition for onValueChangeStart/Finished:
void Slider::OnDragStart( Controls::Base* /*p_control*/ )
{
	m_dragStartValue = m_fValue;
	m_dragStarted = true;
	onValueChangeStart.Call( this );
}


// Two Tribes addition for onValueChangeStart/Finished:
void Slider::OnDragComplete( Controls::Base* /*p_control*/ )
{
	if ( m_fValue != m_dragStartValue && m_dragStarted)
	{
		onValueChangeFinished.Call( this );
	}
}


float Slider::GetFloatValue() const
{
	return m_fMin + ( m_fValue * ( m_fMax - m_fMin ) );
}


// Two Tribes addition
int Slider::GetIntegerValue() const
{
	float value = GetFloatValue();
	value += value > 0.0f ? 0.001 : -0.001;
	
	// Add a small fraction Because of rounding errors
	return static_cast<int>(value);
}


void Slider::SetRange( float fMin, float fMax )
{
	m_fMin = fMin;
	m_fMax = fMax;
}

void Slider::RenderFocus( Gwen::Skin::Base* skin )
{
	if ( Gwen::KeyboardFocus != this ) { return; }

	if ( !IsTabbable() ) { return; }

	skin->DrawKeyboardHighlight( this, GetRenderBounds(), 0 );
}
