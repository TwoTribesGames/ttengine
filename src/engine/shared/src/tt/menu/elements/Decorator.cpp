#include <tt/platform/tt_error.h>
#include <tt/menu/elements/Decorator.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

Decorator::Decorator(MenuElementInterface* p_targetElement)
:
m_decoratedElement(p_targetElement)
{
	TT_ASSERTMSG(m_decoratedElement != 0,
	             "Target element for decorator must be valid.");
	
	MENU_CREATION_Printf("Decorator::Decorator: Decorating element '%s'.\n",
	                     m_decoratedElement->getName().c_str());
}


Decorator::~Decorator()
{
	MENU_CREATION_Printf("Decorator::~Decorator: "
	                     "Freeing memory for decorated element '%s'.\n",
	                     m_decoratedElement->getName().c_str());
	
	// Free memory for the decorated element
	delete m_decoratedElement;
}


void Decorator::loadResources()
{
	m_decoratedElement->loadResources();
}


void Decorator::unloadResources()
{
	m_decoratedElement->unloadResources();
}


std::string Decorator::getName() const
{
	return m_decoratedElement->getName();
}


void Decorator::doLayout(const PointRect& p_rect)
{
	m_decoratedElement->doLayout(p_rect);
}


void Decorator::dumpLayout() const
{
	m_decoratedElement->dumpLayout();
}


void Decorator::render(const PointRect& p_rect, s32 p_z)
{
	m_decoratedElement->render(p_rect, p_z);
}


void Decorator::update()
{
	m_decoratedElement->update();
}


void Decorator::addAction(const MenuAction& p_action)
{
	m_decoratedElement->addAction(p_action);
}


int Decorator::getActionCount() const
{
	return m_decoratedElement->getActionCount();
}


void Decorator::clearActions()
{
	m_decoratedElement->clearActions();
}


MenuAction Decorator::getAction(int p_index) const
{
	return m_decoratedElement->getAction(p_index);
}


s32 Decorator::getMinimumWidth() const
{
	return m_decoratedElement->getMinimumWidth();
}


s32 Decorator::getMinimumHeight() const
{
	return m_decoratedElement->getMinimumHeight();
}


s32 Decorator::getRequestedWidth() const
{
	return m_decoratedElement->getRequestedWidth();
}


s32 Decorator::getRequestedHeight() const
{
	return m_decoratedElement->getRequestedHeight();
}


s32 Decorator::getRequestedHorizontalPosition() const
{
	return m_decoratedElement->getRequestedHorizontalPosition();
}


s32 Decorator::getRequestedVerticalPosition() const
{
	return m_decoratedElement->getRequestedVerticalPosition();
}


MenuLayout& Decorator::getLayout()
{
	return m_decoratedElement->getLayout();
}


const MenuLayout& Decorator::getLayout() const
{
	return m_decoratedElement->getLayout();
}


bool Decorator::canHaveFocus() const
{
	return m_decoratedElement->canHaveFocus();
}


bool Decorator::isSelected() const
{
	return m_decoratedElement->isSelected();
}


bool Decorator::isDefaultSelected() const
{
	return m_decoratedElement->isDefaultSelected();
}


bool Decorator::isEnabled() const
{
	return m_decoratedElement->isEnabled();
}


bool Decorator::isVisible() const
{
	return m_decoratedElement->isVisible();
}


SelectionCursorType Decorator::wantCursor() const
{
	return m_decoratedElement->wantCursor();
}


bool Decorator::isStylusOnly() const
{
	return m_decoratedElement->isStylusOnly();
}


void Decorator::setSelected(bool p_selected)
{
	m_decoratedElement->setSelected(p_selected);
}


void Decorator::setDefaultSelected(bool p_selected)
{
	m_decoratedElement->setDefaultSelected(p_selected);
}


void Decorator::setEnabled(bool p_enabled)
{
	m_decoratedElement->setEnabled(p_enabled);
}


void Decorator::setVisible(bool p_visible)
{
	m_decoratedElement->setVisible(p_visible);
}


void Decorator::setWantCursor(SelectionCursorType p_wantCursor)
{
	m_decoratedElement->setWantCursor(p_wantCursor);
}


void Decorator::setStylusOnly(bool p_stylusOnly)
{
	m_decoratedElement->setStylusOnly(p_stylusOnly);
}


void Decorator::setUserLoopEnable(bool p_enabled)
{
	m_decoratedElement->setUserLoopEnable(p_enabled);
}


