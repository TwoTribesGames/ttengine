#include <tt/platform/tt_error.h>
#include <tt/math/Random.h>

#include <tt/menu/elements/Window.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/elements/ElementLayout.h>
#include <tt/menu/elements/DynamicLabel.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/SkinElementIDs.h>

#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

Window::Window(const std::string& p_name,
               const MenuLayout&  p_layout,
               const std::string& p_caption,
               bool               p_hasPoles)
:
ContainerBase<>(p_name, p_layout),
m_hasPoles(p_hasPoles),
m_innerRect(math::Point2(0, 0), 1, 1),
m_borderRemainder(6),
m_label(0),
m_leftRot(0),
m_rightRot(0)
{
	MENU_CREATION_Printf("Window::Window: Element '%s': New Window with "
	                     "caption '%s'.\n", getName().c_str(),
	                     p_caption.c_str());
	
	// Get skinning information for Window
	MenuSystem*     sys  = MenuSystem::getInstance();
	const MenuSkin* skin = sys->getSkin();
	
	TT_ASSERTMSG(skin != 0, "Cannot create menu elements without a menu skin.");
	TT_ASSERTMSG(skin->hasElementSkin(SkinElement_Window),
	             "Skin does not provide skinning information for Window.");
	
	const MenuSkin::ElementSkin& element(
		skin->getElementSkin(SkinElement_Window));
	const MenuSkin::SkinTexture& skinTexture(
		element.getTexture(WindowSkin_BgTexture));
	
	// FIXME: MUST SUPPORT NAMESPACES
	m_texture = engine::renderer::TextureCache::get(skinTexture.getFilename(),"");
	if(m_texture == 0)
	{
		TT_PANIC("Loading window background texture '%s' failed.",
		         skinTexture.getFilename().c_str());
	}
	
	if (p_caption.empty() == false)
	{
		// TODO: fix layout
		// this will go wrong if size is absolute
		MenuLayout labelLayout;
		labelLayout.setWidthType(MenuLayout::Size_Max);
		labelLayout.setHeightType(MenuLayout::Size_Auto);
		labelLayout.setVerticalPositionType(MenuLayout::Position_Center);
		m_label = new DynamicLabel("", labelLayout, p_caption);
		m_label->setTextColor(element.getVertexColor(WindowSkin_CaptionColor));
		m_label->setHorizontalAlign(engine::glyph::GlyphSet::ALIGN_CENTER);
	}
	
	setWindowTexture(skinTexture.getU(),     skinTexture.getV(),
	                 skinTexture.getWidth(), skinTexture.getHeight(),
	                 element.getVertexColor(WindowSkin_BgColor));
	
	
	// Add the decals
	s32 decalIdx = 0;
	for (int i = WindowSkin_DecalTopTexture;
	     i <= WindowSkin_DecalRightTexture; ++i, ++decalIdx)
	{
		const MenuSkin::SkinTexture& decalSkin(element.getTexture(i));

		// FIXME: MUST SUPPORT NAMESPACES
		engine::renderer::TexturePtr decalTexture = 
			engine::renderer::TextureCache::get(decalSkin.getFilename(), "");
		if(decalTexture == 0)
		{
			TT_PANIC("Loading window decal texture '%s' failed.",
			         decalSkin.getFilename().c_str());
		}
		
		using math::Vector2;
		using math::VectorRect;
		addDecal(static_cast<Decal>(decalIdx),
		         decalTexture,
		         VectorRect(Vector2(decalSkin.getU(), decalSkin.getV()),
		                    decalSkin.getWidth(), decalSkin.getHeight()),
		         element.getVertexColor(i));
	}
	
	// Set the base depth of this window
	// - Poles take one level
	// - Border/background takes one level
	// - Decals take one level
	s32 baseDepth = 2;
	if (m_hasPoles)
	{
		++baseDepth;
	}
	setDepth(baseDepth);
}


Window::Window(const std::string&  p_name,
               const MenuLayout&   p_layout,
               const std::wstring& p_caption,
               bool                p_hasPoles)
