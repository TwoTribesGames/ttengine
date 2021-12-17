#include <tt/math/BezierCurve.h>
#include <tt/platform/tt_printf.h>

namespace tt {
namespace math {


void BezierCurve::print()
{
	TT_Printf("BezierCurve: [Not printable yet]\n");
	/*m_startPoint.Print();
	m_outVector.Print();
	m_inVector.Print();
	m_endPoint.Print();*/
}


bool BezierCurve::load(tt::streams::BIStream& p_stream)
{
	p_stream >> m_startPoint;
	p_stream >> m_outVector;
	p_stream >> m_inVector;
	p_stream >> m_endPoint;
	return true;
}

bool BezierCurve::save(tt::streams::BOStream& p_stream) const
{
	p_stream << m_startPoint;
	p_stream << m_outVector;
	p_stream << m_inVector;
	p_stream << m_endPoint;
	return true;
}


/*! \brief Calculates the vector
	
	Given a time value between 0 and 1 it will calculate the bezier curve point.

	\param p_time - The time index into the curve
    \return The Vector3 of the position based on the time.
*/
Vector3 BezierCurve::model(real p_time) const
{
	Vector3 curve_point;

	// Precalc useable values
	real timeLeft = 1.0f - p_time;
	real timeLeftPow3 = timeLeft * timeLeft * timeLeft;
	real timePow3 = p_time * p_time * p_time;
	real time3timeLeft = p_time * 3.0f * timeLeft;

	// Calculate the point
	curve_point = (m_startPoint * timeLeftPow3) + 
	              (m_outVector  * (time3timeLeft * timeLeft)) + 
	              (m_inVector   * (time3timeLeft * p_time)) + 
	              (m_endPoint   * timePow3);

	// Return the curve point
	return curve_point;
}

// Namespace end
}
}
