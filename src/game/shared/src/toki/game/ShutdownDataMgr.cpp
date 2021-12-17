#include <tt/platform/tt_error.h>


#include <toki/game/ShutdownDataMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions

ShutdownDataMgr::ShutdownDataMgr()
:
m_data(),
m_id()
{
}


void ShutdownDataMgr::setData(const serialization::SerializationMgrPtr& p_data,
                              const std::string&                        p_id)
{
	TT_NULL_ASSERT(p_data);
	if (p_data != 0)
	{
		m_data = p_data->compress();
		m_id   = p_id;
	}
}


serialization::SerializationMgrPtr ShutdownDataMgr::getData() const
{
	TT_NULL_ASSERT(m_data);
	if (m_data != 0)
	{
		return serialization::SerializationMgr::createFromCompressedBuffer(m_data);
	}
	
	return serialization::SerializationMgrPtr();
}


void ShutdownDataMgr::reset()
{
	m_data.reset();
	m_id = "shutdown";
}

// Namespace end
}
}
