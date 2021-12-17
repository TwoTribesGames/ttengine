#include <tt/code/DefaultValue.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/str/str.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlDocument.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/movement/MoveAnimation.h>
#include <toki/game/movement/MoveBase.h>
#include <toki/game/movement/MovementSet.h>
#include <toki/game/movement/MovementSetCache.h>
#include <toki/game/movement/MoveVector.h>
#include <toki/game/movement/Transition.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions

MovementSetPtr MovementSet::create(const std::string& p_filename)
{
	return MovementSetCache::get(p_filename);
}


void MovementSet::add(const movement::MoveBasePtr& p_move)
{
	for (s32 i = 0; i < movement::Direction_Count; ++i)
	{
		if (p_move->hasLocalDir() == false)
		{
			movement::Direction dir = static_cast<movement::Direction>(i);
			if (p_move->getDirections().checkFlag(dir))
			{
				addSorted(dir, p_move);
			}
		}
		else
		{
			movement::Direction down = static_cast<movement::Direction>(i);
			
			if (down != movement::Direction_None) // Don't need a rotation with none because this is the same a a down rotation.
			{
				// Add rotated moves for all orientations directions.
				movement::Direction rotatedLocalDir = entity::getDirectionFromLocalDir(p_move->getLocalDir(), down);
				
				addSorted(rotatedLocalDir , p_move->createRotatedLocalDirForDown(down, false));
				
				// Add fliped result
				rotatedLocalDir = entity::getDirectionFromLocalDir(p_move->getLocalDir(), down, true);
				addSorted(rotatedLocalDir , p_move->createRotatedLocalDirForDown(down, true));
			}
		}
	}
}


void MovementSet::addAlways(const movement::MoveBasePtr& p_move)
{
	if (p_move->hasLocalDir() == false)
	{
		for (s32 i = 0; i < movement::Direction_Count; ++i)
		{
			movement::Direction dir = static_cast<movement::Direction>(i);
			addSorted(dir, p_move);
		}
	}
	else
	{
		for (s32 i = 0; i < movement::Direction_Count; ++i)
		{
			movement::Direction down = static_cast<movement::Direction>(i);
			
			if (down != movement::Direction_None) // Don't need a rotation with none because this is the same a a down rotation.
			{
				// Add rotated moves for all orientations directions.
				const movement::MoveBasePtr&  rotatedMove = p_move->createRotatedLocalDirForDown(down, false);
				
				// Add fliped result
				const movement::MoveBasePtr&  rotatedMoveFliped = p_move->createRotatedLocalDirForDown(down, true);
				
				for (s32 dir = 0; dir < movement::Direction_Count; ++dir)
				{
					movement::Direction direction = static_cast<movement::Direction>(dir);
					addSorted(direction, rotatedMoveFliped);
					addSorted(direction, rotatedMove);
				}
			}
		}
	}
}


