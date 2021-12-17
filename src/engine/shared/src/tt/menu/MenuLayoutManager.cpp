#include <algorithm>

#include <tt/platform/tt_error.h>
#include <tt/menu/MenuLayoutManager.h>

//#define MENU_DEBUG_OUTPUT_ENABLED
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {

using elements::MenuElementInterface;
using math::PointRect;
using math::Point2;


//------------------------------------------------------------------------------
// Public member functions

void MenuLayoutManager::doLayout(const MenuLayout& p_parentLayout,
                                 Elements&         p_elements,
                                 const PointRect&  p_rectangle)
{
	if (p_elements.empty())
	{
		// Nothing to do here
		return;
	}
	
	if (p_parentLayout.getOrder() == MenuLayout::Order_Horizontal)
	{
		Elements leftChildren;
		Elements rightChildren;
		Elements centerChildren;
		
		s32 leftBorder      = 0;
		s32 requestedLeft   = 0;
		s32 rightBorder     = 0;
		s32 requestedRight  = 0;
		s32 requestedCenter = 0;
		
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			MenuElementInterface* elem = (*it);
			
			switch (elem->getLayout().getHorizontalPositionType())
			{
			case MenuLayout::Position_Left:
				requestedLeft += elem->getRequestedWidth();
				leftChildren.push_back(elem);
				break;
				
			case MenuLayout::Position_Center:
				requestedCenter += elem->getRequestedWidth();
				centerChildren.push_back(elem);
				break;
				
			case MenuLayout::Position_Right:
				requestedRight += elem->getRequestedWidth();
				rightChildren.push_back(elem);
				break;
				
			default:
				TT_PANIC("Element '%s': Unknown horizontal position type: %d",
				         elem->getName().c_str(),
				         elem->getLayout().getHorizontalPositionType());
				break;
			}
		}
		
		// Determine the elements that are remaining for layout
		Elements remainingChildren;
		std::copy(leftChildren.begin(), leftChildren.end(),
		          std::back_inserter(remainingChildren));
		std::copy(centerChildren.begin(), centerChildren.end(),
		          std::back_inserter(remainingChildren));
		std::copy(rightChildren.rbegin(), rightChildren.rend(),
		          std::back_inserter(remainingChildren));
		
		if (remainingChildren.empty() == false)
		{
			// There are elements to be laid out
			if ((requestedLeft + requestedCenter + requestedRight) >
			    (p_rectangle.getWidth() - (leftBorder + rightBorder)))
			{
				makeFit(p_parentLayout, remainingChildren, p_rectangle,
				        leftBorder, p_rectangle.getWidth() - rightBorder);
			}
			else
			{
				doHorizontalLayoutRemainingChildren(remainingChildren,
					p_rectangle, leftBorder,
					p_rectangle.getWidth() - rightBorder);
			}
		}
	}
	else if (p_parentLayout.getOrder() == MenuLayout::Order_Vertical)
	{
		Elements topChildren;
		Elements bottomChildren;
		Elements centerChildren;
		
		s32 topBorder       = 0;
		s32 requestedTop    = 0;
		s32 bottomBorder    = 0;
		s32 requestedBottom = 0;
		s32 requestedCenter = 0;
		
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			MenuElementInterface* elem = (*it);
			
			switch (elem->getLayout().getVerticalPositionType())
			{
			case MenuLayout::Position_Top:
				requestedTop += elem->getRequestedHeight();
				topChildren.push_back(elem);
				break;
				
			case MenuLayout::Position_Bottom:
				requestedBottom += elem->getRequestedHeight();
				bottomChildren.push_back(elem);
				break;
				
			case MenuLayout::Position_Center:
				requestedCenter += elem->getRequestedHeight();
				centerChildren.push_back(elem);
				break;
				
			default:
				TT_PANIC("Element '%s': Unknown vertical position type: %d",
				         elem->getName().c_str(),
				         elem->getLayout().getVerticalPositionType());
				break;
			}
		}
		
		// Determine the elements that are remaining for layout
		Elements remainingChildren;
		std::copy(topChildren.begin(), topChildren.end(),
		          std::back_inserter(remainingChildren));
		std::copy(centerChildren.begin(), centerChildren.end(),
		          std::back_inserter(remainingChildren));
		std::copy(bottomChildren.rbegin(), bottomChildren.rend(),
		          std::back_inserter(remainingChildren));
		
		if (remainingChildren.empty() == false)
		{
			if ((requestedTop + requestedCenter + requestedBottom) >
			    (p_rectangle.getHeight() - (topBorder + bottomBorder)))
			{
				makeFit(p_parentLayout, remainingChildren, p_rectangle,
				        topBorder, p_rectangle.getHeight() - bottomBorder);
			}
			else
			{
				doVerticalLayoutRemainingChildren(remainingChildren,
					p_rectangle, topBorder,
					p_rectangle.getHeight() - bottomBorder);
			}
		}
	}
	else
	{
		TT_PANIC("Unknown element order: %d", p_parentLayout.getOrder());
	}
}


