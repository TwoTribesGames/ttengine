#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_PRESENTATIONSTARTSETTINGSWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_PRESENTATIONSTARTSETTINGSWRAPPER_H

#include <tt/code/fwd.h>
#include <tt/math/Vector2.h>
#include <tt/pres/fwd.h>
#include <tt/str/str.h>

#include <toki/pres/PresentationObject.h>
#include <toki/serialization/utils.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief The start settings for a presentation animation */
class PresentationStartSettingsWrapper
{
public:
	PresentationStartSettingsWrapper()
	:
	m_settings()
	{
	}
	
	/*! \brief Position to move to using a game translation */
	inline void setEndPos(const tt::math::Vector2& p_endPos)
	{
		m_settings.setEndPos(p_endPos);
	}
	
	/*! \brief If set, a callback is generated once the presentation animation has ended
		\param p_name is the name provided to the onPresentationAnimationEnded() callback. If empty, no callback is generated */
	inline void setEndCallbackName(const std::string& p_name)
	{
		m_settings.setEndCallbackName(p_name);
	}
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	inline const pres::PresentationStartSettings& getSettings() const { return m_settings; }
	
private:
	pres::PresentationStartSettings m_settings;
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_PRESENTATIONSTARTSETTINGSWRAPPER_H)
