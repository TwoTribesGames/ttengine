#if !defined(INC_TT_PRES_TRIGGERSTACK_H)
#define INC_TT_PRES_TRIGGERSTACK_H
#include <vector>

#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/StackBase.h>
#include <tt/pres/fwd.h>
#include <tt/pres/TriggerInterface.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace pres {

class TriggerStack : public anim2d::StackBase<TriggerInterface>
{
public:
	TriggerStack(){}
	virtual ~TriggerStack(){}
	
	inline void push_back(TriggerInterfacePtr p_trigger)
	{
		m_allAnimations.push_back(p_trigger);
		TT_ASSERT(m_activeAnimations.empty());
	}
	
	virtual bool save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const;
	virtual size_t getBufferSize() const;
	
	void setPresentationObject(const PresentationObjectPtr& p_object );
	
	/*! \brief Gets called when the presentation is inactive.*/
	void presentationEnded();
	
	TriggerStack(const TriggerStack& p_rhs);
private:
	
	TriggerStack& operator=(const TriggerStack&); // disable assignment
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_TRIGGERSTACK_H)
