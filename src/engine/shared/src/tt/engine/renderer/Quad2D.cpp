#include <tt/engine/opengl_headers.h>
#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/GLStateCache.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>


namespace tt {
namespace engine {
namespace renderer {

// We only keep one copy of the normalized quad data around
static const GLshort quadPositions[] =
{
	-Quad2D::quadSize, -Quad2D::quadSize, 0, // BottomLeft
	 Quad2D::quadSize, -Quad2D::quadSize, 0, // BottomRight
	-Quad2D::quadSize,  Quad2D::quadSize, 0, // TopLeft
	 Quad2D::quadSize,  Quad2D::quadSize, 0  // TopRight
};


//--------------------------------------------------------------------------------------------------
// Public member functions

Quad2D::Quad2D(u32 p_vertexType, const ColorRGBA& p_color)
:
m_vertexType(static_cast<u8>(p_vertexType))
{
	// Must be colored, textured or both
	TT_ASSERT(p_vertexType & VertexBuffer::Property_Diffuse ||
	          p_vertexType & VertexBuffer::Property_Texture0);

	if (m_vertexType & VertexBuffer::Property_Diffuse)
	{
		m_color[0] = m_color[1] = m_color[2] = m_color[3] = p_color;
	}
	if (m_vertexType & VertexBuffer::Property_Texture0)
	{
		updateTexcoords(TexturePtr());
	}
}


Quad2D::Quad2D(const Quad2D& p_rhs)
:
m_vertexType(p_rhs.m_vertexType)
{
	std::memcpy(m_color,     p_rhs.m_color,     sizeof(ColorRGBA)     * Vertex_Count);
	std::memcpy(m_texcoords, p_rhs.m_texcoords, sizeof(math::Vector2) * Vertex_Count);
}


Quad2D& Quad2D::operator=(const Quad2D &p_rhs)
{
	m_vertexType = p_rhs.m_vertexType;
	
	std::memcpy(m_color,     p_rhs.m_color,     sizeof(ColorRGBA)     * Vertex_Count);
	std::memcpy(m_texcoords, p_rhs.m_texcoords, sizeof(math::Vector2) * Vertex_Count);
	
	return *this;
}


Quad2D::~Quad2D()
{
}


void Quad2D::setColor(const ColorRGBA& p_color)
{
	for (s32 i = 0; i < Vertex_Count; ++i)
	{
		m_color[i] = p_color;
	}
}


void Quad2D::setColor(const ColorRGB& p_color)
{
	for (s32 i = 0; i < Vertex_Count; ++i)
	{
		m_color[i].r = p_color.r;
		m_color[i].g = p_color.g;
		m_color[i].b = p_color.b;
	}
}


void Quad2D::setColor(const ColorRGBA& p_color, Vertex p_vertex)
{
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index: %d", p_vertex);
	m_color[p_vertex] = p_color;
}


void Quad2D::setColor(const ColorRGB& p_color, Vertex p_vertex)
{
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index: %d", p_vertex);
	m_color[p_vertex].r = p_color.r;
	m_color[p_vertex].g = p_color.g;
	m_color[p_vertex].b = p_color.b;
}


void Quad2D::setAlpha(u8 p_alpha)
{
	for (s32 i = 0; i < Vertex_Count; ++i)
	{
		m_color[i].a = p_alpha;
	}
}


void Quad2D::setTexcoord(Quad2D::Vertex p_vertex, const math::Vector2& p_texcoord)
{
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index");
	m_texcoords[p_vertex] = p_texcoord;
}


void Quad2D::updateTexcoords(const TexturePtr&)
{
	// Initialize new texture coordinates
	math::Vector2 maxTexCoord(1.0f, 1.0f);
	
	setTexcoord(Quad2D::Vertex_TopLeft,     math::Vector2::zero);
	setTexcoord(Quad2D::Vertex_TopRight,    math::Vector2(maxTexCoord.x, 0));
	setTexcoord(Quad2D::Vertex_BottomLeft,  math::Vector2(0, maxTexCoord.y));
	setTexcoord(Quad2D::Vertex_BottomRight, maxTexCoord);
}


void Quad2D::updateTexcoords(const TexturePtr& p_texture, s32 p_frameWidth, s32 p_frameHeight)
{
	TT_ASSERT(p_frameWidth != 0 && p_frameHeight != 0);
	
	// Initialize new texture coordinates
	math::Vector2 maxTexCoord(real(p_frameWidth)  / p_texture->getWidth(),
	                          real(p_frameHeight) / p_texture->getHeight());
	
	/*
	TT_Printf("Quad2D::updateTexcoords - setting texture coords from frame width: %d, height: %d. "
	          "Texture size %d x %d. UVcoords 0,0 to %f, %f\n",
	          p_frameWidth, p_frameHeight, p_texture->getWidth(), p_texture->getHeight(),
	          maxTexCoord.x, maxTexCoord.y);
	*/
	
	setTexcoord(Quad2D::Vertex_TopLeft,     math::Vector2::zero);
	setTexcoord(Quad2D::Vertex_TopRight,    math::Vector2(maxTexCoord.x, 0));
	setTexcoord(Quad2D::Vertex_BottomLeft,  math::Vector2(0, maxTexCoord.y));
	setTexcoord(Quad2D::Vertex_BottomRight, maxTexCoord);
}


void Quad2D::update()
{
}


void Quad2D::render() const
{
	Renderer* renderer = Renderer::getInstance();

	renderer->setVertexType(m_vertexType);

	MatrixStack::getInstance()->updateWorldMatrix();
	
	// Set position array
	glVertexPointer(3, GL_SHORT, 0, quadPositions);
	
	if (m_vertexType & VertexBuffer::Property_Diffuse)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_color);
	}
	
	if (m_vertexType & VertexBuffer::Property_Texture0)
	{
		Texture::setActiveClientChannel(0);
		glTexCoordPointer(2, GL_FLOAT, 0, m_texcoords);
	}

	renderer->stateCache()->apply();

	// Render the quad
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	TT_CHECK_OPENGL_ERROR();
	
	debug::DebugStats::addToQuadsRendered(1);
}

// Namespace end
}
}
}
