#if !defined(INC_TT_ENGINE_ANIMATION_TEXMATRIXCONTROLLER_H)
#define INC_TT_ENGINE_ANIMATION_TEXMATRIXCONTROLLER_H


#include <tt/platform/tt_types.h>
#include <tt/engine/animation/fwd.h>
#include <tt/code/ErrorStatus.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace animation {


class TexMatrixController
{
public:
	TexMatrixController();
	
	void getValue(real p_time, math::Matrix44& p_value) const;

	void load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus);
	
private:
	HermiteFloatControllerPtr m_scaleS;
	HermiteFloatControllerPtr m_scaleT;
	HermiteFloatControllerPtr m_rotate;
	HermiteFloatControllerPtr m_translateS;
	HermiteFloatControllerPtr m_translateT;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_ANIMATION_TEXMATRIXCONTROLLER_H
