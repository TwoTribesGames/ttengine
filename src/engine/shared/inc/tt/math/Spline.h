#if !defined(INC_TT_MATH_SPLINE_H)
#define INC_TT_MATH_SPLINE_H


#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/streams/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/math/SplineKnot.h>



namespace tt {
namespace math {


class Spline
{
public:
	// Available spline modes
	enum Mode
	{
		Mode_Time,
		Mode_Length
	};

	// Constructor / Destructor
	Spline();
	~Spline();
	
	// load/save
	bool load(tt::streams::BIStream& p_stream);
	bool save(tt::streams::BOStream& p_stream) const;
	
	// Knots count control
	real setKnotsCount(s32 p_knots);
	inline s32 getKnotsCount() const {return m_knotsCount;}

	/*! \brief Returns a pointer to the splineknot at specified position
		\return Pointer to SplineKnot at position p_knot */
	inline const SplineKnot* getSplineKnot(u32 p_knot) const {return &m_knots[p_knot];}

	/*! \brief Compute position in spline based on spline mode */
	Vector3	model(real p_u, Mode p_mode = Mode_Time) const;

	/*! \brief Returns the length of spline
		\return The length of the spline in 12bit (errors if overflow) */
	real getLength() const;

	/*! \brief Returns the length of spline
		\return The length of the spline in 64bit precision*/
	inline real64 getLength64() const {return m_length;}

private:
    real recalculateLength(real p_step);

private:
	typedef	std::vector<SplineKnot> KnotsCollection;

	real64			  m_length;
	s32				  m_knotsCount;
	KnotsCollection	  m_knots;
	std::vector<real> m_knotDistances;
};

inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const Spline& sp)
{
	sp.save(s);
	return s;
}

inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, Spline& sp)
{
	sp.load(s);
	return s;
}

// Namespace end
}
}

#endif // TT_MATH_SPLINE_H