const movement::MoveBasePtr& MovementSet::getMove(movement::Direction p_direction,
                                                  const movement::MoveBasePtr& /*p_previousMove*/, // Remove this parameter. Transitions are done differently.
                                                  const entity::Entity& p_entity) const
{
	static movement::MoveBasePtr empty;
	if (movement::isValidDirection(p_direction) == false)
	{
		TT_PANIC("Invalid direction: %d\n", p_direction);
		return empty;
	}
	
	const MoveList& list            = m_moves[p_direction];
	entity::LocalDir localDirection = p_entity.getLocalDirFromDirection(p_direction);
	const movement::SurroundingsSurvey& survey = p_entity.getSurvey();
	
	for (MoveList::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		// We expect the moves to be sorted on priority so the first valid move is returned.
		const movement::MoveBasePtr& newMove = (*it);
		if (newMove->validate(survey, localDirection,
		                      p_entity.getOrientationDown(), p_entity.isOrientationForwardLeft()))
		{
			return newMove;
		}
	}
	
	TT_ASSERT(survey.getCheckMask().checkFlag(SurveyResult_OnWaterLeft)   ||
	          survey.getCheckMask().checkFlag(SurveyResult_OnWaterStatic) ||
	          survey.getCheckMask().checkFlag(SurveyResult_OnWaterRight)  ||
	          survey.getCheckMask().checkFlag(SurveyResult_NotInWater)    ||
	          survey.getCheckMask().checkFlag(SurveyResult_SubmergedInWater));
	
	TT_ASSERT(survey.getCheckMask().checkFlag(SurveyResult_OnLavaLeft)   ||
	          survey.getCheckMask().checkFlag(SurveyResult_OnLavaStatic) ||
	          survey.getCheckMask().checkFlag(SurveyResult_OnLavaRight)  ||
	          survey.getCheckMask().checkFlag(SurveyResult_NotInLava)    ||
	          survey.getCheckMask().checkFlag(SurveyResult_SubmergedInLava));
	
	std::string surveyNames;
	{
		for (s32 i = 0; i < SurveyResult_Count; ++i)
		{
			SurveyResult flag = static_cast<SurveyResult>(i);
			if (survey.getCheckMask().checkFlag(flag))
			{
				surveyNames += getSurveyResultName(flag);
				surveyNames += "\n";
			}
		}
	}
	
	TT_PANIC("No valid move found.\ncheck mask: %s (b: %s)\n"
	         "For direction: '%s' from entity with type: '%s'.\n"
	         "(Orientation down: '%s' forwardIsLeft: %d.)\n"
	         "MovementSet file: '%s'.\n"
	         "SurveyResult names:[\n%s]",
	         tt::str::toStr(survey.getCheckMask().getFlags()).c_str(),
	         survey.getCheckMask().getBitsAsString().c_str(),
	         movement::getDirectionName(p_direction),
	         p_entity.getType().c_str(),
	         movement::getDirectionName(p_entity.getOrientationDown()),
	         p_entity.isOrientationForwardLeft(), m_filename.c_str(),
	         surveyNames.c_str());
	
	// Nothing found;
	return empty;
}


MovementSetPtr MovementSet::clone(removeFunction p_removeFunction) const
{
	MovementSetPtr obj;
	if (p_removeFunction == 0)
	{
		obj = MovementSetPtr(new MovementSet(*this));
	}
	else
	{
		obj = MovementSetPtr(new MovementSet(*this), p_removeFunction);
	}
	
	return obj;
}


MovementSet::MoveID MovementSet::getMoveID(const MoveBasePtr& p_move)
{
	if (p_move == 0)
	{
		return MoveID();
	}
	
	// Find the specified move
	
	for (s32 dirIdx = 0; dirIdx < Direction_Count; ++dirIdx)
	{
		const Direction dir = static_cast<Direction>(dirIdx);
		const MoveList& list = m_moves[dir];
		
		s32 i = 0;
		for (MoveList::const_iterator it = list.begin(); it != list.end(); ++it, ++i)
		{
			if ((*it) == p_move)
			{
				// Found the move.
				MoveID result;
				result.movementSetFilename = getFilename();
				result.direction = dir;
				result.index     = i;
				return result;
			}
		}
	}
	
	TT_PANIC("Didn't find move in movementset!");
	
	return MoveID(); // Not found.
}


MoveBasePtr MovementSet::getFromMoveID(const MoveID& p_id)
{
	if (p_id.isValid() == false)
	{
		return MoveBasePtr();
	}
	
	if (getFilename() != p_id.movementSetFilename)
	{
		TT_PANIC("Trying to load move from other movementset. Move is from '%s', this movementset is: '%s'",
		         p_id.movementSetFilename.c_str(), getFilename().c_str());
		return MoveBasePtr();
	}
	
	if (isValidDirection(p_id.direction) == false)
	{
		TT_PANIC("Invalid direction: %d", p_id.direction);
		return MoveBasePtr();
	}
	
	const MoveList& list = m_moves[p_id.direction];
	
	if (p_id.index < 0 && static_cast<MoveList::size_type>(p_id.index) >= list.size())
	{
		TT_PANIC("Invalid move index: %d", p_id.index);
		return MoveBasePtr();
	}
	
	return list[p_id.index];
}


