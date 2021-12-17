#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/ColorAnimationStack2D.h>
#include <tt/engine/scene2d/PlaneScene.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/scene/Camera.h>

#include <tt/engine/renderer/Quad2D.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/code/helpers.h>
#include <tt/math/Matrix22.h>
#include <tt/math/math.h>


namespace tt {
namespace engine {
namespace scene2d {

PlaneScene::PlaneScene(real p_width,
					   real p_height,
					   renderer::TexturePtr p_texture,
                       const tt::math::Vector3& p_position,
					   real p_rotation,
                       real p_texAnimU,
					   real p_texAnimV,
                       real p_texOffsetScale,
                       s32  p_priority,
					   bool p_withVertexColors)
:
Scene2D(),
m_boundingRadius(static_cast<real>(tt::math::getHalf(tt::math::sqrt(static_cast<real64>(p_width) *
				                                                    static_cast<real64>(p_width) +
				                                                    static_cast<real64>(p_height) *
												                    static_cast<real64>(p_height))))),
m_boundingRect(),
m_needsUpdate(false),
m_renderQuadNeedsUpdate(true),
m_cullCheckDone(false),
m_hasTextureAnim(false),
m_renderQuad(),
m_texture(p_texture),
m_width(p_width),
m_height(p_height),
m_rotation(p_rotation),
m_offsetTime(0.0f),
m_offsetXMax(0.0f),
m_offsetYMax(0.0f),
m_offsetXDuration(0.0f),
m_offsetYDuration(0.0f),
m_offsetXTimeOffset(0.0f),
m_offsetYTimeOffset(0.0f),
m_cameraSpaceScale(0,0),
m_texU(0),
m_texV(0),
m_addressModeU(renderer::AddressMode_Clamp),
m_addressModeV(renderer::AddressMode_Clamp),
m_texAnimU(p_texAnimU),
m_texAnimV(p_texAnimV),
m_texOffsetScale(p_texOffsetScale),
m_offsetPosition(math::Vector3::zero),
m_animations(),
m_textureAnimation(),
m_colorAnimation(),
m_posMtxDirty(true),
m_texMtxDirty(true),
m_posMatrix(),
m_posAnimMatrix(),
m_centerPosition(),
m_texMatrix(),
m_topLeftColor(),
m_topRightColor(),
m_bottomLeftColor(),
m_bottomRightColor()
{
	// Set base class members
	setPosition(p_position.x, p_position.y);
	setDepth(p_position.z);
	setPriority(p_priority);

	using renderer::VertexBuffer;
	using renderer::Quad2D;
	using renderer::Quad2DPtr;

	if (p_withVertexColors)
	{
		// Create a render quad for this plan
		m_renderQuad = Quad2DPtr(new Quad2D(VertexBuffer::Property_Diffuse | VertexBuffer::Property_Texture0));
	}
	else
	{
		m_renderQuad = Quad2DPtr(new Quad2D(VertexBuffer::Property_Texture0));
	}
	TT_NULL_ASSERT(m_renderQuad);

	// Set correct address mode
	m_addressModeU = math::realEqual(m_texAnimU, 0.0f) ? renderer::AddressMode_Clamp : renderer::AddressMode_Wrap;
	m_addressModeV = math::realEqual(m_texAnimV, 0.0f) ? renderer::AddressMode_Clamp : renderer::AddressMode_Wrap;
	
	updateBoundingRect();
	updateNeedsUpdate();
}


PlaneScene::~PlaneScene()
{
}


void PlaneScene::update(real p_deltaTime)
{
	m_hasTextureAnim = false;

	real z = m_centerPosition.z;
	if (m_texAnimU != 0 || m_texAnimV != 0)
	{
		m_texU += (m_texAnimU * p_deltaTime);
		m_texV += (m_texAnimV * p_deltaTime);
		
		{
			real width = static_cast<real>(m_texture->getWidth());
			
			if (m_texU < -width)
			{
				m_texU += width;
			}
			else if (m_texU > width)
			{
				m_texU -= width;
			}
		}
		
		{
			real height = static_cast<real>(m_texture->getHeight());
			
			if (m_texV < -height)
			{
				m_texV += height;
			}
			else if (m_texV > height)
			{
				m_texV -= height;
			}
		}
		
		m_texMtxDirty = true;
		m_hasTextureAnim = true;
	}
	if (m_texMtxDirty)
	{
		m_texMatrix.setIdentity();
		m_texMatrix.translate(m_texU, m_texV);
		m_texMtxDirty = false;
	}
	
	m_offsetTime += p_deltaTime;
	
	updatePosition(p_deltaTime);
	
	if (m_textureAnimation != 0)
	{
		m_textureAnimation->update(p_deltaTime);
		m_hasTextureAnim = true;
	}
	if (m_colorAnimation != 0)
	{
		m_colorAnimation->update(p_deltaTime);

		setQuadColors(m_colorAnimation->getColor(m_topLeftColor),
		              m_colorAnimation->getColor(m_topRightColor),
		              m_colorAnimation->getColor(m_bottomLeftColor),
		              m_colorAnimation->getColor(m_bottomRightColor));
	}
	if (z != m_centerPosition.z)
	{
		resort();
	}

	m_hasTextureAnim = m_hasTextureAnim || m_cameraSpaceScale != math::Vector2::zero;
}


void PlaneScene::render()
{
	if (m_cullCheckDone == false)
	{
		doCullCheck();
	}
	m_cullCheckDone = false;
	
	if (m_isCulled) return;
	
	if(m_renderQuadNeedsUpdate)
	{
		m_renderQuad->update();
		m_renderQuadNeedsUpdate = false;
	}
	
	// Save fog enabled setting
	renderer::Renderer* renderer(renderer::Renderer::getInstance());
	bool fogEnabled = renderer->isFogEnabled();
	
	// Enable/disable fog based on plane settings, only if fog is enabled in the renderer
	if (fogEnabled)
	{
		renderer->setFogEnabled(isFogEnabled());
	}
	
	// Activate texture
	m_texture->setAddressMode(m_addressModeU, m_addressModeV);
	renderer->setTexture(m_texture);
	renderer->setBlendMode(getBlendMode());

	// Setup world transform
	renderer::MatrixStack* stack = renderer::MatrixStack::getInstance();
	stack->setMode(renderer::MatrixStack::Mode_Position);
	stack->push();
	
	real distance = 0;
	if (isScreenSpace())
	{
		const scene::CameraPtr& cam(renderer->getActiveCamera());

		math::Vector3 campos(cam->getActualPosition());
		distance = campos.z - m_offsetPosition.z;
		campos.z = 0;
		//pos += campos;
		stack->multiply44(math::Matrix44::getTranslation(campos));
		
		math::Vector3 texCoords(m_texU, m_texV);
		texCoords.x += (campos.x / cam->getWidth())  * m_texOffsetScale;
		texCoords.y -= (campos.y / cam->getHeight()) * m_texOffsetScale;
		
		// Recalc texture matrix
		m_texMatrix.setIdentity();
		m_texMatrix.translate(texCoords);
	}
	
	stack->multiply44(getMatrix());
	
	using renderer::Quad2D;
	if (isScreenSpace())
	{
		real ppscale = renderer->getActiveCamera()->getPixelPerfectScale() * distance;
		stack->scale(
			math::Vector3((m_width * ppscale) / (Quad2D::quadSize * 2), (m_height * ppscale) / (Quad2D::quadSize * 2), 1.0f));
	}
	else
	{
		stack->scale(
			math::Vector3(m_width / (Quad2D::quadSize * 2), m_height / (Quad2D::quadSize * 2), 1.0f));
	}
	
	
	stack->setMode(renderer::MatrixStack::Mode_Texture);
	
	if(m_hasTextureAnim)
	{
		math::Matrix44 textureMatrix(m_texMatrix);

		if (math::realEqual(m_cameraSpaceScale.x, 0.0f) == false ||
			math::realEqual(m_cameraSpaceScale.y, 0.0f) == false)
		{
			
			// Texture scrolls in camera space
			const math::Vector3 cameraPos = renderer->getActiveCamera()->getActualPosition();

			textureMatrix.translate( cameraPos.x * m_cameraSpaceScale.x,
									-cameraPos.y * m_cameraSpaceScale.y, 0);
		}

		// Update Texture Matrix (Scrolling Only)
		if (m_textureAnimation != 0)
		{
			textureMatrix = m_textureAnimation->getTransform() * textureMatrix;
		}
	
		stack->load44(textureMatrix);
	}
	else
	{
		stack->setIdentity();
	}

	stack->updateTextureMatrix();
	stack->setMode(renderer::MatrixStack::Mode_Position);

	// Render the plane
	m_renderQuad->render();

	stack->pop();
	stack->resetTextureMatrix();
	
	// Restore fog enabled setting
	if (fogEnabled)
	{
		renderer->setFogEnabled(true);
	}
}


void PlaneScene::setTextureCoordinates(real p_topLeftU,     real p_topLeftV,
									   real p_topRightU,    real p_topRightV,
									   real p_bottomRightU, real p_bottomRightV,
									   real p_bottomLeftU,  real p_bottomLeftV)
{
	// Check the U coordinates.
	m_addressModeU = renderer::AddressMode_Wrap;
	if(m_texAnimU == 0.0f && m_texOffsetScale == 0.0f && m_textureAnimation == 0 && m_cameraSpaceScale.x == 0) // No texture animation.
	{
		if(p_topLeftU     >= 0.0f && p_topLeftU     <= 1.0f &&
		   p_topRightU    >= 0.0f && p_topRightU    <= 1.0f &&
		   p_bottomLeftU  >= 0.0f && p_bottomLeftU  <= 1.0f &&
		   p_bottomRightU >= 0.0f && p_bottomRightU <= 1.0f)
		{
			m_addressModeU = renderer::AddressMode_Clamp;
		}
	}
	// Check the V coordinates.
	m_addressModeV = renderer::AddressMode_Wrap;
	if(m_texAnimV == 0.0f && m_texOffsetScale == 0.0f && m_textureAnimation == 0 && m_cameraSpaceScale.y == 0) // No texture animation.
	{
		if(p_topLeftV     >= 0.0f && p_topLeftV     <= 1.0f &&
		   p_topRightV    >= 0.0f && p_topRightV    <= 1.0f &&
		   p_bottomLeftV  >= 0.0f && p_bottomLeftV  <= 1.0f &&
		   p_bottomRightV >= 0.0f && p_bottomRightV <= 1.0f)
		{
			m_addressModeV = renderer::AddressMode_Clamp;
		}
	}
	
	using renderer::Quad2D;
	m_topLeftUV    .setValues(p_topLeftU,     p_topLeftV    );
	m_topRightUV   .setValues(p_topRightU,    p_topRightV   );
	m_bottomRightUV.setValues(p_bottomRightU, p_bottomRightV);
	m_bottomLeftUV .setValues(p_bottomLeftU,  p_bottomLeftV );
	
	m_renderQuad->setTexcoord(Quad2D::Vertex_TopLeft    , m_topLeftUV    );
	m_renderQuad->setTexcoord(Quad2D::Vertex_TopRight   , m_topRightUV   );
	m_renderQuad->setTexcoord(Quad2D::Vertex_BottomRight, m_bottomRightUV);
	m_renderQuad->setTexcoord(Quad2D::Vertex_BottomLeft , m_bottomLeftUV );
	m_renderQuadNeedsUpdate = true;
}


void PlaneScene::setVertexColors(const renderer::ColorRGBA& p_topLeftColor,
								 const renderer::ColorRGBA& p_topRightColor,
								 const renderer::ColorRGBA& p_bottomLeftColor,
								 const renderer::ColorRGBA& p_bottomRightColor)
{
	setQuadColors(p_topLeftColor, p_topRightColor, p_bottomLeftColor, p_bottomRightColor);
	
	m_topLeftColor     = p_topLeftColor;
	m_topRightColor    = p_topRightColor;
	m_bottomLeftColor  = p_bottomLeftColor;
	m_bottomRightColor = p_bottomRightColor;
}


void PlaneScene::setOffsetAnimation(real p_offsetXMax, real p_offsetXDuration, 
								    real p_offsetXTimeOffset, 
								    real p_offsetYMax, real p_offsetYDuration,
								    real p_offsetYTimeOffset)
{
	m_offsetXMax        = p_offsetXMax;
	m_offsetXDuration   = p_offsetXDuration;
	m_offsetXTimeOffset = p_offsetXTimeOffset;
	m_offsetYMax        = p_offsetYMax;
	m_offsetYDuration   = p_offsetYDuration;
	m_offsetYTimeOffset = p_offsetYTimeOffset;
	updateBoundingRect();
	updateNeedsUpdate();
}


void PlaneScene::setAnimations(const anim2d::AnimationStack2DPtr& p_animations)
{
	m_animations.reset();
	if (p_animations != 0 && p_animations->empty() == false)
	{
		// Only assign new animations if it's not empty.
		m_animations = p_animations;
	}
	updatePosition(0.0f);
	updateNeedsUpdate();
}


void PlaneScene::setTextureAnimation(const anim2d::AnimationStack2DPtr& p_animation)
{
	m_textureAnimation.reset();
	if (p_animation != 0 && p_animation->empty() == false)
	{
		// Only assign new animations if it's not empty.
		m_textureAnimation = p_animation;
	}
	
	if (m_textureAnimation != 0)
	{
		m_addressModeU = renderer::AddressMode_Wrap;
		m_addressModeV = renderer::AddressMode_Wrap;
	}
	updateNeedsUpdate();
}


void PlaneScene::setColorAnimation(const anim2d::ColorAnimationStack2DPtr& p_animations)
{
	m_colorAnimation.reset();
	if (p_animations != 0 && p_animations->empty() == false)
	{
		// Only assign new animations if it's not empty.
		m_colorAnimation = p_animations;
	}
	updateNeedsUpdate();
}


bool PlaneScene::doCullCheck()
{
	bool isCulled = false;
	
	// No culling for screen space planes
	if (isScreenSpace() == false)
	{
		const scene::CameraPtr& cam(renderer::Renderer::getInstance()->getActiveCamera());
		TT_NULL_ASSERT(cam);
		
		isCulled = cam->isVisible(m_boundingRect, m_centerPosition.z) == false;
	}
	m_isCulled = isCulled;
	m_cullCheckDone = true;

	return isCulled;
}


void PlaneScene::makeBatchQuad(renderer::BatchQuad& p_quad)
{
	math::Matrix44 transform = math::Matrix44::getTranslation(Scene2D::getPosition());
	
	if(math::realEqual(m_rotation, 0.0f) == false)
	{
		transform.rotateZ(math::degToRad(m_rotation));
	}
	transform.scale(m_width, m_height);
	
	static const tt::math::Vector3 topLeft    (-0.5f,  0.5f, 0.0f);
	static const tt::math::Vector3 topRight   ( 0.5f,  0.5f, 0.0f);
	static const tt::math::Vector3 bottomLeft (-0.5f, -0.5f, 0.0f);
	static const tt::math::Vector3 bottomRight( 0.5f, -0.5f, 0.0f);
	
	p_quad.topLeft    .setPosition(topLeft     * transform);
	p_quad.topRight   .setPosition(topRight    * transform);
	p_quad.bottomLeft .setPosition(bottomLeft  * transform);
	p_quad.bottomRight.setPosition(bottomRight * transform);
	
	p_quad.topLeft    .setTexCoord(m_topLeftUV);
	p_quad.topRight   .setTexCoord(m_topRightUV);
	p_quad.bottomLeft .setTexCoord(m_bottomLeftUV);
	p_quad.bottomRight.setTexCoord(m_bottomRightUV);
	
	if(m_texture != 0 && m_texture->isPremultiplied())
	{
		renderer::ColorRGBA topLeftColor    (m_topLeftColor);
		renderer::ColorRGBA topRightColor   (m_topRightColor);
		renderer::ColorRGBA bottomLeftColor (m_bottomLeftColor);
		renderer::ColorRGBA bottomRightColor(m_bottomRightColor);

		topLeftColor.    premultiply();
		topRightColor.   premultiply();
		bottomLeftColor. premultiply();
		bottomRightColor.premultiply();

		p_quad.topLeft    .setColor(topLeftColor);
		p_quad.topRight   .setColor(topRightColor);
		p_quad.bottomLeft .setColor(bottomLeftColor);
		p_quad.bottomRight.setColor(bottomRightColor);
	}
	else
	{
		p_quad.topLeft    .setColor(m_topLeftColor);
		p_quad.topRight   .setColor(m_topRightColor);
		p_quad.bottomLeft .setColor(m_bottomLeftColor);
		p_quad.bottomRight.setColor(m_bottomRightColor);
	}
}


bool PlaneScene::isSuitableForBatching() const
{
	return isScreenSpace() == false &&
		m_animations == 0 && m_colorAnimation == 0 && m_textureAnimation == 0 &&
		math::realEqual(m_texAnimU, 0.0f) && math::realEqual(m_texAnimV, 0.0f) &&
		math::realEqual(m_cameraSpaceScale.x, 0.0f) && math::realEqual(m_cameraSpaceScale.y, 0.0f);
}


void PlaneScene::updatePosition(real p_deltaTime)
{
	// Get position of the plane
	m_offsetPosition = Scene2D::getPosition();
	
	// Apply offset animation.
	if (m_offsetXMax != 0)
	{
		m_offsetPosition.x += getOffset(m_offsetXMax, m_offsetXDuration, m_offsetXTimeOffset);
		m_posMtxDirty = true;
	}
	if (m_offsetYMax != 0)
	{
		m_offsetPosition.y += getOffset(m_offsetYMax, m_offsetYDuration, m_offsetYTimeOffset);
		m_posMtxDirty = true;
	}
	
	if (m_posMtxDirty)
	{
		m_posMatrix.setIdentity();
		m_posMatrix.translate(m_offsetPosition);
		m_posMatrix.rotateZ(tt::math::degToRad(m_rotation));
		
		// Only calculate centerposition if there are no animations, otherwise this
		// position is calculated twice
		if (m_animations == 0)
		{
			m_centerPosition = tt::math::Vector3::zero * m_posMatrix;
			TT_ASSERT(m_centerPosition == m_offsetPosition);
		}
		m_posMtxDirty = false;
	}
	
	if (m_animations != 0)
	{
		m_animations->update(p_deltaTime);
		m_animations->updateTransform(&m_posAnimMatrix);
		m_posAnimMatrix = m_posAnimMatrix * m_posMatrix;
		
		m_centerPosition = tt::math::Vector3::zero * m_posAnimMatrix;
	}
	updateBoundingRect();
}


void PlaneScene::updateBoundingRect()
{
	const tt::math::Matrix22 transform(tt::math::Matrix22::getRotation(tt::math::degToRad(m_rotation)));
	
	// get all points
	const real halfWidth(getWidth() / 2.0f);
	const real halfHeight(getHeight() / 2.0f);
	tt::math::Vector2 lu(-halfWidth,  halfHeight);
	tt::math::Vector2 ld(-halfWidth, -halfHeight);
	tt::math::Vector2 ru( halfWidth,  halfHeight);
	tt::math::Vector2 rd( halfWidth, -halfHeight);
	
	// rotate all points
	lu = lu * transform;
	ld = ld * transform;
	ru = ru * transform;
	rd = rd * transform;
	
	// get extremes
	const real minx = std::min(std::min(lu.x, ld.x), std::min(ru.x, rd.x)) + m_centerPosition.x - m_offsetXMax;
	const real maxx = std::max(std::max(lu.x, ld.x), std::max(ru.x, rd.x)) + m_centerPosition.x + m_offsetXMax;
	const real miny = std::min(std::min(lu.y, ld.y), std::min(ru.y, rd.y)) + m_centerPosition.y - m_offsetXMax;
	const real maxy = std::max(std::max(lu.y, ld.y), std::max(ru.y, rd.y)) + m_centerPosition.y + m_offsetYMax;
	
	math::Vector2 min(minx, miny);
	math::Vector2 max(maxx, maxy);
	
	m_boundingRect = math::VectorRect(min, max);
}


void PlaneScene::updateNeedsUpdate()
{
	m_needsUpdate = false;
	if (m_offsetXMax != 0 || m_offsetYMax != 0)
	{
		m_needsUpdate = true;
	}
	else if (m_texU != 0 || m_texV != 0)
	{
		m_needsUpdate = true;
	}
	else if (m_animations != 0)
	{
		m_needsUpdate = true;
	}
	else if (m_textureAnimation != 0)
	{
		m_needsUpdate = true;
	}
	else if (m_colorAnimation != 0)
	{
		m_needsUpdate = true;
	}
	else if (m_texAnimU != 0 || m_texAnimV != 0)
	{
		m_needsUpdate = true;
	}
}


void PlaneScene::setQuadColors(const renderer::ColorRGBA& p_topLeftColor,
	                           const renderer::ColorRGBA& p_topRightColor,
	                           const renderer::ColorRGBA& p_bottomLeftColor,
	                           const renderer::ColorRGBA& p_bottomRightColor)
{
	if(m_texture != 0 && m_texture->isPremultiplied())
	{
		renderer::ColorRGBA topLeftColor    (p_topLeftColor);
		renderer::ColorRGBA topRightColor   (p_topRightColor);
		renderer::ColorRGBA bottomLeftColor (p_bottomLeftColor);
		renderer::ColorRGBA bottomRightColor(p_bottomRightColor);

		topLeftColor.    premultiply();
		topRightColor.   premultiply();
		bottomLeftColor. premultiply();
		bottomRightColor.premultiply();

		m_renderQuad->setColor(topLeftColor,     renderer::Quad2D::Vertex_TopLeft);
		m_renderQuad->setColor(topRightColor,    renderer::Quad2D::Vertex_TopRight);
		m_renderQuad->setColor(bottomLeftColor,  renderer::Quad2D::Vertex_BottomLeft);
		m_renderQuad->setColor(bottomRightColor, renderer::Quad2D::Vertex_BottomRight);
	}
	else
	{
		m_renderQuad->setColor(p_topLeftColor,     renderer::Quad2D::Vertex_TopLeft);
		m_renderQuad->setColor(p_topRightColor,    renderer::Quad2D::Vertex_TopRight);
		m_renderQuad->setColor(p_bottomLeftColor,  renderer::Quad2D::Vertex_BottomLeft);
		m_renderQuad->setColor(p_bottomRightColor, renderer::Quad2D::Vertex_BottomRight);
	}
	m_renderQuadNeedsUpdate = true;
}


//namespace end
}
}
}


