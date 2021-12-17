#include <tt/streams/BIOStreamBase.h>


namespace tt {
namespace streams {


BIOStreamBase::BIOStreamBase()
:
m_state(IOState_GoodBit),
m_littleEndian(true)
{
}


BIOStreamBase::~BIOStreamBase()
{
}


void BIOStreamBase::clear(IOState p_state)
{
	m_state = p_state;
	//if (rdbuf_ == 0)
	//	iostate_ |= badbit;
}


void BIOStreamBase::setState(IOState p_state)
{
	clear(static_cast<IOState>(getState() | p_state));
}

// Namespace end
}
}
