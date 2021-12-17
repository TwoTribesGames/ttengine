#if !defined(INC_TT_ENGINE_RENDERER_LINE_H)
#define INC_TT_ENGINE_RENDERER_LINE_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace renderer {


class Sphere;


class Line
{
public:
	Line();
	Line(const math::Vector3& p_start, const math::Vector3& p_end);
	~Line();
	
	bool load(const fs::FilePtr& p_file);
	void print() const;
	
	inline math::Vector3 getStart() const {return m_start;}
	inline math::Vector3 getEnd()   const {return m_end;}
	
	inline void setStartX(real p_x) {m_start.x = p_x;}
	inline void setStartY(real p_y) {m_start.y = p_y;}
	inline void setStartZ(real p_z) {m_start.z = p_z;}
	inline void setStart(real p_x, real p_y, real p_z) {m_start.setValues(p_x, p_y, p_z);}
	inline void setStart(const math::Vector3& p_start) {m_start = p_start;}
	
	inline void setEndX(real p_x) {m_end.x = p_x;}
	inline void setEndY(real p_y) {m_end.y = p_y;}
	inline void setEndZ(real p_z) {m_end.z = p_z;}
	inline void setEnd(real p_x, real p_y, real p_z) {m_end.setValues(p_x, p_y, p_z);}
	inline void setEnd(const math::Vector3& p_end) {m_end = p_end;}
	
	bool intersect(Sphere* p_sphere) const;
	bool intersect(Line* p_line, real* p_time = 0) const;
	real distance(const math::Vector3& p_vector) const;
	
private:
	math::Vector3 m_start;
	math::Vector3 m_end;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_LINE_H
