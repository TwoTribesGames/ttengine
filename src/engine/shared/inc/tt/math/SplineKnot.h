#if !defined(INC_TT_MATH_SPLINEKNOT_H)
#define INC_TT_MATH_SPLINEKNOT_H


#include <tt/platform/tt_types.h>
#include <tt/streams/BOStream.h>
#include <tt/streams/BIStream.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace math {


class SplineKnot
{
public:
	SplineKnot();
	
	// load/save
	inline bool load(tt::streams::BIStream& p_stream)
	{
		p_stream >> m_inVector;
		p_stream >> m_position;
		p_stream >> m_outVector;
		return true;
	}
	
	inline bool save(tt::streams::BOStream& p_stream) const
	{
		p_stream << m_inVector;
		p_stream << m_position;
		p_stream << m_outVector;
		return true;
	}
	
	inline const Vector3& getPosition()  const { return m_position;  }
	inline const Vector3& getInVector()  const { return m_inVector;  }
	inline const Vector3& getOutVector() const { return m_outVector; }
	
private:
	Vector3 m_inVector;
	Vector3 m_position;
	Vector3 m_outVector;
};


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& s, SplineKnot& sk)
{
	sk.load(s);
	return s;
}

inline tt::streams::BOStream& operator<<(tt::streams::BOStream& s, const SplineKnot& sk)
{
	sk.save(s);
	return s;
}

// Namespace end
}
}

#endif // TT_MATH_SPLINEKNOT_H