s32 MenuLayoutManager::getElementWidth(const MenuLayout& p_parentLayout,
                                       const Elements&   p_elements,
                                       bool              p_minimum)
{
	s32 width = 0;
	
	if (p_parentLayout.getOrder() == MenuLayout::Order_Vertical)
	{
		// Elements are laid out in vertical fashion;
		// width is that of the widest element
		s32 maxWidth = 0;
		s32 minWidth = 0;
		
		for (Elements::const_iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			maxWidth = std::max(maxWidth,
			                    (*it)->getRequestedHorizontalPosition() +
			                    (*it)->getRequestedWidth());
			minWidth = std::max(minWidth,
			                    (*it)->getRequestedHorizontalPosition() +
			                    (*it)->getMinimumWidth());
		}
		
		if (p_minimum)
		{
			width = minWidth;
		}
		else
		{
			width = maxWidth;
		}
    }
    else if (p_parentLayout.getOrder() == MenuLayout::Order_Horizontal)
    {
		// Elements are laid out in horizontal fashion;
		// width is that of all the elements added together
		s32 leftBorder         = 0;
		s32 minimumLeftBorder  = 0;
		s32 rightBorder        = 0;
		s32 minimumRightBorder = 0;
		s32 centerWidth        = 0;
		s32 minimumCenterWidth = 0;
		
		for (Elements::const_iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			const MenuElementInterface* elem = (*it);
			
			switch (elem->getLayout().getHorizontalPositionType())
			{
			case MenuLayout::Position_Left:
				leftBorder        += elem->getRequestedWidth();
				minimumLeftBorder += elem->getMinimumWidth();
				break;
				
			case MenuLayout::Position_Center:
				centerWidth        += elem->getRequestedWidth();
				minimumCenterWidth += elem->getMinimumWidth();
				break;
				
			case MenuLayout::Position_Right:
				rightBorder        += elem->getRequestedWidth();
				minimumRightBorder += elem->getMinimumWidth();
				break;
				
			case MenuLayout::Position_Undefined:
				TT_PANIC("Element '%s' has no horizontal position type set.",
				         elem->getName().c_str());
				break;
				
			default:
				TT_PANIC("Element '%s': Unknown horizontal position type: %d",
				         elem->getName().c_str(),
				         elem->getLayout().getHorizontalPositionType());
				break;
			}
		}
		
		if (p_minimum)
		{
			width = minimumLeftBorder + minimumCenterWidth + minimumRightBorder;
		}
		else
		{
			width = leftBorder + centerWidth + rightBorder;
		}
	}
	else
	{
		TT_PANIC("Unknown element order: %d", p_parentLayout.getOrder());
	}
	
	return width;
}


s32 MenuLayoutManager::getElementHeight(const MenuLayout& p_parentLayout,
                                        const Elements&   p_elements,
                                        bool              p_minimum)
{
	s32 height = 0;
	
	if (p_parentLayout.getOrder() == MenuLayout::Order_Horizontal)
	{
		s32 maxHeight = 0;
		s32 minHeight = 0;
		
		for (Elements::const_iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			maxHeight = std::max(maxHeight,
			                     (*it)->getRequestedVerticalPosition() +
			                     (*it)->getRequestedHeight());
			minHeight = std::max(minHeight,
			                     (*it)->getRequestedVerticalPosition() +
			                     (*it)->getMinimumHeight());
		}
		
		if (p_minimum)
		{
			height = minHeight;
		}
		else
		{
			height = maxHeight;
		}
	}
	else if (p_parentLayout.getOrder() == MenuLayout::Order_Vertical)
	{
		s32 topBorder           = 0;
		s32 minimumTopBorder    = 0;
		s32 bottomBorder        = 0;
		s32 minimumBottomBorder = 0;
		s32 centerWidth         = 0;
		s32 minimumCenterWidth  = 0;
		
		for (Elements::const_iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			const MenuElementInterface* elem = (*it);
			
			switch (elem->getLayout().getVerticalPositionType())
			{
			case MenuLayout::Position_Top:
				topBorder        += elem->getRequestedHeight();
				minimumTopBorder += elem->getMinimumHeight();
				break;
				
			case MenuLayout::Position_Center:
				centerWidth        += elem->getRequestedHeight();
				minimumCenterWidth += elem->getMinimumHeight();
				break;
				
			case MenuLayout::Position_Bottom:
				bottomBorder        += elem->getRequestedHeight();
				minimumBottomBorder += elem->getMinimumHeight();
				break;
				
			case MenuLayout::Position_Undefined:
				TT_PANIC("Element '%s' has no vertical position type set.",
				         elem->getName().c_str());
				break;
				
			default:
				TT_PANIC("Element '%s': Unknown vertical position type: %d\n",
				         elem->getName().c_str(),
				         elem->getLayout().getVerticalPositionType());
				break;
			}
		}
		
		if (p_minimum)
		{
			height = minimumTopBorder + minimumCenterWidth + minimumBottomBorder;
		}
		else
		{
			height = topBorder + centerWidth + bottomBorder;
		}
	}
	else
	{
		TT_PANIC("Unknown element order: %d", p_parentLayout.getOrder());
	}
	
	return height;
}


//------------------------------------------------------------------------------
// Private member functions

