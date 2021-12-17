#include <toki/game/script/wrappers/WorkshopLevelPickerWrapper.h>
#include <toki/game/hud/types.h>
#include <toki/game/hud/WorkshopLevelPicker.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/steam/Workshop.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

#if defined(TT_STEAM_BUILD)

static hud::WorkshopLevelPickerPtr getPicker()
{
	return AppGlobal::hasGame() ? AppGlobal::getGame()->getWorkshopLevelPicker() : hud::WorkshopLevelPickerPtr();
}


static void setElementFollowEntity(hud::WorkshopLevelPicker::Element p_element, EntityBase* p_entity)
{
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0,
	             "Cannot set follow entity for Workshop Level Picker element %d if picker has not been created yet.",
	             p_element);
	if (picker != 0)
	{
		entity::EntityHandle handle;
		if (p_entity != 0)
		{
			handle = p_entity->getHandle();
		}
		picker->setElementFollowEntity(p_element, handle);
	}
}


static void setElementSize(hud::WorkshopLevelPicker::Element p_element, real p_width, real p_height)
{
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0,
	             "Cannot set size of Workshop Level Picker element %d if picker has not been created yet.",
	             p_element);
	if (picker != 0)
	{
		picker->setElementSize(p_element, p_width, p_height);
	}
}


static void setElementColor(hud::WorkshopLevelPicker::Element      p_element,
                            const tt::engine::renderer::ColorRGBA& p_color)
{
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0,
	             "Cannot set color of Workshop Level Picker element %d if picker has not been created yet.",
	             p_element);
	if (picker != 0)
	{
		picker->setElementColor(p_element, p_color);
	}
}


static void setElementVerticalAlignment(hud::WorkshopLevelPicker::Element     p_element,
                                        entity::graphics::VerticalAlignment   p_verticalAlignment)
{
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0,
	             "Cannot set vertical alignment of Workshop Level Picker element %d if picker has not been created yet.",
	             p_element);
	if (picker != 0)
	{
		picker->setElementVerticalAlignment(p_element, p_verticalAlignment);
	}
}


static void setElementHorizontalAlignment(hud::WorkshopLevelPicker::Element     p_element,
                                          entity::graphics::HorizontalAlignment p_horizontalAlignment)
{
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0,
	             "Cannot set horizontal alignment of Workshop Level Picker element %d if picker has not been created yet.",
	             p_element);
	if (picker != 0)
	{
		picker->setElementHorizontalAlignment(p_element, p_horizontalAlignment);
	}
}


#endif


//--------------------------------------------------------------------------------------------------
// Public member functions

void WorkshopLevelPickerWrapper::create()
{
#if defined(TT_STEAM_BUILD)
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->createWorkshopLevelPicker();
	}
#endif
}


void WorkshopLevelPickerWrapper::open()
{
#if defined(TT_STEAM_BUILD)
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->openWorkshopLevelPicker();
	}
#endif
}


void WorkshopLevelPickerWrapper::close()
{
#if defined(TT_STEAM_BUILD)
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->closeWorkshopLevelPicker();
	}
#endif
}


bool WorkshopLevelPickerWrapper::isOpen()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		return picker->isOpen();
	}
#endif
	
	return false;
}


float WorkshopLevelPickerWrapper::getSelectedLevelScore()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		return picker->getSelectedLevelScore();
	}
#endif
	
	return 0.0f;
}


bool WorkshopLevelPickerWrapper::isSelectedLevelDownloaded()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		return picker->isSelectedLevelDownloaded();
	}
#endif
	
	return false;
}


bool WorkshopLevelPickerWrapper::isAnyLevelSelected()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		return picker->isAnyLevelSelected();
	}
#endif
	
	return false;
}


void WorkshopLevelPickerWrapper::selectPrevious()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->selectPreviousLevel();
	}
#endif
}


void WorkshopLevelPickerWrapper::selectNext()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->selectNextLevel();
	}
#endif
}


void WorkshopLevelPickerWrapper::playSelectedLevel()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->playSelectedLevel();
	}
#endif
}


void WorkshopLevelPickerWrapper::browseWorkshop()
{
#if defined(TT_STEAM_BUILD)
	steam::Workshop::getInstance()->openBrowseWorkshopPage();
#endif
}


void WorkshopLevelPickerWrapper::showSelectedLevelWorkshopPage()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->showSelectedLevelWorkshopPage();
	}
