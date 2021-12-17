#include <tt/audio/xact/RPCCurve.h>
#include <tt/audio/xact/RuntimeParameterControl.h>
#include <tt/code/helpers.h>
#include <tt/fs/File.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace audio {
namespace xact {

//--------------------------------------------------------------------------------------------------
// Public member functions

RuntimeParameterControl::RuntimeParameterControl(const std::string& p_name)
:
m_name(p_name),
m_curves()
{
}


RuntimeParameterControl::~RuntimeParameterControl()
{
	code::helpers::freePointerContainer(m_curves);
}


RuntimeParameterControl* RuntimeParameterControl::createRPC(const std::string& p_name, const xml::XmlNode* p_node)
{
	TT_ASSERT(p_name.empty() == false);
	TT_NULL_ASSERT(p_node);
	
	RuntimeParameterControl* rpc = new RuntimeParameterControl(p_name);
	
	// Read RPC curves
	const xml::XmlNode* rpcChild = p_node->getChild();
	
	while(rpcChild != 0)
	{
		if(rpcChild->getName() == "RPC Curve")
		{
			RPCCurve* curve = RPCCurve::createCurve(rpcChild);
			
			if(curve != 0)
			{
				rpc->addCurve(curve);
			}
		}
		
		rpcChild = rpcChild->getSibling();
	}
	
	return rpc;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool RuntimeParameterControl::load(const fs::FilePtr& p_file)
{
	// read name
	fs::readNarrowString(p_file, &m_name);
	
	// read curves
	int size;
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		RPCCurve* curve = new RPCCurve();
		curve->load(p_file);
		m_curves.push_back(curve);
	}
	
	return true;
}


bool RuntimeParameterControl::save(const fs::FilePtr& p_file) const
{
	// write name
	fs::writeNarrowString(p_file, m_name);
	
	// write curves
	fs::writeInteger(p_file, m_curves.size());
	for (RPCCurves::const_iterator it = m_curves.begin(); it != m_curves.end(); ++it)
	{
		(*it)->save(p_file);
	}
	
	return true;
}


// namespace end
}
}
}
