#if !defined(INC_TT_ENGINE_RENDERER_ARCQUAD_H)
#define INC_TT_ENGINE_RENDERER_ARCQUAD_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/TriangleBuffer.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector2.h>
#include <tt/engine/fwd.h>


namespace tt {
namespace engine {
namespace renderer {

/*! \brief Renders a Quad with the option of specifying and angle and area of the quad to actualy render. 
           If the area equals 2pi then a normal quad will be renderd if it is zero nothing will be renderd
           In the following examples the left ArcQuad will be rendered with 1 triangle 
           and the right ArcQuad would be rendered with 4 triangles.

Example1:           | Example2:
  center            |         +------------+
    +---+ angle     |         |XXXXXXXXXXXX|
     \%%|           |    area |XXXXXXXXXXXX|
      \%|area       |         |XXXXXX.-----+ angle
       \|           |         |XXXXXXX|
                    |         |XXXXXXXX|
                    |         +---------+

           The center vertex is always used in all triangles.
*/
class ArcQuad
{
public:
	enum Vertex
	{
		// Don't change this order!
		Vertex_BottomLeft,
		Vertex_TopLeft,
		Vertex_BottomRight,
		Vertex_TopRight,
		
		Vertex_Count
	};
	enum BatchFlag
	{
		BatchFlag_None,
		BatchFlag_UseVertexColor
	};
	// We are using a fixed size 8x8 quad to make better use of the available precision
	static const s8 quadSize = 4;
	
	/*! \brief Constructor for creating an arc quad
	    \param p_batchflag  Vertex property
	    \param p_color      [Optional] Initial quad color 
	    \param p_texture    [Optional] Texture to render */
	explicit ArcQuad(BatchFlag         p_batchflag,
	                 const ColorRGBA&  p_color = ColorRGB::white,
	                 const TexturePtr& p_texture = TexturePtr());
	
	/*! \brief Change the color of the arc quad
	    \param p_color New arc quad color */
	void setColor(const ColorRGBA& p_color);
	
	/*! \brief Change the color of the arc quad
	    \param p_color New arc quad color (alpha is not changed) */
	void setColor(const ColorRGB& p_color);
	
	/*! \brief Change the color of an arc quad corner
	    \param p_color New color */
	void setColor(const ColorRGBA& p_color, Vertex p_vertex);
	
	/*! \brief Change the color of an arc quad corner
	    \param p_color New arc quad color (alpha is not changed) */
	void setColor(const ColorRGB& p_color, Vertex p_vertex);
	
	/*! \brief Change the alpha of the arc quad
	    \param p_color New alpha value (0 - 255) */
	void setAlpha(u8 p_alpha);
	
	/*! \brief Change the texture coordinates of a specific vertex
	    \param p_vertex   Vertex to change
	    \param p_texcoord New texture coordinates */
	void setTexcoord(Vertex p_vertex, const math::Vector2& p_texcoord);
	
	/*! \brief Update the texture coordinates based on frame size */
	void updateTexcoords();
	
	/*! \brief Update the texture coordinates based on frame size
	    \param p_frameWidth  Width of a single frame in texels
	    \param p_frameHeight Height of a single frame in texels */
	void updateTexcoords(s32 p_frameWidth, s32 p_frameHeight);
	
	/*! \brief Get the current texture to be used rendering the ArcQuad */
	inline const TexturePtr& getTexture() { return m_triangleBuffer.getTexture(); }
	
	/*! \brief Sets the arc's angle and area (in radians)
	    \param p_angle The starting angle in radians
	    \param p_area  The area covered in radians (between 0 and 2 Pi)*/
	void setAngle(real p_angle, real p_area);
	
	/*! \brief Update the arc quad (must be called to make changes to properties take effect) */
	void update();
	
	/*! \brief Render this arc quad on the screen */
	void render() const;
	
private:
	void updateArcColor();
	void updateArcTextureCoords();
	void updateArcCoords();
	
	ColorRGBA getAngleColor(real p_angle);
	void getAngleTexCoords(real p_angle, real& p_sOUT, real& p_tOUT);
	void getAngleCoords(real p_angle, real& p_xOUT, real& p_yOUT);
	
	void getMinMaxRatio(real p_angle, Vertex& p_minOUT, Vertex& p_maxOUT, real& p_ratioOUT) const;
	real getRatio(real p_angle) const;
	real sanitizeAngle(real p_angle) const;
	real sanitizeArea(real p_area) const;
	Vertex getVertex(real p_angle) const;
	u8 getNextVertex(real& p_angleOUT) const;
	
	void setVertexData();
	
	static BatchFlagTriangle getTriangleBatchflag(BatchFlag p_arcQuadBatchFlag);
	
	// No copying
	ArcQuad(const ArcQuad&);
	ArcQuad& operator=(const ArcQuad&);
	
	
	enum
	{
		Arc_Begin = Vertex_Count,
		Arc_End,
		Arc_Center,
		
		Arc_Points = 3
	};
	
	enum RenderMode
	{
		RenderMode_Full,     // Render as single quad
		RenderMode_Partial,  // Render as a partial quad
		RenderMode_None      // Do not render at all
	};
	
	// Vertex Data
	BatchFlag               m_batchFlag;
	BatchTriangleCollection m_triangleData;
	TriangleBuffer          m_triangleBuffer;
	
	real m_begin; // begin angle
	real m_end;   // end angle
	
	bool m_useBegin;
	bool m_useEnd;
	
	ColorRGBA m_color[Vertex_Count + Arc_Points];
	real      m_coords[Vertex_Count + Arc_Points][2];
	real      m_texcoords[Vertex_Count + Arc_Points][2];
	
	RenderMode m_mode;
	
	u8  m_vertices[6];
	s32 m_vertexCount;
};

// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_RENDERER_ARCQUAD_H)
