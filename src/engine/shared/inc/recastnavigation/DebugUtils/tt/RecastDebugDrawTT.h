#if !defined(RECASTNAVIGATION_DEBUGUTILS_TT_RECASTDEBUGDRAWTT_H)
#define RECASTNAVIGATION_DEBUGUTILS_TT_RECASTDEBUGDRAWTT_H

#include <recastnavigation/DebugUtils/DebugDraw.h>

#include <tt/engine/renderer/TrianglestripBuffer.h>


/// Debug draw implementation.
class RecastDebugDrawTT : public duDebugDraw
{
public:
	RecastDebugDrawTT();
	virtual ~RecastDebugDrawTT();
	
	virtual void depthMask(bool state);
	virtual void texture(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
	
private:
	const bool                                  m_zBufferWasEnabled;
	const bool                                  m_cullingWasEnabled;
	tt::engine::renderer::TrianglestripBuffer   m_vertexBuffer;
	tt::engine::renderer::TrianglestripVertices m_vertexCollection;
	duDebugDrawPrimitives                       m_activePrimitiveType;
	s32                                         m_vtxIdxInPrim; // Vertex index within the primitive type. (only valid and used for quad.)
	
	RecastDebugDrawTT(const RecastDebugDrawTT& p_rhs);                  // No copy
	const RecastDebugDrawTT& operator=(const RecastDebugDrawTT& p_rhs); // No assigment
};


#endif // RECASTNAVIGATION_DEBUGUTILS_TT_RECASTDEBUGDRAWTT_H

