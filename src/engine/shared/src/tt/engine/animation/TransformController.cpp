#include <tt/engine/animation/TransformController.h>
#include <tt/engine/animation/HermiteFloatController.h>
#include <algorithm>

#ifndef TT_BUILD_FINAL
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#endif

namespace tt {
namespace engine {
namespace animation {


TransformController::TransformController()
:
m_posXController(),
m_posYController(),
m_posZController(),
m_rotXController(),
m_rotYController(),
m_rotZController(),
m_sclXController(),
m_sclYController(),
m_sclZController(),
m_startTime(0.0f),
m_endTime(0.0f)
{
}


bool TransformController::getValue(real p_time, math::Matrix44& p_result) const
{
	p_result.setIdentity();

	if(m_posXController != 0) // Assumes that all are loaded when this is (might be stupid)
	{
		// NOTE: All these controllers are going to calculate the current key pair, which will
		//       probably be identical for most of them, we might be able to use this for
		//       optimization (For example, pass the key pair as a hint).
		p_result.translate(m_posXController->getValue(p_time),
		                   m_posYController->getValue(p_time),
		                   m_posZController->getValue(p_time));
		p_result.rotateXYZ(m_rotXController->getValue(p_time),
		                   m_rotYController->getValue(p_time),
		                   m_rotZController->getValue(p_time));
		
		const real sclX = m_sclXController->getValue(p_time);
		const real sclY = m_sclYController->getValue(p_time);
		const real sclZ = m_sclZController->getValue(p_time);
		p_result.scale(sclX, sclY, sclZ);
		
		static const real scaleTolerance = 0.008f;
		
		return sclX > scaleTolerance || sclY > scaleTolerance || sclZ > scaleTolerance;
	}
	else
	{
		return true;
	}
}


void TransformController::load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Transform Controller");

	m_posXController.reset(new HermiteFloatController);
	m_posXController->load(p_file, p_errStatus);

	m_posYController.reset(new HermiteFloatController);
	m_posYController->load(p_file, p_errStatus);

	m_posZController.reset(new HermiteFloatController);
	m_posZController->load(p_file, p_errStatus);

	m_rotXController.reset(new HermiteFloatController);
	m_rotXController->load(p_file, p_errStatus);

	m_rotYController.reset(new HermiteFloatController);
	m_rotYController->load(p_file, p_errStatus);

	m_rotZController.reset(new HermiteFloatController);
	m_rotZController->load(p_file, p_errStatus);

	m_sclXController.reset(new HermiteFloatController);
	m_sclXController->load(p_file, p_errStatus);

	m_sclYController.reset(new HermiteFloatController);
	m_sclYController->load(p_file, p_errStatus);

	m_sclZController.reset(new HermiteFloatController);
	m_sclZController->load(p_file, p_errStatus);

	// Update end time
	getEndTimeFromKeys();
}


void TransformController::drawPath(real p_time, real p_step)
{
#ifndef TT_BUILD_FINAL
	// Get start position
	tt::math::Vector3 startPosition;
	startPosition.x = m_posXController->getValue(0);
	startPosition.y = m_posYController->getValue(0);
	startPosition.z = m_posZController->getValue(0);

	tt::math::Vector3 nextPosition;

	for(real time = p_step; time <= p_time; time += p_step)
	{
		// Get next position
		nextPosition.x = m_posXController->getValue(time);
		nextPosition.y = m_posYController->getValue(time);
		nextPosition.z = m_posZController->getValue(time);

		// Draw line from previous to next
		renderer::Renderer::getInstance()->getDebug()->renderLine(
			renderer::ColorRGB::red, startPosition, nextPosition);

		// Move starting point
		startPosition = nextPosition;
	}
#else
	(void) p_time;
	(void) p_step;
#endif
}


void TransformController::getEndTimeFromKeys()
{
	m_endTime = std::max(m_endTime, m_posXController->getEndTime());
	m_endTime = std::max(m_endTime, m_posYController->getEndTime());
	m_endTime = std::max(m_endTime, m_posZController->getEndTime());
	m_endTime = std::max(m_endTime, m_rotXController->getEndTime());
	m_endTime = std::max(m_endTime, m_rotYController->getEndTime());
	m_endTime = std::max(m_endTime, m_rotZController->getEndTime());
	m_endTime = std::max(m_endTime, m_sclXController->getEndTime());
	m_endTime = std::max(m_endTime, m_sclYController->getEndTime());
	m_endTime = std::max(m_endTime, m_sclZController->getEndTime());
}

// Namespace end
}
}
}
