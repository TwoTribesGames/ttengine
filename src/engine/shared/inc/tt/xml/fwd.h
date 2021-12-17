#ifndef INC_TT_XML_FWD_H
#define INC_TT_XML_FWD_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace xml {

class XmlFileReader;
class XmlFileWriter;
class XmlNode;
class IXmlReader;
class XmlStreamReader;

class XmlDocument;
typedef tt_ptr<      XmlDocument>::shared      XmlDocumentPtr;
typedef tt_ptr<const XmlDocument>::shared ConstXmlDocumentPtr;
typedef tt_ptr<      XmlDocument>::weak    WeakXmlDocumentPtr;

} // namespace end
}

#endif // #ifndef INC_TT_XML_FWD_H
