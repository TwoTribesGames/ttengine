#include <tt/doc/xap/XapWriter.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace doc {
namespace xap {

XapWriter::XapWriter()
:
m_file()
{
}


bool XapWriter::writeFile(xml::XmlNode* p_node, const fs::FilePtr& p_file)
{
	if (p_node == 0)
	{
		TT_PANIC("No xml tree specified.");
		return false;
	}
	
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	m_file = p_file;
	
	writeNode(p_node, -1);
	
	m_file.reset();
	
	return true;
}


bool XapWriter::writeFile(xml::XmlNode* p_node, const std::string& p_filename, fs::identifier p_type)
{
	if (p_node == 0)
	{
		TT_PANIC("No xml tree specified.");
		return false;
	}
	
	fs::FilePtr file = fs::open(p_filename, fs::OpenMode_Write, p_type);
	if (file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return 0;
	}
	
	return writeFile(p_node, file);
}


// Private Functions

void XapWriter::writeNode(xml::XmlNode* p_node, int p_indent)
{
	if (p_indent >= 0)
	{
		writeIndent(p_indent);
		writeString(p_node->getName());
		writeNewline();
		
		writeIndent(p_indent);
		writeString("{");
		writeNewline();
	}
	
	const xml::XmlNode::AttributeMap& map = p_node->getAttributes();
	
	for (xml::XmlNode::AttributeMap::const_iterator it = map.begin(); it != map.end(); ++it)
	{
		if ((*it).second.empty() == false)
		{
			writeIndent(p_indent + 1);
			writeString((*it).first);
			writeString(" = ");
			writeString((*it).second);
			writeString(";");
			writeNewline();
		}
	}
	
	if (p_indent >= 0 && map.size() > 0 && p_node->getChild() != 0)
	{
		writeIndent(p_indent + 1);
		writeNewline();
	}
	
	xml::XmlNode* child = p_node->getChild();
	
	while (child != 0)
	{
		writeNode(child, p_indent + 1);
		child = child->getSibling();
		if (child != 0)
		{
			writeIndent(p_indent + 1);
			writeNewline();
		}
	}
	
	if (p_indent >= 0)
	{
		writeIndent(p_indent);
		writeString("}");
		writeNewline();
	}
}


void XapWriter::writeIndent(int p_indent)
{
	for (int i = 0; i < p_indent * 4; ++i)
	{
		writeString(" ");
	}
}


void XapWriter::writeString(const std::string& p_string)
{
	m_file->write(p_string.c_str(), static_cast<fs::size_type>(p_string.size()));
}


void XapWriter::writeNewline()
{
	writeString("\n");
}


// Namespace end
}
}
}
