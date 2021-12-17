#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlFileWriter.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace xml {

//--------------------------------------------------------------------------------------------------
// Public member functions

XmlFileWriter::XmlFileWriter(const XmlNode* p_tree)
:
m_tree(p_tree)
{
}


bool XmlFileWriter::save(bool p_stripWhitespace, const std::string& p_filename, fs::identifier p_type)
{
	if (p_filename.empty())
	{
		TT_PANIC("No filename specified.");
		return false;
	}
	
	fs::FilePtr file = fs::open(p_filename, fs::OpenMode_Write, p_type);
	
	if (file == 0)
	{
		TT_PANIC("Unable to open %s.", p_filename.c_str());
		return false;
	}
	
	return save(p_stripWhitespace, file);
}


bool XmlFileWriter::save(bool p_stripWhitespace, const fs::FilePtr& p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (m_tree == 0)
	{
		TT_PANIC("No XML tree present.");
		return false;
	}
	
	// FIXME: Either somehow make sure encoding is remembered through load -> save,
	//        or hard-code UTF-8 instead.
	const std::string header("<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
	
	if (write(p_file, header) == false)
	{
		TT_PANIC("Error writing to %s.", p_file->getPath());
		return false;
	}
	
	return writeNode(p_file, m_tree, 0, p_stripWhitespace);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool XmlFileWriter::write(const fs::FilePtr& p_file, const std::string& p_str)
{
	fs::size_type size = static_cast<fs::size_type>(p_str.size());
	return p_file->write(p_str.c_str(), size) == size;
}


bool XmlFileWriter::writeNode(const fs::FilePtr& p_file, const XmlNode* p_node, int p_depth, bool p_strip)
{
	std::string indent;
	if (p_strip == false)
	{
		indent.append(static_cast<std::string::size_type>(p_depth), '\t');
	}
	
	std::string tag = p_node->getName();
	
	std::string attributes;
	
	for (u32 i = 0; i < p_node->getAttributeCount(); ++i)
	{
		const XmlNode::Attribute& v = p_node->getAttribute(i);
		if (v.first.empty() == false)
		{
			attributes += " " + v.first + "=";
			attributes += "\"" + replaceSpecialChars(v.second) + "\"";
		}
	}
	
	if (p_node->getChild() == 0 && p_node->getData().empty())
	{
		tag = indent + "<" + tag + attributes + (p_strip ? "/>" : "/>\n");
		if (write(p_file, tag) == false)
		{
			return false;
		}
	}
	else
	{
		const bool noNewline = p_strip || p_node->getData().empty() == false;
		const std::string opentag(indent + "<" + tag + attributes + (noNewline ? ">" : ">\n"));
		if (write(p_file, opentag) == false)
		{
			return false;
		}
		
		if (p_node->getData().empty() == false &&
		    write(p_file, p_node->getData()) == false)
		{
			return false;
		}
		
		for (const XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
		{
			if (writeNode(p_file, child, p_depth + 1, p_strip) == false)
			{
				return false;
			}
		}
		
		// Only indent close tag if there are any children (otherwise this node was data-only)
		const std::string closeIndent(p_node->getChild() == 0 ? std::string() : indent);
		const std::string closetag(closeIndent + "</" + tag + (p_strip ? ">" : ">\n"));
		if (write(p_file, closetag) == false)
		{
			return false;
		}
	}
	
	return true;
}


std::string XmlFileWriter::replaceSpecialChars(const std::string& p_str)
{
	std::string ret(p_str);
	
	std::string::size_type pos = ret.find('&');
	
	while ( pos != std::string::npos )
	{
		ret = ret.erase(pos, 1);
		ret = ret.insert(pos, "&amp;");
		pos = ret.find('&', pos + 1);
	}
	
	pos = ret.find('>');
	
	while ( pos != std::string::npos )
	{
		ret = ret.erase(pos, 1);
		ret = ret.insert(pos, "&gt;");
		pos = ret.find('>', pos + 1);
	}
	
	pos = ret.find('<');
	
	while ( pos != std::string::npos )
	{
		ret = ret.erase(pos, 1);
		ret = ret.insert(pos, "&lt;");
		pos = ret.find('<', pos + 1);
	}
	
	pos = ret.find('\'');
	
	while ( pos != std::string::npos )
	{
		ret = ret.erase(pos, 1);
		ret = ret.insert(pos, "&apos;");
		pos = ret.find('\'', pos + 1);
	}
	
	pos = ret.find('\"');
	
	while ( pos != std::string::npos )
	{
		ret = ret.erase(pos, 1);
		ret = ret.insert(pos, "&quot;");
		pos = ret.find('\"', pos + 1);
	}
	
	return ret;
}

// Namespace end
}
}
