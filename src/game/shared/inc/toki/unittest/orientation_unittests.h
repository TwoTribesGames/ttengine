#if !defined(TT_INC_TOKI_UNITTEST_ORIENTATION_UNITTESTS_H)
#define TT_INC_TOKI_UNITTEST_ORIENTATION_UNITTESTS_H

#include <unittestpp/unittestpp.h>

#include <toki/game/entity/Entity.h>


SUITE(Orientation)
{


struct TestEntityFixture
{
	toki::game::movement::Direction orientationDown;
	bool                            isForwardLeft;
	tt::math::Vector2               testVector;
	
	TestEntityFixture()
	:
	orientationDown(toki::game::movement::Direction_Down),
	isForwardLeft(false),
	testVector(2.0f, 1.0f)
	{
	}
};


TEST_FIXTURE( TestEntityFixture, OrientationDownFalseUnitTests)
{
	using namespace toki::game;
	
	// Default (Down, false)
	CHECK_EQUAL(movement::Direction_Down, orientationDown);
	CHECK_EQUAL(false,                    isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(testVector.x, result.x);
	CHECK_EQUAL(testVector.y, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationDownTrueUnitTests)
{
	using namespace toki::game;
	
	// (Down, true)
	isForwardLeft = true;
	
	CHECK_EQUAL(movement::Direction_Down, orientationDown);
	CHECK_EQUAL(true,                     isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(-testVector.x, result.x);
	CHECK_EQUAL( testVector.y, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationRightFalseUnitTests)
{
	using namespace toki::game;
	
	// (Right, false)
	orientationDown = movement::Direction_Right;
	isForwardLeft = false;
	
	CHECK_EQUAL(movement::Direction_Right, orientationDown);
	CHECK_EQUAL(false,                      isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(-testVector.y, result.x);
	CHECK_EQUAL(testVector.x,  result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationRightTrueUnitTests)
{
	using namespace toki::game;
	
	// (Right, true)
	orientationDown = movement::Direction_Right;
	isForwardLeft = true;
	
	CHECK_EQUAL(movement::Direction_Right, orientationDown);
	CHECK_EQUAL(true,                      isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(-testVector.y, result.x);
	CHECK_EQUAL(-testVector.x, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationUpFalseUnitTests)
{
	using namespace toki::game;
	
	// (Up, false)
	orientationDown = movement::Direction_Up;
	isForwardLeft = false;
	
	CHECK_EQUAL(movement::Direction_Up, orientationDown);
	CHECK_EQUAL(false,                  isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(-testVector.x, result.x);
	CHECK_EQUAL(-testVector.y, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationUpTrueUnitTests)
{
	using namespace toki::game;
	
	// (Up, true)
	orientationDown = movement::Direction_Up;
	isForwardLeft = true;
	
	CHECK_EQUAL(movement::Direction_Up, orientationDown);
	CHECK_EQUAL(true,                   isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL( testVector.x, result.x);
	CHECK_EQUAL(-testVector.y, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationLeftFalseUnitTests)
{
	using namespace toki::game;
	
	// (Left, false)
	orientationDown = movement::Direction_Left;
	isForwardLeft = false;
	
	CHECK_EQUAL(movement::Direction_Left, orientationDown);
	CHECK_EQUAL(false,                    isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL( testVector.y, result.x);
	CHECK_EQUAL(-testVector.x, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationLeftTrueUnitTests)
{
	using namespace toki::game;
	
	// (Left, true)
	orientationDown = movement::Direction_Left;
	isForwardLeft = true;
	
	CHECK_EQUAL(movement::Direction_Left, orientationDown);
	CHECK_EQUAL(true,                     isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(testVector.y, result.x);
	CHECK_EQUAL(testVector.x, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationNoneFalseUnitTests)
{
	using namespace toki::game;
	
	// (None, false) (Should be same as down.)
	orientationDown = movement::Direction_None;
	isForwardLeft = false;
	
	CHECK_EQUAL(movement::Direction_None, orientationDown);
	CHECK_EQUAL(false,                    isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(testVector.x, result.x);
	CHECK_EQUAL(testVector.y, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
}


TEST_FIXTURE( TestEntityFixture, OrientationNoneTrueUnitTests)
{
	using namespace toki::game;
	
	// (None, true)
	orientationDown = movement::Direction_None;
	isForwardLeft = true;
	
	CHECK_EQUAL(movement::Direction_None, orientationDown);
	CHECK_EQUAL(true,                     isForwardLeft);
	
	const tt::math::Vector2 result = entity::applyOrientationToVector2(testVector, orientationDown, isForwardLeft);
	
	CHECK_EQUAL(-testVector.x, result.x);
	CHECK_EQUAL( testVector.y, result.y);
	
	CHECK_EQUAL(testVector, entity::removeOrientationFromVector2(result, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Forward, entity::getLocalDirFromDirection(movement::Direction_Left , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Up,      entity::getLocalDirFromDirection(movement::Direction_Up   , orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Back,    entity::getLocalDirFromDirection(movement::Direction_Right, orientationDown, isForwardLeft));
	CHECK_EQUAL(entity::LocalDir_Down,    entity::getLocalDirFromDirection(movement::Direction_Down , orientationDown, isForwardLeft));
}


// Suite end
};


#endif // !defined(TT_INC_TOKI_UNITTEST_ORIENTATION_UNITTESTS_H)