:
ContainerBase<>(p_name, p_layout),
m_hasPoles(p_hasPoles),
m_innerRect(math::Point2(0, 0), 1, 1),
m_borderRemainder(6),
m_label(0),
m_leftRot(0),
m_rightRot(0)
{
	MENU_CREATION_Printf("Window::Window: Element '%s': New Window with "
	                     "literal caption.\n", getName().c_str());
	
	// Get skinning information for Window
	MenuSystem*     sys  = MenuSystem::getInstance();
	const MenuSkin* skin = sys->getSkin();
	
	TT_ASSERTMSG(skin != 0, "Cannot create menu elements without a menu skin.");
	TT_ASSERTMSG(skin->hasElementSkin(SkinElement_Window),
	             "Skin does not provide skinning information for Window.");
	
	const MenuSkin::ElementSkin& element(
		skin->getElementSkin(SkinElement_Window));
	const MenuSkin::SkinTexture& skinTexture(
		element.getTexture(WindowSkin_BgTexture));
	
	// FIXME: MUST SUPPORT NAMESPACES
	m_texture = engine::renderer::TextureCache::get(skinTexture.getFilename(), "");
	if(m_texture == 0)
	{
		TT_PANIC("Loading window background texture '%s' failed.",
		         skinTexture.getFilename().c_str());
	}
	
	if (p_caption.empty() == false)
	{
		// TODO: fix layout
		// this will go wrong if size is absolute
		MenuLayout labelLayout;
		labelLayout.setWidthType(MenuLayout::Size_Max);
		labelLayout.setHeightType(MenuLayout::Size_Auto);
		labelLayout.setVerticalPositionType(MenuLayout::Position_Center);
		m_label = new DynamicLabel("", labelLayout, p_caption);
		m_label->setTextColor(element.getVertexColor(WindowSkin_CaptionColor));
		m_label->setHorizontalAlign(engine::glyph::GlyphSet::ALIGN_CENTER);
	}
	
	setWindowTexture(skinTexture.getU(),     skinTexture.getV(),
	                 skinTexture.getWidth(), skinTexture.getHeight(),
	                 element.getVertexColor(WindowSkin_BgColor));
	
	
	// Add the decals
	s32 decalIdx = 0;
	for (int i = WindowSkin_DecalTopTexture;
	     i <= WindowSkin_DecalRightTexture; ++i, ++decalIdx)
	{
		const MenuSkin::SkinTexture& decalSkin(element.getTexture(i));

		// FIXME: MUST SUPPORT NAMESPACES
		engine::renderer::TexturePtr decalTexture = 
			engine::renderer::TextureCache::get(decalSkin.getFilename(), "");
		{
			TT_PANIC("Loading window decal texture '%s' failed.",
			         decalSkin.getFilename().c_str());
		}
		
		using math::Vector2;
		using math::VectorRect;
		addDecal(static_cast<Decal>(decalIdx),
		         decalTexture,
		         VectorRect(Vector2(decalSkin.getU(), decalSkin.getV()),
		                    decalSkin.getWidth(), decalSkin.getHeight()),
		         element.getVertexColor(i));
	}
	
	// Set the base depth of this window
	// - Poles take one level
	// - Border/background takes one level
	// - Decals take one level
	s32 baseDepth = 2;
	if (m_hasPoles)
	{
		++baseDepth;
	}
	setDepth(baseDepth);
}


Window::~Window()
{
	MENU_CREATION_Printf("Window::~Window: Element '%s': Freeing resources.\n",
	                     getName().c_str());
	
	delete m_label;
}


void Window::loadResources()
{
	if (m_label != 0)
	{
		m_label->loadResources();
	}
	ContainerBase<>::loadResources();
}


void Window::unloadResources()
{
	if (m_label != 0)
	{
		m_label->unloadResources();
	}
	ContainerBase<>::unloadResources();
}