void MenuLayoutManager::doHorizontalLayoutRemainingChildren(
		Elements&         p_elements,
		const PointRect&  p_rect,
		s32               p_min,
		s32               p_max)
{
	if (p_elements.empty())
	{
		return;
	}
	
	MENU_Printf("MenuLayoutManager::doHorizontalLayoutRemainingChildren: "
	            "Horizontal layout for %u remaining "
	            "children over range %d - %d.\n",
	            p_elements.size(), p_min, p_max);
	
	// Tracks remaining available horizontal space
	s32 availableSpace = p_max - p_min;
	
	
	{
		Elements maxChildren; // elements requesting max width
		
		// First, set size of each element to requiredsize
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			MenuElementInterface* elem = (*it);
			PointRect rect(elem->getRectangle());
			
			s32 reqW = elem->getRequestedWidth();
			availableSpace -= reqW;
			rect.setWidth(reqW);
			elem->setRectangle(rect);
			
			if (elem->getLayout().getWidthType() == MenuLayout::Size_Max)
			{
				maxChildren.push_back(elem);
			}
		}
		
		// Set size of max children
		if (maxChildren.empty() == false)
		{
			distributeMaxWidthChildren(maxChildren, availableSpace);
		}
	}
	
	
	{
		Elements centerChildren;
		
		// Do positioning for left and right children
		s32 leftBorder  = p_min;
		s32 rightBorder = p_max;
		s32 centerSize  = 0;
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			MenuElementInterface* elem = (*it);
			PointRect rect(elem->getRectangle());
			
			switch (elem->getLayout().getHorizontalPositionType())
			{
			case MenuLayout::Position_Left:
				rect.setPosition(Point2(p_rect.getPosition().x + leftBorder,
				                        rect.getPosition().y));
				elem->setRectangle(rect);
				leftBorder += rect.getWidth();
				break;
				
			case MenuLayout::Position_Center:
				centerChildren.push_back(elem);
				centerSize += rect.getWidth();
				break;
				
			case MenuLayout::Position_Right:
				rect.setPosition(Point2(p_rect.getPosition().x +
					rightBorder - rect.getWidth(), rect.getPosition().y));
				elem->setRectangle(rect);
				rightBorder -= rect.getWidth();
				break;
				
			default:
				TT_PANIC("Element '%s' has unknown horizontal position type: %d",
				         elem->getName().c_str(),
				         elem->getLayout().getHorizontalPositionType());
				break;
			}
		}
		
		if (centerChildren.empty() == false)
		{
			if (centerSize == (rightBorder - leftBorder))
			{
				positionLeftChildren(centerChildren, p_rect,
				                     leftBorder, rightBorder);
			}
			else
			{
				s32 center = (p_rect.getWidth() - centerSize) / 2;
				
				if (center <= leftBorder)
				{
					positionLeftChildren(centerChildren, p_rect,
					                     leftBorder, leftBorder + centerSize);
				}
				else if ((center + centerSize) >= rightBorder)
				{
					positionLeftChildren(centerChildren, p_rect,
					                     rightBorder - centerSize, rightBorder);
				}
				else
				{
					positionLeftChildren(centerChildren, p_rect,
					                     center, center + centerSize);
				}
			}
		}
	}
	
	// Now handle the height and vertical postion
	doSimpleVerticalLayoutChildren(p_elements, p_rect);
	
	// Now set up a rectangle for every child and have them handle their layout
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		const PointRect& rect((*it)->getRectangle());
		(*it)->doLayout(PointRect(Point2(0, 0),
		                          rect.getWidth(),
		                          rect.getHeight()));
	}
}


void MenuLayoutManager::doVerticalLayoutRemainingChildren(
		Elements&         p_elements,
		const PointRect&  p_rect,
		s32               p_min,
		s32               p_max)
{
	if (p_elements.empty())
	{
		return;
	}
	
	MENU_Printf("MenuLayoutManager::doVerticalLayoutRemainingChildren: "
	            "Do vertical layout for %u remaining "
	            "children over range %d - %d.\n",
	            p_elements.size(), p_min, p_max);
	
	// Tracks remaining available vertical space
	s32 availableSpace = p_max - p_min;
	
	
	{
		Elements maxChildren;
		
		// First, set size of each element to requestedsize
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			MenuElementInterface* elem = (*it);
			PointRect rect(elem->getRectangle());
			
			s32 reqH = elem->getRequestedHeight();
			availableSpace -= reqH;
			rect.setHeight(reqH);
			elem->setRectangle(rect);
			
			if (elem->getLayout().getHeightType() == MenuLayout::Size_Max)
			{
				maxChildren.push_back(elem);
			}
		}
		
		// Set size of max children
		if (maxChildren.empty() == false)
		{
			distributeMaxHeightChildren(maxChildren, availableSpace);
		}
	}
	
	
	{
		Elements centerChildren;
		
		// Do positioning for left and right children
		s32 topBorder    = p_min;
		s32 bottomBorder = p_max;
		s32 centerSize   = 0;
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			MenuElementInterface* elem = (*it);
			PointRect rect(elem->getRectangle());
			
			switch (elem->getLayout().getVerticalPositionType())
			{
			case MenuLayout::Position_Top:
				rect.setPosition(Point2(rect.getPosition().x,
				                        p_rect.getPosition().y + topBorder));
				elem->setRectangle(rect);
				topBorder += rect.getHeight();
				break;
				
			case MenuLayout::Position_Center:
				centerChildren.push_back(elem);
				centerSize += rect.getHeight();
				break;
				
			case MenuLayout::Position_Bottom:
				rect.setPosition(Point2(rect.getPosition().x, p_rect.getPosition().y +
					bottomBorder - rect.getHeight()));
				elem->setRectangle(rect);
				bottomBorder -= rect.getHeight();
				break;
				
			default:
				TT_PANIC("Element '%s' has unknown vertical position type: %d",
				         elem->getName().c_str(),
				         elem->getLayout().getVerticalPositionType());
				break;
			}
		}
		
		if (centerChildren.empty() == false)
		{
			if (centerSize == (bottomBorder - topBorder))
			{
				positionTopChildren(centerChildren, p_rect,
				                    topBorder, bottomBorder);
			}
			else
			{
				s32 center = (p_rect.getHeight() - centerSize) / 2;
				
				if (center <= topBorder)
				{
					positionTopChildren(centerChildren, p_rect,
					                    topBorder, topBorder + centerSize);
				}
				else if ((center + centerSize) >= bottomBorder)
				{
					positionBottomChildren(centerChildren, p_rect,
					                       bottomBorder - centerSize, bottomBorder);
				}
				else
				{
					positionTopChildren(centerChildren, p_rect,
					                    center, center + centerSize);
				}
			}
		}
	}
	
	// Now handle the height and vertical postion
	doSimpleHorizontalLayoutChildren(p_elements, p_rect);
	
	// Now set up a rectangle for every child and have them handle their layout
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		const PointRect& rect((*it)->getRectangle());
		(*it)->doLayout(PointRect(Point2(0, 0),
		                          rect.getWidth(), rect.getHeight()));
	}
}