#endif
}


void WorkshopLevelPickerWrapper::setShowTextBorders(bool p_show)
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->setShowTextBorders(p_show);
	}
#else
	(void)p_show;
#endif
}


void WorkshopLevelPickerWrapper::setCallbackEntity(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	if (picker != 0)
	{
		entity::EntityHandle handle;
		if (p_entity != 0)
		{
			handle = p_entity->getHandle();
		}
		picker->setCallbackEntity(handle);
	}
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setFollowEntityLevelList(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	setElementFollowEntity(hud::WorkshopLevelPicker::Element_LevelList, p_entity);
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setFollowEntityTitle(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	setElementFollowEntity(hud::WorkshopLevelPicker::Element_Title, p_entity);
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setFollowEntityDescription(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	setElementFollowEntity(hud::WorkshopLevelPicker::Element_Description, p_entity);
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setFollowEntityPreviewImage(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	setElementFollowEntity(hud::WorkshopLevelPicker::Element_PreviewImage, p_entity);
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setFollowEntityAuthorAvatar(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	setElementFollowEntity(hud::WorkshopLevelPicker::Element_AuthorAvatar, p_entity);
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setFollowEntityAuthorName(EntityBase* p_entity)
{
#if defined(TT_STEAM_BUILD)
	setElementFollowEntity(hud::WorkshopLevelPicker::Element_AuthorName, p_entity);
#else
	(void)p_entity;
#endif
}


void WorkshopLevelPickerWrapper::setSizeLevelList(real p_width, real p_height)
{
#if defined(TT_STEAM_BUILD)
	setElementSize(hud::WorkshopLevelPicker::Element_LevelList, p_width, p_height);
#else
	(void)p_width; (void)p_height;
#endif
}


void WorkshopLevelPickerWrapper::setSizeTitle(real p_width, real p_height)
{
#if defined(TT_STEAM_BUILD)
	setElementSize(hud::WorkshopLevelPicker::Element_Title, p_width, p_height);
#else
	(void)p_width; (void)p_height;
#endif
}


void WorkshopLevelPickerWrapper::setSizeDescription(real p_width, real p_height)
{
#if defined(TT_STEAM_BUILD)
	setElementSize(hud::WorkshopLevelPicker::Element_Description, p_width, p_height);
#else
	(void)p_width; (void)p_height;
#endif
}


void WorkshopLevelPickerWrapper::setSizePreviewImage(real p_width, real p_height)
{
#if defined(TT_STEAM_BUILD)
	setElementSize(hud::WorkshopLevelPicker::Element_PreviewImage, p_width, p_height);
#else
	(void)p_width; (void)p_height;
#endif
}


void WorkshopLevelPickerWrapper::setSizeAuthorAvatar(real p_width, real p_height)
{
#if defined(TT_STEAM_BUILD)
	setElementSize(hud::WorkshopLevelPicker::Element_AuthorAvatar, p_width, p_height);
#else
	(void)p_width; (void)p_height;
#endif
}


void WorkshopLevelPickerWrapper::setSizeAuthorName(real p_width, real p_height)
{
#if defined(TT_STEAM_BUILD)
	setElementSize(hud::WorkshopLevelPicker::Element_AuthorName, p_width, p_height);
#else
	(void)p_width; (void)p_height;
#endif
}


void WorkshopLevelPickerWrapper::setColorTitle(const tt::engine::renderer::ColorRGBA& p_color)
{
#if defined(TT_STEAM_BUILD)
	setElementColor(hud::WorkshopLevelPicker::Element_Title, p_color);
#else
	(void)p_color;
#endif
}


void WorkshopLevelPickerWrapper::setColorDescription(const tt::engine::renderer::ColorRGBA& p_color)
{
#if defined(TT_STEAM_BUILD)
	setElementColor(hud::WorkshopLevelPicker::Element_Description, p_color);
#else
	(void)p_color;
#endif
}


void WorkshopLevelPickerWrapper::setColorAuthorName(const tt::engine::renderer::ColorRGBA& p_color)
{
#if defined(TT_STEAM_BUILD)
	setElementColor(hud::WorkshopLevelPicker::Element_AuthorName, p_color);
#else
	(void)p_color;
#endif
}

void WorkshopLevelPickerWrapper::setElementVerticalAlignmentTitle(entity::graphics::VerticalAlignment p_verticalAlignment)
{
#if defined(TT_STEAM_BUILD)
	setElementVerticalAlignment(hud::WorkshopLevelPicker::Element_Title, p_verticalAlignment);
#else
	(void)p_verticalAlignment;
#endif
}

void WorkshopLevelPickerWrapper::setElementVerticalAlignmentDescription(entity::graphics::VerticalAlignment     p_verticalAlignment)
{
#if defined(TT_STEAM_BUILD)
	setElementVerticalAlignment(hud::WorkshopLevelPicker::Element_Description, p_verticalAlignment);
#else
	(void)p_verticalAlignment;
#endif
}

void WorkshopLevelPickerWrapper::setElementVerticalAlignmentAuthorName( entity::graphics::VerticalAlignment     p_verticalAlignment)
{
#if defined(TT_STEAM_BUILD)
	setElementVerticalAlignment(hud::WorkshopLevelPicker::Element_AuthorName, p_verticalAlignment);
#else
	(void)p_verticalAlignment;
#endif
}

void WorkshopLevelPickerWrapper::setElementHorizontalAlignmentTitle(      entity::graphics::HorizontalAlignment p_horizontalAlignment)
{
#if defined(TT_STEAM_BUILD)
	setElementHorizontalAlignment(hud::WorkshopLevelPicker::Element_Title, p_horizontalAlignment);
#else
	(void)p_horizontalAlignment;
#endif
}

void WorkshopLevelPickerWrapper::setElementHorizontalAlignmentDescription(entity::graphics::HorizontalAlignment p_horizontalAlignment)
{
#if defined(TT_STEAM_BUILD)
	setElementHorizontalAlignment(hud::WorkshopLevelPicker::Element_Description, p_horizontalAlignment);
#else
	(void)p_horizontalAlignment;
#endif
}

void WorkshopLevelPickerWrapper::setElementHorizontalAlignmentAuthorName( entity::graphics::HorizontalAlignment p_horizontalAlignment)
{
#if defined(TT_STEAM_BUILD)
	setElementHorizontalAlignment(hud::WorkshopLevelPicker::Element_AuthorName, p_horizontalAlignment);
#else
	(void)p_horizontalAlignment;
#endif
}

hud::ListBoxColorScheme WorkshopLevelPickerWrapper::getLevelListColorScheme()
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0, "Cannot get color scheme of Workshop Level Picker level list "
	             "if picker has not been created yet.");
	if (picker != 0)
	{
		return picker->getLevelListColorScheme();
	}
#endif
	
	return hud::ListBoxColorScheme();
}


void WorkshopLevelPickerWrapper::setLevelListColorScheme(const hud::ListBoxColorScheme& p_colorScheme)
{
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0, "Cannot set color scheme of Workshop Level Picker level list "
	             "if picker has not been created yet.");
	if (picker != 0)
	{
		picker->setLevelListColorScheme(p_colorScheme);
	}
#else
	(void)p_colorScheme;
#endif
}


void WorkshopLevelPickerWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(WorkshopLevelPickerWrapper, "WorkshopLevelPicker");
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, create);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, open);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, close);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, isOpen);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, getSelectedLevelScore);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, isSelectedLevelDownloaded);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, isAnyLevelSelected);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, selectPrevious);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, selectNext);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, playSelectedLevel);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, browseWorkshop);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, showSelectedLevelWorkshopPage);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setShowTextBorders);
	
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setCallbackEntity);
	
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setFollowEntityLevelList);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setFollowEntityTitle);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setFollowEntityDescription);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setFollowEntityPreviewImage);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setFollowEntityAuthorAvatar);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setFollowEntityAuthorName);
	
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setSizeLevelList);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setSizeTitle);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setSizeDescription);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setSizePreviewImage);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setSizeAuthorAvatar);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setSizeAuthorName);
	
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setColorTitle);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setColorDescription);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setColorAuthorName);
	
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setElementVerticalAlignmentTitle);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setElementVerticalAlignmentDescription);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setElementVerticalAlignmentAuthorName);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setElementHorizontalAlignmentTitle);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setElementHorizontalAlignmentDescription);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setElementHorizontalAlignmentAuthorName);
	
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, getLevelListColorScheme);
	TT_SQBIND_STATIC_METHOD(WorkshopLevelPickerWrapper, setLevelListColorScheme);
}

// Namespace end
}
}
}
}
