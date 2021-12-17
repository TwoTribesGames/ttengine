#include <tt/platform/tt_error.h>
#include <tt/menu/elements/ShowOneValueText.h>
#include <tt/menu/elements/DynamicLabel.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

ShowOneValueText::ShowOneValueText(const std::string& p_name,
                                   const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout),
m_selectedTextIndex(0),
m_looping(true),
m_label(0),
m_shouldInit(true)
{
	MENU_CREATION_Printf("ShowOneValueText::ShowOneValueText: "
	                     "Element '%s': New ShowOneValueText.\n",
	                     getName().c_str());
	m_label = new DynamicLabel("", p_layout, L"");
	m_label->setHorizontalAlign(engine::glyph::GlyphSet::ALIGN_CENTER);
	
	setCanHaveFocus(false);
}


ShowOneValueText::~ShowOneValueText()
{
	MENU_CREATION_Printf("ShowOneValueChild::~ShowOneValueText: "
	                     "Element '%s': Freeing resources.\n",
	                     getName().c_str());
	delete m_label;
}


void ShowOneValueText::loadResources()
{
	m_label->loadResources();
}


void ShowOneValueText::unloadResources()
{
	m_label->unloadResources();
}


std::string ShowOneValueText::getValue() const
{
	return m_texts.at(m_selectedTextIndex).value;
}


void ShowOneValueText::render(const math::PointRect& p_rect, s32 p_z)
{
	// Do not render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Render text of show_text on label
	m_label->render(p_rect, p_z);
}


void ShowOneValueText::doLayout(const math::PointRect& p_rect)
{
	m_label->doLayout(p_rect);
}


void ShowOneValueText::onLayoutDone()
{
	if (m_initialTextValue.empty() == false)
	{
		selectTextByValue(m_initialTextValue);
	}
	else if (m_texts.empty() == false && m_shouldInit)
	{
		selectTextByIndex(0);
	}
}


void ShowOneValueText::setSelected(bool p_selected)
{
	MenuElement::setSelected(p_selected);
	m_label->setSelected(p_selected);
}


void ShowOneValueText::setEnabled(bool p_enabled)
{
	MenuElement::setEnabled(p_enabled);
	m_label->setEnabled(p_enabled);
}


s32 ShowOneValueText::getMinimumWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		// We want to be able to show at least half of the text
		s32 width = 0;
		
		engine::glyph::GlyphSet* gs = MenuSystem::getInstance()->getGlyphSet();
		
		for (TextVector::const_iterator it = m_texts.begin();
		     it != m_texts.end(); ++it)
		{
			width = std::max(width, gs->getStringPixelWidth((*it).text));
		}
		
		return width / 2;
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("%s has undefined width type!", getName().c_str());
		return 0;
	}
}


s32 ShowOneValueText::getMinimumHeight() const
{
	// Return font height
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		return MenuSystem::getInstance()->getGlyphSet()->getHeight();
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("%s has undefined height type!",
		            getName().c_str());
		return 0;
	}
}


s32 ShowOneValueText::getRequestedWidth() const
{
	// Return width of largest text
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		// Return the largest requested width of the children.
		s32 width = 0;
		
		engine::glyph::GlyphSet* gs = MenuSystem::getInstance()->getGlyphSet();
		
		for (TextVector::const_iterator it = m_texts.begin();
		     it != m_texts.end(); ++it)
		{
			width = std::max(width, gs->getStringPixelWidth((*it).text));
		}
		
		return width;
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("%s has undefined width type!",
		            getName().c_str());
		return 0;
	}
}


s32 ShowOneValueText::getRequestedHeight() const
{
	// Return height of font
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		// Return the largest requested height of the children.
		return MenuSystem::getInstance()->getGlyphSet()->getHeight();
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("%s has undefined height type!",
		            getName().c_str());
		return 0;
	}
}


