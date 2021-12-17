/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/


#include <limits>

#include "Gwen/Gwen.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Skin.h"
#include "Gwen/Utility.h"
#include "Gwen/Platform.h"


using namespace Gwen;
using namespace Gwen::Controls;

GWEN_CONTROL_CONSTRUCTOR( TextBoxNumeric ),
m_bIntegerMode( false ),
m_fMinValue( -std::numeric_limits<float>::max() ),
m_fMaxValue(  std::numeric_limits<float>::max() )
{
	SetText( L"0" );
}

void TextBoxNumeric::SetIntegerMode( bool bIntegerMode )
{
	m_bIntegerMode = bIntegerMode;
	// FIXME: Re-validate string?
}

void TextBoxNumeric::SetMinValue( float fMinValue )
{
	m_fMinValue = fMinValue;
	// FIXME: Re-validate string?
}

void TextBoxNumeric::SetMaxValue( float fMaxValue )
{
	m_fMaxValue = fMaxValue;
	// FIXME: Re-validate string?
}

bool TextBoxNumeric::IsTextAllowed( const Gwen::UnicodeString & str, int iPos )
{
	const UnicodeString & strString = GetText().GetUnicode();

	if ( str.length() == 0 )
	{ return true; }

	for ( size_t i = 0; i < str.length(); i++ )
	{
		if ( str[i] == L'-' )
		{
			// Has to be at the start
			if ( i != 0 || iPos != 0 )
			{ return false; }

			// Can only be one
			if ( m_fMinValue >= 0.0f || std::count( strString.begin(), strString.end(), L'-' ) > 0 )
			{ return false; }

			continue;
		}

		if ( str[i] == L'0' ) { continue; }

		if ( str[i] == L'1' ) { continue; }

		if ( str[i] == L'2' ) { continue; }

		if ( str[i] == L'3' ) { continue; }

		if ( str[i] == L'4' ) { continue; }

		if ( str[i] == L'5' ) { continue; }

		if ( str[i] == L'6' ) { continue; }

		if ( str[i] == L'7' ) { continue; }

		if ( str[i] == L'8' ) { continue; }

		if ( str[i] == L'9' ) { continue; }

		if ( str[i] == L'.' )
		{
			// Already a fullstop or no floating point number allowed
			if ( m_bIntegerMode || std::count( strString.begin(), strString.end(), L'.' ) > 0 )
			{ return false; }

			continue;
		}

		return false;
	}

	{
		UnicodeString newText = GetText().GetUnicode();
		newText.insert( iPos, str );
		const float newValue = Gwen::Utility::Strings::To::Float( newText );
		if (newValue < m_fMinValue || newValue > m_fMaxValue)
		{
			return false;
		}
	}

	return true;
}

float TextBoxNumeric::GetFloatFromText()
{
	float temp = Gwen::Utility::Strings::To::Float( GetText().GetUnicode() );
	return temp;
}
