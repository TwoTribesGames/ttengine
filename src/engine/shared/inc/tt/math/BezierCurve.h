#if !defined(INC_TT_MATH_BEZIERCURVE_H)
#define INC_TT_MATH_BEZIERCURVE_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/streams/BIStream.h>
#include <tt/streams/BOStream.h>


namespace tt {
namespace math {


class BezierCurve
{
public:
	inline BezierCurve()
	:
	m_startPoint(Vector3::zero),
	m_outVector(Vector3::zero),
	m_inVector(Vector3::zero),
	m_endPoint(Vector3::zero)
	{ }
	
	bool load(tt::streams::BIStream& p_stream);
	bool save(tt::streams::BOStream& p_stream) const;
	void print(); // Not supported yet
	
	inline void setStartPoint(const Vector3& p_start) {m_startPoint   = p_start;}
	inline void setStartPointX(real p_x)              {m_startPoint.x = p_x;}
	inline void setStartPointY(real p_y)              {m_startPoint.y = p_y;}
	inline void setStartPointZ(real p_z)              {m_startPoint.z = p_z;}
	
	inline void setEndPoint(const Vector3& p_end) {m_endPoint   = p_end;}
	inline void setEndPointX(real p_x)            {m_endPoint.x = p_x;}
	inline void setEndPointY(real p_y)            {m_endPoint.y = p_y;}
	inline void setEndPointZ(real p_z)            {m_endPoint.z = p_z;}
	
	inline void setInVector(const Vector3& p_in) {m_inVector   = p_in;}
	inline void setInVectorX(real p_x)           {m_inVector.x = p_x;}
	inline void setInVectorY(real p_y)           {m_inVector.y = p_y;}
	inline void setInVectorZ(real p_z)           {m_inVector.z = p_z;}
	
	inline void setOutVector(const Vector3& p_out) {m_outVector   = p_out;}
	inline void setOutVectorX(real p_x)            {m_outVector.x = p_x;}
	inline void setOutVectorY(real p_y)            {m_outVector.y = p_y;}
	inline void setOutVectorZ(real p_z)            {m_outVector.z = p_z;}
	
	inline const Vector3& getStartPoint() const { return m_startPoint; }
	inline const Vector3& getEndPoint()   const { return m_endPoint;   }
	inline const Vector3& getInVector()   const { return m_inVector;   }
	inline const Vector3& getOutVector()  const { return m_outVector;  }
	
	Vector3 model(real p_time) const;
	
private:
	Vector3 m_startPoint;
	Vector3 m_outVector;
	Vector3 m_inVector;
	Vector3 m_endPoint;
};


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& p_s, BezierCurve& p_v)
{
	p_v.load(p_s);
	return p_s;
}


inline tt::streams::BOStream& operator<<(tt::streams::BOStream& p_s, const BezierCurve& p_v)
{
	p_v.save(p_s);
	return p_s;
}


// Namespace end
}
}


#endif // INC_TT_MATH_BEZIERCURVE_H