void MenuLayoutManager::makeFit(const MenuLayout& p_parentLayout,
                                Elements&         p_elements,
                                const PointRect&  p_rect,
                                s32               p_min,
                                s32               p_max)
{
	if (p_parentLayout.getOrder() == MenuLayout::Order_Horizontal)
	{
		// First, distribute the available space amongst the children
		distributeWidthChildren(p_elements, p_max - p_min);
		
		// At this point, every child has its width set.
		// Now we need to set the horizontal position.
		positionLeftChildren(p_elements, p_rect, p_min, p_max);
		
		// Now handle the height and vertical postion
		doSimpleVerticalLayoutChildren(p_elements, p_rect);
		
		// Now set up a rectangle for every child
		// and have them handle their layout
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			const PointRect& rect((*it)->getRectangle());
			(*it)->doLayout(PointRect(Point2(0, 0),
			                          rect.getWidth(),
			                          rect.getHeight()));
		}
	}
	else if (p_parentLayout.getOrder() == MenuLayout::Order_Vertical)
	{
		// First, distribute the available space amongst the children
		distributeHeightChildren(p_elements, p_max - p_min);
		
		// At this point, every child has its height set.
		// Now we need to set the vertical position
		positionTopChildren(p_elements, p_rect, p_min, p_max);
		
		// Now handle the width and horizontal postion
		doSimpleHorizontalLayoutChildren(p_elements, p_rect);
		
		// Now set up a rectangle for every child and have them handle their layout
		for (Elements::iterator it = p_elements.begin();
		     it != p_elements.end(); ++it)
		{
			const PointRect& rect((*it)->getRectangle());
			(*it)->doLayout(PointRect(Point2(0, 0),
			                          rect.getWidth(),
			                          rect.getHeight()));
		}
	}
	else
	{
		TT_PANIC("Unknown element order: %d", p_parentLayout.getOrder());
	}
}


void MenuLayoutManager::positionLeftChildren(Elements&        p_elements,
                                             const PointRect& p_rect,
                                             s32              p_min,
                                             s32              p_max)
{
	s32 leftBorder  = p_min;
	s32 rightBorder = p_max;
	
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		switch ((*it)->getLayout().getHorizontalPositionType())
		{
		case MenuLayout::Position_Left:
		case MenuLayout::Position_Center:
			rect.setPosition(Point2(p_rect.getPosition().x + leftBorder,
			                        rect.getPosition().y));
			leftBorder += rect.getWidth();
			break;
			
		case MenuLayout::Position_Right:
			rect.setPosition(Point2(p_rect.getPosition().x +
				rightBorder - rect.getWidth(), rect.getPosition().y));
			rightBorder -= rect.getWidth();
			break;
			
		default:
			TT_PANIC("Element '%s': Unknown horizontal position type: %d",
			         (*it)->getName().c_str(),
			         (*it)->getLayout().getHorizontalPositionType());
			break;
		}
		
		(*it)->setRectangle(rect);
	}
}


void MenuLayoutManager::positionRightChildren(Elements&        p_elements,
                                              const PointRect& p_rect,
                                              s32              p_min,
                                              s32              p_max)
{
	s32 rightBorder = p_max;
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		// Set horizontal position
		rect.setPosition(Point2(p_rect.getPosition().x + rightBorder -
			rect.getWidth(), rect.getPosition().y));
		rightBorder -= rect.getWidth();
		
		(*it)->setRectangle(rect);
		
		if (rightBorder < p_min)
		{
			TT_PANIC("No space left for positioning right children.");
			break;
		}
	}
}


void MenuLayoutManager::positionTopChildren(Elements&        p_elements,
                                            const PointRect& p_rect,
                                            s32              p_min,
                                            s32              p_max)
{
	s32 topBorder = p_min;
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		// Set horizontal position
		rect.setPosition(Point2(rect.getPosition().x,
		                        p_rect.getPosition().y + topBorder));
		topBorder += rect.getHeight();
		
		(*it)->setRectangle(rect);
		
		if (topBorder > p_max)
		{
			TT_PANIC("No space left for positioning top children.");
			break;
		}
	}
}


void MenuLayoutManager::positionBottomChildren(Elements&        p_elements,
                                               const PointRect& p_rect,
                                               s32              p_min,
                                               s32              p_max)
{
	s32 bottomBorder = p_max;
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		// Set horizontal position
		rect.setPosition(Point2(rect.getPosition().x, p_rect.getPosition().y +
			bottomBorder - rect.getHeight()));
		bottomBorder -= rect.getHeight();
		
		(*it)->setRectangle(rect);
		
		if (bottomBorder < p_min)
		{
			TT_PANIC("No space left for positioning bottom children.");
			break;
		}
	}
}


