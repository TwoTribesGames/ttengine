#if defined(TT_STEAM_BUILD)  // Steam Workshop functionality is only available in Steam builds

#include <tt/compression/image.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TextureHardware.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/helpers.h>
#include <tt/str/str.h>

#include <toki/game/entity/EntityMgr.h>
#include <toki/game/hud/ListBox.h>
#include <toki/game/hud/WorkshopLevelPicker.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/steam/Workshop.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace hud {

PublishedFileId_t WorkshopLevelPicker::ms_lastPlayedLevel = 0;


//--------------------------------------------------------------------------------------------------
// Public member functions

WorkshopLevelPickerPtr WorkshopLevelPicker::create()
{
	WorkshopLevelPickerPtr picker(new WorkshopLevelPicker);
	picker->m_this = picker;
	steam::Workshop::getInstance()->registerObserver(picker);
	return picker;
}


WorkshopLevelPicker::~WorkshopLevelPicker()
{
	/* FIXME: Can't unregister like this: this instance is already being destroyed,
	//        so locking the weak pointer will yield a null smart pointer
	if (steam::Workshop::hasInstance())
	{
		steam::Workshop::getInstance()->unregisterObserver(m_this.lock());
	}
	// */
}


void WorkshopLevelPicker::open()
{
	if (m_open)
	{
		// Already open
		return;
	}
	
	m_open = true;
	populateLevelList();
}


void WorkshopLevelPicker::close()
{
	if (m_open == false)
	{
		// Already closed
	}
	
	m_open = false;
	// ... perform any necessary close processing here
}


bool WorkshopLevelPicker::update(real /*p_deltaTime*/)
{
	// Only perform GUI work when the UI is actually visible
	if (m_open == false)
	{
		return false;
	}
	
	const input::Controller::State&              inputState(AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	const input::Controller::State::EditorState& editorState(inputState.editor);
	
	const entity::EntityMgr& entityMgr    (AppGlobal::getGame()->getEntityMgr());
	const tt::math::Vector3  worldPosition(AppGlobal::getGame()->getCamera().screenToWorld(editorState.pointer));
	
	bool handledInput = false;
	
	for (s32 i = 0; i < Element_Count; ++i)
	{
		ElementInfo& el(m_elements[i]);
		
		const entity::Entity* posSource(entityMgr.getEntity(el.followEntity));
		if (posSource != 0)
		{
			const tt::math::Vector2 pos(posSource->getCenterPosition());
			
			if (el.list != 0)
			{
				el.list->setPosition(tt::math::Vector3(
						pos.x - (el.list->getSize().x * 0.5f),
						pos.y + (el.list->getSize().y * 0.5f),
						0.0f));
			}
			else if (el.quad != 0)
			{
				el.quad->setPosition(pos.x, -pos.y, 0.0f);
			}
		}
		
		if (el.list != 0)
		{
			handledInput = handledInput ||
					el.list->handleInput(worldPosition, editorState.pointerLeft, inputState.wheelNotches);
			el.list->update();
		}
		else if (el.quad != 0)
		{
			el.quad->update();
		}
		
		if (el.textNeedsRepaint)
		{
			refreshTextTexture(el);
		}
	}
	
	// Only call into Squirrel from the main thread (assuming update() is always called from the main thread)
	// Steam callbacks may arrive from the main thread while the loading thread is still loading the game,
	// causing two threads to use Squirrel at the same time
	if (m_notifyCallbackEntityThisFrame ||
	    m_notifyCallbackEntityListEmptyThisFrame)
	{
		entity::Entity* callbackEntity = m_callbackEntity.getPtr();
		if (callbackEntity != 0)
		{
			game::script::EntityBasePtr script = callbackEntity->getEntityScript();
			if (script != 0)
			{
				if (m_notifyCallbackEntityThisFrame)
				{
					script->queueSqFun("onWorkshopLevelSelected");
				}
				
				if (m_notifyCallbackEntityListEmptyThisFrame)
				{
					script->queueSqFun("onWorkshopLevelListEmpty", m_notifyCallbackEntityListEmptyValue);
				}
			}
		}
		
		m_notifyCallbackEntityThisFrame          = false;
		m_notifyCallbackEntityListEmptyThisFrame = false;
	}
	
	return handledInput;
}


void WorkshopLevelPicker::render()
{
	if (m_open == false)
	{
		return;
	}
	
	for (s32 i = 0; i < Element_Count; ++i)
	{
		if (m_elements[i].list != 0)
		{
			m_elements[i].list->render();
		}
		else if (m_elements[i].quad != 0)
		{
			m_elements[i].quad->render();
		}
	}
}


float WorkshopLevelPicker::getSelectedLevelScore() const
{
	if (m_elements[Element_LevelList].list                    == 0 ||
	    m_elements[Element_LevelList].list->getSelectedItem() == 0)
	{
		return 0.0f;
	}
	
	const steam::Workshop::FileDetails* details = steam::Workshop::getInstance()->getCachedFileDetails(
			m_elements[Element_LevelList].list->getSelectedItem()->id);
	
	return (details != 0) ? details->globalScore : 0.0f;
}


bool WorkshopLevelPicker::isSelectedLevelDownloaded() const
{
	if (m_elements[Element_LevelList].list                    == 0 ||
	    m_elements[Element_LevelList].list->getSelectedItem() == 0)
	{
		return false;
	}
	
	const PublishedFileId_t fileId = m_elements[Element_LevelList].list->getSelectedItem()->id;
	// NOTE: Could also check Workshop::FileDetails::ValidDetails_FileDownloaded
	const std::string levelPath(steam::Workshop::getInstance()->getLocalPath(fileId));
	return levelPath.empty() == false && tt::fs::fileExists(levelPath);
}


bool WorkshopLevelPicker::isAnyLevelSelected() const
{
	return m_elements[Element_LevelList].list                    != 0 &&
	       m_elements[Element_LevelList].list->getSelectedItem() != 0;
}


void WorkshopLevelPicker::showSelectedLevelWorkshopPage()
{
	if (m_elements[Element_LevelList].list                    != 0 &&
	    m_elements[Element_LevelList].list->getSelectedItem() != 0)
	{
		const PublishedFileId_t fileId = m_elements[Element_LevelList].list->getSelectedItem()->id;
		tt::steam::openURL("http://steamcommunity.com/sharedfiles/filedetails/?id=" +
		                   tt::str::toStr(fileId));
	}
}


void WorkshopLevelPicker::selectPreviousLevel()
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	m_elements[Element_LevelList].list->selectPrevious();
}


