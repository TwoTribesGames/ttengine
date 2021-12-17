#if !defined(INC_TT_ENGINE_FILE_FILETYPE_H)
#define INC_TT_ENGINE_FILE_FILETYPE_H


namespace tt {
namespace engine {
namespace file {

enum FileType
{
	FileType_Object,
	FileType_Texture,
	FileType_Animation,
	FileType_Material,
	FileType_Scene,
	FileType_Layer,
	FileType_Font,
	FileType_Palette,
	FileType_Shader,

	FileType_MaxTypes
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_FILE_FILETYPE_H

