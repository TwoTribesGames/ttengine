#include <tt/platform/tt_error.h>

#include <tt/menu/MenuTransition.h>
#include <tt/menu/MenuTreeNode.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/Menu.h>


namespace tt {
namespace menu {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

MenuTransition::MenuTransition(Direction     p_direction,
                               MenuTreeNode* p_currentMenu,
                               MenuTreeNode* p_targetMenu)
:
m_direction(p_direction),
m_currentMenu(p_currentMenu),
m_targetMenu(p_targetMenu),
m_currentRect(math::Point2(0, 0), Screen_Width, Screen_Height),
m_targetRect(math::Point2(0, 0), Screen_Width, Screen_Height),
m_transitionOffset(Screen_Width),
m_transitionState(State_CurrentOut)
{
	TT_ASSERTMSG(m_currentMenu != 0,
	             "Valid current menu needs to be specified.");
	TT_ASSERTMSG(m_targetMenu != 0,
	             "Valid target menu needs to be specified.");
	TT_ASSERTMSG(m_currentMenu->hasMenu(), "Current menu does not exist.");
	
	// Play menu transition sound effect
	MenuSystem::getInstance()->playSound(MenuSound_MenuTransition);
}


MenuTransition::~MenuTransition()
{
}


void MenuTransition::update()
{
	// Don't update if done
	if (m_transitionState == State_Done)
	{
		return;
	}
	
	
	// Check if the state needs to change
	if (m_transitionOffset == 0)
	{
		if (m_transitionState == State_CurrentOut)
		{
			setStateTargetIn();
		}
		else if (m_transitionState == State_TargetIn)
		{
			setStateDone();
		}
	}
	
	// Animate the menu position
	m_transitionOffset = (16 * m_transitionOffset) / 25;
	if (m_transitionOffset <= 2)
	{
		m_transitionOffset = 0;
	}
	
	
	if (m_transitionState == State_CurrentOut)
	{
		// Update the current menu position
		if (m_direction == Direction_Left)
		{
			m_currentRect.setPosition(math::Point2(
				Screen_Width - m_transitionOffset,
				m_currentRect.getPosition().y));
		}
		else if (m_direction == Direction_Right)
		{
			m_currentRect.setPosition(math::Point2(
				-Screen_Width + m_transitionOffset,
				m_currentRect.getPosition().y));
		}
	}
	else if (m_transitionState == State_TargetIn)
	{
		// Update the target menu position
		if (m_direction == Direction_Left)
		{
			m_targetRect.setPosition(math::Point2(
				-m_transitionOffset, m_targetRect.getPosition().y));
		}
		else if (m_direction == Direction_Right)
		{
			m_targetRect.setPosition(math::Point2(
				m_transitionOffset, m_targetRect.getPosition().y));
		}
	}
}


void MenuTransition::render(s32 p_z)
{
	if (m_currentMenu != 0 && m_currentMenu->hasMenu())
	{
		m_currentMenu->getMenu()->render(m_currentRect, p_z);
	}
	
	if (m_targetMenu != 0 && m_targetMenu->hasMenu())
	{
		m_targetMenu->getMenu()->render(m_targetRect, p_z);
	}
}


bool MenuTransition::isDone() const
{
	return m_transitionState == State_Done;
}


MenuTreeNode* MenuTransition::getTargetMenu()
{
	return m_targetMenu;
}


void MenuTransition::setStateTargetIn()
{
	// Destroy the current menu
	m_currentMenu->destroyMenu();
	
	// If moving up in the tree, clear the selection path
	if (m_currentMenu->getParent() == m_targetMenu)
	{
		m_currentMenu->clearSelectionPath();
	}
	m_currentMenu = 0;
	
	// Create the target menu
	m_targetMenu->createMenu();
	
	// Change transition state
	m_transitionState  = State_TargetIn;
	m_transitionOffset = Screen_Width;
}


void MenuTransition::setStateDone()
{
	// Change the transition state
	m_transitionState = State_Done;
}

// Namespace end
}
}