void WorkshopLevelPicker::selectNextLevel()
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	m_elements[Element_LevelList].list->selectNext();
}


void WorkshopLevelPicker::playSelectedLevel()
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	const ListItem* selectedItem = m_elements[Element_LevelList].list->getSelectedItem();
	if (selectedItem == 0)
	{
		// No level selected: cannot play
		return;
	}
	
	const PublishedFileId_t fileId = selectedItem->id;
	const std::string levelPath(steam::Workshop::getInstance()->getLocalPath(fileId));
	if (levelPath.empty() == false &&
	    tt::fs::fileExists(levelPath))
	{
		/* DEBUG: Test out user voting
		const steam::Workshop::FileDetails* details = steam::Workshop::getInstance()->getCachedFileDetails(fileId);
		bool voteUp = true;
		if (details != 0)
		{
			voteUp = details->currentUserVote != k_EWorkshopVoteFor;
			TT_Printf("CURRENT VOTE FOR LEVEL '%s': %d\n",
			          details->details.m_rgchTitle, details->currentUserVote);
		}
		TT_Printf("---- Setting vote to '%s'\n", voteUp ? "thumbs up" : "thumbs down");
		steam::Workshop::getInstance()->setUserVote(fileId, voteUp);
		// END DEBUG */
		
		ms_lastPlayedLevel = fileId;
		
		StartInfo startInfo;
		startInfo.setWorkshopLevel(levelPath, fileId);
		AppGlobal::setGameStartInfo(startInfo);
		if (AppGlobal::hasGame())
		{
			AppGlobal::getGame()->forceReload();
		}
	}
}


void WorkshopLevelPicker::setShowTextBorders(bool p_show)
{
	if (m_renderTextBorders == p_show)
	{
		return;
	}
	
	m_renderTextBorders = p_show;
	
	// Re-render all text elements to add or remove the borders
	for (s32 i = 0; i < Element_Count; ++i)
	{
		ElementInfo& el(m_elements[i]);
		if (el.quad != 0 && el.isTextQuad && el.quad->getTexture() != 0)
		{
			el.textNeedsRepaint = true;
		}
	}
}


void WorkshopLevelPicker::setCallbackEntity(const entity::EntityHandle& p_entity)
{
	m_callbackEntity = p_entity;
	
	// Immediately notify the new callback entity of a level selection
	// (so that it has the chance to update its own state)
	m_notifyCallbackEntityThisFrame = true;
	
	m_notifyCallbackEntityListEmptyThisFrame = true;
}


