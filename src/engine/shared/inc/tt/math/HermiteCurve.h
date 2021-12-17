#if !defined(INC_TT_MATH_HERMITECURVE_H)
#define INC_TT_MATH_HERMITECURVE_H

#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/fs/types.h>

namespace tt {
namespace math {

// Class definition
class HermiteCurve
{
public:
	inline HermiteCurve()
	:
	m_startPoint(Vector3::zero),
	m_endPoint(Vector3::zero),
	m_startTangent(Vector3::zero),
	m_endTangent(Vector3::zero)
	{ }
	
	bool load(const fs::FilePtr& p_file);
	void print();
	
	inline void setStartPoint(const Vector3& p_start)   { m_startPoint   = p_start; }
	inline void setStartPointX(real p_x)                { m_startPoint.x = p_x;     }
	inline void setStartPointY(real p_y)                { m_startPoint.y = p_y;     }
	inline void setStartPointZ(real p_z)                { m_startPoint.z = p_z;     }
 	
	inline void setEndPoint(const Vector3& p_end)       { m_endPoint   = p_end; }
	inline void setEndPointX(real p_x)                  { m_endPoint.x = p_x;   }
	inline void setEndPointY(real p_y)                  { m_endPoint.y = p_y;   }
	inline void setEndPointZ(real p_z)                  { m_endPoint.z = p_z;   }
	
	inline void setStartTangent(const Vector3& p_start) { m_startTangent   = p_start; }
	inline void setStartTangentX(real p_x)              { m_startTangent.x = p_x;     }
	inline void setStartTangentY(real p_y)              { m_startTangent.y = p_y;     }
	inline void setStartTangentZ(real p_z)              { m_startTangent.z = p_z;     }
	
	inline void setEndTangent(const Vector3& p_end)     { m_endTangent   = p_end; }
	inline void setEndTangentX(real p_x)                { m_endTangent.x = p_x;   }
	inline void setEndTangentY(real p_y)                { m_endTangent.y = p_y;   }
	inline void setEndTangentZ(real p_z)                { m_endTangent.z = p_z;   }
	
	inline const Vector3& getStartPoint() const { return m_startPoint; }
	inline const Vector3& getEndPoint()   const { return m_endPoint;   }
	
	inline const Vector3& getStartTangent() const { return m_startTangent; }
	inline const Vector3& getEndTangent()   const { return m_endTangent;   }
	
	Vector3 model(real p_time);
	
private:
	Vector3 m_startPoint;
	Vector3 m_endPoint;
	Vector3 m_startTangent;
	Vector3 m_endTangent;
};

// Namespace end
}
}


#endif // INC_TT_MATH_HERMITECURVE_H