bool ShowOneValueText::doAction(const MenuElementAction& p_action)
{
	TT_ASSERT(p_action.getTargetElement() == getName());
	
	std::string command(p_action.getCommand());
	if (command == "prev_tab")
	{
		selectPreviousText();
		
		// Return action handled.
		return true;
	}
	else if (command == "next_tab")
	{
		selectNextText();
		
		// Return action handled.
		return true;
	}
	else if (command == "set_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "'%s' set_value command takes 1 param",
		             getName().c_str());
		
		MenuSystem::getInstance()->setMenuVar(p_action.getParameter(0),
		                                      getValue());
		
		// Return action handled.
		return true;
	}
	else if (command == "show_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		             "'%s' show_value command takes 1 param",
		             getName().c_str());
		selectTextByValue(p_action.getParameter(0));
		return true;
	}
	else if (command == "set_value_visible")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "%s: Command 'set_value_visible' takes two parameters: "
		             "the value to show or hide, and whether the value should be visible.",
		             getName().c_str());
		
		// FIXME: Implementation required for this.
		
		return true;
	}
	
	// Return action NOT handled.
	return false;
}


void ShowOneValueText::setInitialTextByValue(const std::string& p_value)
{
	m_initialTextValue = p_value;
}


void ShowOneValueText::selectTextByValue(const std::string& p_value)
{
	// Select the child with the specified value
	s32 index = 0;
	for (TextVector::iterator it = m_texts.begin();
	     it != m_texts.end(); ++it, ++index)
	{
		if ((*it).value == p_value)
		{
			selectTextByIndex(index);
			m_shouldInit = false;
			return;
		}
	}
	
	// Child not found
	TT_PANIC("ShowOneValueText '%s' does not have a text with value '%s'.",
	         getName().c_str(), p_value.c_str());
}


void ShowOneValueText::setUserLoopEnable(bool p_enabled)
{
	m_looping = p_enabled;
}


ShowOneValueText* ShowOneValueText::clone() const
{
	return new ShowOneValueText(*this);
}


void ShowOneValueText::addText(const std::string& p_value, const std::string& p_locID)
{
	ValueText text;
	text.value = p_value;
	text.text  = MenuSystem::getInstance()->translateString(p_locID);
	m_texts.push_back(text);
}


void ShowOneValueText::addText(const std::string& p_value, const std::wstring& p_text)
{
	ValueText text;
	text.value = p_value;
	text.text  = p_text;
	m_texts.push_back(text);
}


void ShowOneValueText::selectNextText()
{
	++m_selectedTextIndex;
	if (m_selectedTextIndex >= m_texts.size())
	{
		if (m_looping)
		{
			m_selectedTextIndex = 0;
		}
		else
		{
			m_selectedTextIndex = m_texts.size() - 1;
		}
	}
	
	m_label->setCaption(m_texts.at(m_selectedTextIndex).text);
}


void ShowOneValueText::selectPreviousText()
{
	if (m_selectedTextIndex == 0)
	{
		if (m_looping)
		{
			m_selectedTextIndex = m_texts.size() - 1;
		}
		else
		{
			//--m_selectedTextIndex;
		}
	}
	else
	{
		--m_selectedTextIndex;
	}
	
	m_label->setCaption(m_texts.at(m_selectedTextIndex).text);
}


void ShowOneValueText::selectTextByIndex(s32 p_index)
{
	m_shouldInit = false;
	TT_ASSERTMSG(p_index < static_cast<s32>(m_texts.size()),
	             "Text index %d out of bounds [0 - %u).",
	             p_index, m_texts.size());
	m_selectedTextIndex = static_cast<TextVector::size_type>(p_index);
	m_label->setCaption(m_texts.at(m_selectedTextIndex).text);
}


engine::glyph::GlyphSet::Alignment ShowOneValueText::getHorizontalAlign() const
{
	return m_label->getHorizontalAlign();
}


engine::glyph::GlyphSet::Alignment ShowOneValueText::getVerticalAlign() const
{
	return m_label->getVerticalAlign();
}


void ShowOneValueText::setHorizontalAlign(engine::glyph::GlyphSet::Alignment p_align)
{
	m_label->setHorizontalAlign(p_align);
}


void ShowOneValueText::setVerticalAlign(engine::glyph::GlyphSet::Alignment p_align)
{
	m_label->setVerticalAlign(p_align);
}


//------------------------------------------------------------------------------
// Protected member functions

ShowOneValueText::ShowOneValueText(const ShowOneValueText& p_rhs)
:
MenuElement(p_rhs),
m_initialTextValue(p_rhs.m_initialTextValue),
m_texts(p_rhs.m_texts),
m_selectedTextIndex(p_rhs.m_selectedTextIndex),
m_looping(p_rhs.m_looping),
m_label(p_rhs.m_label->clone())
{
}

// Namespace end
}
}
}
