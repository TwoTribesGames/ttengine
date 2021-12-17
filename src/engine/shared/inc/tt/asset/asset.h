#ifndef INC_TT_ASSET_ASSET_H
#define INC_TT_ASSET_ASSET_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace asset {

typedef u64 AssetIdType; //! < Assets need to have an id of this type.

// Namespace end
}
}

#include <tt/asset/NameHashFunc.h>
// Macro to convert asset name (std::string) to id (AssetIdType).
#define TT_ASSETID tt::asset::strToHash

#endif  // !defined(INC_TT_ASSET_ASSETMGRBASE_H)
