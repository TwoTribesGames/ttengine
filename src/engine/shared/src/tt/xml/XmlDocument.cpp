#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlFileReader.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace xml {

XmlDocument::XmlDocument(const std::string& p_filename, fs::identifier p_type)
:
m_rootNode(0)
{
	XmlFileReader xfr;
	if (xfr.loadFile(p_filename, p_type))
	{
		m_rootNode = XmlNode::createTree(xfr);
	}
}


XmlDocument::XmlDocument(const fs::FilePtr& p_file)
:
m_rootNode(0)
{
	XmlFileReader xfr;
	if (xfr.loadFile(p_file))
	{
		m_rootNode = XmlNode::createTree(xfr);
	}
}


XmlDocument::~XmlDocument()
{
	delete m_rootNode;
}

// Namespace end
}
}
