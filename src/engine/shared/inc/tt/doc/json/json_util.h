#if !defined (INC_TT_DOCS_JSON_JSON_UTIL_H)
#define INC_TT_DOCS_JSON_JSON_UTIL_H

#include <string>

#include <json/forwards.h>

#include <tt/code/ErrorStatus.h>
#include <tt/fs/types.h>
#include <tt/math/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace doc {
namespace json {


// Binary load/save
void saveBinaryJson(const Json::Value& p_root, const tt::fs::FilePtr& p_file,
                    const char* p_signBuf, s32 p_signSize, s32 p_saveVersion);

bool loadBinaryJson(Json::Value* p_root_OUT, const tt::fs::FilePtr& p_file,
                    const char* p_signBuf, s32 p_signSize, s32 p_saveVersion);


// Read Generic types.
u32          readU32(     const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
s32          readS32(     const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
bool         readBool(    const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
std::string  readString(  const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
std::string  readString(  const Json::Value& p_value, tt::code::ErrorStatus* p_errStatus);
std::wstring readWstring( const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
math::Point2 readPoint2(  const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
u64          readU64(     const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
u64          readU64(     const Json::Value& p_value, tt::code::ErrorStatus* p_errStatus);
s64          readS64(     const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
real         readReal(    const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);
// Read a JSON Object with error checking (no arrays)
const Json::Value& readObject(  const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus);


// Write Generic types.
Json::Value writeU32(    u32                 p_value);
Json::Value writeS32(    s32                 p_value);
Json::Value writeBool(   bool                p_value);
Json::Value writeString( const std::string&  p_value);
Json::Value writeWstring(const std::wstring& p_value);
Json::Value writePoint2( const math::Point2& p_value);
Json::Value writeU64(    u64                 p_value);
Json::Value writeS64(    s64                 p_value);
Json::Value writeReal(   real                p_value);


// namespace end
}
}
}


#endif // #if defined (INC_TT_DOCS_JSON_JSON_UTIL_H)