void Window::render(const PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false)
	{
		return;
	}
	
	// Determine the inner rectangle
	s32 labelHeight = 0;
	
	if (m_label != 0)
	{
		labelHeight = m_label->getRectangle().getHeight() +
		              TITLE_BOTTOM_PADDING;
	}
	
	m_innerRect.setWidth(p_rect.getWidth()  -
		static_cast<s32>((m_blockSize.x - m_borderRemainder) * 2));
	m_innerRect.setHeight(p_rect.getHeight() -
		static_cast<s32>((m_blockSize.y - m_borderRemainder) * 2) -
		labelHeight);
	
	m_innerRect.setPosition(math::Point2(
		p_rect.getPosition().x + static_cast<s32>(m_blockSize.x - m_borderRemainder),
		p_rect.getPosition().y + static_cast<s32>(m_blockSize.y - m_borderRemainder) +
		labelHeight));
	
	
	// ---------- Update the background and border ----------
	
	math::Vector3 pos(static_cast<real>(p_rect.getCenterPosition().x),
	                  static_cast<real>(p_rect.getCenterPosition().y),
	                  static_cast<real>(p_z));
	
	m_size.x = static_cast<real>(m_innerRect.getWidth());
	m_size.y = static_cast<real>(m_innerRect.getHeight() + labelHeight);
	
	// Readability shorts
	real xp = math::getHalf(m_size.x + m_blockSize.x) -
	          static_cast<real>(m_borderRemainder);
	real yp = math::getHalf(m_size.y + m_blockSize.y) -
	          static_cast<real>(m_borderRemainder);
	
	// Center block
	m_windowSegs[WindowSection_Center]->setPosition(pos);
	m_windowSegs[WindowSection_Center]->setWidth(
		m_size.x - math::getTimesTwo(m_borderRemainder));
	m_windowSegs[WindowSection_Center]->setHeight(
		m_size.y - math::getTimesTwo(m_borderRemainder));
	
	
	// Top edge
	m_windowSegs[WindowSection_EdgeTop]->setPosition(
		pos + math::Vector3(0.0f, -yp, 0.0f));
	m_windowSegs[WindowSection_EdgeTop]->setWidth(
		m_size.x - math::getTimesTwo(m_borderRemainder));
	m_windowSegs[WindowSection_EdgeTop]->setHeight(
		m_blockSize.y);
	
	// Bottom edge
	m_windowSegs[WindowSection_EdgeBottom]->setPosition(
		pos + math::Vector3(0.0f, yp, 0.0f));
	m_windowSegs[WindowSection_EdgeBottom]->setWidth(
		m_size.x - math::getTimesTwo(m_borderRemainder));
	m_windowSegs[WindowSection_EdgeBottom]->setHeight(m_blockSize.y);
	
	// Left edge
	m_windowSegs[WindowSection_EdgeLeft]->setPosition(
		pos + math::Vector3(-xp, 0.0f, 0.0f));
	m_windowSegs[WindowSection_EdgeLeft]->setWidth(m_blockSize.x);
	m_windowSegs[WindowSection_EdgeLeft]->setHeight(
		m_size.y - math::getTimesTwo(m_borderRemainder));
	
	// Right edge
	m_windowSegs[WindowSection_EdgeRight]->setPosition(
		pos + math::Vector3(xp, 0.0f, 0.0f));
	m_windowSegs[WindowSection_EdgeRight]->setWidth(m_blockSize.x);
	m_windowSegs[WindowSection_EdgeRight]->setHeight(
		m_size.y - math::getTimesTwo(m_borderRemainder));
	
	
	// Corner top left
	m_windowSegs[WindowSection_CornerTopLeft]->setPosition(
		pos + math::Vector3(-xp, -yp, 0.0f));
	m_windowSegs[WindowSection_CornerTopLeft]->setWidth(m_blockSize.x);
	m_windowSegs[WindowSection_CornerTopLeft]->setHeight(m_blockSize.y);
	
	// Corner top right
	m_windowSegs[WindowSection_CornerTopRight]->setPosition(
		pos + math::Vector3(xp, -yp, 0.0f));
	m_windowSegs[WindowSection_CornerTopRight]->setWidth(m_blockSize.x);
	m_windowSegs[WindowSection_CornerTopRight]->setHeight(m_blockSize.y);
	
	// Corner bottom left
	m_windowSegs[WindowSection_CornerBottomLeft]->setPosition(
		pos + math::Vector3(-xp, yp, 0.0f));
	m_windowSegs[WindowSection_CornerBottomLeft]->setWidth(m_blockSize.x);
	m_windowSegs[WindowSection_CornerBottomLeft]->setHeight(m_blockSize.y);
	
	// Corner bottom right
	m_windowSegs[WindowSection_CornerBottomRight]->setPosition(
		pos + math::Vector3(xp, yp, 0.0f));
	m_windowSegs[WindowSection_CornerBottomRight]->setWidth(m_blockSize.x);
	m_windowSegs[WindowSection_CornerBottomRight]->setHeight(m_blockSize.y);
	
	
	
	// ---------- Update the decals ----------
	
	const real decalZ = static_cast<real>(p_z - 1);
	m_decals[Decal_Top]->setPosition(
		static_cast<real>(p_rect.getCenterPosition().x + m_decalOffsets[Decal_Top]),
		static_cast<real>(p_rect.getPosition().y +
		                  math::getHalf(m_decals[Decal_Top]->getHeight())),
		decalZ);
	
	m_decals[Decal_Bottom]->setPosition(
		static_cast<real>(p_rect.getCenterPosition().x + m_decalOffsets[Decal_Bottom]),
		static_cast<real>(p_rect.getBottom() -
		                  math::getHalf(m_decals[Decal_Bottom]->getHeight())),
		decalZ);
	
	m_decals[Decal_Left]->setPosition(
		static_cast<real>(p_rect.getPosition().x +
		                  math::getHalf(m_decals[Decal_Left]->getWidth())),
		static_cast<real>(p_rect.getCenterPosition().y + m_decalOffsets[Decal_Left]),
		decalZ);
	
	m_decals[Decal_Right]->setPosition(
		static_cast<real>(p_rect.getRight() -
		                  math::getHalf(m_decals[Decal_Right]->getWidth())),
		static_cast<real>(p_rect.getCenterPosition().y + m_decalOffsets[Decal_Right]),
		decalZ);
	
	
	
	// ---------- Update the poles ----------
	
	if (m_hasPoles)
	{
		// FIXME: Remove hard-coded screen dimensions.
		real poleHeight = static_cast<real>(192 - p_rect.getPosition().y -
		                                    p_rect.getHeight());
		
		if (m_singlePole)
		{
			xp = 0;
		}
		else
		{
			xp = static_cast<real>(p_rect.getWidth() / 2) -
			     m_poleBlockSize.x;
		}
		yp = math::getHalf(static_cast<real>(p_rect.getHeight()) +
		                   m_poleBlockSize.y);
		real yp2 = math::getHalf(static_cast<real>(p_rect.getHeight()) +
		                         poleHeight);
		
		if (m_singlePole == false || m_drawLeftPole)
		{
			// Calculate how many pixels the center of the pole should be
			// translated so the bottom pole will still connect properly
			// to the window
			real leftOff = math::getHalf(poleHeight);
			leftOff *= math::tan(m_leftRot);
			leftOff += xp;
			
			// Rotating will cause gaps in the pole at the bottom and top.
			// Calculate how many pixels are needed to compensate this
			real poleRemainder = m_poleBlockSize.x;
			poleRemainder *= math::tan(m_leftRot);
			
			// Calculate vertical distance between top of bottom pole (xp) and
			// center of top pole
			real topOff = static_cast<real>(p_rect.getHeight()) +
			              math::getHalf(m_poleBlockSize.y) - poleRemainder;
			topOff *= math::tan(m_leftRot);
			topOff = xp - topOff;
			
			// Left pole bottom
			m_poleSegs[PoleSection_LeftBottom]->setPosition(
				pos + math::Vector3(-leftOff, yp2, 1.0f));
			m_poleSegs[PoleSection_LeftBottom]->setWidth(m_poleBlockSize.x);
			m_poleSegs[PoleSection_LeftBottom]->setHeight(
				poleHeight + poleRemainder + 0.5f);
			m_poleSegs[PoleSection_LeftBottom]->setRotation(m_leftRot);
			
			// Left pole top
			m_poleSegs[PoleSection_LeftTop]->setPosition(
				pos + math::Vector3(-topOff, -yp + poleRemainder, 1.0f));
			m_poleSegs[PoleSection_LeftTop]->setWidth(m_poleBlockSize.x);
			m_poleSegs[PoleSection_LeftTop]->setHeight(m_poleBlockSize.y);
			m_poleSegs[PoleSection_LeftTop]->setRotation(m_leftRot);
		}
		
		if (m_singlePole == false || m_drawLeftPole == false)
		{
			// Calculate how many pixels the center of the pole should be
			// translated so the bottom pole will still connect properly
			// to the window
			real rightOff = math::getHalf(poleHeight);
			rightOff *= math::tan(m_rightRot);
			rightOff += xp;
			
			// Rotating will cause gaps in the pole at the bottom and top.
			// Calculate how many pixels are needed to compensate this
			real poleRemainder = m_poleBlockSize.x;
			poleRemainder *= math::tan(m_rightRot);
			
			// Calculate vertical distance between top of bottom pole (xp) and
			// center of top pole
			real topOff = static_cast<real>(p_rect.getHeight()) +
			              math::getHalf(m_poleBlockSize.y) - poleRemainder;
			topOff *= math::tan(m_rightRot);
			topOff = xp - topOff;
			
			// Right pole bottom
			m_poleSegs[PoleSection_RightBottom]->setPosition(
				pos + math::Vector3(rightOff, yp2, 1.0f));
			m_poleSegs[PoleSection_RightBottom]->setWidth(m_poleBlockSize.x);
			m_poleSegs[PoleSection_RightBottom]->setHeight(
				poleHeight + poleRemainder + 0.5f);
			m_poleSegs[PoleSection_RightBottom]->setRotation(-m_rightRot);
			
			// Right pole top
			m_poleSegs[PoleSection_RightTop]->setPosition(
				pos + math::Vector3(topOff, -yp + poleRemainder, 1.0f));
			m_poleSegs[PoleSection_RightTop]->setWidth(m_poleBlockSize.x);
			m_poleSegs[PoleSection_RightTop]->setHeight(m_poleBlockSize.y);
			m_poleSegs[PoleSection_RightTop]->setRotation(-m_rightRot);
		}
	}
	
	
	// Render the window background and border
	for (int i = 0; i < WindowSection_Count; ++i)
	{
		m_windowSegs[i]->update();
		m_windowSegs[i]->render();
	}
	
	// Render the decals
	for (int i = 0; i < Decal_Count; ++i)
	{
		m_decals[i]->update();
		m_decals[i]->render();
	}
	
	if (m_hasPoles)
	{
		// Render the poles
		for (int i = 0; i < PoleSection_Count; ++i)
		{
			if (m_poleSegs[i] != 0)
			{
				m_poleSegs[i]->update();
				m_poleSegs[i]->render();
			}
		}
	}
	
	ContainerBase<>::render(m_innerRect, p_z - 2);
	
	if (m_label != 0)
	{
		/*
		PointRect labelRect;
		labelRect.setHeight(labelHeight);
		labelRect.setWidth(p_rect.getWidth()  -
		                   ((m_blockSize.x - m_borderRemainder) * 2));
		labelRect.setX(p_rect.getX() +
		               (m_blockSize.x - m_borderRemainder));
		labelRect.setY(p_rect.getY() +
		               (m_blockSize.y - m_borderRemainder));
		*/
		
		PointRect labelRect(m_label->getRectangle());
		labelRect.translate(p_rect.getPosition());
		/*
		if (labelRect.getWidth() > m_label->getCaptionWidth())
		{
			labelRect.translate((labelRect.getWidth() - m_label->getCaptionWidth()) / 2, 0);
			labelRect.setWidth(m_label->getCaptionWidth());
		}
		//*/
		
		m_label->render(labelRect, p_z - 1);
	}
}


