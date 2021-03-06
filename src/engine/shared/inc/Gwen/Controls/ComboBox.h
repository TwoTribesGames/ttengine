/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_CONTROLS_COMBOBOX_H
#define GWEN_CONTROLS_COMBOBOX_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Controls/Menu.h"


namespace Gwen
{
	namespace Controls
	{
		class GWEN_EXPORT ComboBox : public Button
		{
			public:

				GWEN_CONTROL( ComboBox, Button );

				virtual void Render( Skin::Base* skin );
				virtual void Layout( Skin::Base* skin );
				virtual void UpdateColours();

				virtual void SelectItem( MenuItem* pItem, bool bFireChangeEvents = true );
				virtual void SelectItemByName( const Gwen::String & name, bool bFireChangeEvents = true );
				virtual Gwen::Controls::Label* GetSelectedItem();

				virtual void OnPress();
				virtual void OnItemSelected( Controls::Base* pControl );
				virtual void OpenList();
				virtual void CloseList();

				virtual void ClearItems();

				virtual MenuItem* AddItem( const TextObject & strLabel, const String & strName = "" );
				virtual bool OnKeyUp( bool bDown );
				virtual bool OnKeyDown( bool bDown );
				virtual bool OnKeyReturn( bool bDown ); // Two Tribes change

				virtual void RenderFocus( Gwen::Skin::Base* skin );
				virtual void OnLostKeyboardFocus();
				virtual void OnKeyboardFocus();
				virtual bool OnChar( Gwen::UnicodeChar c ); // Two Tribes change

				virtual bool IsMenuOpen();

				virtual bool IsMenuComponent() { return true; }

				Gwen::Event::Caller	onSelection;

			protected:
				// FIXME: Two Tribes; replace Menu with more logical ListBox
				Menu* m_Menu;
				MenuItem* m_SelectedItem;
				MenuItem* m_HoveredItem; // Two Tribes change

				Controls::Base*	m_Button;
				
			private:
				void setHoveredItem(MenuItem* p_item);


		};

	}
}
#endif
