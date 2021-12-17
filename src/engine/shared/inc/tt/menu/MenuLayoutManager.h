#if !defined(INC_TT_MENU_MENULAYOUTMANAGER_H)
#define INC_TT_MENU_MENULAYOUTMANAGER_H


#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/math/Rect.h>
#include <tt/menu/MenuLayout.h>
#include <tt/menu/elements/MenuElementInterface.h>
#include <tt/menu/elements/ValueDecorator.h>


namespace tt {
namespace menu {

/*! \brief Lays out menu elements according to their layout settings. */
class MenuLayoutManager
{
public:
	typedef std::vector<elements::MenuElementInterface*> Elements;
	
	
	/*! \brief Lays out the specified elements within the specified rectangle,
	           using the specified parent layout settings. */
	static void doLayout(const MenuLayout&      p_parentLayout,
	                     Elements&              p_elements,
	                     const math::PointRect& p_rectangle);
	
	/*! \brief Utility function to convert a vector of MenuElementInterface-derived
	           object pointers to a vector of MenuElementInterface pointers. */
	template<typename ElementType>
	static Elements getElements(const std::vector<ElementType*>& p_elements)
	{
		Elements dest;
		dest.reserve(p_elements.size());
		
		for (typename std::vector<ElementType*>::const_iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			dest.push_back(*it);
		}
		
		return dest;
	}
	
	
	/*! \brief Returns the minimum or requested width of the specified elements. */
	static s32 getElementWidth(const MenuLayout& p_parentLayout,
	                           const Elements&   p_elements,
	                           bool              p_minimum);
	
	/*! \brief Returns the minimum or requested height of the specified elements. */
	static s32 getElementHeight(const MenuLayout& p_parentLayout,
	                            const Elements&   p_elements,
	                            bool              p_minimum);
	
private:
	static void doHorizontalLayoutRemainingChildren(
			Elements&              p_children,
			const math::PointRect& p_rect,
			s32                    p_min,
			s32                    p_max);
	static void doVerticalLayoutRemainingChildren(
			Elements&              p_children,
			const math::PointRect& p_rect,
			s32                    p_min,
			s32                    p_max);
	
	static void makeFit(const MenuLayout&      p_parentLayout,
	                    Elements&              p_elements,
	                    const math::PointRect& p_rect,
	                    s32                    p_min,
	                    s32                    p_max);
	
	
	static void positionLeftChildren(Elements&              p_elements,
	                                 const math::PointRect& p_rect,
	                                 s32                    p_min,
	                                 s32                    p_max);
	static void positionRightChildren(Elements&              p_elements,
	                                  const math::PointRect& p_rect,
	                                  s32                    p_min,
	                                  s32                    p_max);
	static void positionTopChildren(Elements&              p_elements,
	                                const math::PointRect& p_rect,
	                                s32                    p_min,
	                                s32                    p_max);
	static void positionBottomChildren(Elements&              p_elements,
	                                   const math::PointRect& p_rect,
	                                   s32                    p_min,
	                                   s32                    p_max);
	
	static void distributeWidthChildren(Elements& p_elements,
	                                    s32       p_availableSpace);
	static void distributeHeightChildren(Elements& p_elements,
	                                     s32       p_availableSpace);
	
	static void distributeMaxWidthChildren(Elements& p_elements,
	                                       s32       p_availableSpace);
	static void distributeMaxHeightChildren(Elements& p_elements,
	                                        s32       p_availableSpace);
	
	static void distributeWidthChildrenEqually(Elements& p_elements,
	                                           s32       p_availableSpace);
	static void distributeHeightChildrenEqually(Elements& p_elements,
	                                            s32       p_availableSpace);
	
	static void doSimpleVerticalLayoutChildren(
			Elements&              p_elements,
			const math::PointRect& p_rect);
	static void doSimpleHorizontalLayoutChildren(
			Elements&              p_elements,
			const math::PointRect& p_rect);
	
	MenuLayoutManager();
	~MenuLayoutManager();
	
	// No copying or assignment
	MenuLayoutManager(const MenuLayoutManager&);
	const MenuLayoutManager& operator=(const MenuLayoutManager&);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENULAYOUTMANAGER_H)
