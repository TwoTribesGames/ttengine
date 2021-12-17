#if !defined(INC_TOKI_APPOPTIONS_H)
#define INC_TOKI_APPOPTIONS_H

#include <string>

#include <tt/engine/scene2d/fwd.h>
#include <tt/platform/tt_types.h>
#include <tt/math/Point2.h>
#include <toki/game/light/fwd.h>


#define APP_SUPPORTS_OPTIONS_SAVING 1

namespace toki {

struct AppOptions
{
	tt::math::Point2 fullscreenSize;
	tt::math::Point2 windowedSize;
	tt::math::Point2 upscaleSize;
	bool             windowed;
	s32              blurQuality;
	s32              shadowQuality;
	bool             postProcessing; // This is only Color Grading.
	bool             in30FpsMode;
	bool             lowPerfReported;
	
	void setToDefaults();
	
	// Use the following functions to change settings which will make this class dirty if changed.
	bool setWindowed(      bool                    p_windowed);
	bool setWindowedSize(  const tt::math::Point2& p_size    );
	bool setFullscreenSize(const tt::math::Point2& p_size    );
	bool setUpscaleSize   (const tt::math::Point2& p_size, const tt::math::Point2& p_fullscreenSize);
	bool setBlurQuality   (s32                     p_quality );
	bool setShadowQuality (s32                     p_quality );
	bool setPostProcessing(bool                    p_enabled );
	bool set30FpsMode     (bool                    p_enabled );
	bool setLowPerfReported(bool                   p_reported);
	
	void setBlurQualityInGame() const;
	void setShadowQualityInGame() const;
	void setFullscreenSizeInApp() const;
	void setUpscaleSizeInApp() const;
	
	inline bool isDirty() const { return m_dirty; }
	bool saveIfDirty() { if (isDirty()) { return save(); } return true; }
	
	bool load();
	bool save() const;
	
	static void createInstance();
	static AppOptions& getInstance();
	static void destroyInstance();
	
private:
	AppOptions();
	
	tt::engine::scene2d::BlurQuality makeValidBlurQuality  (s32 p_value) const;
	game::light::ShadowQuality       makeValidShadowQuality(s32 p_value) const;
	
	bool save(const std::string& p_relativeFilename) const;
	bool load(const std::string& p_relativeFilename);
	
	AppOptions(const AppOptions&) = delete;                  // Disable copy.
	
	inline void makeDirty() { m_dirty = true; }
	mutable bool m_dirty;
	
	static AppOptions* ms_instance;
};

// Namespace end
}


#endif  // !defined(INC_TOKI_APPOPTIONS_H)
