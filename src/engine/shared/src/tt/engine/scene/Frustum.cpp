#include <tt/engine/scene/Frustum.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene/Camera.h>

namespace tt {
namespace engine {
namespace scene {


/*! Frustrum calculations, Radar approach as described in an article in Game Programming Gems 5
    http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
    Actually we use a modified version that runs in normalized view space, making the calculations
    much simpler. As Revolution & Nitro keep the current matrix (MatrixStack) in view space it very
    fast and lightweight for those platforms.
*/


Frustum::Frustum()
:
m_radarTangent(0.0f),
m_far(0.0f),
m_near(0.0f),
m_aspect(0.0f),
m_passTop(false)
{
}


void Frustum::updateRatio(real p_fov, real p_aspect)
{
	m_radarTangent = math::tan(p_fov * 0.5f);
	m_aspect = p_aspect;
}


void Frustum::updatePlanes(real p_near, real p_far)
{
	m_near = p_near;
	m_far  = p_far;
}


bool Frustum::containsPoint(const math::Vector3& p_point) const
{
	// Check point against near/far plane
	if(p_point.z < -m_far || p_point.z > -m_near)
	{
		return false;
	}
	
	// Compute max y
	real limit(-p_point.z * m_radarTangent);

	// Check point against y boundaries
	if(p_point.y < -limit || p_point.y > limit)
	{
		return false;
	}

	// Compute max x
	limit *= m_aspect;

	// Check point against x boundaries
	if(p_point.x < -limit || p_point.x > limit)
	{
		return false;
	}

	// Point lies within frustum
	return true;
}


bool Frustum::containsSphere(const math::Vector3& p_pos, real p_radius) const
{
	// Check sphere against near/far plane
	if(p_pos.z < -(m_far + p_radius) || p_pos.z > -(m_near - p_radius))
	{
		return false;
	}
	
	// Compute max y
	real limit(-p_pos.z * m_radarTangent + p_radius);

	// Check sphere against y boundaries
	if(p_pos.y < -limit || p_pos.y > limit)
	{
		return false;
	}

	// Compute max x
	limit *= m_aspect;

	// Check sphere against x boundaries
	if(p_pos.x < -limit || p_pos.x > limit)
	{
		return false;
	}

	// Sphere lies within frustum
	return true;
}


math::VectorRect Frustum::getCullRect(real p_distance) const
{
	real height = m_radarTangent * p_distance * 2;
	real width  = height * m_aspect;
	return math::VectorRect(math::Vector2::zero, width, height);
}


void Frustum::render(Camera* p_camera) const
{
	(void)p_camera;

#if !defined(TT_BUILD_FINAL)
	using math::Vector3;

	real y_near(m_radarTangent * m_near);
	real y_far (m_radarTangent * m_far);

	math::Vector3 nearv(y_near * m_aspect, y_near, -m_near);
	math::Vector3 farv (y_far  * m_aspect, y_far , -m_far);

	debug::DebugRendererPtr dbgRender(renderer::Renderer::getInstance()->getDebug());
	renderer::ColorRGBA color(0,255,0,255);

	renderer::MatrixStack::getInstance()->push();

	// Transform from view to world space
	renderer::MatrixStack::getInstance()->multiply44(p_camera->getViewMatrixInverse());

	// Near Plane
	dbgRender->renderLine(color, Vector3(-nearv.x,  nearv.y, nearv.z), Vector3( nearv.x,  nearv.y, nearv.z));
	dbgRender->renderLine(color, Vector3( nearv.x,  nearv.y, nearv.z), Vector3( nearv.x, -nearv.y, nearv.z));
	dbgRender->renderLine(color, Vector3( nearv.x, -nearv.y, nearv.z), Vector3(-nearv.x, -nearv.y, nearv.z));
	dbgRender->renderLine(color, Vector3(-nearv.x, -nearv.y, nearv.z), Vector3(-nearv.x,  nearv.y, nearv.z));

	// Far Plane
	dbgRender->renderLine(color, Vector3(-farv.x,  farv.y, farv.z), Vector3( farv.x,  farv.y, farv.z));
	dbgRender->renderLine(color, Vector3( farv.x,  farv.y, farv.z), Vector3( farv.x, -farv.y, farv.z));
	dbgRender->renderLine(color, Vector3( farv.x, -farv.y, farv.z), Vector3(-farv.x, -farv.y, farv.z));
	dbgRender->renderLine(color, Vector3(-farv.x, -farv.y, farv.z), Vector3(-farv.x,  farv.y, farv.z));

	// Frustum boundaries
	dbgRender->renderLine(color, Vector3::zero, Vector3(-farv.x,  farv.y, farv.z));
	dbgRender->renderLine(color, Vector3::zero, Vector3(-farv.x, -farv.y, farv.z));
	dbgRender->renderLine(color, Vector3::zero, Vector3( farv.x, -farv.y, farv.z));
	dbgRender->renderLine(color, Vector3::zero, Vector3( farv.x,  farv.y, farv.z));

	renderer::MatrixStack::getInstance()->pop();
#endif
}


// Namespace end
}
} 
}
