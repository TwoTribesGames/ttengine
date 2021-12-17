#include <tt/platform/tt_error.h>
#include <tt/math/QuaternionUtils.h>
#include <tt/math/Quaternion.h>
#include <tt/math/math.h>


namespace tt {
namespace math {

//------------------------------------------------------------------------------
// Public member functions

void QuaternionUtils::snapToAxis(Quaternion& p_q)
{
	// FIXME: This function desperately needs descriptive comments.
	if (realLessThan(p_q.m_w, real(0)))
	{
		p_q = -p_q;
	}
	
	TT_ASSERTMSG(realEqual(p_q.getMagnitude(), real(1)),
	             "Quaternion must be normalized (magnitude is: %f).",
	             realToFloat(p_q.getMagnitude()));
	
	Quaternion in(p_q);
	
	// Snap all components
	snapComponent(p_q.m_w);
	snapComponent(p_q.m_x);
	snapComponent(p_q.m_y);
	snapComponent(p_q.m_z);
	
	// DEBUG: Make sure the incoming and outgoing quaternions are similar.
	static const real tolerance = 0.2f;
	if (fabs(in.m_w - p_q.m_w) > tolerance ||
	    fabs(in.m_x - p_q.m_x) > tolerance ||
	    fabs(in.m_y - p_q.m_y) > tolerance ||
	    fabs(in.m_z - p_q.m_z) > tolerance)
	{
		TT_PANIC("Incoming and outgoing quaternions differ too much.\n"
		         "IN : (w: %f, x: %f, y: %f, z: %f)\n"
		         "OUT: (w: %f, x: %f, y: %f, z: %f)",
		         realToFloat(in.m_w),  realToFloat(in.m_x), 
				 realToFloat(in.m_y),   realToFloat(in.m_z),
		         realToFloat(p_q.m_w), realToFloat(p_q.m_x), 
				 realToFloat(p_q.m_y), realToFloat(p_q.m_z));
	}
}


//------------------------------------------------------------------------------
// Private member functions

void QuaternionUtils::snapComponent(real& p_value)
{
	static const real cosQuarterPi = cos(pi * 0.25f);
	
	bool invert = false;
	if (p_value < 0.0f)
	{
		invert = true;
		p_value = -p_value;
	}
	
	// Check if component is closer to 0, 0.5, cos(.25pi) or 1
	// (and then snap to one or the other)
	if (realLessEqual(p_value, real(0.25f)))
	{
		// Component is closer to 0
		p_value = 0;
	}
	else if (realLessEqual(p_value, cosQuarterPi - ((cosQuarterPi - real(0.5f)) * real(0.5f))))
	{
		// Component is closer to 0.5
		p_value = 0.5f;
	}
	else if (realGreaterEqual(p_value, cosQuarterPi + (real(1) - cosQuarterPi)))
	{
		// Component is closer to 1
		p_value = 1;
	}
	else
	{
		// Component is closer to cos(.25pi)
		p_value = cosQuarterPi;
	}
	
	if (invert)
	{
		p_value = -p_value;
	}
}

// Namespace end
}
}
