#if !defined(INC_TT_MENU_MENUSKIN_H)
#define INC_TT_MENU_MENUSKIN_H


#include <string>
#include <vector>
#include <map>

#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace menu {

/*! \brief Provides skinning (theming) information for menu elements. */
class MenuSkin
{
public:
	class SkinTexture
	{
	public:
		SkinTexture(const std::string& p_filename,
		            real               p_u,
		            real               p_v,
		            real               p_width,
		            real               p_height);
		
		inline std::string getFilename() const { return m_filename; }
		inline real        getU()        const { return m_u;        }
		inline real        getV()        const { return m_v;        }
		inline real        getWidth()    const { return m_width;    }
		inline real        getHeight()   const { return m_height;   }
		
	private:
		std::string m_filename;
		real        m_u;
		real        m_v;
		
		// Dimensions of requested texture area (not necessarily image size).
		real        m_width;
		real        m_height;
	};
	
	class ElementSkin
	{
	public:
		inline void addTexture(const SkinTexture& p_texture)
		{ m_textures.push_back(p_texture); }
		
		inline void addTexture(const std::string& p_filename,
		                       real p_u, real p_v,
		                       real p_width, real p_height)
		{
			m_textures.push_back(SkinTexture(p_filename, p_u, p_v,
			                                 p_width, p_height));
		}
		
		inline int getTextureCount() const
		{ return static_cast<int>(m_textures.size()); }
		const SkinTexture& getTexture(int p_index) const;
		
		inline void addVertexColor(const engine::renderer::ColorRGBA& p_color)
		{ m_vertexColors.push_back(p_color); }
		inline int getVertexColorCount() const
		{ return static_cast<int>(m_vertexColors.size()); }
		engine::renderer::ColorRGBA getVertexColor(int p_index) const;
		
	private:
		typedef std::vector<SkinTexture>                 Textures;
		typedef std::vector<engine::renderer::ColorRGBA> VertexColors;
		
		Textures     m_textures;
		VertexColors m_vertexColors;
	};
	
	
	MenuSkin();
	~MenuSkin();
	
	void               addElementSkin(int p_elementID,
	                                  const ElementSkin& p_elementSkin);
	const ElementSkin& getElementSkin(int p_elementID) const;
	bool               hasElementSkin(int p_elementID) const;
	
	// Menu skins need to be on the safe heap
	/*
	static void* operator new(size_t p_blockSize);
	static void operator delete(void* p_block);
	//*/
	
private:
	typedef std::map<int, ElementSkin> ElementSkins;
	
	
	// Skins may not be copied
	MenuSkin(const MenuSkin&);
	const MenuSkin& operator=(const MenuSkin&);
	
	
	ElementSkins m_elementSkins;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUSKIN_H)
