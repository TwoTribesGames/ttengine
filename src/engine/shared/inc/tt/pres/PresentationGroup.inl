#if !defined(INC_TT_PRES_PRESENTATIONGROUP_INL)
#define INC_TT_PRES_PRESENTATIONGROUP_INL


#if !defined(INC_TT_PRES_PRESENTATIONGROUP_H)
#error This inline file should only be included by its parent headerfile.
#endif

#include <tt/code/helpers.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/PresentationMgr.h>


namespace tt {
namespace pres {


template<typename Pred>
GroupInterfacePtr PresentationGroup<Pred>::create(const math::Range& p_zRange,
                                                  const Pred& p_sortPred,
                                                  PresentationMgr* p_mgr)
{
	PresentationGroup* group(new PresentationGroup(p_zRange, p_sortPred, p_mgr));
	
	GroupInterfacePtr groupInterface(group);
	group->m_this = groupInterface;
	
	return groupInterface;
}


template<typename Pred>
PresentationGroup<Pred>::PresentationGroup(const math::Range& p_zRange,
                                           const Pred& p_sortPred,
                                           PresentationMgr* p_mgr)
:
m_this(),
m_objects(),
m_mgr(p_mgr),
m_zRange(p_zRange),
m_sortPred(p_sortPred),
m_resort(true)
{
	TT_NULL_ASSERT(m_mgr);
}


template<typename Pred>
void PresentationGroup<Pred>::add(PresentationObject* p_object)
{
	TT_NULL_ASSERT(p_object);
	
	if (p_object != 0)
	{
		TT_ASSERTMSG(p_object->getObjectGroup() == 0,
		             "Can't add an object that already belongs to an group!");
		p_object->setObjectGroup( m_this.lock());
		m_objects.push_back(p_object);
		m_resort = true;
	}
}


template<typename Pred>
void PresentationGroup<Pred>::remove(PresentationObject* p_object)
{
	TT_NULL_ASSERT(p_object);
	
	for (PresentationObjects::iterator it(m_objects.begin()) ; it != m_objects.end(); ++it)
	{
		if (*it == p_object)
		{
			TT_ASSERTMSG(p_object->getObjectGroup() == m_this.lock(),
			             "Found object in group which didn't have the correct group!");
			p_object->resetObjectGroup();
			code::helpers::unorderedErase(m_objects, it);
			m_resort = true;
			return;
		}
	}
	
	// Only allow not finding a null pointer.
	TT_ASSERTMSG(p_object == 0,
	             "Can't remove PresentationObject because it was not found within group!");
}


template<typename Pred>
void PresentationGroup<Pred>::resort(PresentationObject* p_object)
{
	TT_NULL_ASSERT(p_object);
	
	const real objectZPos(p_object->getWorldPosition().z);
	if (objectZPos < m_zRange.getMin() || objectZPos >= m_zRange.getMax())
	{
		m_mgr->reinsertPresentationObject(p_object);
	}
	else
	{
		m_resort = true;
	}
}


template<typename Pred>
s32 PresentationGroup<Pred>::particleRenderGroupFromName(const std::string& p_name) const
{
	return m_mgr->particleRenderGroupFromName(p_name);
}


template<typename Pred>
void PresentationGroup<Pred>::update()
{
	if (m_resort)
	{
		std::sort(m_objects.begin(), m_objects.end(), m_sortPred);
		m_resort = false;
	}
}


template<typename Pred>
void PresentationGroup<Pred>::render() const
{
	for (PresentationObjects::const_iterator it(m_objects.begin()) ; it != m_objects.end() ; ++it)
	{
		(*it)->render();
	}
}


template<typename Pred>
void PresentationGroup<Pred>::renderPass(const std::string& p_passName) const
{
	for (PresentationObjects::const_iterator it(m_objects.begin()) ; it != m_objects.end() ; ++it)
	{
		(*it)->renderPass(p_passName);
	}
}


template<typename Pred>
GroupInterface::PresentationObjects PresentationGroup<Pred>::split(real p_depth)
{
	PresentationObjects returnObjects;
	m_zRange.setMinMax(m_zRange.getMin(), p_depth);
	
	for (PresentationObjects::iterator it(m_objects.begin()) ; it != m_objects.end() ;)
	{
		PresentationObject* object(*it);
		real objZPos(object->getWorldPosition().z);
		if (objZPos < m_zRange.getMin() || objZPos >= m_zRange.getMax())
		{
			returnObjects.push_back(object);
			TT_ASSERTMSG(object->getObjectGroup() == m_this.lock(),
				            "Found object in group which didn't have the correct group!");
			object->resetObjectGroup(); // Remove reference to this group.
			it = code::helpers::unorderedErase(m_objects, it);
			m_resort = true;
		}
		else
		{
			++it;
		}
	}
	return returnObjects;
}


// Namespace end
}
}


#endif // !defined(INC_TT_PRES_PRESENTATIONGROUP_INL)

