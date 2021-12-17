#if !defined(INC_TOKI_LEVEL_SKIN_TILEMATERIAL_H)
#define INC_TOKI_LEVEL_SKIN_TILEMATERIAL_H


#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>

#include <toki/level/skin/types.h>


namespace toki {
namespace level {
namespace skin {


class TileMaterial
{
public:
	inline TileMaterial()
	:
	m_material(MaterialType_None),
	m_theme(   MaterialTheme_None)
	{}
	
	inline TileMaterial(MaterialType p_material, MaterialTheme p_theme)
	:
	m_material(static_cast<u8>(p_material)),
	m_theme(   static_cast<u8>(p_theme   ))
	{
		TT_ASSERT(isValidMaterialType( p_material));
		TT_ASSERT(isValidMaterialTheme(p_theme   ));
		
		TT_ASSERTMSG(static_cast<s32>(p_material) <= (1 << Constants_BitMaskMaterialType),
		             "Material:   %u is too large for only %d bits. (Max is %d.)",
		             p_material, Constants_BitSizeMaterialType,  (1 << Constants_BitSizeMaterialType));
		TT_ASSERTMSG(static_cast<s32>(p_theme)    <= (1 << Constants_BitSizeMaterialTheme),
		             "Theme: %u is too large for only %d bits. (Max is %d.)",
		             p_theme,    Constants_BitSizeMaterialTheme, (1 << Constants_BitSizeMaterialTheme));
	}
	
	static inline TileMaterial createFromRaw(u8 p_rawValue)
	{
		TileMaterial tileMaterial(static_cast<MaterialType> ((p_rawValue >> Constants_BitSizeMaterialTheme) & Constants_BitMaskMaterialType ),
	                              static_cast<MaterialTheme>( p_rawValue                                    & Constants_BitMaskMaterialTheme));
		TT_ASSERT(tileMaterial.getRawValue() == p_rawValue);
		return tileMaterial;
	}
	
	inline void set(CollisionType p_collisionType, level::ThemeType p_themeType, ThemeType p_defaultLevelTheme)
	{
		// Replace level default with level theme.
		const level::ThemeType rawTheme = (p_themeType == ThemeType_UseLevelDefault) ?
		                                  p_defaultLevelTheme : p_themeType;
		
		switch (rawTheme)
		{
		case level::ThemeType_DoNotTheme:
		{
			m_material = MaterialType_None;
			m_theme    = MaterialTheme_None;
			return; // Done
		}
		case ThemeType_Sand:      m_theme = MaterialTheme_Sand;      break;
		case ThemeType_Rocks:     m_theme = MaterialTheme_Rocks;     break;
		case ThemeType_Beach:     m_theme = MaterialTheme_Beach;     break;
		case ThemeType_DarkRocks: m_theme = MaterialTheme_DarkRocks; break;
		default:
			TT_PANIC("Unsupported theme (%d)", rawTheme);
			break;
		}
		
		if (level::isSolid(p_collisionType))
		{
			m_material = MaterialType_Solid;
		}
		else
		{
			m_material = MaterialType_None;
			return; // Done
		}
		
		// Special case theme override for crystal.
		if (level::isCrystal(p_collisionType))
		{
			m_theme = MaterialTheme_Crystal;
		}
	}
	
	inline MaterialType  getMaterialType()  const { return static_cast<MaterialType>( m_material);}
	inline void setMaterialType(MaterialType p_type) { m_material = p_type; TT_ASSERT(isValidMaterialType(p_type));}
	inline MaterialTheme getMaterialTheme() const { return static_cast<MaterialTheme>(m_theme);   }
	inline bool isSolid()                   const { return skin::isSolid( getMaterialType());     }
	
	inline u8 getRawValue() const { return (m_material << Constants_BitSizeMaterialTheme) | m_theme; }
	
	inline void makeNone()     { m_material = MaterialType_None;  }
	inline bool isNone() const { return m_material == MaterialType_None; }
	
private:
	enum Constants
	{
		Constants_BitSizeMaterialType  = 4,
		Constants_BitSizeMaterialTheme = 4,
		
		Constants_BitMaskMaterialType  = (1 << Constants_BitSizeMaterialType ) - 1,
		Constants_BitMaskMaterialTheme = (1 << Constants_BitSizeMaterialTheme) - 1
	};
	TT_STATIC_ASSERT(Constants_BitSizeMaterialType + Constants_BitSizeMaterialTheme <= sizeof(u8) * 8);
	
	// Check that bit size is large enough to hold all (valid) enum values.
	TT_STATIC_ASSERT((1 << Constants_BitSizeMaterialType ) > MaterialType_Count );
	TT_STATIC_ASSERT((1 << Constants_BitSizeMaterialTheme) > MaterialTheme_Count);
	
	u8 m_material : Constants_BitSizeMaterialType;
	u8 m_theme    : Constants_BitSizeMaterialTheme;
};


inline bool operator==(const TileMaterial& p_lhs, const TileMaterial& p_rhs)
{
	return p_lhs.getMaterialType()  == p_rhs.getMaterialType() &&
	       p_lhs.getMaterialTheme() == p_rhs.getMaterialTheme();
}


inline bool operator!=(const TileMaterial& p_lhs, const TileMaterial& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_TILEMATERIAL_H)
