#if !defined(INC_TT_PRES_PRESENTATIONGROUP_H)
#define INC_TT_PRES_PRESENTATIONGROUP_H


#include <algorithm>
#include <functional>

#include <tt/math/Range.h>
#include <tt/pres/fwd.h>
#include <tt/pres/PresentationGroupInterface.h>


namespace tt {
namespace pres {


template<typename Pred>
class PresentationGroup : public GroupInterface
{
public:
	static GroupInterfacePtr create(const math::Range& p_zRange,
	                                const Pred& p_sortPred, 
                                    PresentationMgr* p_mgr);
	
	virtual ~PresentationGroup() { }
	
private:
	PresentationGroup(const math::Range& p_zRange,
	                  const Pred& p_sortPred, PresentationMgr* p_mgr);
	
	void add(PresentationObject*    p_object);
	void remove(PresentationObject* p_object);
	
	/*! \brief callback for presentationObject to resort it in the group or even the Mgr */
	void resort(PresentationObject* p_object);
	/*! \brief Gets the particle render group from layer name */
	s32 particleRenderGroupFromName(const std::string& p_name) const;
	/*! \brief gets the z range this group spans */
	inline const math::Range& getZRange() const { return m_zRange; }
	
	void update();
	void render()const;
	void renderPass(const std::string& p_passName) const;
	
	PresentationObjects split(real p_depth);
	
	
	GroupInterfaceWeakPtr m_this;
	PresentationObjects   m_objects;
	PresentationMgr*      m_mgr;
	math::Range           m_zRange;
	Pred                  m_sortPred;
	bool                  m_resort;
	
	
	//Enable copy / disable assignment
	PresentationGroup(const PresentationGroup& p_rhs);
	PresentationGroup& operator=(const PresentationGroup&);
};


//namespace end
}
}

#include "PresentationGroup.inl"


#endif // !defined(INC_TT_PRES_PRESENTATIONGROUP_H)

