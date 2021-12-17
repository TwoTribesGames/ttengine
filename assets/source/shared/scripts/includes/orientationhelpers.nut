/*
orientation and direction helpers
*/

// get a normal vector in the given direction (in world space)
function getVectorFromDirection(p_direction)
{
	switch (p_direction)
	{
	case Direction_Up:    return ::Vector2( 0,  1);
	case Direction_Down:  return ::Vector2( 0, -1);
	case Direction_Right: return ::Vector2( 1,  0);
	case Direction_Left:  return ::Vector2(-1,  0);
	case Direction_None:  return ::Vector2( 0,  0);
	default:
		::tt_panic("Cannot convert direction " + getDirectionName(p_direction) + " to a Vector2");
		break;
	}
	return ::Vector2(0, 0);
}

function getDirectionFromAngle(p_direction)
{
	// Translate to [0, 360)
	p_angle = ::wrapAngle(p_angle);
	if      (p_angle >= 45.0  && p_angle < 135.0) return Direction_Right;
	else if (p_angle >= 135.0 && p_angle < 225.0) return Direction_Down;
	else if (p_angle >= 225.0 && p_angle < 315.0) return Direction_Left;
	return Direction_Up;
}


function getAngleFromDirection(p_direction)
{
	switch (p_direction)
	{
	case Direction_Up:    return 0;
	case Direction_Down:  return 180;
	case Direction_Right: return 90;
	case Direction_Left:  return 270;
	default:
		::tt_panic("Cannot convert direction " + getDirectionName(p_direction) + " to an angle");
		break;
	}
	return 0;

}

function isDirectionHorizontal(p_direction)
{
	return (p_direction == Direction_Left || p_direction == Direction_Right);
}

function isDirectionVertical(p_direction)
{
	return (p_direction == Direction_Up || p_direction == Direction_Down);
}

// returns true if the entities are facing each other
function areEntitiesFacing(p_entityA, p_entityB)
{
	local dirA = p_entityA.getDirectionFromLocalDir(LocalDir_Forward);
	local dirB = p_entityB.getDirectionFromLocalDir(LocalDir_Forward);

	return (dirA == getInverseDirection(dirB));
}

// makes p_entity face p_target // FIXME this does not take floor direction into account
function faceEntity(p_entity, p_target)
{
	if (isEntityFacingPosition(p_entity, p_target.getPosition()) == false)
	{
		p_entity.setForwardAsLeft(p_entity.isForwardLeft() == false);
	}
}

// makes p_entity face away from p_target // FIXME this does not take floor direction into account
function faceAwayFromEntity(p_entity, p_target)
{
	if (isEntityFacingPosition(p_entity, p_target.getPosition()))
	{
		p_entity.setForwardAsLeft(p_entity.isForwardLeft() == false);
	}
}

// Returns true if the entity is facing the direction of p_position
// (only works with orthagonal directions, see computationally intensive
// version for a solution that works with "free" directions)
function isEntityFacingPosition(p_entity, p_position)
{
	local viewnormal = ::getVectorFromDirection(p_entity.getDirectionFromLocalDir(LocalDir_Forward));
	local delta      = (p_position - p_entity.getPosition()) * viewnormal;
	
	return (delta.x >= 0 && delta.y >= 0);
/*
	// computationally "intensive" version.
	local viewnormal = ::getVectorFromDirection(p_entity.getDirectionFromLocalDir());
	local delta      = (p_position - p_entity.getPosition()).normalize();
	
	return dotProduct(viewnormal, delta) >= 0;
*/
}

// makes an entity face a world position // FIXME this does not take floor direction into account
function facePosition(p_entity, p_position)
{
	if (isEntityFacingPosition(p_entity, p_position) == false)
	{
		p_entity.setForwardAsLeft(p_entity.isForwardLeft() == false);
	}
}

// makes an entity face a world position // FIXME this does not take floor direction into account
function faceAwayFromPosition(p_entity, p_position)
{
	if (isEntityFacingPosition(p_entity, p_position))
	{
		p_entity.setForwardAsLeft(p_entity.isForwardLeft() == false);
	}
}


function setForwardFromString(p_entity, p_forwardString)
{
	p_entity.setForwardAsLeft(p_forwardString.tolower() == "left");
}

function getTouchingSolid(p_entity, p_fallbackDirection = null) 
{
	local isUpdatingSurvey = p_entity.shouldUpdateSurvey();
	if (isUpdatingSurvey == false)
	{
		p_entity.setUpdateSurvey(true);
	}
	
	local result = Direction_None;
	
	// NOTE that p_fallbackDirection can't be Direction_None because Direction_None does not
	//       yet exist at the moment this is evaluated.
	
	// start with down, because we're a floor-oriented bunch
	if (p_entity.hasSurveyResult(SurveyResult_Floor))
	{
		result = Direction_Down
	}
	else if (p_entity.hasSurveyResult(SurveyResult_Ceiling))
	{
		result = Direction_Up;
	}
	else if (p_entity.hasSurveyResult(SurveyResult_WallRight))
	{
		result = Direction_Right;
	}
	else if (p_entity.hasSurveyResult(SurveyResult_WallLeft))
	{
		result = Direction_Left
	}
	
	// Restore update survey
	if (isUpdatingSurvey == false)
	{
		p_entity.setUpdateSurvey(false);
	}
	
	if (result != Direction_None)
	{
		return result;
	}
	
	return (p_fallbackDirection != null) ? p_fallbackDirection : Direction_None;
}

function isStandingOnLevelSolid(p_entity)
{
	local pos       = getFloorTilePosition(p_entity);
	local collision = getCollisionTypeLevelOnly(pos);
	
	return isSolid(collision);
}
 
function getFloorTilePosition(p_entity)
{
	local pos = p_entity.getPosition();
	pos += p_entity.applyOrientationToVector2(::Vector2(0, -0.5));
	
	//DebugView.drawCircle(pos, 0.3, 5);
	return pos;
}
 
// Gets relative offset for an entity. 
// p_source is the entity from which to calculate the offset
// p_target can be an entity or position
// if p_inLocalSpace is true the orientation of p_source is applied
function getRelativeOffset(p_source, p_target, p_inLocalSpace = true)
{
	local entityPos = p_source.getCenterPosition();
	if ("getCenterPosition" in p_target) p_target = p_target.getCenterPosition();
	
	local diff      = p_target - entityPos;
	
	if (p_inLocalSpace)
	{
		diff = p_source.applyOrientationToVector2(diff);
	}
	
	return diff;
}

function getCWDirection(p_direction)
{
	switch (p_direction)
	{
		case Direction_Up:    return Direction_Right;
		case Direction_Right: return Direction_Down;
		case Direction_Down:  return Direction_Left;
		case Direction_Left:  return Direction_Up;
		default:              return Direction_None;
	}
}

function getCCWDirection(p_direction)
{
	switch (p_direction)
	{
		case Direction_Up:    return Direction_Left;
		case Direction_Left:  return Direction_Down;
		case Direction_Down:  return Direction_Right;
		case Direction_Right: return Direction_Up;
		default:              return Direction_None;
	}
}

// Generator to iterate over all directions in Clockwise direction.
// for example:
//
// foreach(dir in allDirections())
// {
// 	echo(getDirectionName(dir), this);
// }
function allDirections()
{
	yield Direction_Up;
	yield Direction_Right;
	yield Direction_Down;
	yield Direction_Left;
}

// Generator to iterate over all directions in Clockwise direction
function allOrientations()
{
	yield LocalDir_Up;
	yield Direction_Forward;
	yield Direction_Down;
	yield Direction_Back;
}