void Window::doLayout(const PointRect& p_rect)
{
	MENU_Printf("Window::doLayout: Element '%s': Rect dimensions: (%d, %d).\n",
	            getName().c_str(), p_rect.getWidth(), p_rect.getHeight());
	
	// Remove size taken by the border from the layout rect
	PointRect rect(p_rect.getPosition(),
		p_rect.getWidth()  - static_cast<s32>((m_blockSize.x - m_borderRemainder) * 2),
		p_rect.getHeight() - static_cast<s32>((m_blockSize.y - m_borderRemainder) * 2));
	
	if (m_label != 0)
	{
		// Remove the size taken by the label from the layout rect
		s32 labelReqH = m_label->getRequestedHeight();
		rect.setHeight(rect.getHeight() - labelReqH - TITLE_BOTTOM_PADDING);
		MENU_Printf("Window::doLayout: Element '%s': "
		            "Set inner rectangle height to %d.\n",
		            getName().c_str(), rect.getHeight());
		
		// Set the rectangle for the label
		PointRect labelRect(m_label->getRectangle());
		
		labelRect.setPosition(math::Point2(
			static_cast<s32>(m_blockSize.x - m_borderRemainder),
			static_cast<s32>(m_blockSize.y - m_borderRemainder)));
		labelRect.setWidth(p_rect.getWidth() -
			static_cast<s32>((m_blockSize.x - m_borderRemainder) * 2));
		labelRect.setHeight(labelReqH);
		
		m_label->setRectangle(labelRect);
		
		// Have the layout perform its layout
		m_label->doLayout(labelRect);
	}
	
	
	// Kill any existing pole segments
	for (int i = 0; i < PoleSection_Count; ++i)
	{
		m_poleSegs[i].reset();
	}
	
	
	using math::Random;
	Random& rng = Random::getStatic();
	
	
	// Randomize decal offsets
	{
		// Maximum offsets from center
		real maxX = (p_rect.getWidth()  * 0.5f) - m_blockSize.x;
		real maxY = (p_rect.getHeight() * 0.5f) - m_blockSize.y;
		
		// Relevant dimensions of the decals
		real topW    = m_decals[Decal_Top   ]->getWidth();
		real bottomW = m_decals[Decal_Bottom]->getWidth();
		real leftH   = m_decals[Decal_Left  ]->getHeight();
		real rightH  = m_decals[Decal_Right ]->getHeight();
		
		// Available space to move in
		real topSpace    = maxX - topW;
		real bottomSpace = maxX - bottomW;
		real leftSpace   = maxY - leftH;
		real rightSpace  = maxY - rightH;
		
		m_decalOffsets[Decal_Top   ] = -topSpace +
			static_cast<real>(rng.getNext(static_cast<u32>(2 * topSpace)));
		m_decalOffsets[Decal_Bottom] = -bottomSpace +
			static_cast<real>(rng.getNext(static_cast<u32>(2 * bottomSpace)));
		m_decalOffsets[Decal_Left  ] = -leftSpace +
			static_cast<real>(rng.getNext(static_cast<u32>(2 * leftSpace)));
		m_decalOffsets[Decal_Right ] = -rightSpace +
			static_cast<real>(rng.getNext(static_cast<u32>(2 * rightSpace)));
	}
	
	
	if (m_hasPoles)
	{
		// Get skinning information for Window poles
		MenuSystem*     sys  = MenuSystem::getInstance();
		const MenuSkin* skin = sys->getSkin();
		
		TT_ASSERTMSG(skin != 0,
		             "Cannot create menu elements without a menu skin.");
		TT_ASSERTMSG(skin->hasElementSkin(SkinElement_Window),
		             "Skin does not provide skinning information for Window.");
		
		const MenuSkin::ElementSkin& element(
			skin->getElementSkin(SkinElement_Window));
		const MenuSkin::SkinTexture& skinTexture(
			element.getTexture(WindowSkin_PoleTexture));
		
		// Determine how to draw the poles, and how many to draw
		m_singlePole = (p_rect.getWidth() < (skinTexture.getWidth() * 5));
		
		if (m_singlePole)
		{
			m_leftRot       = static_cast<real>(rng.getNext(64));
			m_rightRot      = static_cast<real>(rng.getNext(64));
			m_drawLeftPole = (rng.getNext(64) > 32);
		}
		else
		{
			m_leftRot  = static_cast<real>(rng.getNext(128));
			m_rightRot = static_cast<real>(rng.getNext(128));
		}
		
		// Load the texture for the poles
		// FIXME: MUST SUPPORT NAMESPACES
		m_poleTexture = engine::renderer::TextureCache::get(skinTexture.getFilename(), "");
		if(m_poleTexture == 0)
		{
			TT_PANIC("Loading window pole texture '%s' failed.",
			         skinTexture.getFilename().c_str());
		}
		
		// Create the pole quads
		setPoleTexture(skinTexture.getU(),     skinTexture.getV(),
		               skinTexture.getWidth(), skinTexture.getHeight(),
		               WINDOW_POLE_TOPS,
		               element.getVertexColor(WindowSkin_PoleColor));
	}
	
	// Lay out the children
	ContainerBase<>::doLayout(rect);
}


