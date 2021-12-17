#ifndef INC_ASSETNAMEHASHFUNC_H
#define INC_ASSETNAMEHASHFUNC_H

#include <string>

#include <tt/platform/tt_types.h>
#include <tt/asset/asset.h>


namespace tt {
namespace asset {

AssetIdType strToHash( const std::string& p_strkey );
AssetIdType strToHash( const std::wstring& p_strkey );
AssetIdType chunkToHash( u8* p_chunk, std::size_t p_length );


}
}

#endif

