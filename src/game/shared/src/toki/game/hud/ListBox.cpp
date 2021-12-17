#include <algorithm>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/input/Button.h>
#include <tt/input/Pointer.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/engine/scene/Camera.h>
#include <tt/math/Vector2.h>

#include <toki/game/hud/ListBox.h>


namespace toki {
namespace game {
namespace hud {


ListBox::ListBox(const tt::engine::glyph::GlyphSetPtr& p_glyphSet, real p_pixelsPerWorldUnit)
:
m_position(0,0),
m_size(200, 500),
m_itemHeight(0.0f),
m_itemSpacing(0.0f, 16.0f),
m_pixelsPerWorldUnit(p_pixelsPerWorldUnit),
m_selectedItemIndex(-1),
m_hoverItemIndex(-1),
m_itemSelectedCallback(0),
m_colorScheme(),
m_background(tt::engine::renderer::QuadSprite::createQuad(m_size.x, m_size.y, m_colorScheme.background)),
m_itemsChanged(false),
m_glyphSet(p_glyphSet),
m_scrollOffset(0.0f),
m_scrollSpeed(1.0f),
m_autoScrollOffset(50.0f / p_pixelsPerWorldUnit),
m_scrollBarWidth(20.0f / p_pixelsPerWorldUnit),
m_previousMouseY(0.0f),
m_scrollBar(tt::engine::renderer::QuadSprite::createQuad(m_scrollBarWidth, m_size.y, m_colorScheme.scrollBar)),
m_scrollBarGrip(tt::engine::renderer::QuadSprite::createQuad(m_scrollBarWidth, 50.0f, m_colorScheme.scrollBarGrip)),
m_scrollBarVisible(false),
m_scrollBarGripActive(false),
m_hasFocus(false)
{
	using tt::engine::renderer::QuadSprite;
	m_background->   setFlag(QuadSprite::Flag_WorldSpace);
	m_scrollBar->    setFlag(QuadSprite::Flag_WorldSpace);
	m_scrollBarGrip->setFlag(QuadSprite::Flag_WorldSpace);
	m_scrollBar->    resetFlag(QuadSprite::Flag_Visible);
	m_scrollBarGrip->resetFlag(QuadSprite::Flag_Visible);
	setSize(m_size);
}


ListBox::~ListBox()
{ }


ListItem* ListBox::addItem(const std::wstring& p_label, u64 p_id)
{
	m_items.push_back(ListItem(p_label, p_id));
	m_itemsChanged = true;

	return &m_items.back();
}


void ListBox::removeItem(ListItem* p_item)
{
	for (ItemList::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (it->id == p_item->id)
		{
			removeItem(it);
			break;
		}
	}
}


void ListBox::removeItemById(u64 p_id)
{
	for (ItemList::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (it->id == p_id)
		{
			removeItem(it);
			break;
		}
	}
}


void ListBox::removeAll()
{
	m_selectedItemIndex = -1;
	m_hoverItemIndex    = -1;
	m_items.clear();
	m_itemsChanged = true;
}


s32 ListBox::selectPrevious()
{
	if (m_selectedItemIndex > 0)
	{
		selectItemByIndex(m_selectedItemIndex - 1);
	}
	return m_selectedItemIndex;
}


s32 ListBox::selectNext()
{
	if (m_selectedItemIndex < static_cast<s32>(m_items.size() - 1))
	{
		selectItemByIndex(m_selectedItemIndex + 1);
	}
	return m_selectedItemIndex;
}


s32 ListBox::selectItem(ListItem* p_item)
{
	for (ItemList::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (p_item == &(*it))
		{
			selectItemByIndex(static_cast<s32>(std::distance(m_items.begin(), it)));
			break;
		}
	}
	return m_selectedItemIndex;
}


ListItem* ListBox::getItemByIndex(s32 p_index)
{
	if (p_index < 0 || p_index >= static_cast<s32>(m_items.size()))
	{
		return 0;
	}
	return &m_items[p_index];
}


ListItem* ListBox::getItemById(u64 p_id)
{
	for (ItemList::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (it->id == p_id)
		{
			return &(*it);
		}
	}
	return 0;
}


ListItem* ListBox::getItemByLabel(const std::wstring& p_label)
{
	for (ItemList::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (it->label == p_label)
		{
			return &(*it);
		}
	}
	return 0;
}


ListItem* ListBox::getSelectedItem()
{
	if (m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<s32>(m_items.size()))
	{
		return &m_items[m_selectedItemIndex];
	}
	return 0;
}


void ListBox::setItemSelectedCallback(ItemSelectedCallback p_callback, void* p_userData)
{
	m_itemSelectedCallback = p_callback;
	m_callbackUserData     = p_userData;
}


bool ListBox::handleInput(const tt::math::Vector3& p_currentPos,
	                      const tt::input::Button&  p_action,
	                      s32 p_wheelNotches)
{
	if (m_scrollBarGripActive)
	{
		if (p_action.released)
		{
			m_scrollBarGripActive = false;
		}
		else
		{
			const real unitsPerItem = m_size.y / m_items.size();
			
			// Move scrollbar in mouse direction
			const real unitsMoved = p_currentPos.y - m_previousMouseY;
			scroll(unitsMoved / unitsPerItem);
			m_previousMouseY = p_currentPos.y;
		}
		return true;
	}
	
	const bool cursorOnControl =
		p_currentPos.x > m_position.x && p_currentPos.x < (m_position.x + m_size.x) &&
		p_currentPos.y < m_position.y && p_currentPos.y > (m_position.y - m_size.y);
	
	const real scrollBarFadeTime = 0.5f;
	
	if (cursorOnControl)
	{
		if (m_scrollBarVisible && m_hasFocus == false &&
			m_scrollBar->checkFlag(tt::engine::renderer::QuadSprite::Flag_FadingIn) == false)
		{
			m_scrollBar->    fadeIn(scrollBarFadeTime, m_colorScheme.scrollBar.a);
			m_scrollBarGrip->fadeIn(scrollBarFadeTime, m_colorScheme.scrollBarGrip.a);
		}
		
		m_background->setColor(m_colorScheme.backgroundHover);

		if (p_currentPos.x > (m_position.x + m_size.x - m_scrollBarWidth) && m_scrollBarVisible)
		{
			// Over scroll control
			const real gripHeight = m_scrollBarGrip->getHeight();
			if (p_currentPos.y > m_scrollBarGrip->getPosition().y - gripHeight * 0.5f &&
				p_currentPos.y < m_scrollBarGrip->getPosition().y + gripHeight * 0.5f)
			{
				m_scrollBarGrip->setColor(m_colorScheme.scrollBarHover);
				if (p_action.pressed)
				{
					// Clicked on grip
					m_scrollBarGripActive = true;
					m_previousMouseY = p_currentPos.y;
				}
			}
		}
		else
		{
			// Compute offset from top of control
			const real offsetFromTop = m_scrollOffset + m_position.y - p_currentPos.y;
			s32 itemIndex = static_cast<s32>(offsetFromTop / m_itemHeight);

			if (itemIndex >= 0 && itemIndex < static_cast<s32>(m_items.size()))
			{
				if (p_action.pressed)
				{
					setHoverItemByIndex(-1);
					selectItemByIndex(itemIndex);
				}
				else
				{
					setHoverItemByIndex(itemIndex);
				}
			}
			else
			{
				setHoverItemByIndex(-1);
			}
		}
		
		if (p_wheelNotches != 0)
		{
			scroll(p_wheelNotches * m_scrollSpeed);
		}
		
		m_hasFocus = true;
	}
	else
	{
		m_background->setColor(m_colorScheme.background);
		setHoverItemByIndex(-1);
		
		if (m_scrollBarVisible && m_hasFocus)
		{
			m_scrollBar->    fadeOut(scrollBarFadeTime);
			m_scrollBarGrip->fadeOut(scrollBarFadeTime);
		}
		
		m_hasFocus = false;
		return false;
	}
	
	return true;
}


void ListBox::update()
{
	m_scrollBar->update();
	m_scrollBarGrip->update();
}


void ListBox::render()
{
	if (m_itemsChanged)
	{
		createGraphics();
		positionItems();
		m_itemsChanged = false;
	}

	using namespace tt::engine::renderer;
	Renderer* renderer(Renderer::getInstance());
	
	// Determine screen position of the ListBox control
	ViewPortID viewportID = renderer->getActiveViewPort();
	tt::engine::scene::CameraPtr camera = ViewPort::getViewPorts()[viewportID].getCamera();
	
	tt::math::Point2 minPosition;
	camera->convert3Dto2D(m_position, minPosition.x, minPosition.y);
	
	tt::math::Point2 maxPosition;
	camera->convert3Dto2D(
		tt::math::Vector3(m_position.x + m_size.x, m_position.y - m_size.y, m_position.z),
		maxPosition.x, maxPosition.y);
	
	// Clamp to screen
	tt::math::clamp(minPosition.x, s32(0), renderer->getScreenWidth()  - 1);
	tt::math::clamp(maxPosition.x, s32(0), renderer->getScreenWidth()  - 1);
	tt::math::clamp(minPosition.y, s32(0), renderer->getScreenHeight() - 1);
	tt::math::clamp(maxPosition.y, s32(0), renderer->getScreenHeight() - 1);

	const tt::math::PointRect scissorRect(minPosition, maxPosition);
	
	if (scissorRect.hasArea() == false)
	{
		// Not on screen
		return;
	}
	
	renderer->setScissorRect(scissorRect);
	
	if (m_background != 0)
	{
		m_background->render();
	}
	
	for (ItemGraphics::iterator it = m_listItemGraphics.begin(); it != m_listItemGraphics.end(); ++it)
	{
		if (it->background->getPosition().y > (m_position.y + m_itemHeight))
		{
			// Skip items above listbox area
			continue;
		}
		
		if (it->background->getPosition().y < (m_position.y - m_size.y - m_itemHeight))
		{
			// Stop when below listbox area
			break;
		}
		
		TT_NULL_ASSERT(it->background);
		it->background->render();
		
		TT_NULL_ASSERT(it->textLabel);
		it->textLabel->render();
	}
	
	if (m_scrollBarVisible)
	{
		m_scrollBar->render();
		m_scrollBarGrip->render();
	}
	
	renderer->resetScissorRect();
}


void ListBox::setColorScheme(const ListBoxColorScheme& p_colorScheme)
{
	m_colorScheme = p_colorScheme;
	createGraphics();
}


void ListBox::setPosition(const tt::math::Vector3& p_position)
{
	m_position = p_position;

	const real verticalPosition = m_position.y - 0.5f * m_size.y;
	m_background->setPosition(m_position.x + 0.5f * m_size.x, verticalPosition, 0);
	m_background->update();
	m_scrollBar->setPosition(m_position.x + m_size.x - (m_scrollBarWidth * 0.5f), verticalPosition, 0);
	m_scrollBar->update();
}


void ListBox::setSize(const tt::math::Vector2& p_size)
{
	m_size = p_size;
	m_background->setWidth (m_size.x);
	m_background->setHeight(m_size.y);
	m_scrollBar->setHeight (m_size.y);
	setPosition(m_position);
}


//--------------------------------------------------------------------------------------------------

void ListBox::createGraphics()
{
	u64 selectedId(0);
	bool selectedIdValid(false);
	ListItem* selectedItem = getSelectedItem();
	
	if (selectedItem != 0)
	{
		selectedId = selectedItem->id;
		selectedIdValid = true;
	}
	
	m_background   ->setColor(m_colorScheme.background);
	m_scrollBar    ->setColor(m_colorScheme.scrollBar);
	m_scrollBarGrip->setColor(m_colorScheme.scrollBarGrip);
	m_itemHeight = (m_glyphSet->getHeight() + m_itemSpacing.y * 2) / m_pixelsPerWorldUnit;
	
	m_listItemGraphics.clear();
	bool isEven = false;
	for (ItemList::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		m_listItemGraphics.push_back( createGraphicForItem(*it,
			isEven ? m_colorScheme.itemEven : m_colorScheme.itemOdd));
		isEven = (isEven == false);
	}
	
	// Restore selection
	bool doCallback(true);
	selectedItem = getSelectedItem();
	if (selectedItem != 0 && selectedIdValid && selectedItem->id == selectedId)
	{
		doCallback = false;
	}

	s32 index = m_selectedItemIndex;
	m_selectedItemIndex = -1;
	selectItemByIndex(index, doCallback);
}


ListItemGraphic ListBox::createGraphicForItem(const ListItem& p_item,
	                                          const tt::engine::renderer::ColorRGBA& p_color)
{
	using namespace tt::engine;
	ListItemGraphic graphic;
	graphic.background = renderer::QuadSprite::createQuad(m_size.x, m_itemHeight, p_color);
	graphic.background->setFlag(renderer::QuadSprite::Flag_WorldSpace);
	
	TT_NULL_ASSERT(m_glyphSet);
	
	renderer::TexturePtr texture = renderer::Texture::createForText(
		static_cast<s16>(m_size.x     * m_pixelsPerWorldUnit),
		static_cast<s16>(m_itemHeight * m_pixelsPerWorldUnit), true);
	
	{
		renderer::TexturePainter painter(texture->lock());
		m_glyphSet->drawTruncatedString(
			p_item.label,
			painter,
			renderer::ColorRGB::white,
			glyph::GlyphSet::ALIGN_LEFT,
			glyph::GlyphSet::ALIGN_CENTER,
			0,0,0,0,0);
	}
	graphic.textLabel = renderer::QuadSprite::createQuad(m_size.x, m_itemHeight,
		renderer::VertexBuffer::Property_Diffuse|renderer::VertexBuffer::Property_Texture0);
	graphic.textLabel->setTexture(texture);
	graphic.textLabel->setColor(m_colorScheme.itemText);
	graphic.textLabel->setFlag(renderer::QuadSprite::Flag_WorldSpace);

	return graphic;
}


void ListBox::positionItems()
{
	const real maxScrollOffset = m_listItemGraphics.size() * m_itemHeight - m_size.y;
	if (maxScrollOffset > 0.0f)
	{
		tt::math::clamp(m_scrollOffset, 0.0f, maxScrollOffset);
	}
	else
	{
		m_scrollOffset = 0;
	}
	
	tt::math::Vector2 pos(m_position.xy());
	pos.x += (m_size.x * 0.5f);
	pos.y += m_scrollOffset;
	pos.y -= m_itemHeight * 0.5f;
	
	for (ItemGraphics::iterator it = m_listItemGraphics.begin(); it != m_listItemGraphics.end(); ++it)
	{
		it->textLabel->setPosition(pos);
		it->textLabel->update();
		it->background->setPosition(pos);
		it->background->update();

		pos.y -= m_itemHeight;
	}
	updateScrollBarGrip();
}


void ListBox::selectItemByIndex(s32 p_index, bool p_doCallbacks)
{
	if (p_index >= static_cast<s32>(m_items.size()))
	{
		m_selectedItemIndex = -1;
	}
	
	if (p_index != m_selectedItemIndex)
	{
		s32 previousSelected = m_selectedItemIndex;
		m_selectedItemIndex = p_index;
		
		if (m_itemSelectedCallback != 0 && p_doCallbacks)
		{
			m_itemSelectedCallback(getItemByIndex(m_selectedItemIndex), m_callbackUserData);
		}
		
		if (m_listItemGraphics.empty())
		{
			return;
		}
		
		if (previousSelected >= 0)
		{
			m_listItemGraphics[previousSelected].background->setColor(
				previousSelected % 2 == 1 ? m_colorScheme.itemEven : m_colorScheme.itemOdd);
			m_listItemGraphics[previousSelected].textLabel ->setColor(m_colorScheme.itemText);
		}

		if (m_selectedItemIndex >= 0)
		{
			m_listItemGraphics[m_selectedItemIndex].background->setColor(m_colorScheme.itemSelected);
			m_listItemGraphics[m_selectedItemIndex].textLabel ->setColor(m_colorScheme.itemTextSelected);
			
			if (m_listItemGraphics[m_selectedItemIndex].background->getPosition().y <
				(m_position.y - m_size.y + m_autoScrollOffset))
			{
				m_scrollOffset = (m_selectedItemIndex * m_itemHeight) - m_size.y + m_autoScrollOffset;
				positionItems();
			}
		
			if (m_listItemGraphics[m_selectedItemIndex].background->getPosition().y >
				(m_position.y - m_autoScrollOffset))
			{
				m_scrollOffset = (m_selectedItemIndex * m_itemHeight) - m_autoScrollOffset;
				positionItems();
			}
		}
	}
}


void ListBox::setHoverItemByIndex(s32 p_index)
{
	if (m_hoverItemIndex == p_index) return;
	
	if (m_hoverItemIndex >= 0 && m_hoverItemIndex < static_cast<s32>(m_listItemGraphics.size()))
	{
		m_listItemGraphics[m_hoverItemIndex].background->setColor(
			m_hoverItemIndex % 2 == 1 ? m_colorScheme.itemEven : m_colorScheme.itemOdd);
		m_listItemGraphics[m_hoverItemIndex].textLabel->setColor(m_colorScheme.itemText);
	}
	
	if (p_index < 0 || p_index >= static_cast<s32>(m_listItemGraphics.size()) || p_index == m_selectedItemIndex)
	{
		m_hoverItemIndex = -1;
	}
	else
	{
		m_listItemGraphics[p_index].background->setColor(m_colorScheme.itemHover);
		m_listItemGraphics[p_index].textLabel ->setColor(m_colorScheme.itemTextHover);
		m_hoverItemIndex = p_index;
	}
}


void ListBox::scroll(real p_numberOfItems)
{
	m_scrollOffset -= (p_numberOfItems * m_itemHeight);
	positionItems();
}


void ListBox::removeItem(const ItemList::iterator& p_iterator)
{
	s32 index = static_cast<s32>(std::distance(m_items.begin(), p_iterator));
	m_items.erase(p_iterator);
	m_itemsChanged = true;

	if (index < m_selectedItemIndex && m_selectedItemIndex != 0)
	{
		m_selectedItemIndex--;
	}
	
	if (m_items.empty())
	{
		// List is empty: nothing can be selected
		const s32 prevSelectedItemIndex = m_selectedItemIndex;
		m_selectedItemIndex = -1;
		m_hoverItemIndex    = -1;
		
		if (m_selectedItemIndex != prevSelectedItemIndex &&
		    m_itemSelectedCallback != 0)
		{
			m_itemSelectedCallback(getItemByIndex(m_selectedItemIndex), m_callbackUserData);
		}
	}
}


void ListBox::updateScrollBarGrip()
{
	const real numberOfItemsInView = m_size.y / m_itemHeight;
	const real totalItems          = static_cast<real>(m_items.size());
	if (numberOfItemsInView > totalItems)
	{
		m_scrollBarVisible = false;
		return;
	}
	
	const real minimumGripHeight = 20.0f / m_pixelsPerWorldUnit;
	const real sizePercentage = numberOfItemsInView / totalItems;
	const real maxScrollOffset = m_listItemGraphics.size() * m_itemHeight - m_size.y;
	const real relativePosition = m_scrollOffset / maxScrollOffset;

	const real gripHeight = std::max(sizePercentage * m_size.y, minimumGripHeight);
	m_scrollBarGrip->setHeight(gripHeight);
	m_scrollBarGrip->setPositionX(m_scrollBar->getPosition().x);
	m_scrollBarGrip->setPositionY(
		(m_position.y - gripHeight * 0.5f) - (m_size.y - gripHeight) * relativePosition);
	m_scrollBarGrip->update();
	m_scrollBarVisible = true;
}


// Namespace end
}
}
}
