#if !defined(INC_TOKI_AUDIO_CONSTANTS_H)
#define INC_TOKI_AUDIO_CONSTANTS_H


#include <string>


namespace toki {
namespace audio {

/*! \brief The audio categories supported by the audio player. */
enum Category
{
	Category_Ambient,
	Category_Effects,
	Category_VoiceOver,
	Category_Music,
	
	Category_Count,
	Category_Invalid
};

inline bool isValidCategory(Category p_cat) { return p_cat >= 0 && p_cat < Category_Count; }
const char* getCategoryName(Category p_cat);
Category    getCategoryFromName(const std::string& p_name);


/*! \brief The reverb effects supported by the audio player. */
enum ReverbEffect
{
	ReverbEffect_None,   //!< No reverb.
	ReverbEffect_Forest, //!< Suitable for forest.
	ReverbEffect_Cave,   //!< Suitable for cave.
	
	ReverbEffect_Count,
	ReverbEffect_Invalid
};

inline bool  isValidReverbEffect(ReverbEffect p_effect) { return p_effect >= 0 && p_effect < ReverbEffect_Count; }
const char*  getReverbEffectName(ReverbEffect p_effect);
ReverbEffect getReverbEffectFromName(const std::string& p_name);


enum Device
{
	Device_TV,
	Device_DRC,
	Device_Count
};

inline bool isValidDevice(Device p_device) { return p_device >= 0 && p_device < Device_Count; }

// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_CONSTANTS_H)