void WorkshopLevelPicker::setElementFollowEntity(Element p_element, const entity::EntityHandle& p_entity)
{
	TT_ASSERTMSG(isValidElement(p_element), "Invalid interface element: %d", p_element);
	if (isValidElement(p_element))
	{
		m_elements[p_element].followEntity = p_entity;
	}
}


void WorkshopLevelPicker::setElementSize(Element p_element, real p_width, real p_height)
{
	if (isValidElement(p_element) == false)
	{
		TT_PANIC("Invalid interface element: %d", p_element);
		return;
	}
	
	ElementInfo& el(m_elements[p_element]);
	el.width  = p_width;
	el.height = p_height;
	
	// Also update the actual UI element size
	if (el.list != 0)
	{
		el.list->setSize(tt::math::Vector2(p_width, p_height));
	}
	else if (el.quad != 0)
	{
		if (el.isTextQuad)
		{
			tt::math::Point2 sizeInPixels(static_cast<s32>(el.width  * m_textPixelsPerWorldUnit),
			                              static_cast<s32>(el.height * m_textPixelsPerWorldUnit));
			
			using namespace tt::engine::renderer;
			TexturePtr tex(el.quad->getTexture());
			if (tex == 0 || tex->getWidth() < sizeInPixels.x || tex->getHeight() < sizeInPixels.y)
			{
				// Current texture isn't large enough for the quad: create a new one
				tt::math::Point2 texSize(TextureHardware::getRequirements().correctDimension(
						sizeInPixels));
				tex = Texture::createForText(texSize, true);
				el.quad->setTexture(tex);
			}
			
			el.quad->setFrame (sizeInPixels.x, sizeInPixels.y);
			el.quad->setWidth (el.width);
			el.quad->setHeight(el.height);
			el.textNeedsRepaint = true;
		}
		else
		{
			el.quad->setWidth (el.width);
			el.quad->setHeight(el.height);
		}
	}
}


void WorkshopLevelPicker::setElementColor(Element p_element, const tt::engine::renderer::ColorRGBA& p_color)
{
	TT_ASSERTMSG(isValidElement(p_element), "Invalid interface element: %d", p_element);
	if (isValidElement(p_element))
	{
		TT_ASSERTMSG(m_elements[p_element].quad != 0,
		             "Can only set element color for quad elements. Element %d is not a quad.", p_element);
		if (m_elements[p_element].quad != 0)
		{
			m_elements[p_element].quad->setColor(p_color);
		}
	}
}

void WorkshopLevelPicker::setElementVerticalAlignment(Element p_element, 
                                                      entity::graphics::VerticalAlignment p_verticalAlignment)
{
	if (isValidElement(p_element) == false)
	{
		TT_PANIC("Invalid interface element: %d", p_element);
		return;
	}
	
	ElementInfo& el(m_elements[p_element]);
	if (el.verticalAlignment != p_verticalAlignment)
	{
		el.verticalAlignment = p_verticalAlignment;
		el.textNeedsRepaint  = true;;
	}
}


void WorkshopLevelPicker::setElementHorizontalAlignment(Element p_element,
                                                        entity::graphics::HorizontalAlignment p_horizontalAlignment)
{
	if (isValidElement(p_element) == false)
	{
		TT_PANIC("Invalid interface element: %d", p_element);
		return;
	}
	
	ElementInfo& el(m_elements[p_element]);
	if (el.horizontalAlignment != p_horizontalAlignment)
	{
		el.horizontalAlignment = p_horizontalAlignment;
		el.textNeedsRepaint    = true;;
	}
}


void WorkshopLevelPicker::setLevelListColorScheme(const hud::ListBoxColorScheme& p_colorScheme)
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	m_elements[Element_LevelList].list->setColorScheme(p_colorScheme);
}


const hud::ListBoxColorScheme& WorkshopLevelPicker::getLevelListColorScheme() const
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	return m_elements[Element_LevelList].list->getColorScheme();
}


