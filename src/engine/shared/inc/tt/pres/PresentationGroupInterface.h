#if !defined(INC_TT_PRES_PRESENTATIONGROUPINTERFACE_H)
#define INC_TT_PRES_PRESENTATIONGROUPINTERFACE_H

#include <algorithm>
#include <functional>

#include <tt/math/Range.h>
#include <tt/pres/fwd.h>
#include <tt/pres/PresentationObject.h>


namespace tt {
namespace pres {


class GroupInterface
{
public:
	typedef std::vector<PresentationObject*> PresentationObjects;
	
	virtual ~GroupInterface(){}
	
	/*! \brief callback for presentationObject to resort it in the group or even the Mgr */
	virtual void resort(PresentationObject* p_object) = 0;
	
	/*! \brief Gets the particle render group from layer name */
	virtual s32 particleRenderGroupFromName(const std::string& p_name) const = 0;
	
	/*! \brief gets the z range this group spans */
	virtual const math::Range& getZRange() const = 0;
	
	/*! \brief Resorts the group if needed */
	virtual void update() = 0;
	
	/*! \brief Renders the group in the right order */
	virtual void render() const = 0;
	virtual void renderPass(const std::string& p_passName) const = 0;
	
	/*! \brief Adds a presentationObject to this group */
	virtual void add(PresentationObject* p_object) = 0;
	
	/*! \brief Removes a presentationObject from this group */
	virtual void remove(PresentationObject* p_object) = 0;
	
	/*! \brief Splits the group at the given depth and creates a new group */
	virtual PresentationObjects split(real p_depth) = 0;
};


struct GroupLess
{
	inline bool operator()(const GroupInterfacePtr& p_lhs,
	                       const GroupInterfacePtr& p_rhs) const
	{
		if(p_lhs->getZRange().getMax() <= p_rhs->getZRange().getMin())
		{
			return true;
		}
		else if(p_lhs->getZRange().getMin() >= p_rhs->getZRange().getMax())
		{
			return false;
		}
		else if (p_lhs->getZRange().getMax() == p_rhs->getZRange().getMax() && 
		         p_lhs->getZRange().getMin() == p_rhs->getZRange().getMin())
		{
			return false;
		}
		TT_PANIC("Error overlapping groups");
		return false;
	}
};



//namespace end
}
}


#endif // !defined(INC_TT_PRES_PRESENTATIONGROUPINTERFACE_H)

