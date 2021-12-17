#include <tt/app/Application.h>

#include <toki/game/script/wrappers/OptionsWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


void OptionsWrapper::setToDefaults()
{
	AppOptions::getInstance().setToDefaults();
	AppOptions::getInstance().save();
}


bool OptionsWrapper::isFullScreen()
{
	if (tt::app::hasApplication() == false)
	{
		return false;
	}
	return tt::app::getApplication()->isFullScreen();
}


tt::math::Point2 OptionsWrapper::getFullscreenSize()
{
	return AppOptions::getInstance().fullscreenSize;
}


tt::math::Point2 OptionsWrapper::getWindowedSize()
{
	return AppOptions::getInstance().windowedSize;
}


s32 OptionsWrapper::getBlurQuality()
{
	return AppOptions::getInstance().blurQuality;
}


s32 OptionsWrapper::getShadowQuality()
{
	return AppOptions::getInstance().shadowQuality;
}


bool OptionsWrapper::hasPostProcessing()
{
	return AppOptions::getInstance().postProcessing;
}


bool OptionsWrapper::in30FpsMode()
{
	return AppOptions::getInstance().in30FpsMode;
}


bool OptionsWrapper::wasLowPerfReported()
{
	return AppOptions::getInstance().lowPerfReported;
}


void OptionsWrapper::setFullScreen(bool p_fullscreen)
{
	if (tt::app::hasApplication() == false)
	{
		return;
	}
	tt::app::getApplication()->setFullScreen(p_fullscreen);
}


void OptionsWrapper::setWindowedSize(const tt::math::Point2& p_size)
{
	AppOptions::getInstance().setWindowedSize(p_size);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::setFullscreenSize(const tt::math::Point2& p_size)
{
	AppOptions::getInstance().setFullscreenSize(p_size);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::setBlurQuality(s32 p_quality)
{
	AppOptions::getInstance().setBlurQuality(p_quality);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::setShadowQuality(s32 p_quality)
{
	AppOptions::getInstance().setShadowQuality(p_quality);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::setPostProcessing(bool p_enabled)
{
	AppOptions::getInstance().setPostProcessing(p_enabled);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::set30FpsMode(bool p_enabled)
{
	AppOptions::getInstance().set30FpsMode(p_enabled);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::setLowPerfReported(bool p_reported)
{
	AppOptions::getInstance().setLowPerfReported(p_reported);
	AppOptions::getInstance().saveIfDirty();
}


void OptionsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(OptionsWrapper, "Options");
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setToDefaults);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, isFullScreen);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, getFullscreenSize);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, getWindowedSize);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, getBlurQuality);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, getShadowQuality);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, hasPostProcessing);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, in30FpsMode);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, wasLowPerfReported);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setFullScreen);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setWindowedSize);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setFullscreenSize);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setBlurQuality);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setShadowQuality);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setPostProcessing);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, set30FpsMode);
	TT_SQBIND_STATIC_METHOD(OptionsWrapper, setLowPerfReported);
}

// Namespace end
}
}
}
}