void WorkshopLevelPicker::onWorkshopFileChange(
		PublishedFileId_t                   p_id,
		steam::WorkshopObserver::FileAction p_action)
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	
	PublishedFileId_t selectedId = 0;
	{
		const ListItem* selectedItem = m_elements[Element_LevelList].list->getSelectedItem();
		if (selectedItem != 0)
		{
			selectedId = selectedItem->id;
		}
	}
	
	const bool isSubscribed = steam::Workshop::getInstance()->isSubscribedToFile(p_id);
	
	switch (p_action)
	{
	case steam::WorkshopObserver::FileAction_Unsubscribed:
		// Remove file from list (if in list)
		m_elements[Element_LevelList].list->removeItemById(p_id);
		
		// If the user is no longer subscribed to any files, tell script about this
		//TT_Printf("WorkshopLevelPicker::onWorkshopFileChange: [UNSUBSCRIBED] Total files subscribed now: %u\n",
		//          steam::Workshop::getInstance()->getSubscribedFiles().size());
		if (steam::Workshop::getInstance()->getSubscribedFiles().empty())
		{
			//TT_Printf("WorkshopLevelPicker::onWorkshopFileChange: Notifying script about empty list.\n");
			m_notifyCallbackEntityListEmptyThisFrame = true;
			m_notifyCallbackEntityListEmptyValue     = true;
		}
		break;
		
	case steam::WorkshopObserver::FileAction_Subscribed:
		// If this is the first file the user subscribed to, tell script the list is no longer empty
		//TT_Printf("WorkshopLevelPicker::onWorkshopFileChange: [SUBSCRIBED] Total files subscribed now: %u\n",
		//          steam::Workshop::getInstance()->getSubscribedFiles().size());
		if (steam::Workshop::getInstance()->getSubscribedFiles().size() == 1)
		{
			//TT_Printf("WorkshopLevelPicker::onWorkshopFileChange: Notifying script about non-empty list.\n");
			m_notifyCallbackEntityListEmptyThisFrame = true;
			m_notifyCallbackEntityListEmptyValue     = false;
		}
		
		// Intentional fall-through: we also want to add file details to the list if we have them
		
	case steam::WorkshopObserver::FileAction_DetailsAvailable:
		// FIXME: Would be nice if ListBox had a "hasItem" or something
		if (isSubscribed && m_elements[Element_LevelList].list->getItemById(p_id) == 0)
		{
			// Item not in list yet: add a new one!
			ListItem* newItem = addListItemForId(p_id);
			if (newItem != 0 && m_elements[Element_LevelList].list->getSelectedItem() == 0)
			{
				// There was no selection yet: select this new item, so there is something selected
				m_elements[Element_LevelList].list->selectItem(newItem);
			}
			break;
		}
		
		// NOTE: Intentional fall-through: in all the cases below,
		//       the details need to be refreshed if this is the selected item
		
	case steam::WorkshopObserver::FileAction_GlobalVotesAvailable:
	case steam::WorkshopObserver::FileAction_PreviewDownloaded:
		// Refresh the current level details if we received new information for the selected level
		if (p_id == selectedId)
		{
			onLevelSelected();
		}
		break;
		
	case steam::WorkshopObserver::FileAction_FileDownloaded:
		if (p_id == selectedId)
		{
			// The selected level is now available for playing. Notify script about this.
			m_notifyCallbackEntityThisFrame = true;
		}
		break;
		
	default:
		// Don't care about this notification
		break;
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

WorkshopLevelPicker::WorkshopLevelPicker()
:
m_this(),
m_open(false),
m_currentOwnerID(0),
m_renderTextBorders(false),
m_textPixelsPerWorldUnit(64),
m_callbackEntity(),
m_notifyCallbackEntityThisFrame(false),
m_notifyCallbackEntityListEmptyThisFrame(false),
m_notifyCallbackEntityListEmptyValue(true),
m_callbackPersonaStateChange(),
m_callbackAvatarImageLoaded()
{
	m_callbackPersonaStateChange.Register(this, &WorkshopLevelPicker::onPersonaStateChange);
	m_callbackAvatarImageLoaded .Register(this, &WorkshopLevelPicker::onAvatarImageLoaded);
	
	setupUi();
}


void WorkshopLevelPicker::setupUi()
{
	// Create the interface elements
	using tt::engine::renderer::ColorRGBA;
	using tt::engine::renderer::QuadSprite;
	using tt::engine::renderer::TextureCache;
	using tt::engine::renderer::TexturePtr;
	
	static const tt::engine::renderer::ColorRGBA defaultTextColor(tt::engine::renderer::ColorRGB::black);
	
	ListBoxColorScheme colorScheme;
	colorScheme.background      = ColorRGBA(  0,   0,   0,   0);
	colorScheme.backgroundHover = ColorRGBA(  0,   0,   0,   0);
	colorScheme.itemText        = ColorRGBA( 17, 102, 182, 255);
	colorScheme.itemEven        = ColorRGBA(  0,   0,   0,   0);
	colorScheme.itemOdd         = ColorRGBA(  0,   0,   0,   0);
	colorScheme.itemSelected    = ColorRGBA(255, 255, 255, 255);
	
	m_elements[Element_LevelList].glyphSet = utils::GlyphSetID_Text;
	m_elements[Element_LevelList].list.reset(new ListBox(utils::GlyphSetMgr::get(m_elements[Element_LevelList].glyphSet), 60));
	m_elements[Element_LevelList].list->setColorScheme(colorScheme);
	m_elements[Element_LevelList].list->setItemSelectedCallback(callbackLevelSelected, this);
	
	m_elements[Element_Title       ].quad       = QuadSprite::createQuad(TexturePtr(), defaultTextColor);
	m_elements[Element_Title       ].isTextQuad = true;
	m_elements[Element_Title       ].glyphSet   = utils::GlyphSetID_Title;
	
	m_elements[Element_Description ].quad       = QuadSprite::createQuad(TexturePtr(), defaultTextColor);
	m_elements[Element_Description ].isTextQuad = true;
	m_elements[Element_Description ].glyphSet   = utils::GlyphSetID_Text;
	
	m_elements[Element_AuthorName  ].quad       = QuadSprite::createQuad(TexturePtr(), defaultTextColor);
	m_elements[Element_AuthorName  ].isTextQuad = true;
	m_elements[Element_AuthorName  ].glyphSet   = utils::GlyphSetID_Text;
	
	m_elements[Element_PreviewImage].quad = QuadSprite::createQuad(TexturePtr(), tt::engine::renderer::ColorRGB::white);
	m_elements[Element_AuthorAvatar].quad = QuadSprite::createQuad(TexturePtr(), tt::engine::renderer::ColorRGB::white);
	
	// Sanity check: all element pointers should have been set up at this point
	for (s32 i = 0; i < Element_Count; ++i)
	{
		TT_ASSERTMSG(m_elements[i].list != 0 || m_elements[i].quad != 0,
		             "Interface element %d does not have a list box or quad set for it.",
		             i);
	}
	
	// Set up default sizes and positions for the interface elements
	setElementSize(Element_LevelList,    3.90625f, 6.25f);
	setElementSize(Element_Title,        8.0f,     0.78125f);
	setElementSize(Element_Description,  4.46875f, 4.6875f);
	setElementSize(Element_PreviewImage, 4.46875f, 2.96875f);
	setElementSize(Element_AuthorAvatar, 1.8125f,  1.8125f);
	setElementSize(Element_AuthorName,   1.8125f,  0.5f);
}


void WorkshopLevelPicker::populateLevelList()
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	
	PublishedFileId_t prevSelectedID = ms_lastPlayedLevel;
	if (m_elements[Element_LevelList].list->getSelectedItem() != 0)
	{
		prevSelectedID = m_elements[Element_LevelList].list->getSelectedItem()->id;
	}
	
	m_elements[Element_LevelList].list->removeAll();
	
	steam::Workshop* workshop = steam::Workshop::getInstance();
	const steam::PublishedFileIds& files(workshop->getSubscribedFiles());
	
	for (steam::PublishedFileIds::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		ListItem* item = addListItemForId(*it);
		if (item != 0 && *it == prevSelectedID)
		{
			m_elements[Element_LevelList].list->selectItem(item);
		}
	}
	
	if (m_elements[Element_LevelList].list->getSelectedItem() == 0)
	{
		ListItem* item = m_elements[Element_LevelList].list->getItemByIndex(0);
		if (item != 0)
		{
			m_elements[Element_LevelList].list->selectItem(item);
		}
		
		// If still nothing selected, give up and simply let the GUI handle "nothing selected"
		if (m_elements[Element_LevelList].list->getSelectedItem() == 0)
		{
			onLevelSelected();
		}
	}
	
	// Tell script whether there are any subscribed files
	// (they don't have to be in the list yet: just having them is enough)
	m_notifyCallbackEntityListEmptyThisFrame = true;
	m_notifyCallbackEntityListEmptyValue     = files.empty();
	//TT_Printf("WorkshopLevelPicker::populateLevelList: Number of subscribed files: %u\n", files.size());
}


