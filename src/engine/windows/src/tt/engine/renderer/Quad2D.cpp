
#include <tt/engine/renderer/Quad2D.h>

#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/Texture.h>


namespace tt {
namespace engine {
namespace renderer {

// Layout structures for vertices
struct VtxCol
{
	math::Vector3 pos;
	D3DCOLOR      col;
};

struct VtxTex
{
	math::Vector3 pos;
	math::Vector2 tex;
};

struct VtxColTex
{
	math::Vector3 pos;
	D3DCOLOR      col;
	math::Vector2 tex;
};


Quad2D::Quad2D(u32 p_vertexType, const ColorRGBA& p_color)
:
m_vertexData(0),
m_vertexType(static_cast<u8>(p_vertexType)),
m_vertexBuffer(0),
m_vertexSize(sizeof(math::Vector3))
{
	// Must be colored, textured or both
	TT_ASSERT(p_vertexType & VertexBuffer::Property_Diffuse ||
		      p_vertexType & VertexBuffer::Property_Texture0);
	
	// Calculate size of single vertex
	if(m_vertexType & VertexBuffer::Property_Diffuse)
	{
		m_vertexSize += sizeof(ColorRGBA);
	}
	if(m_vertexType & VertexBuffer::Property_Texture0)
	{
		m_vertexSize += sizeof(math::Vector2);
	}
	
	// Create the vertex data
	initVertexData(p_color);
	
	// Register D3D resource
	D3DResourceRegistry::registerResource(this);
}


Quad2D::Quad2D(const Quad2D& p_rhs)
:
m_vertexType(p_rhs.m_vertexType),
m_vertexSize(p_rhs.m_vertexSize),
m_vertexData(0),
m_vertexBuffer(0)
{
	// Copy vertex data from source
	m_vertexData = ::operator new(m_vertexSize * Vertex_Count);
	std::memcpy(m_vertexData, p_rhs.m_vertexData, m_vertexSize * Vertex_Count);

	// Register D3D resource
	D3DResourceRegistry::registerResource(this);
}


Quad2D& Quad2D::operator =(const Quad2D &p_rhs)
{
	m_vertexType = p_rhs.m_vertexType;
	m_vertexSize = p_rhs.m_vertexSize;
	
	// Delete old data
	if(m_vertexData != 0)
	{
		::operator delete(m_vertexData);
	}
	
	// Copy data from source
	m_vertexData = ::operator new(m_vertexSize * Vertex_Count);
	std::memcpy(m_vertexData, p_rhs.m_vertexData, m_vertexSize * Vertex_Count);
	
	safeRelease(m_vertexBuffer);
	
	return (*this);
}


Quad2D::~Quad2D()
{
	// Unregister D3D resource
	D3DResourceRegistry::unregisterResource(this);
	Renderer::addToDeathRow(m_vertexBuffer);
	
	if(m_vertexData != 0)
	{
		::operator delete(m_vertexData);
	}
}


void Quad2D::setColor(const ColorRGBA& p_color)
{
	TT_ASSERT(m_vertexType & VertexBuffer::Property_Diffuse);

	u8* color = static_cast<u8*>(m_vertexData) + sizeof(math::Vector3);

	for(s32 i=0; i < Vertex_Count; ++i)
	{
		*reinterpret_cast<D3DCOLOR*>(color) = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);

		color += m_vertexSize;
	}
}


void Quad2D::setColor(const ColorRGB& p_color)
{
	TT_ASSERT(m_vertexType & VertexBuffer::Property_Diffuse);

	u8* color = static_cast<u8*>(m_vertexData) + sizeof(math::Vector3);

	for(s32 i=0; i < Vertex_Count; ++i)
	{
		// NOTE: DirectX stores color channels in different order (BGRA)
		*reinterpret_cast<ColorRGB*>(color) = ColorRGB(p_color.b, p_color.g, p_color.r);

		color += m_vertexSize;
	}
}


void Quad2D::setColor(const ColorRGBA& p_color, Vertex p_vertex)
{
	TT_ASSERT(m_vertexType & VertexBuffer::Property_Diffuse);

	u32 offset(sizeof(math::Vector3));

	// Set offset to right vertex
	offset += m_vertexSize * p_vertex;

	u8* color = static_cast<u8*>(m_vertexData) + offset;
	*reinterpret_cast<D3DCOLOR*>(color) = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
}


void Quad2D::setColor(const ColorRGB& p_color, Vertex p_vertex)
{
	TT_ASSERT(m_vertexType & VertexBuffer::Property_Diffuse);

	u32 offset(sizeof(math::Vector3));

	// Set offset to right vertex
	offset += m_vertexSize * p_vertex;

	u8* color = static_cast<u8*>(m_vertexData) + offset;

	// NOTE: DirectX stores color channels in different order (BGRA)
	*reinterpret_cast<ColorRGB*>(color) = ColorRGB(p_color.b, p_color.g, p_color.r);
}


void Quad2D::setAlpha(u8 p_alpha)
{
	TT_ASSERT(m_vertexType & VertexBuffer::Property_Diffuse);

	u8* alpha = static_cast<u8*>(m_vertexData) + sizeof(math::Vector3) + sizeof(ColorRGB);

	for(s32 i=0; i < Vertex_Count; ++i)
	{
		*alpha = p_alpha;

		alpha += m_vertexSize;
	}
}


void Quad2D::setTexcoord(Quad2D::Vertex p_vertex, const math::Vector2& p_texcoord)
{
	TT_ASSERT(m_vertexType & VertexBuffer::Property_Texture0);
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index");

	if(m_vertexType == VertexBuffer::Property_Texture0)
	{
		VtxTex* vtx = static_cast<VtxTex*>(m_vertexData);

		vtx[p_vertex].tex = p_texcoord;
	}
	else if(m_vertexType == (VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0))
	{
		VtxColTex* vtx = static_cast<VtxColTex*>(m_vertexData);

		vtx[p_vertex].tex = p_texcoord;
	}
}


void Quad2D::updateTexcoords(const TexturePtr& p_texture)
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
	Renderer::getInstance()->checkFromRenderThread();
	
