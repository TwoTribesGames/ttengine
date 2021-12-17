#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Renderer.h>


namespace tt {
namespace engine {
namespace renderer {

MatrixStack* MatrixStack::ms_instance = 0;


//--------------------------------------------------------------------------------------------------
// Public member functions

bool MatrixStack::createInstance()
{
	TT_ASSERTMSG(ms_instance == 0, "MatrixStack instance was already created.");
	if (ms_instance == 0)
	{
		ms_instance = new MatrixStack;
	}
	return ms_instance != 0;
}


void MatrixStack::destroyInstance()
{
	delete ms_instance;
	ms_instance = 0;
}


void MatrixStack::load44(const math::Matrix44& p_matrix)
{
	switch (m_matrixMode)
	{
		case Mode_Projection: m_projectionStack.back() = p_matrix; break;
		case Mode_Position  : m_worldStack     .back() = p_matrix; break;
		case Mode_Texture   : m_textureStack   .back() = p_matrix; break;
		default:
			TT_PANIC("Invalid matrix mode: %d", m_matrixMode);
			break;
	}
}


void MatrixStack::multiply44(const math::Matrix44& p_matrix)
{
	switch (m_matrixMode)
	{
		case Mode_Projection: m_projectionStack.back() = p_matrix * m_projectionStack.back(); break;
		case Mode_Position  : m_worldStack     .back() = p_matrix * m_worldStack     .back(); break;
		case Mode_Texture   : m_textureStack   .back() = p_matrix * m_textureStack   .back(); break;
		default:
			TT_PANIC("Invalid matrix mode: %d", m_matrixMode);
			break;
	}
}


void MatrixStack::setIdentity()
{
	switch (m_matrixMode)
	{
		case Mode_Projection: m_projectionStack.back().setIdentity(); break;
		case Mode_Position  : m_worldStack     .back().setIdentity(); break;
		case Mode_Texture   : m_textureStack   .back().setIdentity(); break;
		default:
			TT_PANIC("Invalid matrix mode: %d", m_matrixMode);
			break;
	}
}


void MatrixStack::push()
{
	switch (m_matrixMode)
	{
		case Mode_Projection: m_projectionStack.push_back(m_projectionStack.back()); break;
		case Mode_Position  : m_worldStack     .push_back(m_worldStack     .back()); break;
		case Mode_Texture   : m_textureStack   .push_back(m_textureStack   .back()); break;
		default:
			TT_PANIC("Invalid matrix mode: %d", m_matrixMode);
			break;
	}
}


void MatrixStack::pop()
{
	switch (m_matrixMode)
	{
		case Mode_Projection: m_projectionStack.pop_back(); break;
		case Mode_Position  : m_worldStack     .pop_back(); break;
		case Mode_Texture   : m_textureStack   .pop_back(); break;
		default:
			TT_PANIC("Invalid matrix mode: %d", m_matrixMode);
			break;
	}
}


void MatrixStack::updateProjectionMatrix()
{
	TT_WARN("Projection matrix is managed by Camera.");
}


void MatrixStack::updateWorldMatrix()
{
	FixedFunction::setWorldMatrix(m_worldStack.back());
}


void MatrixStack::updateTextureMatrix()
{
	FixedFunction::setTextureMatrix(m_textureStack.back());
}


//--------------------------------------------------------------------------------------------------
// Private member functions

MatrixStack::MatrixStack()
: m_matrixMode(Mode_Position)
{
	m_worldStack     .reserve(16);
	m_projectionStack.reserve(8);
	m_textureStack   .reserve(8);
	m_worldStack     .push_back(math::Matrix44::identity);
	m_projectionStack.push_back (math::Matrix44::identity);
	m_textureStack   .push_back (math::Matrix44::identity);
}

// Namespace end
}
}
}