ListItem* WorkshopLevelPicker::addListItemForId(PublishedFileId_t p_id)
{
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	ListItem* item = 0;
	
	const steam::Workshop::FileDetails* details = steam::Workshop::getInstance()->getCachedFileDetails(p_id);
	if (details != 0 &&
	    details->validMask.checkFlag(steam::Workshop::FileDetails::ValidDetails_File))
	{
		// NOTE: Adding a space in front of the name as cheap way of adding left padding for list item text
		std::wstring displayName(L" " + tt::str::utf8ToUtf16(details->details.m_rgchTitle));
		
		item = m_elements[Element_LevelList].list->addItem(displayName, p_id);
	}
	
	return item;
}


void WorkshopLevelPicker::setAuthorAvatarToEmpty()
{
	TT_NULL_ASSERT(m_elements[Element_AuthorAvatar].quad);
	if (m_elements[Element_AuthorAvatar].quad->checkFlag(tt::engine::renderer::QuadSprite::Flag_FadingOut) == false)
	{
		m_elements[Element_AuthorAvatar].quad->fadeOut(0.25f);
	}
}


void WorkshopLevelPicker::setPreviewImageToEmpty()
{
	TT_NULL_ASSERT(m_elements[Element_PreviewImage].quad);
	if (m_elements[Element_PreviewImage].quad->checkFlag(tt::engine::renderer::QuadSprite::Flag_FadingOut) == false)
	{
		m_elements[Element_PreviewImage].quad->fadeOut(0.25f);
	}
}


