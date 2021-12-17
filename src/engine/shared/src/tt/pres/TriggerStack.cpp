#include <tt/pres/PresentationValue.h>
#include <tt/pres/TriggerInterface.h>
#include <tt/pres/TriggerStack.h>
#include <tt/xml/util/parse.h>

namespace tt {
namespace pres {


size_t TriggerStack::getBufferSize() const
{
	return 0; // loading and saving of triggers is done in presentationLoader
}


bool TriggerStack::save( u8*& , size_t& , code::ErrorStatus* ) const
{
	TT_PANIC("Saving of triggers is done in PresentationLoader"); return false;
}


void TriggerStack::setPresentationObject( const PresentationObjectPtr& p_object )
{
	for(Stack::iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->setPresentationObject(p_object); 
	}
}


void TriggerStack::presentationEnded()
{
	for(Stack::iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->presentationEnded();
	}
}


TriggerStack::TriggerStack(const TriggerStack& p_rhs)
:
anim2d::StackBase<TriggerInterface>(p_rhs)
{
}

//namespace end
}
}
