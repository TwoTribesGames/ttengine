#include <tt/engine/renderer/ArcQuad.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>


namespace tt {
namespace engine {
namespace renderer {

static const real topRightAngle    = math::pi * 0.25f;
static const real topLeftAngle     = math::pi * 0.75f;
static const real bottomLeftAngle  = math::pi * 1.25f;
static const real bottomRightAngle = math::pi * 1.75f;


ArcQuad::ArcQuad(BatchFlag p_batchflag, const ColorRGBA& p_color, const TexturePtr& p_texture)
:
m_batchFlag(p_batchflag),
m_triangleData(),
// vertexcount = Vertex_Count (4 vertices for quad) + Arc_Points (3 vertices: begin, end and center)
// maxBufferCapacity = vertexcount - 2: converting vertexcount in fan mode to trianglecount
m_triangleBuffer((Vertex_Count + Arc_Points) - 2, 1, p_texture, getTriangleBatchflag(m_batchFlag)),
m_begin(0.0f),
m_end(0.0f),
m_useBegin(false),
m_useEnd(false),
m_mode(RenderMode_Full),
m_vertexCount(4)
{
	// Must be colored, textured or both
	TT_ASSERT(m_batchFlag == BatchFlag_UseVertexColor || p_texture != 0);
	
	if(m_batchFlag == BatchFlag_UseVertexColor)
	{
		m_color[0] = p_color;
		m_color[1] = p_color;
		m_color[2] = p_color;
		m_color[3] = p_color;
		m_color[4] = p_color;
		m_color[5] = p_color;
		m_color[6] = p_color;
	}
	if (p_texture != 0)
	{
		m_texcoords[Vertex_TopLeft][0]     = 0;
		m_texcoords[Vertex_TopLeft][1]     = 0;
		m_texcoords[Vertex_TopRight][0]    = 1;
		m_texcoords[Vertex_TopRight][1]    = 0;
		m_texcoords[Vertex_BottomRight][0] = 1;
		m_texcoords[Vertex_BottomRight][1] = 1;
		m_texcoords[Vertex_BottomLeft][0]  = 0;
		m_texcoords[Vertex_BottomLeft][1]  = 1;
	}
	
	m_coords[Vertex_TopLeft][0]     = -quadSize;
	m_coords[Vertex_TopLeft][1]     =  quadSize;
	m_coords[Vertex_TopRight][0]    =  quadSize;
	m_coords[Vertex_TopRight][1]    =  quadSize;
	m_coords[Vertex_BottomRight][0] =  quadSize;
	m_coords[Vertex_BottomRight][1] = -quadSize;
	m_coords[Vertex_BottomLeft][0]  = -quadSize;
	m_coords[Vertex_BottomLeft][1]  = -quadSize;
	m_coords[Arc_Center][0]  = 0.0f;
	m_coords[Arc_Center][1]  = 0.0f;
	
	// Create the vertex data
	setVertexData();
}


void ArcQuad::setColor(const ColorRGBA& p_color)
{
	TT_ASSERT(m_batchFlag == BatchFlag_UseVertexColor);
	
	for (int i = 0; i < (Vertex_Count + Arc_Points); ++i)
	{
		m_color[i] = p_color;
	}
}


void ArcQuad::setColor(const ColorRGB& p_color)
{
	TT_ASSERT(m_batchFlag == BatchFlag_UseVertexColor);
	
	for (int i = 0; i < (Vertex_Count + Arc_Points); ++i)
	{
		m_color[i] = p_color;
	}
}


void ArcQuad::setColor(const ColorRGBA& p_color, Vertex p_vertex)
{
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index");
	TT_ASSERT(m_batchFlag == BatchFlag_UseVertexColor);
	
	m_color[p_vertex] = p_color;
	
	updateArcColor();
}


void ArcQuad::setColor(const ColorRGB& p_color, Vertex p_vertex)
{
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index");
	TT_ASSERT(m_batchFlag == BatchFlag_UseVertexColor);
	
	m_color[p_vertex] = p_color;
	
	updateArcColor();
}


void ArcQuad::setAlpha(u8 p_alpha)
{
	TT_ASSERT(m_batchFlag == BatchFlag_UseVertexColor);
	
	for (int i = 0; i < (Vertex_Count + Arc_Points); ++i)
	{
		m_color[i].a = p_alpha;
	}
}


void ArcQuad::setTexcoord(ArcQuad::Vertex p_vertex, const math::Vector2& p_texcoord)
{
	TT_ASSERTMSG(p_vertex >= 0 && p_vertex < Vertex_Count, "Invalid vertex index");
	TT_NULL_ASSERT(m_triangleBuffer.getTexture());
	
	m_texcoords[p_vertex][0] = p_texcoord.x;
	m_texcoords[p_vertex][1] = p_texcoord.y;
	
	updateArcTextureCoords();
}


void ArcQuad::updateTexcoords()
{
	TT_NULL_ASSERT(m_triangleBuffer.getTexture());
	
	math::Vector2 maxTexCoord(1.0f, 1.0f);
	
	setTexcoord(ArcQuad::Vertex_TopLeft,     math::Vector2::zero);
	setTexcoord(ArcQuad::Vertex_TopRight,    math::Vector2(maxTexCoord.x, 0));
	setTexcoord(ArcQuad::Vertex_BottomLeft,  math::Vector2(0, maxTexCoord.y));
	setTexcoord(ArcQuad::Vertex_BottomRight, maxTexCoord);
	
	updateArcTextureCoords();
}


void ArcQuad::updateTexcoords(s32 p_frameWidth, s32 p_frameHeight)
{
	TT_NULL_ASSERT(m_triangleBuffer.getTexture());
	TT_ASSERT(p_frameWidth != 0 && p_frameHeight != 0);
	
	// Initialize new texture coordinates
	math::Vector2 maxTexCoord(real(p_frameWidth)  / m_triangleBuffer.getTexture()->getWidth(),
	                          real(p_frameHeight) / m_triangleBuffer.getTexture()->getHeight());
	
	setTexcoord(ArcQuad::Vertex_TopLeft,     math::Vector2::zero);
	setTexcoord(ArcQuad::Vertex_TopRight,    math::Vector2(maxTexCoord.x, 0));
	setTexcoord(ArcQuad::Vertex_BottomLeft,  math::Vector2(0, maxTexCoord.y));
	setTexcoord(ArcQuad::Vertex_BottomRight, maxTexCoord);
	
	updateArcTextureCoords();
}


void ArcQuad::setAngle(real p_angle, real p_area)
{
	TT_ASSERT(p_area >= 0.0f && p_area <= math::twoPi);
	m_begin = sanitizeAngle(p_angle);
	m_end   = sanitizeAngle(p_angle + p_area);
	
	m_useBegin = false;
	m_useEnd   = false;
	
	// calculate new mode
	if (p_area <= 0.0f)
	{
		m_mode = RenderMode_None;
	}
	else if (p_area >= math::twoPi)
	{
		m_mode = RenderMode_Full;
	}
	else
	{
		m_mode = RenderMode_Partial;
		m_useBegin = getVertex(m_begin) == Vertex_Count;
		m_useEnd   = getVertex(m_end)   == Vertex_Count;
		
		m_vertexCount = 0;
		real angle = -0.1f;
		for (;;)
		{
			u8 next = getNextVertex(angle);
			if (next == 6)
			{
				break;
			}
			m_vertices[m_vertexCount] = next;
			++m_vertexCount;
		}
		
		// vertices have now been ordered from 0 to 360 degrees
		// now strip away the vertices that won't be used
		
		u8 first = static_cast<u8>(m_useBegin ? 4 : getVertex(m_begin));
		u8 last = static_cast<u8>(m_useEnd ? 5 : getVertex(m_end));
		
		// now rotate the vertex list until the first vertex is the first element
		while (m_vertices[0] != first)
		{
			// rotate towards beginning
			u8 begin = m_vertices[0];
			for (int i = 0; i < m_vertexCount - 1; ++i)
			{
				m_vertices[i] = m_vertices[i + 1];
			}
			m_vertices[m_vertexCount - 1] = begin;
		}
		for (int i = 0; i < m_vertexCount; ++i)
		{
			if (m_vertices[i] == last)
			{
				m_vertexCount = i + 1;
				break;
			}
		}
	}
	
	updateArcColor();
	updateArcTextureCoords();
	updateArcCoords();
}


void ArcQuad::update()
{
	setVertexData();
	
	m_triangleBuffer.applyChanges();
}


void ArcQuad::render() const
{
	if (m_mode == RenderMode_None)
	{
		return;
	}
	
	m_triangleBuffer.render();
}


//////////////////////////
// Privates


void ArcQuad::updateArcColor()
{
	if (m_mode != RenderMode_Partial)
	{
		return;
	}
	
	m_color[Arc_Center].r = u8((m_color[0].r + m_color[1].r + m_color[2].r + m_color[3].r) >> 2);
	m_color[Arc_Center].g = u8((m_color[0].g + m_color[1].g + m_color[2].g + m_color[3].g) >> 2);
	m_color[Arc_Center].b = u8((m_color[0].b + m_color[1].b + m_color[2].b + m_color[3].b) >> 2);
	m_color[Arc_Center].a = u8((m_color[0].a + m_color[1].a + m_color[2].a + m_color[3].a) >> 2);
	
	if (m_useBegin) m_color[Arc_Begin] = getAngleColor(m_begin);
	if (m_useEnd)   m_color[Arc_End]   = getAngleColor(m_end);
}


void ArcQuad::updateArcTextureCoords()
{
	if (m_mode != RenderMode_Partial)
	{
		return;
	}
	
	m_texcoords[Arc_Center][0] = (m_texcoords[0][0] + m_texcoords[1][0] + m_texcoords[2][0] + m_texcoords[3][0]) / 4.0f;
	m_texcoords[Arc_Center][1] = (m_texcoords[0][1] + m_texcoords[1][1] + m_texcoords[2][1] + m_texcoords[3][1]) / 4.0f;
	
	if (m_useBegin) getAngleTexCoords(m_begin, m_texcoords[Arc_Begin][0], m_texcoords[Arc_Begin][1]);
	if (m_useEnd)   getAngleTexCoords(m_end,   m_texcoords[Arc_End][0],   m_texcoords[Arc_End][1]);
}


void ArcQuad::updateArcCoords()
{
	if (m_mode != RenderMode_Partial)
	{
		return;
	}
	
	if (m_useBegin) getAngleCoords(m_begin, m_coords[Arc_Begin][0], m_coords[Arc_Begin][1]);
	if (m_useEnd)   getAngleCoords(m_end,   m_coords[Arc_End][0],   m_coords[Arc_End][1]);
}


ColorRGBA ArcQuad::getAngleColor(real p_angle)
{
	switch (Vertex vtx = getVertex(p_angle))
	{
	case Vertex_Count:
		break;
		
	default:
		return m_color[vtx];
	}
	
	Vertex min(Vertex_BottomLeft);
	Vertex max(Vertex_BottomLeft);
	real ratio(0.0f);
	
	getMinMaxRatio(p_angle, min, max, ratio);
	u32 rat = static_cast<u32>(ratio * 256);
	
	// calculate average between min and max with ratio (max * ratio) + (min * (1 - ratio)
	u32 r = m_color[max].r * rat;
	u32 g = m_color[max].g * rat;
	u32 b = m_color[max].b * rat;
	u32 a = m_color[max].a * rat;
	
	rat = 256 - rat;
	r += m_color[min].r * rat;
	g += m_color[min].g * rat;
	b += m_color[min].b * rat;
	a += m_color[min].a * rat;
	
	return ColorRGBA(u8(r / 256), u8(g / 256), u8(b / 256), u8(a / 256));
}


void ArcQuad::getAngleTexCoords(real p_angle, real& p_sOUT, real& p_tOUT)
{
	switch (Vertex vtx = getVertex(p_angle))
	{
	case Vertex_Count:
		break;
		
	default:
		p_sOUT = m_texcoords[vtx][0];
		p_tOUT = m_texcoords[vtx][1];
		return;
	}
	
	Vertex min(Vertex_BottomLeft);
	Vertex max(Vertex_BottomLeft);
	real ratio(0.0f);
	
	getMinMaxRatio(p_angle, min, max, ratio);
	
	// calculate average between min and max with ratio (min * ratio) + (max * (1 - ratio)
	p_sOUT = m_texcoords[max][0] * ratio;
	p_tOUT = m_texcoords[max][1] * ratio;
	
	ratio = 1.0f - ratio;
	p_sOUT += m_texcoords[min][0] * ratio;
	p_tOUT += m_texcoords[min][1] * ratio;
}


void ArcQuad::getAngleCoords(real p_angle, real& p_xOUT, real& p_yOUT)
{
	switch (Vertex vtx = getVertex(p_angle))
	{
	case Vertex_Count:
		break;
		
	default:
		p_xOUT = m_coords[vtx][0];
		p_yOUT = m_coords[vtx][1];
		return;
	}
	
	Vertex min(Vertex_BottomLeft);
	Vertex max(Vertex_BottomLeft);
	real ratio(0.0f);
	
	getMinMaxRatio(p_angle, min, max, ratio);
	
	// calculate average between min and max with ratio (min * ratio) + (max * (1 - ratio)
	p_xOUT = m_coords[max][0] * ratio;
	p_yOUT = m_coords[max][1] * ratio;
	
	ratio = 1.0f - ratio;
	
	p_xOUT += m_coords[min][0] * ratio;
	p_yOUT += m_coords[min][1] * ratio;
}


real ArcQuad::getRatio(real p_angle) const
{
	// angle lies between +0.25 pi and -0.25 pi
	
	// consider the following:
	// 
	// max           min
	//  ---------------
	//  \      |      /
	//   \     |     /
	//    \    |    /
	//     \   |   /
	//      \  |  /
	//       \ | /
	//        \|/
	//         v
	//      center
	//
	// center is the center of the quad, min and max are two adjacent corners
	// p_angle is the angle of the line we need to draw. When p_angle is negative
	// the line will lie towards min, otherwise towards max. p_angle describes the angle
	// between the centerline and the line to be drawn in radians.
	// the length of the center line is 0.5, the length of the line between min and max is 1.0
	// since tan(p_angle) = line / centerline,  line = tan(p_angle) * centerline.
	// this will give the distance from the center of the min-max line to the end point of the
	// desired line. A positive result will lie closer to max, a negative closer to min.
	// The range of the result is [-1 - 1]
	// Thus the final equation is ratio = tan(p_angle) * 0.5 + 0.5
	return (tt::math::tan(p_angle) * 0.5f) + 0.5f;
}


real ArcQuad::sanitizeAngle(real p_angle) const
{
	// sanitize input
	while (p_angle >= tt::math::twoPi)
	{
		p_angle -= tt::math::twoPi;
	}
	while (p_angle < 0.0f)
	{
		p_angle += tt::math::twoPi;
	}
	return p_angle;
}


real ArcQuad::sanitizeArea(real p_area) const
{
	// sanitize input
	while (p_area > tt::math::twoPi)
	{
		p_area -= tt::math::twoPi;
	}
	while (p_area < 0.0f)
	{
		p_area += tt::math::twoPi;
	}
	return p_area;
}


void ArcQuad::getMinMaxRatio(real p_angle, Vertex& p_minOUT, Vertex& p_maxOUT, real& p_ratioOUT) const
{
	if (p_angle < topRightAngle)
	{
		// calculate between topright and bottomright
		p_minOUT = Vertex_BottomRight;
		p_maxOUT = Vertex_TopRight;
		p_ratioOUT = getRatio(p_angle);
	}
	else if (p_angle < topLeftAngle)
	{
		// calculate between topleft and topright
		p_minOUT = Vertex_TopRight;
		p_maxOUT = Vertex_TopLeft;
		p_ratioOUT = getRatio(p_angle - tt::math::halfPi);
	}
	else if (p_angle < bottomLeftAngle)
	{
		// calculate between bottomleft and topleft
		p_minOUT = Vertex_TopLeft;
		p_maxOUT = Vertex_BottomLeft;
		p_ratioOUT = getRatio(p_angle - tt::math::pi);
	}
	else if (p_angle < bottomRightAngle)
	{
		// calculate between bottomright and bottomleft
		p_minOUT = Vertex_BottomLeft;
		p_maxOUT = Vertex_BottomRight;
		p_ratioOUT = getRatio(p_angle - (tt::math::halfPi + tt::math::pi));
	}
	else
	{
		// calculate between topright and bottomright
		p_minOUT = Vertex_BottomRight;
		p_maxOUT = Vertex_TopRight;
		p_ratioOUT = getRatio(p_angle - tt::math::twoPi);
	}
}


ArcQuad::Vertex ArcQuad::getVertex(real p_angle) const
{
	// get accompanying vertex by approximation
	if (p_angle == topRightAngle)    return Vertex_TopRight;
	if (p_angle == topLeftAngle)     return Vertex_TopLeft;
	if (p_angle == bottomLeftAngle)  return Vertex_BottomLeft;
	if (p_angle == bottomRightAngle) return Vertex_BottomRight;
	return Vertex_Count;
}


u8 ArcQuad::getNextVertex(real& p_angleOUT) const
{
	// first, try all corners
	real angle = math::twoPi;
	u8 ret = 6;
	
	if (topRightAngle > p_angleOUT && topRightAngle < angle)
	{
		angle = topRightAngle;
		ret = Vertex_TopRight;
	}
	if (topLeftAngle > p_angleOUT && topLeftAngle < angle)
	{
		angle = topLeftAngle;
		ret = Vertex_TopLeft;
	}
	if (bottomLeftAngle > p_angleOUT && bottomLeftAngle < angle)
	{
		angle = bottomLeftAngle;
		ret = Vertex_BottomLeft;
	}
	if (bottomRightAngle > p_angleOUT && bottomRightAngle < angle)
	{
		angle = bottomRightAngle;
		ret = Vertex_BottomRight;
	}
	if (m_useBegin && m_begin > p_angleOUT && m_begin <= angle)
	{
		angle = m_begin;
		ret = Arc_Begin;
	}
	if (m_useEnd && m_end > p_angleOUT && m_end <= angle)
	{
		angle = m_end;
		ret = Arc_End;
	}
	p_angleOUT = angle;
	return ret;
}


void ArcQuad::setVertexData()
{
	if (m_mode != RenderMode_Partial)
	{
		// not rendering partial so just render a quad
		
		// 2 triangles in a quad
		m_triangleData.resize(2);
		for(s32 i = 0; i < 2; ++i)
		{
			// set the 3 vertices of the triangle
			for(s32 v = 0; v < 3; ++v)
			{
				BufferVtx* bufferVtx = 0;
				BatchTriangle& btuv(m_triangleData[static_cast<BatchTriangleCollection::size_type>(i)]);
				switch(v)
				{
				case 0: bufferVtx = &btuv.a; break;
				// Switching the second triangle's winding (vertices are in triangle strip order)
				case 1: bufferVtx = (i == 0) ? &btuv.b : &btuv.c; break;
				case 2: bufferVtx = (i == 0) ? &btuv.c : &btuv.b; break;
				default: TT_PANIC("Invalid vertex nr"); return;
				}
				
				bufferVtx->setPosition(m_coords[i + v][0], m_coords[i + v][1], 0);
				
				if(m_batchFlag == BatchFlag_UseVertexColor)
				{
					bufferVtx->setColor(m_color[i + v].r, m_color[i + v].g, 
					                    m_color[i + v].b, m_color[i + v].a);
				}
				if (m_triangleBuffer.getTexture() != 0)
				{
					bufferVtx->setTexCoord(m_texcoords[i + v][0], m_texcoords[i + v][1]);
				}
			}
		}
	}
	else
	{
		// to convert the vertexcount to trianglecount in fan mode there is 2 less triangles then vertices
		// m_vertexCount holds the number of vertices without the center 
		const BatchTriangleCollection::size_type triangleCount =
			static_cast<BatchTriangleCollection::size_type>(m_vertexCount - 1);
		m_triangleData.resize(triangleCount);
		for (BatchTriangleCollection::size_type i = 0; i < triangleCount; ++i)
		{
			// set the 3 vertices of the triangle
			for(s32 v = 0; v < 3; ++v)
			{
				BufferVtxUV<1>* bufferVtx(0);
				u8 idx = 0;
				switch(v)
				{
				case 0: // fan: always start the triangle with the first vertex (the center vertex)
					bufferVtx = &m_triangleData[i].a;
					idx = Arc_Center;
					break;
				case 1: 
					bufferVtx = &m_triangleData[i].b;
					idx = m_vertices[m_vertexCount - (i + 1)];
					break;
				case 2: 
					bufferVtx = &m_triangleData[i].c;
					idx = m_vertices[m_vertexCount - (i + 2)];
					break;
				default: TT_PANIC("Invalid vertex nr"); return;
				}
				
				bufferVtx->setPosition(m_coords[idx][0], m_coords[idx][1], 0);
				
				if(m_batchFlag == BatchFlag_UseVertexColor)
				{
					bufferVtx->setColor(m_color[idx].r, m_color[idx].g,
					                    m_color[idx].b, m_color[idx].a);
				}
				if (m_triangleBuffer.getTexture() != 0)
				{
					bufferVtx->setTexCoord(m_texcoords[idx][0], m_texcoords[idx][1]);
				}
			}
		}
	}
	// set the new data in the trianglebuffer
	m_triangleBuffer.setCollection(m_triangleData);
}


tt::engine::renderer::BatchFlagTriangle ArcQuad::getTriangleBatchflag( BatchFlag p_arcQuadBatchFlag )
{
	switch(p_arcQuadBatchFlag)
	{
	case BatchFlag_None:           return BatchFlagTriangle_None;
	case BatchFlag_UseVertexColor: return BatchFlagTriangle_UseVertexColor;
	
	default: TT_PANIC("Unsupported Batchflag"); return BatchFlagTriangle_None;
	}
}
// Namespace end
}
}
}