void WorkshopLevelPicker::setAvatarFromUser(uint64 p_steamID)
{
	const int avatarID = SteamFriends()->GetLargeFriendAvatar(p_steamID);
	uint32    imageW   = 0;
	uint32    imageH   = 0;
	
	if (avatarID <= 0)
	{
		// No avatar available (yet)
		//TT_Printf("No avatar available for Steam user %llu.\n", p_steamID);
		setAuthorAvatarToEmpty();
		return;
	}
	
	if (SteamUtils()->GetImageSize(avatarID, &imageW, &imageH) == false)
	{
		TT_WARN("Could not get image size for avatar %d.", avatarID);
		setAuthorAvatarToEmpty();
		return;
	}
	
	const int imageDataSize = imageW * imageH * 4;
	uint8* imageData = new uint8[imageDataSize];
	if (SteamUtils()->GetImageRGBA(avatarID, imageData, imageDataSize))
	{
		using namespace tt::engine::renderer;
		
#ifdef TT_PLATFORM_WIN
		// Swap Red / Blue channels on windows
		ColorRGBA* pixels = reinterpret_cast<ColorRGBA*>(imageData);
		for (s32 i = 0; i < static_cast<s32>(imageW * imageH); ++i)
		{
			std::swap(pixels->r, pixels->b);
			++pixels;
		}
#endif
		
		TextureBaseInfo textureInfo;
		textureInfo.width  = static_cast<u16>(imageW);
		textureInfo.height = static_cast<u16>(imageH);
		textureInfo.format = tt::ImageFormat_RGBA8;
		textureInfo.usage  = Usage_Text; // Indicates that data is not loaded from asset
		
		TexturePtr avatarTex = Texture::createFromBuffer(textureInfo, imageData, imageDataSize);
		TT_NULL_ASSERT(avatarTex);
		
		using tt::engine::renderer::QuadSprite;
		m_elements[Element_AuthorAvatar].quad->setTexture(avatarTex);
		if (m_elements[Element_AuthorAvatar].quad->checkFlag(QuadSprite::Flag_Visible) == false ||
		    m_elements[Element_AuthorAvatar].quad->checkFlag(QuadSprite::Flag_FadingOut))
		{
			m_elements[Element_AuthorAvatar].quad->fadeIn(0.25f);
		}
	}
	else
	{
		TT_WARN("Could not get image data for avatar %d.", avatarID);
		setAuthorAvatarToEmpty();
	}
	delete[] imageData;
}


void WorkshopLevelPicker::setNameFromUser(uint64 p_steamID)
{
	setText(Element_AuthorName, tt::str::utf8ToUtf16(SteamFriends()->GetFriendPersonaName(p_steamID)));
}


void WorkshopLevelPicker::setText(Element p_element, const std::wstring& p_text)
{
	TT_ASSERT(isValidElement(p_element));
	m_elements[p_element].renderedText     = p_text;
	tt::str::replace(m_elements[p_element].renderedText, L"\r\n", L"\n");  // replace Windows line endings with simple newlines
	tt::str::replace(m_elements[p_element].renderedText, L"\r", L"");  // remove stray carriage returns
	m_elements[p_element].textNeedsRepaint = true;
}