	if (m_vertexBuffer == 0)
	{
		// Must return after reset, because deviceReset() will call update() if successful
		deviceReset();
		return;
	}
	
	void* vertices = 0;
	DWORD lockFlags = 0;
	lockFlags |= D3DLOCK_DISCARD;
	
	if (checkD3DSucceeded(m_vertexBuffer->Lock(0, m_vertexSize * Vertex_Count, &vertices, lockFlags)))
	{
		// Copy vertex data into D3D buffer
		memcpy(vertices, m_vertexData, m_vertexSize * Vertex_Count);
		
		checkD3DSucceeded(m_vertexBuffer->Unlock());
	}
}


void Quad2D::render(bool p_useFixedPipe) const
{
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	if (m_vertexBuffer == 0)
	{
		TT_PANIC("Vertex Buffer does not exist, call update() first.");
		return;
	}
	
	// Set up vertex type and transform
	Renderer::getInstance()->setVertexType(m_vertexType);
	MatrixStack::getInstance()->updateWorldMatrix();
	
	// Setup the vertex buffer
	if (checkD3DSucceeded(device->SetStreamSource(0, m_vertexBuffer, 0, m_vertexSize)) == false)
	{
		return;
	}
	
	// Render the quad
	checkD3DSucceeded( device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
	
	debug::DebugStats::addToQuadsRendered(1);
}


void Quad2D::deviceLost()
{
	safeRelease(m_vertexBuffer);
}


void Quad2D::deviceReset()
{
	safeRelease(m_vertexBuffer);
	
	IDirect3DDevice9* device = getRenderDevice(true);
	if (device == nullptr)
	{
		return;
	}
	
	u32 vertexFormat(D3DFVF_XYZ);
	
	if(m_vertexType & VertexBuffer::Property_Diffuse)
	{
		vertexFormat |= D3DFVF_DIFFUSE;
	}
	if(m_vertexType & VertexBuffer::Property_Texture0)
	{
		vertexFormat |= D3DFVF_TEX1;
	}
	
	DWORD usageFlags = D3DUSAGE_WRITEONLY;
	usageFlags |= D3DUSAGE_DYNAMIC;
	if (checkD3DSucceeded( device->CreateVertexBuffer(Vertex_Count * m_vertexSize,
	                                           usageFlags,
	                                           vertexFormat,
	                                           D3DPOOL_DEFAULT,
	                                           &m_vertexBuffer,
	                                           0)))
	{
		update();
	}
}


//////////////////////////
// Privates


void Quad2D::initVertexData(const ColorRGBA& p_color)
{
	// Should only be called once!
	TT_ASSERT(m_vertexData == 0);

	// Allocate space for 4 vertices
	m_vertexData = ::operator new(m_vertexSize * Vertex_Count);
	
	// Construct & write vertex based on vertex format
	if(m_vertexType == VertexBuffer::Property_Diffuse)
	{
		VtxCol* vtx = static_cast<VtxCol*>(m_vertexData);

		vtx[Vertex_BottomLeft ].pos = math::Vector3(-quadSize, -quadSize, 0);
		vtx[Vertex_BottomRight].pos = math::Vector3( quadSize, -quadSize, 0);
		vtx[Vertex_TopLeft    ].pos = math::Vector3(-quadSize,  quadSize, 0);
		vtx[Vertex_TopRight   ].pos = math::Vector3( quadSize,  quadSize, 0);

		vtx[Vertex_BottomLeft ].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
		vtx[Vertex_BottomRight].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
		vtx[Vertex_TopLeft    ].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
		vtx[Vertex_TopRight   ].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
	}
	else if(m_vertexType == VertexBuffer::Property_Texture0)
	{
		VtxTex* vtx = static_cast<VtxTex*>(m_vertexData);

		vtx[Vertex_BottomLeft ].pos = math::Vector3(-quadSize, -quadSize, 0);
		vtx[Vertex_BottomRight].pos = math::Vector3( quadSize, -quadSize, 0);
		vtx[Vertex_TopLeft    ].pos = math::Vector3(-quadSize,  quadSize, 0);
		vtx[Vertex_TopRight   ].pos = math::Vector3( quadSize,  quadSize, 0);

		vtx[Vertex_BottomLeft ].tex = math::Vector2(0,1);
		vtx[Vertex_BottomRight].tex = math::Vector2(1,1);
		vtx[Vertex_TopLeft    ].tex = math::Vector2(0,0);
		vtx[Vertex_TopRight   ].tex = math::Vector2(1,0);
	}
	else if(m_vertexType == (VertexBuffer::Property_Diffuse|VertexBuffer::Property_Texture0))
	{
		VtxColTex* vtx = static_cast<VtxColTex*>(m_vertexData);

		vtx[Vertex_BottomLeft ].pos = math::Vector3(-quadSize, -quadSize, 0);
		vtx[Vertex_BottomRight].pos = math::Vector3( quadSize, -quadSize, 0);
		vtx[Vertex_TopLeft    ].pos = math::Vector3(-quadSize,  quadSize, 0);
		vtx[Vertex_TopRight   ].pos = math::Vector3( quadSize,  quadSize, 0);

		vtx[Vertex_BottomLeft ].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
		vtx[Vertex_BottomRight].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
		vtx[Vertex_TopLeft    ].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);
		vtx[Vertex_TopRight   ].col = D3DCOLOR_ARGB(p_color.a, p_color.r, p_color.g, p_color.b);

		vtx[Vertex_BottomLeft ].tex = math::Vector2(0,1);
		vtx[Vertex_BottomRight].tex = math::Vector2(1,1);
		vtx[Vertex_TopLeft    ].tex = math::Vector2(0,0);
		vtx[Vertex_TopRight   ].tex = math::Vector2(1,0);
	}
}


// Namespace end
}
}
}
