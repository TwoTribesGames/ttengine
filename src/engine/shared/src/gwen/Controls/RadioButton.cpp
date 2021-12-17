/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include "Gwen/Controls/RadioButton.h"

using namespace Gwen;
using namespace Gwen::Controls;

GWEN_CONTROL_CONSTRUCTOR( RadioButton )
{
	SetSize( 15, 15 );
	SetMouseInputEnabled( true );
	SetTabbable( false );
}

void RadioButton::Render( Skin::Base* skin )
{
	skin->DrawRadioButton( this, IsChecked(), IsDepressed() );
}

