#if !defined(INC_TT_MENU_ELEMENTS_ELEMENTTRAITS_H)
#define INC_TT_MENU_ELEMENTS_ELEMENTTRAITS_H


namespace tt {
namespace menu {
namespace elements {

// Element traits tag structs

/*! \brief Element can contain actions. */
struct action_element_tag { };

/*! \brief Element can contain actions and values. */
struct value_element_tag : public action_element_tag { };

/*! \brief Element can contain actions and children. */
struct container_element_tag : public action_element_tag { };

/*! \brief Element can contain actions, children, listeners and vars. */
struct menu_element_tag : public container_element_tag { };


template<typename ElementType>
struct element_traits
{
	typedef typename ElementType::element_category element_category;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_ELEMENTTRAITS_H)
