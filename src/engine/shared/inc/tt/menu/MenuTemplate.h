#if !defined(INC_TT_MENU_MENUTEMPLATE_H)
#define INC_TT_MENU_MENUTEMPLATE_H


#include <tt/platform/tt_types.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace menu {

/*! \brief Template Parser. */
class MenuTemplate
{
public:
	~MenuTemplate();
	
	/*! \brief Finds the template that matches the passed node.
	    \param p_node The usage of the template. */
	static MenuTemplate* getTemplate(xml::XmlNode* p_node);
	
	inline u32 getNodeCount()  const { return m_template->getChildCount(); }
	inline u32 getChildCount() const { return m_data->getChildCount();     }
	
	inline xml::XmlNode* getNode()  { return m_template->getChild(); }
	inline xml::XmlNode* getChild() { return m_data->getChild(); }
	/*
	inline xml::XmlNode* getTemplate() { return m_template; }
	inline xml::XmlNode* getData()     { return m_data;     }
	*/
	
	const std::string& getVariable(std::string p_name);
	
private:
	/*! \param p_data The usage of the template.
	    \param p_template The template itself. */
	MenuTemplate(xml::XmlNode* p_data, xml::XmlNode* p_template);
	int getChildrenTagCount(xml::XmlNode* p_node) const;
	
	MenuTemplate(const MenuTemplate&);
	const MenuTemplate& operator=(const MenuTemplate&);
	
	
	xml::XmlNode* m_template;
	xml::XmlNode* m_data;
	bool          m_hasChildrenNode;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUTEMPLATE_H)