bool Window::doAction(const MenuElementAction& p_action)
{
	// Let base handle the action first
	if (ContainerBase<>::doAction(p_action))
	{
		return true;
	}
	
	// Let the caption label handle any remaining actions
	if (m_label != 0)
	{
		return m_label->doAction(p_action);
	}
	
	return false;
}


s32 Window::getMinimumWidth() const
{
	if (getLayout().getWidthType() != MenuLayout::Size_Absolute)
	{
		s32 minWidth = ContainerBase<>::getMinimumWidth();
		
		if (m_label != 0)
		{
			s32 labelMin = m_label->getMinimumWidth();
			if ((labelMin + ((m_blockSize.x - m_borderRemainder) * 2)) > minWidth)
			{
				minWidth = labelMin;
			}
		}
		
		minWidth += static_cast<s32>((m_blockSize.x - m_borderRemainder) * 2);
		
		// Minimum width can be no smaller than the borders
		minWidth = std::max(minWidth, static_cast<s32>(m_blockSize.x * 2));
		return minWidth;
	}
	else
	{
		return ContainerBase<>::getMinimumWidth();
	}
}


s32 Window::getMinimumHeight() const
{
	if (getLayout().getHeightType() != MenuLayout::Size_Absolute)
	{
		s32 minHeight = ContainerBase<>::getMinimumHeight() +
			static_cast<s32>((m_blockSize.y - m_borderRemainder) * 2);
		minHeight = std::max(minHeight, static_cast<s32>(m_blockSize.y * 2));
		
		if (m_label != 0)
		{
			minHeight += m_label->getMinimumHeight() + TITLE_BOTTOM_PADDING;
		}
		
		return minHeight;
	}
	else
	{
		return ContainerBase<>::getMinimumHeight();
	}
}


