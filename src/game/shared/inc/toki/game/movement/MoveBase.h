#if !defined(INC_TOKI_GAME_MOVEMENT_MOVEBASE_H)
#define INC_TOKI_GAME_MOVEMENT_MOVEBASE_H


#include <tt/math/Point2.h>
#include <tt/pres/fwd.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/movement/Validator.h>
#include <toki/game/movement/fwd.h>
#include <toki/pres/fwd.h>


namespace toki {
namespace game {
namespace movement {


class MoveBase
{
public:
	typedef std::vector<tt::math::Point2> SubSteps; // FIXME: Move to MoveAnimation
	enum Flag
	{
		Flag_StartNeedsXPositionSnap, //!< Should snap X at start of movement step.
		Flag_StartNeedsYPositionSnap, //!< Should snap Y at start of movement step.
		Flag_EndNeedsXPositionSnap,   //!< If movement ends with this step should X be snapped.
		Flag_EndNeedsYPositionSnap,   //!< If movement ends with this step should X be snapped.
		Flag_PersistentMove,          //!< If this move fails it will turn on persistent movement in the controller.
		
		Flag_IgnoreCollision, //!< FIXME: Remove this!. (QuickFix to make toki move down solid stairs again, needs to be fixed differently)
		
		Flag_Count
	};
	typedef tt::code::BitMask<Flag, Flag_Count> Flags;
	
	virtual ~MoveBase() { }
	
	inline const Directions& getDirections() const { return m_directions;                        }
	inline entity::LocalDir  getLocalDir()   const { return m_localDir;                          }
	inline bool              hasLocalDir()   const { return entity::isValidLocalDir(m_localDir); }
	inline bool              isForwardLeft() const { return m_forwardIsLeft;                     }
	bool validate(const SurroundingsSurvey& p_survey, entity::LocalDir p_localDir,
	              movement::Direction p_down, bool p_forwardIsLeft) const;
	
	inline const Flags& getFlags() const { return m_flags; }
	
	inline const std::string& getName()     const { return m_name; }
	inline NameHash           getNameHash() const { return m_nameHash; }
	inline const std::string& getAnimationName() const { return m_animationName; }
	
	inline bool               hasStartCallback() const { return m_startCallback.empty() == false; }
	inline const std::string& getStartCallback() const { return m_startCallback;                  }
	inline bool               hasEndCallback()   const { return m_endCallback.empty()   == false; }
	inline const std::string& getEndCallback()   const { return m_endCallback;                    }
	
	inline s32 getPriority() const { return m_priority; }
	inline bool isAlways()   const { return m_always;   }
	
	// FIXME: Remove virtual functions
	virtual const tt::math::Vector2& getSpeed() const = 0;
	virtual void startAllPresentationObjects(entity::Entity& p_entity,
                                             entity::movementcontroller::DirectionalMovementController& p_movementController) const = 0;
	virtual inline bool isAnimation() const { return false; };        // FIXME: Remove type check.
	virtual const MoveAnimation* getMoveAnimation() const { return 0; }
	
	virtual MoveBasePtr createRotatedLocalDirForDown(Direction p_down, bool p_forwardIsLeft = false) const = 0;
	
	/*! \brief Starts a presentation object if the move data specifies animation tags to start */
	void startPresentationObjectImpl(toki::pres::PresentationObject& p_pres, bool p_forceRestart) const;
	
	/*! \brief Starts all presentation objects of an entity if the move data specifies animation tags to start */
	void startAllPresentationObjectsImpl(entity::Entity& p_entity,
                                         entity::movementcontroller::DirectionalMovementController& p_movementController, bool p_forceRestart) const;
	
	inline const tt::pres::Tags& getAnimationTags() const { return m_animationTags; }
	
protected:
	inline MoveBase(const Directions&     p_directions,
	                entity::LocalDir      p_localDir,
	                const Validator&      p_validator,
	                const Flags&          p_flags,
	                s32                   p_priority,
	                const tt::pres::Tags& p_animationTags,
	                const std::string&    p_animationName,
	                const std::string&    p_name,
	                const std::string&    p_startCallback,
	                const std::string&    p_endCallback,
	                bool                  p_always)
	:
	m_directions(p_directions),
	m_localDir(p_localDir),
	m_always(p_always),
	m_orientationDown(movement::Direction_Down),
	m_forwardIsLeft(false),
	m_validator(p_validator),
	m_flags(p_flags),
	m_priority(p_priority),
	m_animationTags(p_animationTags),
	m_animationName(p_animationName),
	m_name(p_name),
	m_nameHash(p_name),
	m_startCallback(p_startCallback),
	m_endCallback(p_endCallback)
	{
		TT_ASSERTMSG( (m_directions.isEmpty() == false && hasLocalDir() == false) ||
		              (m_directions.isEmpty()          && hasLocalDir()),
		              "Must have a direction, but both types at the same time. (has%s directions, has local_dir '%s').",
		              (m_directions.isEmpty() ? " no" : ""), entity::getLocalDirName(m_localDir) );
	}
	
	void rotateForDown(Direction p_down, bool p_forwardIsLeft);
	
private:
	Directions       m_directions; // Which (World) direction(s) this Move moves in.
	entity::LocalDir m_localDir;   // Which  Local  direction this Move moves in. (can be none)
	bool             m_always;     // Whether this move is valid for all directions.
	Direction        m_orientationDown;
	bool             m_forwardIsLeft;
	Validator        m_validator;  // Use to check if this Move may be done at a specific location.
	Flags            m_flags;
	s32              m_priority; // Higher is more important. (Can be < 0)
	tt::pres::Tags   m_animationTags;  // If not empty, starts a presentation animation with these tags
	std::string      m_animationName;  // Name of the presentation animation
	
	const std::string m_name;          // Movementset-specified name of this move (passed to callbacks)
	const NameHash    m_nameHash;      // Hash value for name.
	std::string       m_startCallback; // Name of the callback to call when move starts (empty = no callback)
	std::string       m_endCallback;   // Name of the callback to call when move ends (empty = no callback)
	
	const MoveBase& operator=(const MoveBase&); // Disable assigment
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_MOVEBASE_H)
