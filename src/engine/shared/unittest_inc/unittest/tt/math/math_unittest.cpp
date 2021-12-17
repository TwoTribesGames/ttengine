#include <unittestpp/unittestpp.h>
#include <unittestpp/src/checks.h>

#include <tt/math/Matrix44.h>



//--------------------------------------------------------------------------------------------------
// Helper functions
namespace UnitTest {


template< >
bool AreClose(tt::math::Matrix44 const& expected, tt::math::Matrix44 const& actual, real const& tolerance)
{
	return tt::math::realEqual(expected.m_11, actual.m_11, tolerance) &&
		   tt::math::realEqual(expected.m_12, actual.m_12, tolerance) &&
		   tt::math::realEqual(expected.m_13, actual.m_13, tolerance) &&
		   tt::math::realEqual(expected.m_14, actual.m_14, tolerance) &&
		   
		   tt::math::realEqual(expected.m_21, actual.m_21, tolerance) &&
		   tt::math::realEqual(expected.m_22, actual.m_22, tolerance) &&
		   tt::math::realEqual(expected.m_23, actual.m_23, tolerance) &&
		   tt::math::realEqual(expected.m_24, actual.m_24, tolerance) &&
		   
		   tt::math::realEqual(expected.m_31, actual.m_31, tolerance) &&
		   tt::math::realEqual(expected.m_32, actual.m_32, tolerance) &&
		   tt::math::realEqual(expected.m_33, actual.m_33, tolerance) &&
		   tt::math::realEqual(expected.m_34, actual.m_34, tolerance) &&

		   tt::math::realEqual(expected.m_41, actual.m_41, tolerance) &&
		   tt::math::realEqual(expected.m_42, actual.m_42, tolerance) &&
		   tt::math::realEqual(expected.m_43, actual.m_43, tolerance) &&
		   tt::math::realEqual(expected.m_44, actual.m_44, tolerance);
}

}

//--------------------------------------------------------------------------------------------------
// The tests


SUITE(tt_math)
{
	TEST(Matrix44)
	{
		const real tolerance = 0.0001f;

		tt::math::Matrix44 matA = tt::math::Matrix44::identity;
		tt::math::Matrix44 invA = tt::math::Matrix44::identity;

		CHECK_CLOSE(invA, matA.getInverse(), tolerance);
		CHECK_CLOSE(invA, matA.getInverse(), tolerance);

		matA = tt::math::Matrix44::getTranslation( 1,  2,  5);
		invA = tt::math::Matrix44::getTranslation(-1, -2, -5);

		CHECK_CLOSE(matA, invA.getInverse(), tolerance);
		CHECK_CLOSE(invA, matA.getInverse(), tolerance);

		matA = tt::math::Matrix44::getScale(5, 10, 2);
		invA = tt::math::Matrix44::getScale(0.2f, 0.1f, 0.5f);

		CHECK_CLOSE(matA, invA.getInverse(), tolerance);
		CHECK_CLOSE(invA, matA.getInverse(), tolerance);

		matA = tt::math::Matrix44::getRotationX( 0.5f);
		invA = tt::math::Matrix44::getRotationX(-0.5f);

		CHECK_CLOSE(matA, invA.getInverse(), tolerance);
		CHECK_CLOSE(invA, matA.getInverse(), tolerance);

		matA = tt::math::Matrix44::getRotationY( 12.5f);
		invA = tt::math::Matrix44::getRotationY(-12.5f);

		CHECK_CLOSE(matA, invA.getInverse(), tolerance);
		CHECK_CLOSE(invA, matA.getInverse(), tolerance);

		matA = tt::math::Matrix44::getRotationZ( 3.14f);
		invA = tt::math::Matrix44::getRotationZ(-3.14f);

		CHECK_CLOSE(matA, invA.getInverse(), tolerance);
		CHECK_CLOSE(invA, matA.getInverse(), tolerance);

		matA = tt::math::Matrix44::getSRT(
			tt::math::Vector3(1,2,3), tt::math::Vector3(3,4,6), tt::math::Vector3(0,-8,12));
		invA = matA.getInverse();

		CHECK_CLOSE(matA, invA.getInverse(), tolerance);
		CHECK_CLOSE(invA * matA, tt::math::Matrix44::identity, tolerance);
	}

// End SUITE
}
