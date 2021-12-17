#if !defined(INC_TT_ENGINE_ANIMATION_TRANSFORMCONTROLLER_H)
#define INC_TT_ENGINE_ANIMATION_TRANSFORMCONTROLLER_H


#include <tt/platform/tt_types.h>
#include <tt/engine/animation/fwd.h>
#include <tt/code/ErrorStatus.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace animation {

class TransformController
{
public:
	TransformController();
	inline ~TransformController() { }
	
	bool getValue(real p_time, math::Matrix44& p_result) const;
	
	void load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus);
	
	inline real getStartTime() const {return m_startTime;}
	inline real getEndTime()   const {return m_endTime;}
	
	void drawPath(real p_time, real p_step);
	
private:
	void getEndTimeFromKeys();

	HermiteFloatControllerPtr m_posXController;
	HermiteFloatControllerPtr m_posYController;
	HermiteFloatControllerPtr m_posZController;

	HermiteFloatControllerPtr m_rotXController;
	HermiteFloatControllerPtr m_rotYController;
	HermiteFloatControllerPtr m_rotZController;

	HermiteFloatControllerPtr m_sclXController;
	HermiteFloatControllerPtr m_sclYController;
	HermiteFloatControllerPtr m_sclZController;

	real m_startTime;
	real m_endTime;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_ANIMATION_TRANSFORMCONTROLLER_H