s32 Window::getRequestedWidth() const
{
	if (getLayout().getWidthType() != MenuLayout::Size_Absolute)
	{
		s32 reqWidth = ContainerBase<>::getRequestedWidth();
		
		if (m_label != 0)
		{
			s32 labelReq = m_label->getRequestedWidth();
			if ((labelReq + ((m_blockSize.x - m_borderRemainder) * 2)) > reqWidth)
			{
				reqWidth = labelReq;
			}
		}
		
		reqWidth += static_cast<s32>((m_blockSize.x - m_borderRemainder) * 2);
		
		reqWidth = std::max(reqWidth, static_cast<s32>(m_blockSize.x * 2));
		return reqWidth;
	}
	else
	{
		return ContainerBase<>::getRequestedWidth();
	}
}


s32 Window::getRequestedHeight() const
{
	if (getLayout().getHeightType() != MenuLayout::Size_Absolute)
	{
		s32 reqHeight = ContainerBase<>::getRequestedHeight() +
			static_cast<s32>((m_blockSize.y - m_borderRemainder) * 2);
		reqHeight = std::max(reqHeight, static_cast<s32>(m_blockSize.y * 2));
		
		if (m_label != 0)
		{
			reqHeight += m_label->getRequestedHeight() + TITLE_BOTTOM_PADDING;
		}
		
		return reqHeight;
	}
	else
	{
		return ContainerBase<>::getRequestedHeight();
	}
}


bool Window::onStylusPressed(s32 p_x, s32 p_y)
{
	// Offset the coordinates to account for the border
	p_x -= static_cast<s32>(m_blockSize.x - m_borderRemainder);
	p_y -= static_cast<s32>(m_blockSize.y - m_borderRemainder);
	
	if (m_label != 0)
	{
		p_y -= m_label->getRectangle().getHeight() + TITLE_BOTTOM_PADDING;
	}
	
	// Call the base class function
	return ContainerBase<>::onStylusPressed(p_x, p_y);
}