void MenuLayoutManager::distributeWidthChildren(Elements& p_elements,
                                                s32       p_availableSpace)
{
	if (p_elements.empty())
	{
		return;
	}
	
	// First, give every child its minimum width
	Elements todo;
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		rect.setWidth((*it)->getMinimumWidth());
		p_availableSpace -= rect.getWidth();
		
		(*it)->setRectangle(rect);
		
		// If the elements wants to be larger than it is now,
		// add it to the todo vector
		if ((*it)->getRequestedWidth() > rect.getWidth())
		{
			todo.push_back(*it);
		}
		
		TT_ASSERTMSG(p_availableSpace >= 0,
		             "Width distribution failed on element '%s'. "
		             "Available space: %d",
		             (*it)->getName().c_str(), p_availableSpace);
	}
	
	// Now, determine smallest minimumwidth, and second smallest width
	Elements smallest;
	s32      smallestWidth       = std::numeric_limits<s32>::max();
	s32      secondSmallestWidth = std::numeric_limits<s32>::max();
	
	// First, determine smallest size and size above that
	for (Elements::iterator it = todo.begin(); it != todo.end(); ++it)
	{
		const PointRect& rect((*it)->getRectangle());
		
		if (rect.getWidth() < smallestWidth)
		{
			secondSmallestWidth = smallestWidth;
			smallestWidth       = rect.getWidth();
		}
	}
	
	// Then add all the elements with the smallest size to the vector
	for (Elements::iterator it = todo.begin(); it != todo.end(); ++it)
	{
		const PointRect& rect((*it)->getRectangle());
		
		if (rect.getWidth() == smallestWidth)
		{
			smallest.push_back(*it);
		}
	}
	
	// Now keep on distributing space until all is done
	while (p_availableSpace > 0)
	{
		s32 addSize;
		
		// First see if secondSmallestWidth has a valid value
		// If not, all elements have the same size
		if (secondSmallestWidth != std::numeric_limits<s32>::max())
		{
			addSize = secondSmallestWidth - smallestWidth;
		}
		else
		{
			// Divide available space among smallest elements
			addSize = p_availableSpace / static_cast<s32>(smallest.size());
		}
		
		// Divide available space
		for (Elements::iterator it = smallest.begin();
		     it != smallest.end(); ++it)
		{
			PointRect rect((*it)->getRectangle());
			
			// Check if the element will get its requested size
			s32 reqW = (*it)->getRequestedWidth();
			if ((rect.getWidth() + addSize) >= reqW)
			{
				p_availableSpace -= reqW - rect.getWidth();
				rect.setWidth(reqW);
			}
			else
			{
				p_availableSpace -= addSize;
				rect.setWidth(rect.getWidth() + addSize);
			}
			
			(*it)->setRectangle(rect);
			
			TT_ASSERTMSG(p_availableSpace >= 0,
			             "Width distribution failed on element '%s'. "
			             "Available space: %d",
			             (*it)->getName().c_str(), p_availableSpace);
		}
		
		// Update smallest list, and update todo list
		smallest.clear();
		
		s32 innerSmallestWidth       = std::numeric_limits<s32>::max();
		s32 innerSecondSmallestWidth = std::numeric_limits<s32>::max();
		
		// First, determine smallest size and size above that
		for (Elements::iterator it = todo.begin();
		     it != todo.end();)
		{
			const PointRect& rect((*it)->getRectangle());
			
			if (rect.getWidth() < innerSmallestWidth)
			{
				innerSecondSmallestWidth = innerSmallestWidth;
				innerSmallestWidth       = rect.getWidth();
			}
			
			// See if element has its required size
			if (rect.getWidth() == (*it)->getRequestedWidth())
			{
				// Remove from todo list if so
				it = todo.erase(it);
			}
			else
			{
				++it;
			}
		}
		
		// Then add all the elements with the smallest size to the vector
		for (Elements::iterator it = todo.begin();
		     it != todo.end(); ++it)
		{
			const PointRect& rect((*it)->getRectangle());
			
			if (rect.getWidth() == innerSmallestWidth)
			{
				smallest.push_back(*it);
			}
		}
		
		if (p_availableSpace <= static_cast<s32>(smallest.size()))
		{
			// Divide remaining space among elements
			for (Elements::iterator it = smallest.begin();
			     it != smallest.end() && p_availableSpace > 0; ++it)
			{
				PointRect rect((*it)->getRectangle());
				rect.setWidth(rect.getWidth() + 1);
				(*it)->setRectangle(rect);
				--p_availableSpace;
			}
		}
	}
}