void WorkshopLevelPicker::refreshTextTexture(ElementInfo& p_element)
{
	if (p_element.isTextQuad         == false ||
	    p_element.quad               == 0     ||
	    p_element.quad->getTexture() == 0)
	{
		TT_PANIC("Trying to refresh text texture for a non-text element.");
		return;
	}
	
	tt::engine::glyph::GlyphSetPtr glyphSet(utils::GlyphSetMgr::get(p_element.glyphSet));
	TT_NULL_ASSERT(glyphSet);
	if (glyphSet == 0)
	{
		return;
	}
	
	tt::engine::renderer::TexturePtr tex(p_element.quad->getTexture());
	const tt::math::Point2 sizeInPixels(static_cast<s32>(p_element.width  * m_textPixelsPerWorldUnit),
	                                    static_cast<s32>(p_element.height * m_textPixelsPerWorldUnit));
	
	tt::engine::renderer::TexturePainter painter(tex->lock());
	painter.clear();
	if (p_element.renderedText.empty() == false)
	{
		using tt::engine::glyph::GlyphSet;
		
		GlyphSet::Alignment verticalAlignment   = GlyphSet::ALIGN_TOP;
		switch (p_element.verticalAlignment)
		{
		case entity::graphics::VerticalAlignment_Top:    verticalAlignment = GlyphSet::ALIGN_TOP;    break;
		case entity::graphics::VerticalAlignment_Center: verticalAlignment = GlyphSet::ALIGN_CENTER; break;
		case entity::graphics::VerticalAlignment_Bottom: verticalAlignment = GlyphSet::ALIGN_BOTTOM; break;
		default: TT_PANIC("Unknown HorizontalAlignment: %d\n", p_element.verticalAlignment);
		}
		GlyphSet::Alignment horizontalAlignment = GlyphSet::ALIGN_LEFT;
		switch (p_element.horizontalAlignment)
		{
		case entity::graphics::HorizontalAlignment_Left:   horizontalAlignment = GlyphSet::ALIGN_LEFT;   break;
		case entity::graphics::HorizontalAlignment_Center: horizontalAlignment = GlyphSet::ALIGN_CENTER; break;
		case entity::graphics::HorizontalAlignment_Right:  horizontalAlignment = GlyphSet::ALIGN_RIGHT;  break;
		default: TT_PANIC("Unknown HorizontalAlignment: %d\n", p_element.horizontalAlignment);
		}
		
		const s32 rightMargin = static_cast<s32>(tex->getWidth() - sizeInPixels.x);
		
		// Calculate which size we'll need for the text.
		const s32 maxWidthInPixels = painter.getTextureWidth();
		const s32       linesNeeded = glyphSet->getLineCount(p_element.renderedText, maxWidthInPixels, 0, rightMargin);
		const s32 pixelHeightNeeded = glyphSet->getMultiLinePixelHeight(linesNeeded, verticalAlignment);
		
		if (pixelHeightNeeded > painter.getTextureHeight())
		{
			// Not enough space to render text!
			
			// Can we fit a single line, but not a double line?
			// (We only want to shorten single line labels, not multiline.)
			const s32 singleLinePixelHeight = glyphSet->getMultiLinePixelHeight(1, verticalAlignment);
			const s32 doubleLinePixelHeight = glyphSet->getMultiLinePixelHeight(2, verticalAlignment);
			if (singleLinePixelHeight < painter.getTextureHeight() &&
			    doubleLinePixelHeight > painter.getTextureHeight())
			{
				// Shorten string and add ... to it.
				static const std::wstring ellipsis(L"...");
				static const s32 dotwidth = glyphSet->getStringPixelWidth(ellipsis);
				
				std::wstring::size_type strIndex = glyphSet->getFit(
						p_element.renderedText,
						painter.getTextureWidth() - dotwidth - rightMargin);
				p_element.renderedText.erase(strIndex); // Drop the part that's too long.
				p_element.renderedText += ellipsis;
			}
		}
		
		glyphSet->drawMultiLineString(
				p_element.renderedText,
				painter,
				tt::engine::renderer::ColorRGB::white,
				horizontalAlignment,
				verticalAlignment,
				0,
				0,
				0,
				rightMargin,
				0);
	}
	
#if !defined(TT_BUILD_FINAL)
	if (m_renderTextBorders)
	{
		// Debug helper: render texture borders to find alignment issues
		for (s32 y = 0; y < painter.getTextureHeight(); ++y)
		{
			painter.setPixel(0,                             y, tt::engine::renderer::ColorRGB::red);
			painter.setPixel(painter.getTextureWidth() - 1, y, tt::engine::renderer::ColorRGB::red);
			painter.setPixel(sizeInPixels.x - 1,            y, tt::engine::renderer::ColorRGB::blue);
		}
		for (s32 x = 0; x < painter.getTextureWidth(); ++x)
		{
			painter.setPixel(x, 0,                              tt::engine::renderer::ColorRGB::red);
			painter.setPixel(x, painter.getTextureHeight() - 1, tt::engine::renderer::ColorRGB::red);
			painter.setPixel(x, sizeInPixels.y - 1,             tt::engine::renderer::ColorRGB::blue);
		}
	}
#endif
	
	p_element.textNeedsRepaint = false;
}