bool Window::onStylusDragging(s32 p_x, s32 p_y, bool p_isInside)
{
	// Offset the coordinates to account for the border
	p_x -= static_cast<s32>(m_blockSize.x - m_borderRemainder);
	p_y -= static_cast<s32>(m_blockSize.y - m_borderRemainder);
	
	if (m_label != 0)
	{
		p_y -= m_label->getRectangle().getHeight() + TITLE_BOTTOM_PADDING;
	}
	
	// Call the base class function
	return ContainerBase<>::onStylusDragging(p_x, p_y, p_isInside);
}


bool Window::onStylusReleased(s32 p_x, s32 p_y)
{
	// Offset the coordinates to account for the border
	p_x -= static_cast<s32>(m_blockSize.x - m_borderRemainder);
	p_y -= static_cast<s32>(m_blockSize.y - m_borderRemainder);
	
	if (m_label != 0)
	{
		p_y -= m_label->getRectangle().getHeight() + TITLE_BOTTOM_PADDING;
	}
	
	// Call the base class function
	return ContainerBase<>::onStylusReleased(p_x, p_y);
}


bool Window::onStylusRepeat(s32 p_x, s32 p_y)
{
	// Offset the coordinates to account for the border
	p_x -= static_cast<s32>(m_blockSize.x - m_borderRemainder);
	p_y -= static_cast<s32>(m_blockSize.y - m_borderRemainder);
	
	if (m_label != 0)
	{
		p_y -= m_label->getRectangle().getHeight() + TITLE_BOTTOM_PADDING;
	}
	
	// Call the base class function
	return ContainerBase<>::onStylusRepeat(p_x, p_y);
}


bool Window::getSelectedElementRect(math::PointRect& p_rect) const
{
	// Offset the rectangle for the border
	p_rect.translate(math::Point2(
		static_cast<s32>(m_blockSize.x - m_borderRemainder),
		static_cast<s32>(m_blockSize.y - m_borderRemainder)));
	
	if (m_label != 0)
	{
		// Offset for the label
		p_rect.translate(math::Point2(0,
			m_label->getRectangle().getHeight() + TITLE_BOTTOM_PADDING));
	}
	
	return ContainerBase<>::getSelectedElementRect(p_rect);
}


Window* Window::clone() const
{
	return new Window(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

Window::Window(const Window& p_rhs)
:
ContainerBase<>(p_rhs),
m_texture(p_rhs.m_texture),
m_poleTexture(p_rhs.m_poleTexture),
m_hasPoles(p_rhs.m_hasPoles),
m_topLeft(p_rhs.m_topLeft),
m_bottomRight(p_rhs.m_bottomRight),
m_size(p_rhs.m_size),
m_blockSize(p_rhs.m_blockSize),
m_poleBlockSize(p_rhs.m_poleBlockSize),
m_innerRect(p_rhs.m_innerRect),
m_borderRemainder(p_rhs.m_borderRemainder),
m_singlePole(p_rhs.m_singlePole),
m_drawLeftPole(p_rhs.m_drawLeftPole),
m_leftRot(p_rhs.m_leftRot),
m_rightRot(p_rhs.m_rightRot)
{
	using engine::renderer::QuadSprite;
	
	for (int i = 0; i < WindowSection_Count; ++i)
	{
		m_windowSegs[i].reset(new QuadSprite(*(p_rhs.m_windowSegs[i])));
	}
	
	for (int i = 0; i < PoleSection_Count; ++i)
	{
		if (p_rhs.m_poleSegs[i] != 0)
		{
			m_poleSegs[i].reset(new QuadSprite(*(p_rhs.m_poleSegs[i])));
		}
	}
	
	if (p_rhs.m_label != 0)
	{
		m_label = p_rhs.m_label->clone();
	}
	
	for (int i = 0; i < Decal_Count; ++i)
	{
		m_decals[i].reset(new QuadSprite(*(p_rhs.m_decals[i])));
		m_decalOffsets[i] = p_rhs.m_decalOffsets[i];
	}
}


//------------------------------------------------------------------------------
// Private member functions

void Window::setWindowTexture(
		real p_x, real p_y, real p_w, real p_h,
		const engine::renderer::ColorRGBA& p_color)
{
	m_blockSize.x = p_w / 3.0f;
	m_blockSize.y = p_h / 3.0f;
	
	// Build the quads
	using engine::renderer::QuadSprite;
	
	// - Center
	m_windowSegs[WindowSection_Center] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_Center], 1, 1, m_blockSize, p_x, p_y);
	
	// - Top edge
	m_windowSegs[WindowSection_EdgeTop] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_EdgeTop], 1, 0, m_blockSize, p_x, p_y);
	
	// - Bot edge
	m_windowSegs[WindowSection_EdgeBottom] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_EdgeBottom], 1, 2, m_blockSize, p_x, p_y);
	
	// - Left edge
	m_windowSegs[WindowSection_EdgeLeft] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_EdgeLeft], 0, 1, m_blockSize, p_x, p_y);
	
	// - Right edge
	m_windowSegs[WindowSection_EdgeRight] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_EdgeRight], 2, 1, m_blockSize, p_x, p_y);
	
	// - Topleft corner
	m_windowSegs[WindowSection_CornerTopLeft] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_CornerTopLeft], 0, 0, m_blockSize, p_x, p_y);
	
	// - Topright corner
	m_windowSegs[WindowSection_CornerTopRight] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_CornerTopRight], 2, 0, m_blockSize, p_x, p_y);
	
	// - Botleft corner
	m_windowSegs[WindowSection_CornerBottomLeft] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_CornerBottomLeft], 0, 2, m_blockSize, p_x, p_y);
	
	// - Botright corner
	m_windowSegs[WindowSection_CornerBottomRight] = QuadSprite::createQuad(m_texture, p_color);
	setUVs(m_windowSegs[WindowSection_CornerBottomRight], 2, 2, m_blockSize, p_x, p_y);
}


