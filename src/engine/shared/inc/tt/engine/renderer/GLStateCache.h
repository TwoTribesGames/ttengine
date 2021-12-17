#if !defined(INC_TT_ENGINE_RENDERER_GLSTATECACHE_H)
#define INC_TT_ENGINE_RENDERER_GLSTATECACHE_H

#include <unordered_map>

#include <tt/engine/opengl_headers.h>
#include <tt/math/Vector4.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/platform/tt_printf.h>

namespace tt {
namespace engine {
namespace renderer {

class GLStateCache
{
public:
	GLStateCache();

	// State management
	void setClientState(GLenum, bool enabled);
	void setState(GLenum, bool enabled);

	void setColor(GLfloat, GLfloat, GLfloat, GLfloat);

	void setBlendFunc(GLenum, GLenum, GLenum, GLenum);

	void setBlendFunc(GLenum src, GLenum dst) {
		setBlendFunc(src, dst, src, dst);
	}

//	void setTextureCientState(GLenum, bool enabled);
//	void setTextureState(GLenum, bool enabled);

	// Apply any pending state changes
	void apply();

private:
	// No copying
	GLStateCache(const GLStateCache&);
	const GLStateCache& operator=(const GLStateCache&);

	template<typename T>
	struct ChangeTracker {
		T _current;
		T _new;
		bool _changed;

		void operator = (T value) {
			if (_current != value) {
				_changed = true;
			}
			_new = value;
		}

		// mark as synced
		void synced() {
			_current = _new;
			_changed = false;
		}

		ChangeTracker() {
			// assume changed on create
			_changed = true;
		}
	};

	typedef std::unordered_map<GLenum, ChangeTracker<bool>> tStateCache;

	tStateCache m_stateCache;
	tStateCache m_clientStateCache;
	ChangeTracker<tt::math::Vector4> m_frontColor;

	struct BlendFunc {
		GLenum srcRGB;
		GLenum dstRGB;
		GLenum srcAlpha;
		GLenum dstAlpha;

		BlendFunc() : srcRGB(GL_ONE), dstRGB(GL_ZERO), srcAlpha(GL_ONE), dstAlpha(GL_ZERO) {}

		BlendFunc(GLenum sRGB, GLenum dRGB, GLenum sA, GLenum dA) :
		srcRGB(sRGB), dstRGB(dRGB), srcAlpha(sA), dstAlpha(dA)
		{}

		bool operator==(const BlendFunc& o) {
			return srcRGB == o.srcRGB
					&& dstRGB == o.dstRGB
					&& srcAlpha == o.srcAlpha
					&& dstAlpha == o.dstAlpha;
		}

		bool operator!=(const BlendFunc& o) {
			return !(*this == o);
		}
	};

	ChangeTracker<BlendFunc> m_blendFunc;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_GLSTATECACHE_H)
