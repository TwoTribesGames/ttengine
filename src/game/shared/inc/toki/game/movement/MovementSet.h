#if !defined(INC_TOKI_GAME_MOVEMENT_MOVEMENTSET_H)
#define INC_TOKI_GAME_MOVEMENT_MOVEMENTSET_H

#include <vector>
#include <map>

#include <tt/code/ErrorStatus.h>
#include <tt/xml/XmlNode.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/movement/fwd.h>

namespace toki {
namespace game {
namespace movement {


class MovementSet
{
public:
	typedef void (*removeFunction)(MovementSet*);
	struct MoveID //<! \brief Used by serialization code.
	{
		Direction   direction;
		s32         index;
		std::string movementSetFilename; // Used to make sure this move is from the same movement set.
		                                 // The HASH or CRC of the data file might be even better.
		
		MoveID() : direction(Direction_Invalid), index(-1), movementSetFilename() {}
		inline bool isValid() const { return movementSetFilename.empty() == false; }
	};
	
	static MovementSetPtr create(const std::string& p_filename);
	
	void add(const movement::MoveBasePtr& p_move);
	void addAlways(const movement::MoveBasePtr& p_move);
	
	const MoveBasePtr& getMove(Direction p_direction,
	                           const MoveBasePtr& p_previousMove,
	                           const entity::Entity& p_entity) const;
	
	const std::string& getFilename() const { return m_filename; }
	
	MovementSetPtr clone(removeFunction p_removeFunction = 0) const;
	
	MoveID getMoveID(const MoveBasePtr& p_move);
	MoveBasePtr getFromMoveID(const MoveID& p_id);
	
	ConstTransitionPtr findTransition(const movement::MoveBasePtr& p_fromMove,
	                                  const movement::MoveBasePtr& p_toMove) const;
	
private:
	typedef std::map<NameHash, TransitionPtr> MoveTransitionMap;
	typedef std::map<NameHash, MoveTransitionMap> MoveToMoveMap;
	
	MovementSet(const std::string& p_filename);
	MovementSet(const MovementSet& p_rhs);
	
	static MovementSet* load(const std::string& p_filename);
	static void parseMove(const tt::xml::XmlNode* p_node,
	                      MovementSet* p_moveSet,
	                      tt::code::ErrorStatus* p_errStatus);
	static NameHashes parseMoveReferences(const tt::xml::XmlNode* p_node,
	                                      const std::string& p_nodeName,
	                                      tt::code::ErrorStatus* p_errStatus);
	static SurveyResults parseSurveyResults(const tt::xml::XmlNode* p_node,
	                                        tt::code::ErrorStatus* p_errStatus);
	
	void addSorted(Direction p_direction,
	               const movement::MoveBasePtr& p_move);
	
	static ConstTransitionPtr findTransitionInMap(const MovementSet::MoveTransitionMap& p_moveMap,
	                                              NameHash p_hash);
	
	typedef std::vector<MoveBasePtr> MoveList;
	MoveList m_moves[movement::Direction_Count];
	std::string m_filename;
	
	MoveTransitionMap m_alwaysFrom;
	MoveTransitionMap m_alwaysTo;
	MoveTransitionMap m_turns;
	MoveToMoveMap     m_moveToMove;
	
	MovementSet& operator=(const MovementSet&); // Disable assignment
	
	friend class MovementSetCache;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_MOVEMENTSET_H)
