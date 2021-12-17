/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/Slider.h"
#include "Gwen/Controls/VerticalSlider.h"

using namespace Gwen;
using namespace Gwen::Controls;
using namespace Gwen::ControlsInternal;


GWEN_CONTROL_CONSTRUCTOR( VerticalSlider )
{
	m_SliderBar->SetHorizontal( false );
}

float VerticalSlider::CalculateValue()
{
	return  1 - ( float ) m_SliderBar->Y() / ( float )( Height() - m_SliderBar->Height() );
}

void VerticalSlider::UpdateBarFromValue()
{
	m_SliderBar->MoveTo( m_SliderBar->X(), ( Height() - m_SliderBar->Height() ) * ( 1 - m_fValue ) );
}


void VerticalSlider::OnMouseClickLeft( int x, int y, bool bDown )
{
	m_SliderBar->MoveTo( m_SliderBar->X(), CanvasPosToLocal( Gwen::Point( x, y ) ).y - m_SliderBar->Height() * 0.5f );
	m_SliderBar->OnMouseClickLeft( x, y, bDown );
	OnMoved( m_SliderBar );
}


void VerticalSlider::Layout( Skin::Base* /*skin*/ )
{
	m_SliderBar->SetSize( Width(), 15 );
}


void VerticalSlider::Render( Skin::Base* skin )
{
	skin->DrawSlider( this, false, m_bClampToNotches ? m_iNumNotches : 0, m_SliderBar->Height() );
}


// Two Tribes addition to fix resize handling:
void VerticalSlider::OnBoundsChanged( Gwen::Rect oldBounds )
{
	BaseClass::OnBoundsChanged( oldBounds );

	// If the control's height changed, the slider bar's position must
	// be updated to keep pointing at the actual slider value
	if ( GetBounds().h != oldBounds.h )
	{
		m_SliderBar->SetWidth( Width() );  // always keep the bar the same width as the slider
		UpdateBarFromValue();
	}
}


// Two Tribes addition to fix resize handling:
void VerticalSlider::OnChildBoundsChanged( Gwen::Rect oldChildBounds, Base* pChild )
{
	BaseClass::OnChildBoundsChanged( oldChildBounds, pChild );

	// If the slider bar's height changed, the bar's position must
	// be updated to keep pointing at the actual slider value
	if ( pChild == m_SliderBar && pChild->GetBounds().h != oldChildBounds.h )
	{
		UpdateBarFromValue();
	}
}
