#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
#include <tt/engine/renderer/DXUT/DXUT.h>
#endif

#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/engine/renderer/pp/PostProcessor.h>

#include <json/json.h>

#include <tt/app/Application.h>
#include <tt/doc/json/json_util.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#include <toki/game/Game.h>
#include <toki/game/light/LightMgr.h>
#include <toki/savedata/utils.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>


namespace toki {

AppOptions* AppOptions::ms_instance = 0;

static const char* const g_appOptionsFile       = "applicationOptions.json";
static const char* const g_appOptionsFileEditor = "applicationOptionsEditor.json";

static inline const char* const getAppOptionsFile()
{
	return (AppGlobal::isInLevelEditorMode()) ? g_appOptionsFileEditor : g_appOptionsFile;
}

//--------------------------------------------------------------------------------------------------
// Public member functions


void AppOptions::setToDefaults()
{
	*this = AppOptions();
	setBlurQualityInGame();
	setShadowQualityInGame();
	setFullscreenSizeInApp();
	setUpscaleSizeInApp();
}


bool AppOptions::setWindowed(bool p_windowed)
{
	if (windowed != p_windowed)
	{
		windowed = p_windowed;
		makeDirty();
		return true;
	}
	return false;
}


bool AppOptions::setWindowedSize(const tt::math::Point2& p_size)
{
	if (windowedSize != p_size)
	{
		windowedSize = p_size;
		makeDirty();
		return true;
	}
	return false;
}


bool AppOptions::setUpscaleSize(const tt::math::Point2& p_size, const tt::math::Point2& p_fullscreenSize)
{
	if (upscaleSize != p_size)
	{
		upscaleSize    = p_size;
		fullscreenSize = p_fullscreenSize;
		makeDirty();
		setUpscaleSizeInApp();
		return true;
	}
	return false;
}


bool AppOptions::setFullscreenSize(const tt::math::Point2& p_size)
{
	if (fullscreenSize != p_size)
	{
		fullscreenSize = p_size;
		makeDirty();
		setFullscreenSizeInApp();
		return true;
	}
	return false;
}


bool AppOptions::setBlurQuality(s32 p_quality)
{
	if (blurQuality != p_quality)
	{
		blurQuality = makeValidBlurQuality(p_quality);
		makeDirty();
		setBlurQualityInGame();
		return true;
	}
	return false;
}


bool AppOptions::setShadowQuality(s32 p_quality)
{
	if (shadowQuality != p_quality )
	{
		shadowQuality = makeValidShadowQuality(p_quality);
		makeDirty();
		setShadowQualityInGame();
		return true;
	}
	return false;
}


bool AppOptions::setPostProcessing(bool p_enabled)
{
	if (postProcessing != p_enabled )
	{
		postProcessing = p_enabled;
		makeDirty();
		if (tt::engine::renderer::Renderer::hasInstance())
		{
			tt::engine::renderer::Renderer::getInstance()->getPP()->setActive(postProcessing);
		}
		return true;
	}
	return false;
}


bool AppOptions::set30FpsMode(bool p_enabled )
{
	if (in30FpsMode != p_enabled )
	{
		in30FpsMode = p_enabled;
		makeDirty();
		AppGlobal::setDoubleUpdateMode(in30FpsMode);
		return true;
	}
	return false;
}


bool AppOptions::setLowPerfReported(bool p_reported)
{
	if (lowPerfReported != p_reported )
	{
		lowPerfReported = p_reported;
		makeDirty();
		return true;
	}
	return false;
}


void AppOptions::setBlurQualityInGame() const
{
	if (AppGlobal::hasGame())
	{
		const tt::engine::scene2d::shoebox::ShoeboxPtr& shoebox = AppGlobal::getGame()->getShoebox();
		TT_NULL_ASSERT(shoebox);
		if (shoebox != 0)
		{
			shoebox->setBlurQuality(makeValidBlurQuality(blurQuality));
		}
	}
}


void AppOptions::setShadowQualityInGame() const
{
	game::light::LightMgr::setShadowQuality(makeValidShadowQuality(shadowQuality));
}


void AppOptions::setFullscreenSizeInApp() const
{
	if (tt::app::hasApplication() && fullscreenSize.x > 0 && fullscreenSize.y > 0)
	{
		tt::app::Application* app = tt::app::getApplication();
		app->setFullScreenResolution(fullscreenSize);
		if (app->isFullScreen())
		{
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
			DXUTForceDeviceReset();
#else
			app->handleResolutionChanged();
#endif
		}
	}
}


void AppOptions::setUpscaleSizeInApp() const
{
	if (tt::app::hasApplication() && upscaleSize.x > 0 && upscaleSize.y > 0)
	{
		using namespace tt::engine::renderer;
		if (Renderer::hasInstance())
		{
			tt::app::Application* app = tt::app::getApplication();
			Renderer::getInstance()->getUpScaler()->setMaxSize(upscaleSize);
			app->setFullScreenResolution(fullscreenSize);
#if defined(TT_PLATFORM_WIN) && !defined(TT_PLATFORM_SDL)
			if (getRenderDevice() != 0)
			{
				DXUTForceDeviceReset();
			}
#else
			app->handleResolutionChanged();
#endif
		}
	}
}


bool AppOptions::load()       { return load(getAppOptionsFile()); }
bool AppOptions::save() const { return save(getAppOptionsFile()); }


void AppOptions::createInstance()
{
	if (ms_instance != 0)
	{
		TT_PANIC("Instance already exists!");
		return;
	}
	
	ms_instance = new AppOptions;
	
	if (ms_instance->load() == false || ms_instance->isDirty())
	{
		// load failed, (re)create new file.
		
		if (tt::fs::createSaveRootDir() == false)
		{
			TT_PANIC("Failed to create save root dir!");
		}
		else if (ms_instance->save() == false)
		{
			TT_PANIC("Failed to save AppOptions to '%s'!", getAppOptionsFile());
		}
	}
}


AppOptions& AppOptions::getInstance()
{
	TT_NULL_ASSERT(ms_instance);
	return *ms_instance;
}


void AppOptions::destroyInstance()
{
	if (ms_instance == 0)
	{
		TT_PANIC("Instance already deleted!");
		return;
	}
	
	delete ms_instance;
	ms_instance = 0;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

AppOptions::AppOptions()
:
fullscreenSize(),
windowedSize(),
upscaleSize(),
#if defined(TT_BUILD_FINAL)
// Start in full-screen by default (only for final builds, so that debugging doesn't become annoying)
windowed(false),
#else
windowed(true),
#endif
blurQuality(tt::engine::scene2d::BlurQuality_TwoPassConvolution),
shadowQuality(game::light::ShadowQuality_High),
postProcessing(true),
in30FpsMode(false),
lowPerfReported(false),
m_dirty(true)
{
}


tt::engine::scene2d::BlurQuality AppOptions::makeValidBlurQuality(s32 p_value) const
{
	using namespace tt::engine::scene2d;
	switch (p_value)
	{
		// These are allowed
	case BlurQuality_OnePassThreeSamples:
	case BlurQuality_OnePassFourSamples:
	case BlurQuality_TwoPassConvolution:
		return static_cast<BlurQuality>(p_value);
		
		// These are not allowed. (Return default)
	case BlurQuality_NoBlur:
	default:
		return BlurQuality_TwoPassConvolution;
	}
}


game::light::ShadowQuality AppOptions::makeValidShadowQuality(s32 p_value) const
{
	switch (p_value)
	{
		// These are allowed
	case game::light::ShadowQuality_Low:
	case game::light::ShadowQuality_Medium:
	case game::light::ShadowQuality_High:
		return static_cast<game::light::ShadowQuality>(p_value);
		
	default:
		return game::light::ShadowQuality_High;
	}
}


bool AppOptions::save(const std::string& p_relativeFilename) const
{
	m_dirty = false;
	
#if APP_SUPPORTS_OPTIONS_SAVING
	tt::fs::FilePtr file = savedata::createSaveFile(p_relativeFilename, false);
	if (file == 0)
	{
		return false;
	}
	
	// Create a JSON document containing the settings
	Json::Value rootNode(Json::objectValue);
	
	// - Graphical settings
	rootNode["fullscreenSize" ] = tt::doc::json::writePoint2(fullscreenSize );
	rootNode["windowedSize"   ] = tt::doc::json::writePoint2(windowedSize   );
	rootNode["upscaleSize"    ] = tt::doc::json::writePoint2(upscaleSize    );
	rootNode["windowed"       ] = tt::doc::json::writeBool  (windowed       );
	rootNode["blurQuality"    ] = tt::doc::json::writeS32   (blurQuality    );
	rootNode["shadowQuality"  ] = tt::doc::json::writeS32   (shadowQuality  );
	rootNode["postProcessing" ] = tt::doc::json::writeBool  (postProcessing );
	rootNode["in30FpsMode"    ] = tt::doc::json::writeBool  (in30FpsMode    );
	rootNode["lowPerfReported"] = tt::doc::json::writeBool  (lowPerfReported);
	
	// Write the settings data as nicely formatted JSON
	const std::string jsonText = Json::StyledWriter().write(rootNode);
	const tt::fs::size_type bytesToWrite = static_cast<tt::fs::size_type>(jsonText.length());
	
	if (file->write(jsonText.c_str(), bytesToWrite) != bytesToWrite)
	{
		return false;
	}
	
	return true;
#else
	// Do not save editor settings in final CAT builds (unnecessary save data usage)
	(void)p_relativeFilename;
	return true;
#endif
}


bool AppOptions::load(const std::string& p_relativeFilename)
{
	// Start out with default values
	setToDefaults();
	
	m_dirty = false;
	
#if APP_SUPPORTS_OPTIONS_SAVING
	tt::fs::FilePtr file = savedata::openSaveFile(p_relativeFilename, false);
	if (file == 0 || file->getLength() <= 0)
	{
		return false;
	}
	
	// Parse the entire file as JSON
	tt::code::BufferPtr fileContent = file->getContent();
	if (fileContent == 0)
	{
		return false;
	}
	
	const char* jsonDataBegin = reinterpret_cast<const char*>(fileContent->getData());
	const char* jsonDataEnd   = jsonDataBegin + fileContent->getSize();
	
	Json::Value rootNode;
	Json::Reader reader;
	if (reader.parse(jsonDataBegin, jsonDataEnd, rootNode, false) == false)
	{
		TT_PANIC("Editor settings file could not be parsed as JSON.");
		return false;
	}
	
	TT_ERR_CREATE("Loading AppOptions from file: " << p_relativeFilename);
	
	// We only assert on parsing warnings
	// Also, only override the default value if their was not error during pasring.
	
	// Retrieve the settings from the JSON data
	tt::math::Point2 fullscreenValue = tt::doc::json::readPoint2(rootNode, "fullscreenSize", &errStatus);
	if (errStatus.hasError() == false)
	{
		setFullscreenSize(fullscreenValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	tt::math::Point2 point2Value = tt::doc::json::readPoint2(rootNode, "windowedSize"  , &errStatus);
	if (errStatus.hasError() == false)
	{
		setWindowedSize(point2Value);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	point2Value = tt::doc::json::readPoint2(rootNode, "upscaleSize"  , &errStatus);
	if (errStatus.hasError() == false)
	{
		setUpscaleSize(point2Value, fullscreenValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	bool boolValue = tt::doc::json::readBool(  rootNode, "windowed"      , &errStatus);
	if (errStatus.hasError() == false)
	{
		setWindowed(boolValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	s32 blurValue = tt::doc::json::readS32( rootNode, "blurQuality", &errStatus );
	if (errStatus.hasError() == false)
	{
		setBlurQuality(blurValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	s32 shadowValue = tt::doc::json::readS32( rootNode, "shadowQuality", &errStatus );
	if (errStatus.hasError() == false)
	{
		setShadowQuality(shadowValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	boolValue = tt::doc::json::readBool(rootNode, "postProcessing", &errStatus );
	if (errStatus.hasError() == false)
	{
		setPostProcessing(boolValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	
	boolValue = tt::doc::json::readBool(rootNode, "in30FpsMode", &errStatus );
	if (errStatus.hasError() == false)
	{
		set30FpsMode(boolValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
	boolValue = tt::doc::json::readBool(rootNode, "lowPerfReported", &errStatus );
	if (errStatus.hasError() == false)
	{
		setLowPerfReported(boolValue);
	}
	TT_ERR_ASSERT_ON_ERROR();
	errStatus.resetError();
	
#else
	(void)p_relativeFilename;
#endif
	
	return true;
}

// Namespace end
}