void MenuLayoutManager::distributeHeightChildren(Elements& p_elements,
                                                 s32       p_availableSpace)
{
	if (p_elements.empty())
	{
		return;
	}
	
	MENU_Printf("MenuLayoutManager::distributeHeightChildren: "
	            "Distributing %d pixels among %u elements.\n",
	            p_availableSpace, p_elements.size());
	
	// First, give every child its minimum height
	Elements todo;
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		rect.setHeight((*it)->getMinimumHeight());
		MENU_Printf("MenuLayoutManager::distributeHeightChildren: "
		            "Giving element '%s' %d pixels. ",
		            (*it)->getName().c_str(), rect.getHeight());
		p_availableSpace -= rect.getHeight();
		MENU_Printf("Remaining: %d pixels.\n", p_availableSpace);
		
		(*it)->setRectangle(rect);
		
		// If the elements wants to be larger than it is now, add it to the todo vector
		if ((*it)->getRequestedHeight() > rect.getHeight())
		{
			todo.push_back(*it);
		}
		
		TT_ASSERTMSG(p_availableSpace >= 0,
		             "Height distribution failed on child '%s'. "
		             "Remaining space: %d pixels. Last child minimum height: "
		             "%d pixels.", (*it)->getName().c_str(), p_availableSpace,
		             rect.getHeight());
	}
	
	// Now, determine smallest minimumwidth, and second smallest width
	Elements smallest;
	s32      smallestHeight       = std::numeric_limits<s32>::max();
	s32      secondSmallestHeight = std::numeric_limits<s32>::max();
	
	// First, determine smallest size and size above that
	for (Elements::iterator it = todo.begin(); it != todo.end(); ++it)
	{
		const s32 height = (*it)->getRectangle().getHeight();
		if (height < smallestHeight)
		{
			secondSmallestHeight = smallestHeight;
			smallestHeight       = height;
		}
	}
	
	// Then add all the elements with the smallest size to the vector
	for (Elements::iterator it = todo.begin(); it != todo.end(); ++it)
	{
		if ((*it)->getRectangle().getHeight() == smallestHeight)
		{
			smallest.push_back(*it);
		}
	}
	
	// Now keep on distributing space until all is done
	while (p_availableSpace > 0)
	{
		s32 addSize;
		
		// First see if secondSmallestWidth has a valid value
		// If not, all elements have the same size
		if (secondSmallestHeight != std::numeric_limits<s32>::max())
		{
			addSize = secondSmallestHeight - smallestHeight;
		}
		else
		{
			// Divide available space among smallest elements
			addSize = p_availableSpace / static_cast<s32>(smallest.size());
		}
		
		// Divide available space
		for (Elements::iterator it = smallest.begin();
		     it != smallest.end(); ++it)
		{
			PointRect rect((*it)->getRectangle());
			
			// Check if the element will get its requested size
			const s32 reqH = (*it)->getRequestedHeight();
			if ((rect.getHeight() + addSize) >= reqH)
			{
				p_availableSpace -= reqH - rect.getHeight();
				rect.setHeight(reqH);
			}
			else
			{
				p_availableSpace -= addSize;
				rect.setHeight(rect.getHeight() + addSize);
			}
			
			(*it)->setRectangle(rect);
			
			TT_ASSERTMSG(p_availableSpace >= 0,
			             "Children height distribution failed on element '%s'. "
			             "Available space: %d",
			             (*it)->getName().c_str(), p_availableSpace);
		}
		
		// Update smallest list, and update todo list
		smallest.clear();
		
		s32 innerSmallestHeight       = std::numeric_limits<s32>::max();
		s32 innerSecondSmallestHeight = std::numeric_limits<s32>::max();
		
		// First, determine smallest size and size above that
		for (Elements::iterator it = todo.begin(); it != todo.end();)
		{
			const PointRect& rect((*it)->getRectangle());
			
			if (rect.getHeight() < innerSmallestHeight)
			{
				innerSecondSmallestHeight = innerSmallestHeight;
				innerSmallestHeight       = rect.getHeight();
			}
			
			// See if element has its required size
			if (rect.getHeight() == (*it)->getRequestedHeight())
			{
				// Remove from todo list if so
				it = todo.erase(it);
			}
			else
			{
				++it;
			}
		}
		
		// Then add all the elements with the smallest size to the vector
		for (Elements::iterator it = todo.begin(); it != todo.end(); ++it)
		{
			if ((*it)->getRectangle().getHeight() == innerSmallestHeight)
			{
				smallest.push_back(*it);
			}
		}
		
		if (p_availableSpace <= static_cast<s32>(smallest.size()))
		{
			// Divide remaining space among elements
			for (Elements::iterator it = smallest.begin();
			     it != smallest.end() && p_availableSpace > 0; ++it)
			{
				PointRect rect((*it)->getRectangle());
				rect.setHeight(rect.getHeight() + 1);
				(*it)->setRectangle(rect);
				--p_availableSpace;
			}
		}
	}
}


void MenuLayoutManager::distributeMaxWidthChildren(Elements& p_elements,
                                                   s32       p_availableSpace)
{
	if (p_elements.empty())
	{
		return;
	}
	
	MENU_Printf("MenuLayoutManager::distributeMaxWidthChildren: "
	            "Distribute max width %d over %d children.\n",
	            p_availableSpace, p_elements.size());
	
	Elements smallest;
	s32      smallestWidth       = std::numeric_limits<s32>::max();
	s32      secondSmallestWidth = std::numeric_limits<s32>::max();
	
	// First, determine smallest size and size above that
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		const PointRect& rect((*it)->getRectangle());
		
		TT_ASSERTMSG((*it)->getLayout().getWidthType() == MenuLayout::Size_Max,
		             "Child '%s' has width type other than MAX!",
		             (*it)->getName().c_str());
		
		if (rect.getWidth() < smallestWidth)
		{
			secondSmallestWidth = smallestWidth;
			smallestWidth       = rect.getWidth();
		}
	}
	
	TT_ASSERTMSG(smallestWidth != std::numeric_limits<s32>::max(),
	             "No smallest width found!");
	
	// Then add all the elements with the smallest size to the vector
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		if ((*it)->getRectangle().getWidth() == smallestWidth)
		{
			smallest.push_back(*it);
		}
	}
	
	// Now keep on distributing space until all is done
	while (p_availableSpace > 0)
	{
		// If all elements are of equal size,
		// smallest will have the same amount of elements as p_children
		if (smallest.size() < p_elements.size())
		{
			TT_ASSERTMSG(secondSmallestWidth != std::numeric_limits<s32>::max(),
			             "No second smallest width found!");
			
			// First increase the smallest elements in size so they will be
			// as large as the second smallest element
			
			// Check if there will be any space left after that
			if (p_availableSpace < (static_cast<s32>(smallest.size()) *
			    (secondSmallestWidth - smallestWidth)))
			{
				// If not, distribute the available space amongst the smallest children
				distributeWidthChildrenEqually(smallest, p_availableSpace);
			}
			else
			{
				// There will be space left, so increase smallest items
				// to be just as large as the second smallest items
				for (Elements::iterator it = smallest.begin();
				     it != smallest.end(); ++it)
				{
					PointRect rect((*it)->getRectangle());
					rect.setWidth(rect.getWidth() +
					              (secondSmallestWidth - smallestWidth));
					(*it)->setRectangle(rect);
					p_availableSpace -= (secondSmallestWidth - smallestWidth);
				}
				
				// Now clear the smallest vector, and rebuild it
				smallest.clear();
				for (Elements::iterator it = p_elements.begin();
				     it != p_elements.end(); ++it)
				{
					const PointRect& rect((*it)->getRectangle());
					
					TT_ASSERTMSG((*it)->getLayout().getWidthType() == MenuLayout::Size_Max,
					             "Child '%s': Width type is not MAX!",
					             (*it)->getName().c_str());
					
					if (rect.getWidth() < smallestWidth)
					{
						secondSmallestWidth = smallestWidth;
						smallestWidth       = rect.getWidth();
					}
				}
				
				// Then add all the elements with the smallest size to the vector
				for (Elements::iterator it = p_elements.begin();
				     it != p_elements.end(); ++it)
				{
					if ((*it)->getRectangle().getWidth() == smallestWidth)
					{
						smallest.push_back(*it);
					}
				}
			}
		}
		else
		{
			// Distribute remaining width amongst all children
			distributeWidthChildrenEqually(smallest, p_availableSpace);
			p_availableSpace = 0;
		}
	}
	
	// All done!
	TT_ASSERTMSG(p_availableSpace == 0, "Distributing max width failed "
	             "(%d pixels of available space remain).", p_availableSpace);
}


