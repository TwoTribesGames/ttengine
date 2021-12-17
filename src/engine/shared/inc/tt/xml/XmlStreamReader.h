#if !defined(INC_TT_XML_XMLSTREAMREADER_H)
#define INC_TT_XML_XMLSTREAMREADER_H


#include <fstream>
#include <limits>

#include <tt/xml/XmlReader.h>


namespace tt {
namespace xml {

/*! \brief Implementation of the XMLReader. */
class XmlStreamReader : public IXmlReader
{
public:
	XmlStreamReader();
	virtual ~XmlStreamReader();
	
	/*! \brief Initializes an XML file as a stream.
	    \return false if an error occured during initialization. */
	bool initStream(const std::string& p_filename);
	
	//! Reads forward to the next xml node. 
	//! \return Returns false, if there was no further node. 
	virtual bool read();
	
	virtual std::string getSourceName() const;
	
private:
	typedef std::basic_ifstream<char, std::char_traits<char> > ifstream;
	
	/*! \brief Reads the current XML node. */
	bool parseCurrentNode();
	
	/*! \brief Sets the state that text was found.
	    \return true if set should be set. */
	bool setText(std::string& p_text);
	
	/*! \brief Ignores an XML definition like <?xml something /> */
	void ignoreDefinition();
	
	/*! \brief Parses a comment. */
	void parseComment();
	
	/*! \brief Parses an opening XML element and reads attributes. */
	void parseOpeningXMLElement();
	
	/*! \brief Parses a closing XML tag. */
	void parseClosingXMLElement();
	
	/*! \brief Parses a possible CDATA section.
	    \return false if it is not a CDATA section. */
	bool parseCDATA();
	
	ifstream m_xmlStream;
	std::string  m_filename;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_XML_XMLSTREAMREADER_H)
