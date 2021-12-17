/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/Slider.h"
#include "Gwen/Controls/HorizontalSlider.h"

using namespace Gwen;
using namespace Gwen::Controls;
using namespace Gwen::ControlsInternal;


GWEN_CONTROL_CONSTRUCTOR( HorizontalSlider )
{
	m_SliderBar->SetHorizontal( true );
}

float HorizontalSlider::CalculateValue()
{
	return ( float ) m_SliderBar->X() / ( float )( Width() - m_SliderBar->Width() );
}

void HorizontalSlider::UpdateBarFromValue()
{
	m_SliderBar->MoveTo( ( Width() - m_SliderBar->Width() ) * ( m_fValue ), m_SliderBar->Y() );
}

void HorizontalSlider::OnMouseClickLeft( int x, int y, bool bDown )
{
	m_SliderBar->MoveTo( CanvasPosToLocal( Gwen::Point( x, y ) ).x - m_SliderBar->Width() * 0.5f,  m_SliderBar->Y() );
	m_SliderBar->OnMouseClickLeft( x, y, bDown );
	OnMoved( m_SliderBar );
}

void HorizontalSlider::Layout( Skin::Base* /*skin*/ )
{
	m_SliderBar->SetSize( 15, Height() );
}

void HorizontalSlider::Render( Skin::Base* skin )
{
	skin->DrawSlider( this, true, m_bClampToNotches ? m_iNumNotches : 0, m_SliderBar->Width() );
}

// Two Tribes addition to fix resize handling:
void HorizontalSlider::OnBoundsChanged( Gwen::Rect oldBounds )
{
	BaseClass::OnBoundsChanged( oldBounds );

	// If the control's width changed, the slider bar's position must
	// be updated to keep pointing at the actual slider value
	if ( GetBounds().w != oldBounds.w )
	{
		m_SliderBar->SetHeight( Height() );  // always keep the bar the same height as the slider
		UpdateBarFromValue();
	}
}

// Two Tribes addition to fix resize handling:
void HorizontalSlider::OnChildBoundsChanged( Gwen::Rect oldChildBounds, Base* pChild )
{
	BaseClass::OnChildBoundsChanged( oldChildBounds, pChild );

	// If the slider bar's width changed, the bar's position must
	// be updated to keep pointing at the actual slider value
	if ( pChild == m_SliderBar && pChild->GetBounds().w != oldChildBounds.w )
	{
		UpdateBarFromValue();
	}
}