void MenuLayoutManager::distributeMaxHeightChildren(Elements& p_elements,
                                                    s32       p_availableSpace)
{
	if (p_elements.empty())
	{
		return;
	}
	
	MENU_Printf("MenuLayoutManager::distributeMaxHeightChildren: "
	            "Distribute max height %d over %u children.\n",
	            p_availableSpace, p_elements.size());
	
	s32      smallestHeight       = std::numeric_limits<s32>::max();
	s32      secondSmallestHeight = std::numeric_limits<s32>::max();
	Elements smallest;
	
	// First, determine smallest size and size above that
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		const PointRect& rect((*it)->getRectangle());
		
		TT_ASSERTMSG((*it)->getLayout().getHeightType() == MenuLayout::Size_Max,
		             "Child '%s' has height type other than MAX!",
		             (*it)->getName().c_str());
		
		if (rect.getHeight() < smallestHeight)
		{
			secondSmallestHeight = smallestHeight;
			smallestHeight       = rect.getHeight();
		}
	}
	
	TT_ASSERTMSG(smallestHeight != std::numeric_limits<s32>::max(),
	             "No smallest height found!");
	
	// Then add all the elements with the smallest size to the vector
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		if ((*it)->getRectangle().getHeight() == smallestHeight)
		{
			smallest.push_back(*it);
		}
	}
	
	// Now keep on distributing space until all is done
	while (p_availableSpace > 0)
	{
		// If all elements are of equal size,
		// smallest will have the same amount of elements as p_children
		if (smallest.size() < p_elements.size())
		{
			TT_ASSERTMSG(secondSmallestHeight != std::numeric_limits<s32>::max(),
			             "No second smallest height found!");
			
			// First increase the smallest elements in size so they will be
			// as large as the second smallest element
			
			// Check if there will be any space left after that
			if (p_availableSpace < (static_cast<s32>(smallest.size()) *
			    (secondSmallestHeight - smallestHeight)))
			{
				// If not, distribute the available space amongst the smallest children
				distributeHeightChildrenEqually(smallest, p_availableSpace);
			}
			else
			{
				// There will be space left, so increase smallest items
				// to be just as large as the second smallest items
				for (Elements::iterator it = smallest.begin();
				     it != smallest.end(); ++it)
				{
					PointRect rect((*it)->getRectangle());
					rect.setHeight(rect.getHeight() + (secondSmallestHeight - smallestHeight));
					(*it)->setRectangle(rect);
					p_availableSpace -= (secondSmallestHeight - smallestHeight);
				}
				
				// Now clear the smallest vector, and rebuild it
				smallest.clear();
				for (Elements::iterator it = p_elements.begin();
				     it != p_elements.end(); ++it)
				{
					const PointRect& rect((*it)->getRectangle());
					
					TT_ASSERTMSG((*it)->getLayout().getHeightType() == MenuLayout::Size_Max,
					                "Element '%s': Height type is not MAX!",
					                (*it)->getName().c_str());
					
					if (rect.getHeight() < smallestHeight)
					{
						secondSmallestHeight = smallestHeight;
						smallestHeight       = rect.getHeight();
					}
				}
				
				// Then add all the elements with the smallest size to the vector
				for (Elements::iterator it = p_elements.begin();
				     it != p_elements.end(); ++it)
				{
					if ((*it)->getRectangle().getHeight() == smallestHeight)
					{
						smallest.push_back(*it);
					}
				}
			}
		}
		else
		{
			// Distribute remaining height amongst all children
			distributeHeightChildrenEqually(smallest, p_availableSpace);
			p_availableSpace = 0;
		}
	}
	
	// All done!
	TT_ASSERTMSG(p_availableSpace == 0, "Distributing max height failed "
	             "(%d pixels of available space remain).", p_availableSpace);
}


void MenuLayoutManager::distributeWidthChildrenEqually(
		Elements& p_elements,
		s32       p_availableSpace)
{
	if (p_elements.empty())
	{
		return;
	}
	
	TT_ASSERTMSG(p_availableSpace >= 0,
	             "No space available to distribute width in.");
	
	// Amount of pixels available for each smallest element
	s32 perMax = p_availableSpace / static_cast<s32>(p_elements.size());
	
	// Remainder will be divided amongst first elements in line
	s32 remainder = p_availableSpace % static_cast<s32>(p_elements.size());
	
	s32 counter = 0;
	
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		rect.setWidth(rect.getWidth() + perMax);
		p_availableSpace -= perMax;
		
		// Divide remainder amongst first elements in line
		if (counter < remainder)
		{
			rect.setWidth(rect.getWidth() + 1);
			--p_availableSpace;
		}
		
		(*it)->setRectangle(rect);
		
		++counter;
		TT_ASSERTMSG(p_availableSpace >= 0, "Width distribution failed "
		             "(%d pixels of available space remain).",
		             p_availableSpace);
	}
}