ConstTransitionPtr MovementSet::findTransition(const movement::MoveBasePtr& p_fromMove,
                                               const movement::MoveBasePtr& p_toMove) const
{
	// Name to name check. (wild card is possible)
	// Start animation
	if (p_fromMove == 0 || p_toMove == 0)
	{
		TT_NULL_ASSERT(p_fromMove);
		TT_NULL_ASSERT(p_toMove);
		return ConstTransitionPtr();
	}
	
	NameHash fromHash(p_fromMove->getNameHash());
	NameHash toHash(  p_toMove->getNameHash());
	
	ConstTransitionPtr transition;
	
	// Turn transition
	if (fromHash == toHash)
	{
		if (p_fromMove->isForwardLeft() != p_toMove->isForwardLeft())
		{
			transition = findTransitionInMap(m_turns, fromHash);
			if (transition != 0)
			{
				return transition;
			}
		}
	}
	
	// Transition from specific move to another specific move
	MoveToMoveMap::const_iterator moveToMoveIt = m_moveToMove.find(fromHash);
	if (moveToMoveIt != m_moveToMove.end())
	{
		transition = findTransitionInMap(moveToMoveIt->second, toHash);
		if (transition != 0)
		{
			return transition;
		}
	}
	
	// Wildcard from, check if we can find to.
	transition = findTransitionInMap(m_alwaysFrom, toHash);
	if (transition != 0)
	{
		return transition;
	}
	
	// Wildcard to, check if we can find from.
	transition = findTransitionInMap(m_alwaysTo, fromHash);
	if (transition != 0)
	{
		return transition;
	}
	
	return transition;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

MovementSet::MovementSet(const std::string& p_filename)
:
m_filename(p_filename)
{
}


MovementSet::MovementSet(const MovementSet& p_rhs)
:
m_filename(p_rhs.m_filename)
{
	for (s32 i = 0; i < movement::Direction_Count; ++i)
	{
		m_moves[i] = p_rhs.m_moves[i];
	}
}


MovementSet* MovementSet::load(const std::string& p_filename)
{
	if (tt::fs::fileExists(p_filename))
	{
		using namespace tt::xml;

		TT_ERR_CREATE("MovementSet create from file: '" << p_filename << "'.");
		
		// load the xml
		XmlDocument doc(p_filename);
		XmlNode* rootNode(doc.getRootNode());
		if (rootNode == 0 || rootNode->getName() != "movementset")
		{
			TT_ERROR("Rootnode of movementset should be 'movementset'");
			return 0;
		}
		
		// parse moves
		XmlNode* movesNode = rootNode->getFirstChild("moves");
		if (movesNode == 0)
		{
			TT_ERROR("Movementset should contain moves");
			return 0;
		}

		MovementSet* moveSet = new MovementSet(p_filename);

		for (const XmlNode* node = movesNode->getChild(); node != 0; node = node->getSibling())
		{
			parseMove(node, moveSet, &errStatus);
			
			// Report error
			TT_ERR_ASSERT_ON_ERROR();
			errStatus.resetError(); // Reset status and try next node.
		}
		
		return moveSet;
	}
	
	TT_PANIC("Couldn't find file '%s'", p_filename.c_str());
	return 0;
}


void MovementSet::parseMove(const tt::xml::XmlNode* p_node, MovementSet* p_moveSet,
                            tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("parseMove");
	TT_ERR_NULL_ASSERT(p_node);
	TT_ERR_NULL_ASSERT(p_moveSet);
	
	using namespace tt::xml;
	
	if (p_node->getName() == "transition" || p_node->getName() == "turn_transition")
	{
		// Load transition move
		TT_ERR_ADD_LOC(" XML node 'transition'.");
		
		TransitionPtr transition = Transition::createFromXML(p_node, &errStatus);
		TT_ERR_RETURN_ON_ERROR();
		TT_ERR_NULL_ASSERT(transition);
		
		TT_ERR_ADD_LOC(" animation_name: '" << transition->getAnimationName() << "'");
		
		NameHashes toMoves   = parseMoveReferences(p_node, "to"  , &errStatus);
		NameHashes fromMoves = parseMoveReferences(p_node, "from", &errStatus);
		const bool isTurn = (p_node->getName() == "turn_transition");
		
		if (isTurn)
		{
			TT_ASSERTMSG(toMoves.empty(), "Found 'to' moves for turn transition, these are ignored!");
			TT_ASSERTMSG(fromMoves.empty() == false, "Found NO 'from' moves for turn transition, will never get triggered!");
			
			for (NameHashes::const_iterator it = fromMoves.begin(); it != fromMoves.end(); ++it)
			{
				const NameHash& move = (*it);
				TT_ERR_ASSERTMSG(p_moveSet->m_turns[move] == 0,
				                 "Already found a turn transition from move: '" << move.getName() << "'\n");
				p_moveSet->m_turns[move] = transition;
			}
		}
		else
		{
			if (fromMoves.empty())
			{
				TT_ASSERTMSG(toMoves.empty() == false, "Found NO 'to' and 'from' moves for transition!");
				for (NameHashes::const_iterator it = fromMoves.begin(); it != fromMoves.end(); ++it)
				{
					const NameHash& move = (*it);
					TT_ERR_ASSERTMSG(p_moveSet->m_alwaysFrom[move] == 0,
					                 "Already found a always from transition for move: '" << 
					                 move.getName() << "'\n");
					p_moveSet->m_alwaysFrom[move] = transition;
				}
			}
			else if (toMoves.empty())
			{
				for (NameHashes::const_iterator it = toMoves.begin(); it != toMoves.end(); ++it)
				{
					const NameHash& move = (*it);
					TT_ERR_ASSERTMSG(p_moveSet->m_alwaysTo[move] == 0,
					                 "Already found a always to transition for move: '" << 
					                 move.getName() << "'\n");
					p_moveSet->m_alwaysTo[move] = transition;
				}
			}
			else
			{
				for (NameHashes::const_iterator fromIt = fromMoves.begin(); fromIt != fromMoves.end(); ++fromIt)
				{
					const NameHash& fromMove = (*fromIt);
					MoveTransitionMap& moveMap = p_moveSet->m_moveToMove[fromMove];
					
					for (NameHashes::const_iterator toIt = toMoves.begin(); toIt != toMoves.end(); ++toIt)
					{
						const NameHash& toMove = (*toIt);
						TT_ERR_ASSERTMSG(moveMap[toMove] == 0,
						                 "Already found a transition from: '" << 
						                 fromMove.getName() << "' to: '" << toMove.getName() << "'\n");
						moveMap[toMove] = transition;
					}
				}
			}
		}
		
		return;
	}
	
	TT_ERR_ASSERT(p_node->getName() == "vector_move" || p_node->getName() == "anim_move");
	
	bool isVectorMove = p_node->getName() == "vector_move";
	
	// Name (optional)
	const std::string name(p_node->getAttribute("name"));
	
	TT_ERR_ADD_LOC(" XML node '" << p_node->getName() << "' with name: '" << name << "'.");
	
	// Always (required)
	bool always = util::parseBool(p_node, "always", &errStatus);
	
	// Priority (optional)
	tt::code::DefaultValue<s32> priority(0);
	priority = util::parseOptionalS32(p_node, "priority", &errStatus);
	
	// Directions (required)
	Directions directions;
	{
		const XmlNode* node = p_node->getFirstChild("directions");
		TT_ERR_ASSERTMSG(node != 0, "Missing node 'directions'");
		
		for (const XmlNode* child = node->getChild(); child != 0; child = child->getSibling())
		{
			if (child->getName() == "direction")
			{
				Direction dir = movement::getDirectionFromName(child->getData());
				if (movement::isValidDirection(dir))
				{
					directions.setFlag(dir);
				}
				else
				{
					TT_ERROR("Invalid direction '" << child->getData() << "'");
				}
			}
			else
			{
				TT_ERROR("Invalid child node '" << child->getName() << "' found in 'directions'.");
			}
		}
	}
	
	entity::LocalDir localDir = entity::LocalDir_Invalid;
	{
		const XmlNode* node = p_node->getFirstChild("local_dir");
		if (node != 0)
		{
			entity::LocalDir dir = entity::getLocalDirFromName(node->getData());
			if (entity::isValidLocalDir(dir))
			{
				localDir = dir;
			}
			else
			{
				TT_ERROR("Invalid direction '" << node->getData() << "'");
			}
		}
	}
	
	
	// Speed (required for vector moves)
	tt::math::Vector2 speed;
	if (isVectorMove)
	{
		const XmlNode* node = p_node->getFirstChild("speed");
		tt::math::parseVector2(node, &speed);
	}
	
	// Substeps (required for animation moves)
	MoveAnimation::SubSteps substeps;
	if (isVectorMove == false)
	{
		const XmlNode* node = p_node->getFirstChild("substeps");
		TT_ERR_ASSERTMSG(node != 0, "Missing node 'substeps'");
		
		for (const XmlNode* child = node->getChild(); child != 0; child = child->getSibling())
		{
			substeps.push_back(util::parsePoint2(child, &errStatus));
		}
	}
	
	// Duration (required for animation moves)
	real duration = 0.0f;
	if (isVectorMove == false)
	{
		const XmlNode* node = p_node->getFirstChild("duration");
		TT_ERR_ASSERTMSG(node != 0, "Missing node 'duration'");
		duration = tt::str::parseReal(node->getData(), &errStatus);
	}
	
	Validator validator;
	
	// SurveyResult Must Have (optional)
	{
		const XmlNode* node = p_node->getChild();
		
		while (node != 0)
		{
			if (node->getName() == "survey_musthave")
			{
				validator.addMustHave(parseSurveyResults(node, &errStatus));
				TT_ERR_RETURN_ON_ERROR();
			}
			node = node->getSibling();
		}
	}
	
	// SurveyResult Must Not Have (optional)
	{
		const XmlNode* node = p_node->getChild();
		
		while (node != 0)
		{
			if (node->getName() == "survey_mustnothave")
			{
				validator.addMustNotHave(parseSurveyResults(node, &errStatus));
				TT_ERR_RETURN_ON_ERROR();
			}
			node = node->getSibling();
		}
	}
	
	// Flags (optional)
	MoveBase::Flags flags;
	{
		const XmlNode* node = p_node->getFirstChild("flags");
		
		if (node != 0)
		{
			for (const XmlNode* child = node->getChild(); child != 0; child = child->getSibling())
			{
				TT_ERR_ASSERT(child->getName() == "flag");
				if      (child->getData() == "x_snap")              flags.setFlag(MoveBase::Flag_StartNeedsXPositionSnap);
				else if (child->getData() == "y_snap")              flags.setFlag(MoveBase::Flag_StartNeedsYPositionSnap);
				else if (child->getData() == "x_snap_end")          flags.setFlag(MoveBase::Flag_EndNeedsXPositionSnap);
				else if (child->getData() == "y_snap_end")          flags.setFlag(MoveBase::Flag_EndNeedsYPositionSnap);
				else if (child->getData() == "persistent_move")     flags.setFlag(MoveBase::Flag_PersistentMove);
				else if (child->getData() == "ignore_collision")    flags.setFlag(MoveBase::Flag_IgnoreCollision);
				else
				{
					TT_ERROR("Invalid flag '" << child->getData() << "'");
				}
			}
		}
	}
	
	// Presentation animation tags (optional; plays an animation with the specified tags if specified)
	tt::pres::Tags animationTags;
	{
		const XmlNode* node = p_node->getFirstChild("animation_tags");
		
		if (node != 0)
		{
			for (const XmlNode* child = node->getChild(); child != 0; child = child->getSibling())
			{
				TT_ERR_ASSERT(child->getName() == "tag");
				TT_ERR_ASSERT(child->getData().empty() == false);
				animationTags.insert(tt::pres::Tag(child->getData()));
			}
		}
	}
	
	// Presentation animation name
	std::string animationName;
	{
		const XmlNode* node = p_node->getFirstChild("animation_name");
		
		if (node != 0)
		{
			animationName = node->getData();
		}
	}
	
	// Callback names (optional; no callback triggered if no callback name specified)
	std::string startCallback;
	std::string endCallback;
	{
		const XmlNode* node = p_node->getFirstChild("callbacks");
		
		if (node != 0)
		{
			const XmlNode* startNode = node->getFirstChild("start");
			if (startNode != 0)
			{
				startCallback = startNode->getData();
			}
			
			const XmlNode* endNode = node->getFirstChild("end");
			if (endNode != 0)
			{
				endCallback = endNode->getData();
			}
		}
	}
	
	TT_ERR_RETURN_ON_ERROR();
	
	// All parsed, add move
	movement::MoveBasePtr movePtr;
	
	if (isVectorMove)
	{
		movePtr.reset(new MoveVector(
			directions,
			localDir,
			speed,
			validator,
			flags,
			priority.get(),
			animationTags,
			animationName,
			name,
			startCallback,
			endCallback,
			always));
	}
	else
	{
		movePtr.reset(new MoveAnimation(
			directions,
			localDir,
			substeps,
			duration,
			validator,
			flags,
			priority.get(),
			animationTags,
			animationName,
			name,
			startCallback,
			endCallback,
			always));
	}
	
	if (always)
	{
		p_moveSet->addAlways(movePtr);
	}
	else
	{
		p_moveSet->add(movePtr);
	}
}


NameHashes MovementSet::parseMoveReferences(const tt::xml::XmlNode* p_node,
                                            const std::string& p_nodeName,
                                            tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(NameHashes, NameHashes(), "parsing move reference");
	
	const tt::xml::XmlNode* node = p_node->getFirstChild(p_nodeName);
	if (node == 0)
	{
		return NameHashes();
	}
	
	NameHashes moves;
	for (const tt::xml::XmlNode* child = node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "move")
		{
			moves.push_back(NameHash(child->getData()));
		}
		else
		{
			TT_ERROR("Invalid child node '" << child->getName() << "' found in 'to'.");
		}
	}
	return moves;
}