void Window::setPoleTexture(
		real p_x, real p_y, real p_w, real p_h, u32 p_count,
		const engine::renderer::ColorRGBA& p_color)
{
	TT_ASSERT(p_count > 0);
	
	m_poleBlockSize.x = p_w;
	m_poleBlockSize.y = p_h / 4.0f;
	
	// Make sure all pole quad sprites are released
	for (int i = 0; i < PoleSection_Count; ++i)
	{
		m_poleSegs[i].reset();
	}
	
	// Build the quads
	using engine::renderer::QuadSprite;
	
	if (m_singlePole == false || m_drawLeftPole)
	{
		// Construct quad sprite for center
		m_poleSegs[PoleSection_LeftBottom] =
			QuadSprite::createQuad(m_poleTexture, p_color);
		setUVs(m_poleSegs[PoleSection_LeftBottom], 0, static_cast<s32>(p_count),
		       m_poleBlockSize, p_x, p_y);
		
		// Top edge
		m_poleSegs[PoleSection_LeftTop] =
			QuadSprite::createQuad(m_poleTexture, p_color);
		setUVs(m_poleSegs[PoleSection_LeftTop], 0,
		       static_cast<s32>(math::Random::getStatic().getNext(p_count)),
		       m_poleBlockSize, p_x, p_y);
	}
	
	if (m_singlePole == false || m_drawLeftPole == false)
	{
		// Bot edge
		m_poleSegs[PoleSection_RightBottom] =
			QuadSprite::createQuad(m_poleTexture, p_color);
		setUVs(m_poleSegs[PoleSection_RightBottom], 0,
		       static_cast<s32>(p_count), m_poleBlockSize, p_x, p_y);
		
		// Left edge
		m_poleSegs[PoleSection_RightTop] =
			QuadSprite::createQuad(m_poleTexture, p_color);
		setUVs(m_poleSegs[PoleSection_RightTop], 0,
		       static_cast<s32>(math::Random::getStatic().getNext(p_count)),
		       m_poleBlockSize, p_x, p_y);
	}
}


void Window::addDecal(Window::Decal                       p_decal,
                      const engine::renderer::TexturePtr& p_texture,
                      const math::VectorRect&             p_uvRect,
                      const engine::renderer::ColorRGBA&  p_color)
{
	using engine::renderer::QuadSprite;
	m_decals[p_decal] = QuadSprite::createQuad(p_texture, p_color);
	
	m_decals[p_decal]->setWidth (p_uvRect.getWidth());
	m_decals[p_decal]->setHeight(p_uvRect.getHeight());
	
	/*
	m_decals[p_decal]->setTexcoordLeft(p_uvRect.getPosition().x);
	m_decals[p_decal]->setTexcoordTop(p_uvRect.getPosition().y);
	
	m_decals[p_decal]->setTexcoordRight(p_uvRect.getRight());
	m_decals[p_decal]->setTexcoordBottom(p_uvRect.getBottom());
	*/
}


void Window::setUVs(const engine::renderer::QuadSpritePtr& p_quad,
                    s32                                    p_blockX,
                    s32                                    p_blockY,
                    const math::Vector2&                   p_blockSize,
                    real                                   p_topLeftX,
                    real                                   p_topLeftY)
{
	real blockW = p_blockSize.x;
	real blockH = p_blockSize.y;
	
	TT_ASSERTMSG(blockW > 0.0f && blockH > 0.0f,
	             "Attempt to construct Window with fishy UV bounds.");
	
	/*
	p_quad->setTexcoordLeft(p_topLeftX + (blockW * p_blockX));
	p_quad->setTexcoordTop(p_topLeftY + (blockH * p_blockY));
	
	p_quad->setTexcoordRight(p_topLeftX + (blockW * (p_blockX + 1)) - 1);
	p_quad->setTexcoordBottom(p_topLeftY + (blockH * (p_blockY + 1)) - 1);
	*/ (void)blockW; (void)blockH;
}

// Namespace end
}
}
}