void MenuLayoutManager::distributeHeightChildrenEqually(
		Elements& p_elements,
		s32       p_availableSpace)
{
	if (p_elements.empty())
	{
		return;
	}
	
	TT_ASSERTMSG(p_availableSpace >= 0,
	             "No space available to distribute width in.");
	
	// Amount of pixels available for each smallest element
	s32 perMax = p_availableSpace / static_cast<s32>(p_elements.size());
	
	// Remainder will be divided amongst first elements in line
	s32 remainder = p_availableSpace % static_cast<s32>(p_elements.size());
	
	s32 counter = 0;
	
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		rect.setHeight(rect.getHeight() + perMax);
		p_availableSpace -= perMax;
		
		// Divide remainder amongst first elements in line
		if (counter < remainder)
		{
			rect.setHeight(rect.getHeight() + 1);
			--p_availableSpace;
		}
		
		(*it)->setRectangle(rect);
		
		++counter;
		TT_ASSERTMSG(p_availableSpace >= 0, "Height distribution failed "
		             "(%d pixels of available space remain).",
		             p_availableSpace);
	}
}


void MenuLayoutManager::doSimpleVerticalLayoutChildren(
		Elements&        p_elements,
		const PointRect& p_rect)
{
	// Set up height first
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		// Make sure it fits
		TT_ASSERTMSG(((*it)->getMinimumHeight() +
		              (*it)->getRequestedVerticalPosition()) <=
		             p_rect.getHeight(),
		             "Child '%s' is too high.",
		             (*it)->getName().c_str());
		
		// Set up height
		switch ((*it)->getLayout().getHeightType())
		{
		case MenuLayout::Size_Absolute:
		case MenuLayout::Size_Auto:
			if (((*it)->getRequestedHeight() +
			     (*it)->getRequestedVerticalPosition()) <=
			    p_rect.getHeight())
			{
				// If it fits, give it its requested height
				rect.setHeight((*it)->getRequestedHeight());
			}
			else
			{
				// If not, give it as much height as possible
				rect.setHeight(p_rect.getHeight() -
				               (*it)->getRequestedVerticalPosition());
			}
			break;
			
		case MenuLayout::Size_Max:
			// Give it as much height as possible
			rect.setHeight(p_rect.getHeight() -
			               (*it)->getRequestedVerticalPosition());
			break;
			
		default:
			TT_PANIC("Element '%s' has unknown height type: %d",
			         (*it)->getName().c_str(),
			         (*it)->getLayout().getHeightType());
			break;
		}
		
		// Now set the vertical position
		switch ((*it)->getLayout().getVerticalPositionType())
		{
		case MenuLayout::Position_Top:
			rect.setPosition(Point2(rect.getPosition().x, p_rect.getPosition().y));
			break;
			
		case MenuLayout::Position_Center:
			rect.setPosition(Point2(rect.getPosition().x, p_rect.getPosition().y +
				((p_rect.getHeight() - rect.getHeight()) / 2)));
			break;
			
		case MenuLayout::Position_Bottom:
			rect.setPosition(Point2(rect.getPosition().x, p_rect.getPosition().y +
				p_rect.getHeight() - rect.getHeight()));
			break;
			
		default:
			TT_PANIC("Element '%s' has unknown vertical position type: %d",
			         (*it)->getName().c_str(),
			         (*it)->getLayout().getVerticalPositionType());
			break;
		}
		
		(*it)->setRectangle(rect);
	}
}


void MenuLayoutManager::doSimpleHorizontalLayoutChildren(
		Elements&        p_elements,
		const PointRect& p_rect)
{
	// Set up width first
	for (Elements::iterator it = p_elements.begin();
	     it != p_elements.end(); ++it)
	{
		PointRect rect((*it)->getRectangle());
		
		// Make sure it fits
		TT_ASSERTMSG(((*it)->getMinimumWidth() +
		              (*it)->getRequestedHorizontalPosition()) <=
		             p_rect.getWidth(),
		             "Can't make element '%s' fit. The element is too wide. "
		             "Available width: %d. Child needs a minimum width of %d, "
		             "starting from a (requested) X pos of %d.",
		             (*it)->getName().c_str(),
		             p_rect.getWidth(), (*it)->getMinimumWidth(),
		             (*it)->getRequestedHorizontalPosition());
		
		// Set up width
		switch ((*it)->getLayout().getWidthType())
		{
		case MenuLayout::Size_Absolute:
		case MenuLayout::Size_Auto:
			if (((*it)->getRequestedWidth() +
			     (*it)->getRequestedHorizontalPosition()) <=
			    p_rect.getWidth())
			{
				// If it fits, give it its requested width
				rect.setWidth((*it)->getRequestedWidth());
			}
			else
			{
				// If not, give it as much width as possible
				rect.setWidth(p_rect.getWidth() -
				              (*it)->getRequestedHorizontalPosition());
			}
			break;
			
		case MenuLayout::Size_Max:
			// Give it as much width as possible
			rect.setWidth(p_rect.getWidth() -
			              (*it)->getRequestedHorizontalPosition());
			break;
			
		default:
			TT_PANIC("Element '%s' has unknown width type: %d",
			         (*it)->getName().c_str(),
			         (*it)->getLayout().getWidthType());
			break;
		}
		
		// Now set the horizontal position
		switch ((*it)->getLayout().getHorizontalPositionType())
		{
		case MenuLayout::Position_Left:
			rect.setPosition(Point2(p_rect.getPosition().x, rect.getPosition().y));
			break;
			
		case MenuLayout::Position_Center:
			rect.setPosition(Point2(p_rect.getPosition().x +
				((p_rect.getWidth() - rect.getWidth()) / 2), rect.getPosition().y));
			break;
			
		case MenuLayout::Position_Right:
			rect.setPosition(Point2(p_rect.getPosition().x +
				(p_rect.getWidth() - rect.getWidth()), rect.getPosition().y));
			break;
			
		default:
			TT_PANIC("Element '%s' has unknown horizontal position type: %d",
			         (*it)->getName().c_str(),
			         (*it)->getLayout().getHorizontalPositionType());
			break;
		}
		
		(*it)->setRectangle(rect);
	}
}


MenuLayoutManager::MenuLayoutManager()
{
}


MenuLayoutManager::~MenuLayoutManager()
{
}

// Namespace end
}
}
