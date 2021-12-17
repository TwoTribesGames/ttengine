#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_OPTIONSWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_OPTIONSWRAPPER_H


#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>
#include <tt/script/helpers.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief Options static class. */
class OptionsWrapper
{
public:
	static void setToDefaults(); //!< \brief Reset the options to their defaults.
	
	static bool             isFullScreen();       //!< \brief Are we running in fullscreen? (Non-windowed).
	static tt::math::Point2 getFullscreenSize();  //!< \brief Get Fullscreen size
	static tt::math::Point2 getWindowedSize();    //!< \brief Get Windowed size
	static s32              getBlurQuality();     //!< \brief Get blur quality
	static s32              getShadowQuality();   //!< \brief Get Shadow quality
	static bool             hasPostProcessing();  //!< \brief Is postprocessing enabled? (Color grading)
	static bool             in30FpsMode();        //!< \brief Is in 30 fps mode? (Otherwise 60 fps)
	static bool             wasLowPerfReported(); //!< \brief Returns wether the low / bad performance was reported to the user.
	
	static void setFullScreen(    bool                    p_fullscreen); //!< \brief Change to fullscreen (or windowed) mode.
	static void setWindowedSize(  const tt::math::Point2& p_size      ); //!< \brief Set Windowed size
	static void setFullscreenSize(const tt::math::Point2& p_size      ); //!< \brief Set Fullscreen size
	static void setBlurQuality(   s32                     p_quality   ); //!< \brief Set blur quality
	static void setShadowQuality( s32                     p_quality   ); //!< \brief Set Shadow quality
	static void setPostProcessing(bool                    p_enabled   ); //!< \brief Enabled or disable postprocessing (Color grading)
	static void set30FpsMode(     bool                    p_enabled   ); //!< \brief Enabled or disable 30 fps mode.
	static void setLowPerfReported(bool                   p_reported  ); //!< \brief Call when the low / bad performance was reported to the user.
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_OPTIONSWRAPPER_H)
