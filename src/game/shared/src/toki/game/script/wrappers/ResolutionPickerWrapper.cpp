#include <toki/game/script/wrappers/ResolutionPickerWrapper.h>
#include <toki/game/hud/types.h>
#include <toki/game/hud/ResolutionPicker.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/Game.h>
#include <toki/steam/Workshop.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


static hud::ResolutionPickerPtr getPicker()
{
	return AppGlobal::hasGame() ? AppGlobal::getGame()->getResolutionPicker() : hud::ResolutionPickerPtr();
}


//--------------------------------------------------------------------------------------------------
// Public member functions

void ResolutionPickerWrapper::create()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->createResolutionPicker();
	}
}


void ResolutionPickerWrapper::open()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->openResolutionPicker();
	}
}


void ResolutionPickerWrapper::close()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->closeResolutionPicker();
	}
}


bool ResolutionPickerWrapper::isOpen()
{
	hud::ResolutionPickerPtr picker(getPicker());
	if (picker != 0)
	{
		return picker->isOpen();
	}
	
	return false;
}


void ResolutionPickerWrapper::selectPrevious()
{
	hud::ResolutionPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->selectPrevious();
	}
}


void ResolutionPickerWrapper::selectNext()
{
	hud::ResolutionPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->selectNext();
	}
}


void ResolutionPickerWrapper::applySelectedResolution()
{
	hud::ResolutionPickerPtr picker(getPicker());
	if (picker != 0)
	{
		picker->applySelectedResolution();
	}
}


void ResolutionPickerWrapper::setCallbackEntity(EntityBase* p_entity)
{
	hud::ResolutionPickerPtr picker(getPicker());
	if (picker != 0)
	{
		entity::EntityHandle handle;
		if (p_entity != 0)
		{
			handle = p_entity->getHandle();
		}
		picker->setCallbackEntity(handle);
	}
}


void ResolutionPickerWrapper::setFollowEntity(EntityBase* p_entity)
{
	hud::ResolutionPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0, "Cannot set follow entity if picker has not been created yet.");
	if (picker != 0)
	{
		entity::EntityHandle handle;
		if (p_entity != 0)
		{
			handle = p_entity->getHandle();
		}
		picker->setFollowEntity(handle);
	}
}


void ResolutionPickerWrapper::setSize(real p_width, real p_height)
{
	hud::ResolutionPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0, "Cannot set size if picker has not been created yet.");
	if (picker != 0)
	{
		picker->setSize(p_width, p_height);
	}
}


hud::ListBoxColorScheme ResolutionPickerWrapper::getColorScheme()
{
	hud::ResolutionPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0, "Cannot get color scheme if picker has not been created yet.");
	if (picker != 0)
	{
		return picker->getColorScheme();
	}
	return hud::ListBoxColorScheme();
}


void ResolutionPickerWrapper::setColorScheme(const hud::ListBoxColorScheme& p_colorScheme)
{
	hud::ResolutionPickerPtr picker(getPicker());
	TT_ASSERTMSG(picker != 0, "Cannot set color scheme if picker has not been created yet.");
	if (picker != 0)
	{
		picker->setColorScheme(p_colorScheme);
	}
}


void ResolutionPickerWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(ResolutionPickerWrapper, "ResolutionPicker");
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, create);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, open);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, close);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, isOpen);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, selectPrevious);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, selectNext);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, applySelectedResolution);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, setCallbackEntity);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, setFollowEntity);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, setSize);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, getColorScheme);
	TT_SQBIND_STATIC_METHOD(ResolutionPickerWrapper, setColorScheme);
}

// Namespace end
}
}
}
}
