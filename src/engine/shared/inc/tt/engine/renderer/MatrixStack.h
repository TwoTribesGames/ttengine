#if !defined(INC_TT_ENGINE_RENDERER_MATRIXSTACK_H)
#define INC_TT_ENGINE_RENDERER_MATRIXSTACK_H


#include <vector>

#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {


class MatrixStack
{
public:
	enum Mode
	{
		Mode_Projection     = 0, /*!< Projection Matrix */
		Mode_Position       = 1, /*!< Model-View Matrix */
		Mode_Texture        = 2, /*!< Texture Matrix */

		Mode_Count
	};
	
	
	static bool createInstance();
	inline static MatrixStack* getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	inline static bool hasInstance() { return ms_instance != 0; }
	static void destroyInstance();

	inline void setMode(Mode p_mode)
	{
		TT_ASSERTMSG(p_mode < Mode_Count, "Invalid Matrix Mode (Value = %d)", p_mode);
		m_matrixMode = p_mode;
	}
	
	void setIdentity();
	void load44(const math::Matrix44& p_matrix);
	void multiply44(const math::Matrix44& p_matrix);
	void push();
	void pop();
	
	inline void translate(const math::Vector3& p_translation)
	{
		multiply44(math::Matrix44::getTranslation(p_translation.x, p_translation.y, p_translation.z));
	}
	
	inline void rotateX(real p_angleRad)
	{
		multiply44(math::Matrix44::getRotationX(p_angleRad));
	}
	
	inline void rotateY(real p_angleRad)
	{
		multiply44(math::Matrix44::getRotationY(p_angleRad));
	}
	
	inline void rotateZ(real p_angleRad)
	{
		multiply44(math::Matrix44::getRotationZ(p_angleRad));
	}
	
	inline void scale(const math::Vector3& p_scale)
	{
		multiply44(math::Matrix44::getScale(p_scale.x, p_scale.y, p_scale.z));
	}
	
	inline void uniformScale(real p_scale)
	{
		multiply44(math::Matrix44::getScale(p_scale, p_scale, p_scale));
	}
	
	inline void resetTextureMatrix()
	{
		m_textureStack.back().setIdentity();
		updateTextureMatrix();
	}
	
	inline void resetProjectionMatrix() { m_projectionStack.back().setIdentity(); }
	inline void resetPositionMatrix()   { m_worldStack     .back().setIdentity(); }
	
	inline void getCurrent(math::Matrix44& p_matrix) { p_matrix = m_worldStack.back(); }
	inline const math::Matrix44& getCurrent() const { return m_worldStack.back(); }
	
	inline void enterIdentityProjection() { /* DS legacy */ }
	inline void leaveIdentityProjection() { /* DS legacy */ }
	
	void updateProjectionMatrix();
	void updateWorldMatrix();
	void updateTextureMatrix();

	void checkIntegrity() const
	{
		TT_ASSERTMSG(m_worldStack.size() == 1, "MatrixStack integrity check failed: "
			"Are all push() calls matched by pop() calls?");
		TT_ASSERTMSG(m_worldStack[0].isIdentity(), "MatrixStack integrity check failed: "
			"Matrix is not identity. Is stack restored to an identity matrix after usage?");
	}
	
private:
	MatrixStack();
	~MatrixStack() { }
	
	// No copying
	MatrixStack(const MatrixStack&);
	const MatrixStack& operator=(const MatrixStack&);
	
	static MatrixStack* ms_instance;
	
	Mode m_matrixMode;
	
	typedef std::vector<math::Matrix44> MatrixContainer;
	MatrixContainer m_worldStack;
	MatrixContainer m_projectionStack;
	MatrixContainer m_textureStack;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_MATRIXSTACK_H)
