#include <tt/engine/animation/TexMatrixController.h>
#include <tt/engine/animation/HermiteFloatController.h>
#include <tt/fs/File.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace animation {

TexMatrixController::TexMatrixController()
:
m_scaleS(),
m_scaleT(),
m_rotate(),
m_translateS(),
m_translateT()
{
}


void TexMatrixController::getValue(real p_time, math::Matrix44& p_result) const
{
	p_result = math::Matrix44::getMayaTextureMatrix(
		m_scaleS->getValue(p_time),
		m_scaleT->getValue(p_time),
		m_rotate->getValue(p_time),
		m_translateS->getValue(p_time),
		m_translateT->getValue(p_time));
}


void TexMatrixController::load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Texture Matrix Controller");

	m_scaleS.reset(new HermiteFloatController);
	m_scaleS->load(p_file, p_errStatus);

	m_scaleT.reset(new HermiteFloatController);
	m_scaleT->load(p_file, p_errStatus);

	m_rotate.reset(new HermiteFloatController);
	m_rotate->load(p_file, p_errStatus);

	m_translateS.reset(new HermiteFloatController);
	m_translateS->load(p_file, p_errStatus);

	m_translateT.reset(new HermiteFloatController);
	m_translateT->load(p_file, p_errStatus);
}



// Namespace end
}
}
}