void WorkshopLevelPicker::callbackLevelSelected(ListItem* /*p_item*/, void* p_userData)
{
	WorkshopLevelPicker* picker = reinterpret_cast<WorkshopLevelPicker*>(p_userData);
	TT_NULL_ASSERT(picker);
	if (picker != 0)
	{
		picker->onLevelSelected();
	}
}


void WorkshopLevelPicker::onLevelSelected()
{
	// Notify the callback entity that the selection changed
	m_notifyCallbackEntityThisFrame = true;
	
	TT_NULL_ASSERT(m_elements[Element_LevelList].list);
	
	const steam::Workshop::FileDetails* details = 0;
	if (m_elements[Element_LevelList].list->getSelectedItem() != 0)
	{
		details = steam::Workshop::getInstance()->getCachedFileDetails(
				m_elements[Element_LevelList].list->getSelectedItem()->id);
	}
	
	if (details == 0)
	{
		setText(Element_Title,       L"");
		setText(Element_Description, L"");
		setText(Element_AuthorName,  L"");
		setAuthorAvatarToEmpty();
		setPreviewImageToEmpty();
		m_currentOwnerID = 0;
		return;
	}
	
	m_currentOwnerID = details->details.m_ulSteamIDOwner;
	
	setText(Element_Title,       tt::str::utf8ToUtf16(details->details.m_rgchTitle));
	setText(Element_Description, tt::str::utf8ToUtf16(details->details.m_rgchDescription));
	
	// Update the level preview image
	const std::string previewFilename(steam::Workshop::getInstance()->getLocalPreviewImagePath(
			details->details.m_nPublishedFileId));
	if (previewFilename.empty() || tt::fs::fileExists(previewFilename) == false)
	{
		setPreviewImageToEmpty();
	}
	else
	{
		tt::engine::renderer::TexturePtr tex(tt::engine::cache::FileTextureCache::get(previewFilename));
		if (tex != 0)
		{
			using tt::engine::renderer::QuadSprite;
			m_elements[Element_PreviewImage].quad->setTexture(tex);
			if (m_elements[Element_PreviewImage].quad->checkFlag(QuadSprite::Flag_Visible) == false ||
			    m_elements[Element_PreviewImage].quad->checkFlag(QuadSprite::Flag_FadingOut))
			{
				m_elements[Element_PreviewImage].quad->fadeIn(0.25f);
			}
		}
		else
		{
			setPreviewImageToEmpty();
		}
	}
	
	// Get the level author's name and avatar
	if (SteamFriends()->RequestUserInformation(m_currentOwnerID, false) == false)
	{
		setAvatarFromUser(m_currentOwnerID);
		setNameFromUser  (m_currentOwnerID);
	}
	else
	{
		// Info not yet available: it will arrive later via Steam callbacks
		setAuthorAvatarToEmpty();
		setText(Element_AuthorName, L"");
	}
}


void WorkshopLevelPicker::onPersonaStateChange(PersonaStateChange_t* p_param)
{
	if (p_param->m_ulSteamID != m_currentOwnerID)
	{
		// Not interested in this user
		return;
	}
	
	if ((p_param->m_nChangeFlags & k_EPersonaChangeName) == k_EPersonaChangeName)
	{
		//TT_Printf("====> Owner name available.\n");
		setNameFromUser(p_param->m_ulSteamID);
	}
	else if ((p_param->m_nChangeFlags & k_EPersonaChangeAvatar) == k_EPersonaChangeAvatar)
	{
		//TT_Printf("====> Owner avatar available.\n");
		setAvatarFromUser(p_param->m_ulSteamID);
	}
}


void WorkshopLevelPicker::onAvatarImageLoaded(AvatarImageLoaded_t* p_param)
{
	const uint64 id = p_param->m_steamID.ConvertToUint64();
	if (id == m_currentOwnerID)
	{
		//TT_Printf("====> Owner avatar has been loaded (image %d, size %d x %d).\n",
		//          p_param->m_iImage, p_param->m_iWide, p_param->m_iTall);
		setAvatarFromUser(id);
	}
}

// Namespace end
}
}
}

#endif  // defined(TT_STEAM_BUILD)
