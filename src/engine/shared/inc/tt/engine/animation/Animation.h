#if !defined(INC_TT_ENGINE_ANIMATION_ANIMATION_H)
#define INC_TT_ENGINE_ANIMATION_ANIMATION_H


#include <string>

#include <tt/platform/tt_types.h>
#include <tt/math/Matrix44.h>
#include <tt/engine/EngineID.h>
#include <tt/engine/animation/fwd.h>
#include <tt/engine/animation/AnimationControl.h>
#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/scene/fwd.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace animation {

// Resource Management
typedef cache::ResourceCache<Animation> AnimationCache;


class Animation
{
public:
	static const file::FileType fileType = file::FileType_Animation;
	static const bool hasResourceHeader = true;
	
public:
	explicit Animation(const EngineID& p_id);
	inline ~Animation() { }
	
	// Loading functionality
	static Animation* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	bool load(const fs::FilePtr& p_file);

	s32 getMemSize() const;

	void setTimeRecursive(real p_time);
	bool loadMatrix();

	math::Vector3 getPosition();

	void visualize();
	void resolveEndTime(real& p_time, bool p_recursive = false);
	
	inline void setSibling(const AnimationPtr& p_sibling) {m_sibling = p_sibling;}
	inline void setChild  (const AnimationPtr& p_child)   {m_child   = p_child;}

	inline real         getStartTime() const {return m_animControl.getStartTime();}
	inline real         getEndTime()   const {return m_animControl.getEndTime();}

	inline const AnimationPtr& getChild()     const {return m_child;}
	inline const AnimationPtr& getSibling()   const {return m_sibling;}
	inline const EngineID&     getEngineID()  const {return m_id;}
	
private:
	// Only accessible by Instance class
	inline AnimationControl* getControl() {return &m_animControl;}
	friend class scene::Instance;

	bool loadRecursive(const fs::FilePtr& p_file);

	// Unique engine ID
	EngineID m_id;
	
	// Hierarchy
	AnimationPtr m_child;
	AnimationPtr m_sibling;

	// Animation control ==> move to Instance
	AnimationControl m_animControl;
	
	// Controllers
	TransformControllerPtr m_transformController;

	math::Matrix44 m_matrix;
};

// Namespace end
}
}
}


#endif // INC_TT_ENGINE_ANIMATION_ANIMATION_H