SurveyResults MovementSet::parseSurveyResults(const tt::xml::XmlNode* p_node,
                                              tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(SurveyResults, SurveyResults(), "parseSurveyResults");
	TT_ERR_NULL_ASSERT(p_node);
	
	SurveyResults results;
	
	using namespace tt::xml;
	for (const XmlNode* node = p_node->getChild(); node != 0; node = node->getSibling())
	{
		TT_ERR_ASSERT(node->getName() == "survey");
		SurveyResult result = getSurveyResultFromName(node->getData());
		if (isValidSurveyResult(result))
		{
			results.setFlag(result);
		}
		else
		{
			TT_ERROR("Invalid survey '" << node->getData() << "'");
		}
	}
	
	return results;
}


inline void MovementSet::addSorted(movement::Direction p_direction,
                                   const movement::MoveBasePtr& p_move)
{
	TT_ASSERT(isValidDirection(p_direction));
	MoveList& list = m_moves[p_direction];
	for (MoveList::iterator it = list.begin(); it != list.end(); ++it)
	{
		// We expect the moves to be sorted on priority so the first valid move is returned.
		if ((*it)->getPriority() < p_move->getPriority())
		{
			list.insert(it, p_move);
			return;
		}
	}
	list.push_back(p_move);
}


ConstTransitionPtr MovementSet::findTransitionInMap(const MovementSet::MoveTransitionMap& p_moveMap, 
                                                    NameHash p_hash)
{
	MoveTransitionMap::const_iterator findResult = p_moveMap.find(p_hash);
	if (findResult != p_moveMap.end())
	{
		return findResult->second;
	}
	return ConstTransitionPtr();
}


// Namespace end
}
}
}
