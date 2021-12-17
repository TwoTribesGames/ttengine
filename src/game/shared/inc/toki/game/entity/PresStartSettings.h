#if !defined(INC_TOKI_GAME_ENTITY_PRESSTARTSETTINGS_H)
#define INC_TOKI_GAME_ENTITY_PRESSTARTSETTINGS_H

#include <string>

#include <tt/math/Vector2.h>
#include <tt/script/VirtualMachine.h>

namespace toki   /*! */ {
namespace game   /*! */ {
namespace entity /*! */ {

/*! \brief SOON DEPRECATED The start settings for a presentation animation */
class PresStartSettings
{
public:
	PresStartSettings()
	:
	m_enableEndPos(false),
	m_endPos(tt::math::Vector2::zero),
	m_endCallbackName()
	{
	}
	
	/*! \brief Position to move to using a game translation */
	inline void setEndPos(const tt::math::Vector2& p_endPos)
	{
		m_enableEndPos = true;
		m_endPos = p_endPos;
	}
	
	/*! \brief If set, a callback is generated once the presentation animation has ended
		\param p_name is the name provided to the onPresentationAnimationEnded() callback. If empty, no callback is generated */
	inline void setEndCallbackName(const std::string& p_name) { m_endCallbackName = p_name; }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	bool                     isEndPosEnabled()    const { return m_enableEndPos;    }
	const tt::math::Vector2& getEndPos()          const { return m_endPos;          }
	const std::string&       getEndCallbackName() const { return m_endCallbackName; }
	
private:
	bool              m_enableEndPos;
	tt::math::Vector2 m_endPos;
	std::string       m_endCallbackName;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_GAME_ENTITY_PRESSTARTSETTINGS_H)
