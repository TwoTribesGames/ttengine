#if !defined(INC_TOKI_GAME_MOVEMENT_TRANSITION_H)
#define INC_TOKI_GAME_MOVEMENT_TRANSITION_H

#include <tt/code/ErrorStatus.h>
#include <tt/pres/fwd.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/movement/fwd.h>


namespace toki {
namespace game {
namespace movement {


class Transition
{
public:
	inline bool isTurn() const { return m_isTurn; }
	
	inline const std::string&    getAnimationName() const { return m_animationName; }
	inline const tt::pres::Tags& getAnimationTags() const { return m_animationTags; }
	
	inline bool               hasStartCallback() const { return m_startCallback.empty() == false; }
	inline const std::string& getStartCallback() const { return m_startCallback;                  }
	inline bool               hasEndCallback()   const { return m_endCallback.empty()   == false; }
	inline const std::string& getEndCallback()   const { return m_endCallback;                    }
	
	inline TransitionSpeed getSpeed() const { return m_speed; }
	
	static TransitionPtr createFromXML(const tt::xml::XmlNode* p_node, tt::code::ErrorStatus* p_errStatus);
	
	static void serialize(const ConstTransitionPtr& p_transition, tt::code::BufferWriteContext* p_context);
	static TransitionPtr unserialize(tt::code::BufferReadContext* p_context);
	
private:
	Transition(bool p_isTurn)
	:
	m_animationName(),
	m_animationTags(),
	m_startCallback(),
	m_endCallback(),
	m_isTurn(p_isTurn),
	m_speed(TransitionSpeed_Zero)
	{}
	
	std::string    m_animationName; // Name of the presentation animation
	tt::pres::Tags m_animationTags; // If not empty, starts a presentation animation with these tags
	
	std::string    m_startCallback; // Name of the callback to call when transition starts (empty = no callback)
	std::string    m_endCallback;   // Name of the callback to call when transition ends (empty = no callback)
	
	bool m_isTurn;
	
	TransitionSpeed m_speed;
	
	const Transition& operator=(const Transition&); // Disable assigment
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_TRANSITION_H)
