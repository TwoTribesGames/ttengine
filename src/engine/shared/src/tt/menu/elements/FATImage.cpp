#include <tt/platform/tt_error.h>
//#include <tt/save/SaveFAT.h>
//#include <tt/save/SaveFS.h>
#include <tt/menu/elements/FATImage.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/DefaultColours.h>


#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/QuadSprite.h>


namespace tt {
namespace menu {
namespace elements {

//using save::SaveFAT;
//using save::SaveFS;


//------------------------------------------------------------------------------
// Public member functions

FATImage::FATImage(const std::string& p_name,
                   const MenuLayout&  p_layout)
:
MenuElement(p_name, p_layout)
{
	MENU_CREATION_Printf("FATImage::FATImage: Element '%s'\n",
	                     getName().c_str());
	
	/* FIXME: This code requires a SaveFAT implementation.
	s32 blocks = SaveFAT::getTotalBlockCount();
	
	s32 roundedblocks = 32 - static_cast<s32>(
		MATH_CountLeadingZeros(static_cast<u32>(blocks - 1)));
	
	s32 blockheight = roundedblocks >> 1;
	s32 blockwidth  = roundedblocks - blockheight;
	
	blockheight = 1 << blockheight;
	blockwidth  = 1 << blockwidth;
	
	roundedblocks = 1 << roundedblocks;
	
	setMinimumWidth(blockwidth);
	setMinimumHeight(blockheight);
	
	setRequestedWidth(4 * blockwidth);
	setRequestedHeight(4 * blockheight);
	
	m_image.reset(new engine::renderer::Texture(blockwidth, blockheight));
	m_imageQuad = engine::renderer::QuadSprite::createQuad(m_image);
	
	m_imageQuad->setWidth(blockwidth);
	m_imageQuad->setHeight(blockheight);
	m_imageQuad->setTexcoordRight(static_cast<u16>(blockwidth));
	m_imageQuad->setTexcoordBottom(static_cast<u16>(blockheight));
	
	
	// Image created, now create a visual representation of the FAT
	u8* img = m_image->LockImage();
	TT_NULL_ASSERT(img);
	
	for (s32 i = 0; i < roundedblocks; ++i)
	{
		img[i] = 0;
	}
	
	u8 color = 3; // 0 = unused, 1 = free, 2 = system
	
	for (s32 i = 0; i < blocks; ++i)
	{
		s32 b = SaveFAT::getFatEntry(i);
		switch (b)
		{
		case SaveFAT::BLOCK_FREE:
			img[i] = 1;
			break;
			
		case SaveFAT::BLOCK_SYSTEM:
			img[i] = 2;
			break;
			
		case SaveFAT::BLOCK_EOF:
			{
				img[i] = color;
				bool done = false;
				s32  k    = i;
				while (done == false)
				{
					s32 j = 0;
					for (; j < blocks; ++j)
					{
						if (SaveFAT::getFatEntry(j) == k)
						{
							break;
						}
					}
					
					if (j == blocks)
					{
						done = true;
					}
					else
					{
						img[j] = color;
						k = j;
					}
				}
				++color;
				break;
			}
		}
	}
	m_image->UnlockImage();
	
	SaveFAT::dumpFat();
	SaveFS::dumpFileTable();
	//*/
	
	
	setCanHaveFocus(false);
}


FATImage::~FATImage()
{
	MENU_CREATION_Printf("FATImage::~FATImage: Element '%s'.\n ",
	                     getName().c_str());
}


void FATImage::render(const math::PointRect& p_rect, s32 p_z)
{
	// Don't render if not visible
	if (isVisible() == false || m_imageQuad == 0)
	{
		return;
	}
	
	m_imageQuad->setWidth(static_cast<real>(p_rect.getWidth()));
	m_imageQuad->setHeight(static_cast<real>(p_rect.getHeight()));
	m_imageQuad->setPosition(math::Vector3(
		static_cast<real>(p_rect.getCenterPosition().x),
		static_cast<real>(p_rect.getCenterPosition().y),
		static_cast<real>(p_z)));
	m_imageQuad->update();
	m_imageQuad->render();
}


FATImage* FATImage::clone() const
{
	return new FATImage(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

FATImage::FATImage(const FATImage& p_rhs)
:
MenuElement(p_rhs),
m_image(p_rhs.m_image)
{
	if (p_rhs.m_imageQuad != 0)
	{
		using engine::renderer::QuadSprite;
		m_imageQuad.reset(new QuadSprite(*(p_rhs.m_imageQuad)));
	}
}

// Namespace end
}
}
}
