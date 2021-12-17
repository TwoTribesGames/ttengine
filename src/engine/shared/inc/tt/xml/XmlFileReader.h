#if !defined(INC_TT_XML_XMLFILEREADER_H)
#define INC_TT_XML_XMLFILEREADER_H


#include <tt/fs/types.h>
#include <tt/xml/XmlReader.h>


namespace tt {
namespace xml {

/*! \brief Implementation of the XMLReader. */
class XmlFileReader : public IXmlReader
{
public:
	XmlFileReader();
	virtual ~XmlFileReader();
	
	/*! \brief Loads an XML file into memory.
	    \return false if an error occured during loading / reading the file. */
	bool loadFile(const std::string& p_filename, fs::identifier p_type = 0);
	bool loadFile(const fs::FilePtr& p_file);
	
	/*! \brief Reads forward to the next XML node.
	    \return false, if there was no further node. */
	virtual bool read();
	
	virtual std::string getSourceName() const;
	
private:
	/*! \brief Reads the current XML node. */
	bool parseCurrentNode();
	
	/*! \brief Sets the state that text was found.
	    \return true if text should be set. */
	bool setText(char* p_start, char* p_end);
	
	/*! \brief Ignores an XML definition like <?xml something /> */
	void ignoreDefinition();
	
	/*! \brief Parses a comment. */
	void parseComment();
	
	/*! \brief Parses an opening XML element and reads attributes. */
	void parseOpeningXMLElement();
	
	/*! \brief Parses a closing XML tag. */
	void parseClosingXMLElement();
	
	/*! \brief Parses a possible CDATA section.
	    \return false if begin was not a CDATA section. */
	bool parseCDATA();
	
	/*! \brief Reads the XML file. */
	bool readFile(const fs::FilePtr& p_file);
	
	// No copying
	XmlFileReader(const XmlFileReader&);
	XmlFileReader& operator=(const XmlFileReader&);
	
	
	char* m_textData;  // data block of the text file
	char* m_position;  // current point in text to parse
	char* m_textBegin; // start of text to parse
	int   m_textSize;  // size of text to parse in characters
	
	std::string m_filename;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_XML_XMLFILEREADER_H)
