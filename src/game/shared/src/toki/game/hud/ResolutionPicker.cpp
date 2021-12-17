#include <tt/app/Application.h>
#include <tt/compression/image.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/device_enumeration.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TextureHardware.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#include <toki/game/entity/EntityMgr.h>
#include <toki/game/hud/ListBox.h>
#include <toki/game/hud/ResolutionPicker.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>


namespace toki {
namespace game {
namespace hud {

//--------------------------------------------------------------------------------------------------
// Public member functions

ResolutionPickerPtr ResolutionPicker::create()
{
	ResolutionPickerPtr picker(new ResolutionPicker);
	picker->m_this = picker;
	return picker;
}


ResolutionPicker::~ResolutionPicker()
{
}


void ResolutionPicker::open()
{
	if (m_open)
	{
		// Already open
		return;
	}
	
	m_open = true;
	populateLevelList();
}


void ResolutionPicker::close()
{
	if (m_open == false)
	{
		// Already closed
	}
	
	m_open = false;
	// ... perform any necessary close processing here
}


bool ResolutionPicker::update(real /*p_deltaTime*/)
{
	// Only perform GUI work when the UI is actually visible
	if (m_open == false)
	{
		return false;
	}
	
	bool handledInput = false;
	
	if (m_list != 0)
	{
		const entity::Entity* posSource(m_followEntity.getPtr());
		if (posSource != 0)
		{
			const tt::math::Vector2 pos(posSource->getCenterPosition());
			
			m_list->setPosition(tt::math::Vector3(
					pos.x - (m_list->getSize().x * 0.5f),
					pos.y + (m_list->getSize().y * 0.5f),
					0.0f));
		}
		else
		{
			TT_PANIC("No follow entity for ResolutionPicker");
		}
		
		const input::Controller::State&              inputState(AppGlobal::getController(tt::input::ControllerIndex_One).cur);
		const input::Controller::State::EditorState& editorState(inputState.editor);
		
		const tt::math::Vector3  worldPosition(AppGlobal::getGame()->getCamera().screenToWorld(editorState.pointer));
		
		handledInput = handledInput ||
				m_list->handleInput(worldPosition, editorState.pointerLeft, inputState.wheelNotches);
		m_list->update();
	}
	
	return handledInput;
}


void ResolutionPicker::render()
{
	if (m_open == false)
	{
		return;
	}
	
	if (m_list != 0)
	{
		m_list->render();
	}
}


void ResolutionPicker::selectPrevious()
{
	TT_NULL_ASSERT(m_list);
	m_list->selectPrevious();
}


void ResolutionPicker::selectNext()
{
	TT_NULL_ASSERT(m_list);
	m_list->selectNext();
}


void ResolutionPicker::applySelectedResolution()
{
	TT_NULL_ASSERT(m_list);
	const ListItem* selectedItem = m_list->getSelectedItem();
	if (selectedItem == 0)
	{
		return;
	}
	
	const Point2s::size_type index = static_cast<Point2s::size_type>(selectedItem->id);
	if (index >= m_upscaleResolutions.size())
	{
		TT_PANIC("Invalid id/index: %d. (0 - %d)", index, m_upscaleResolutions.size());
		return;
	}
	
	const tt::math::Point2 upscaleSize = m_upscaleResolutions[index];
	const tt::math::Point2 fullscreenSize(tt::app::getApplication()->getDesktopSize());
	
	/*
	// Find closest fullscreen resolution
	// MARTIJN: Don't change fullscreen resolution; always stick to desktop one
	for (Point2s::const_iterator it = m_supportedResolutions.begin(); 
	     it != m_supportedResolutions.end(); ++it)
	{
		if (it->x >= upscaleSize.x && it->x < fullscreenSize.x &&
			it->y >= upscaleSize.y && it->y < fullscreenSize.y)
		{
			fullscreenSize = *it;
		}
	}
	*/
	
	AppOptions::getInstance().setUpscaleSize(upscaleSize, fullscreenSize);
	AppOptions::getInstance().saveIfDirty();
}


void ResolutionPicker::setCallbackEntity(const entity::EntityHandle& p_entity)
{
	m_callbackEntity = p_entity;
	
	// Immediately notify the new callback entity of a level selection
	// (so that it has the chance to update its own state)
	notifyCallbackEntity("onWorkshopLevelSelected");
}


void ResolutionPicker::setFollowEntity(const entity::EntityHandle& p_entity)
{
	m_followEntity = p_entity;
}


void ResolutionPicker::setSize(real p_width, real p_height)
{
	m_width  = p_width;
	m_height = p_height;
	
	// Also update the actual UI element size
	if (m_list != 0)
	{
		m_list->setSize(tt::math::Vector2(p_width, p_height));
	}
}


void ResolutionPicker::setColorScheme(const hud::ListBoxColorScheme& p_colorScheme)
{
	TT_NULL_ASSERT(m_list);
	m_list->setColorScheme(p_colorScheme);
}


const hud::ListBoxColorScheme& ResolutionPicker::getColorScheme() const
{
	TT_NULL_ASSERT(m_list);
	return m_list->getColorScheme();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ResolutionPicker::ResolutionPicker()
:
m_this(),
m_list(),
m_followEntity(),
m_width(0.0f),
m_height(0.0f),
m_open(false),
m_supportedResolutions(),
m_upscaleResolutions(),
m_callbackEntity()
{
	tt::engine::renderer::Resolutions supportedResolutions = tt::engine::renderer::getSupportedResolutions(true);
	
	for (tt::engine::renderer::Resolutions::const_iterator it = supportedResolutions.begin(); 
	     it != supportedResolutions.end(); ++it)
	{
		m_supportedResolutions.push_back(*it);
	}
	
	setupUi();
}


void ResolutionPicker::setupUi()
{
	// Create the interface elements
	using tt::engine::renderer::ColorRGBA;
	
	static const tt::engine::renderer::ColorRGBA defaultTextColor(tt::engine::renderer::ColorRGB::black);
	
	ListBoxColorScheme colorScheme;
	colorScheme.background      = ColorRGBA(  0,   0,   0,   0);
	colorScheme.backgroundHover = ColorRGBA(  0,   0,   0,   0);
	colorScheme.itemText        = ColorRGBA( 17, 102, 182, 255);
	colorScheme.itemEven        = ColorRGBA(  0,   0,   0,   0);
	colorScheme.itemOdd         = ColorRGBA(  0,   0,   0,   0);
	colorScheme.itemSelected    = ColorRGBA(255, 255, 255, 255);
	
	m_list.reset(new ListBox(utils::GlyphSetMgr::get(utils::GlyphSetID_Text), 60));
	m_list->setColorScheme(colorScheme);
	m_list->setItemSelectedCallback(callbackResolutionSelected, this);
	
	// Set up default sizes and positions for the interface elements
	setSize(3.90625f, 6.25f);
}


void ResolutionPicker::populateLevelList()
{
	TT_NULL_ASSERT(m_list);
	
	m_list->removeAll();
	m_upscaleResolutions.clear();
	
	static const real percentages[] = {1.0f, 0.9f, 0.8f, (2 / 3.0f), 0.5f, 0.4f, (1 / 3.0f), 0.2f, 0.1f};
	static const s32 minimumVerticalResolution = 480;
	const tt::math::Point2 nativeResolution(tt::app::getApplication()->getDesktopSize());
	const tt::math::Point2 currentResolution(AppOptions::getInstance().upscaleSize);
	
	tt::math::Point2 resolution(nativeResolution);
	s32 i(0);
	while (resolution.y >= minimumVerticalResolution)
	{
		// NOTE: Adding a space in front of the name as cheap way of adding left padding for list item text
		std::string displayName(" " + tt::str::toStr(resolution.x) + "x" + tt::str::toStr(resolution.y));
		if (i == 0)
		{
			displayName += " (native)";
		}
		else
		{
			displayName += " (" + tt::str::toStr(static_cast<s32>(percentages[i] * 100 + 0.5f)) + "%)";
		}
		
		ListItem* item = m_list->addItem(tt::str::widen(displayName), i);
		
		if (resolution == currentResolution)
		{
			m_list->selectItem(item);
		}
		
		m_upscaleResolutions.push_back(resolution);
		++i;
		resolution.x = static_cast<s32>(nativeResolution.x * percentages[i] + 0.5f);
		resolution.y = static_cast<s32>(nativeResolution.y * percentages[i] + 0.5f);
	}
	
	if (m_list->getSelectedItem() == 0) // If nothing was selected, select the first item.
	{
		ListItem* item = m_list->getItemByIndex(0);
		if (item != 0)
		{
			m_list->selectItem(item);
		}
		
		// If still nothing selected, give up and simply let the GUI handle "nothing selected"
		if (m_list->getSelectedItem() == 0)
		{
			onResolutionSelected();
		}
	}
}


void ResolutionPicker::notifyCallbackEntity(const std::string& p_notificatonFunction)
{
	entity::Entity* callbackEntity = m_callbackEntity.getPtr();
	if (callbackEntity != 0)
	{
		game::script::EntityBasePtr script = callbackEntity->getEntityScript();
		if (script != 0)
		{
			script->queueSqFun(p_notificatonFunction);
		}
	}
}


void ResolutionPicker::callbackResolutionSelected(ListItem* /*p_item*/, void* p_userData)
{
	ResolutionPicker* picker = reinterpret_cast<ResolutionPicker*>(p_userData);
	TT_NULL_ASSERT(picker);
	if (picker != 0)
	{
		picker->onResolutionSelected();
	}
}


void ResolutionPicker::onResolutionSelected()
{
	// Notify the callback entity that the selection changed
	notifyCallbackEntity("onResolutionSelected");
}


// Namespace end
}
}
}