void Decorator::setContainerLoopEnable(bool                  p_enabled,
                                       MenuLayout::OrderType p_parentOrder)
{
	m_decoratedElement->setContainerLoopEnable(p_enabled, p_parentOrder);
}


bool Decorator::isUserLoopEnabled() const
{
	return m_decoratedElement->isUserLoopEnabled();
}


bool Decorator::isContainerLoopEnabled() const
{
	return m_decoratedElement->isContainerLoopEnabled();
}


bool Decorator::onStylusPressed(s32 p_x, s32 p_y)
{
	return m_decoratedElement->onStylusPressed(p_x, p_y);
}


bool Decorator::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	return m_decoratedElement->onStylusDragging(p_x, p_y, p_isInside);
}


bool Decorator::onStylusReleased(s32 p_x, s32 p_y)
{
	return m_decoratedElement->onStylusReleased(p_x, p_y);
}


bool Decorator::onStylusRepeat(s32 p_x, s32 p_y)
{
	return m_decoratedElement->onStylusRepeat(p_x, p_y);
}


bool Decorator::onKeyPressed(const MenuKeyboard& p_keys)
{
	return m_decoratedElement->onKeyPressed(p_keys);
}


bool Decorator::onKeyHold(const MenuKeyboard& p_keys)
{
	return m_decoratedElement->onKeyHold(p_keys);
}


bool Decorator::onKeyReleased(const MenuKeyboard& p_keys)
{
	return m_decoratedElement->onKeyReleased(p_keys);
}


bool Decorator::onKeyRepeat(const MenuKeyboard& p_keys)
{
	return m_decoratedElement->onKeyRepeat(p_keys);
}


bool Decorator::doAction(const MenuElementAction& p_action)
{
	return m_decoratedElement->doAction(p_action);
}


MenuElementInterface* Decorator::getMenuElement(const std::string& p_name)
{
	return m_decoratedElement->getMenuElement(p_name);
}


MenuElementInterface* Decorator::getSelectedElement()
{
	return m_decoratedElement->getSelectedElement();
}


const MenuElementInterface* Decorator::getSelectedElement() const
{
	return m_decoratedElement->getSelectedElement();
}


bool Decorator::getSelectedElementRect(math::PointRect& p_rect) const
{
	return m_decoratedElement->getSelectedElementRect(p_rect);
}


const PointRect& Decorator::getRectangle() const
{
	return m_decoratedElement->getRectangle();
}


void Decorator::setRectangle(const PointRect& p_rect)
{
	m_decoratedElement->setRectangle(p_rect);
}


s32 Decorator::getDepth() const
{
	return m_decoratedElement->getDepth();
}


void Decorator::onLayoutDone()
{
	m_decoratedElement->onLayoutDone();
}


void Decorator::onMenuActivated()
{
	m_decoratedElement->onMenuActivated();
}


void Decorator::onMenuDeactivated()
{
	m_decoratedElement->onMenuDeactivated();
}


void Decorator::setSelectionPath(MenuElementInterface::SelectionPath& p_path)
{
	m_decoratedElement->setSelectionPath(p_path);
}


void Decorator::getSelectionPath(
		MenuElementInterface::SelectionPath& p_path) const
{
	m_decoratedElement->getSelectionPath(p_path);
}


bool Decorator::getSelectionPathForElement(
		MenuElementInterface::SelectionPath& p_path,
		const std::string&                   p_name) const
{
	return m_decoratedElement->getSelectionPathForElement(p_path, p_name);
}


void Decorator::dumpSelectionTree(int p_treeLevel) const
{
	m_decoratedElement->dumpSelectionTree(p_treeLevel);
}


Decorator* Decorator::clone() const
{
	return new Decorator(*this);
}


void Decorator::setParent(MenuElementInterface* p_parent)
{
	m_decoratedElement->setParent(p_parent);
}


MenuElementInterface* Decorator::getParent()
{
	return m_decoratedElement->getParent();
}


const MenuElementInterface* Decorator::getParent() const
{
	return m_decoratedElement->getParent();
}


MenuElementInterface* Decorator::getRoot()
{
	return m_decoratedElement->getRoot();
}


const MenuElementInterface* Decorator::getRoot() const
{
	return m_decoratedElement->getRoot();
}


void Decorator::recalculateChildSelection()
{
	m_decoratedElement->recalculateChildSelection();
}


//------------------------------------------------------------------------------
// Protected member functions

Decorator::Decorator(const Decorator& p_rhs)
:
m_decoratedElement(p_rhs.m_decoratedElement->clone())
{
}

// Namespace end
}
}
}
