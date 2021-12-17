#include <tt/math/HermiteCurve.h>
#include <tt/platform/tt_printf.h>
#include <tt/fs/File.h>

namespace tt {
namespace math {


void HermiteCurve::print()
{
	TT_Printf("HermiteCurve: [Not printable yet]\n");
	/*m_startPoint.print();
	m_endPoint.print();
	m_startTangent.print();
	m_endTangent.print();*/
}

bool HermiteCurve::load(const fs::FilePtr& p_file)
{
	// Attempt to load the start / end points
	if (m_startPoint.load(p_file) == false)
	{
		return false;
	}

	if (m_endPoint.load(p_file) == false)
	{
		return false;
	}

	// Attempt to load the start / end tangents
	if (m_startTangent.load(p_file) == false)
	{
		return false;
	}

	if (m_endTangent.load(p_file) == false)
	{
		return false;
	}

	return true;
}

/*! \brief Calculates the vector
	
	Given a time value [0.0f - 1.0f] it calculates the hermite curve point.

	\param p_time - The time index into the curve
    \return The Vector3 of the position based on the time.
*/
Vector3 HermiteCurve::model(real p_time)
{
	Vector3 curve_point;

	// Precalc useable values
	real timePow2		= p_time * p_time;
	real timePow3		= timePow2 * p_time;
	real twoTimePow3	= timePow3 + timePow3;
	real threeTimePow2	= timePow2 + timePow2 + timePow2;

	// Calculate the 4 hermite values
	real h4 = timePow3 - timePow2;
	real h3 = (h4 - timePow2) + p_time;
	real h2 = threeTimePow2 - twoTimePow3;
	real h1 = twoTimePow3 - threeTimePow2 + 1.0f;

	// Calculate the point
	curve_point = (m_startPoint * h1) + (m_endPoint * h2) + 
				  (m_startTangent * h3) + (m_endTangent * h4);

	// Return the curve point
	return curve_point;
}


// Namespace end
}
}
