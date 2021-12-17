#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <utf8/utf8.h>

#include <tt/code/helpers.h>
#include <tt/compression/compression.h>
#include <tt/doc/json/json_util.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/math/hash/CRC32.h>
#include <tt/math/Point2.h>
#include <tt/mem/util.h>
#include <tt/str/str.h>


namespace tt {
namespace doc {
namespace json {


// -------------------------------------------------------------------------------------------------
// Binary load/save


void saveBinaryJson(const Json::Value& p_root, const tt::fs::FilePtr& p_file,
                    const char* p_signBuf, s32 p_signSize, s32 p_saveVersion)
{
	if (p_file == 0)
	{
		TT_PANIC("File ptr is null!");
		return;
	}
	
	tt::fs::write(p_file, p_signBuf, p_signSize);
	tt::fs::writeInteger(p_file, p_saveVersion);
	
	Json::FastWriter writer;
	std::string strBuf = writer.write(p_root);
	
	bool isCompressed = false;
	u32 size = static_cast<u32>(strBuf.size());
	tt::code::Buffer compressBuffer(static_cast<tt::code::Buffer::size_type>(
		(strBuf.size() * 1.2f) + 1024));
	
	// Compress if larger than 128 bytes
	// FIXME: remove hardcoded value of 128 bytes
	if (strBuf.size() > 128)
	{
		size = tt::compression::compressLZ(reinterpret_cast<const u8*>(strBuf.c_str()),
		                                   static_cast<u32>(strBuf.size()),
		                                   reinterpret_cast<u8*>(compressBuffer.getData()),
		                                   true, true);
		
		// Compression successful
		if (size > 0)
		{
			isCompressed = true;
		}
		else
		{
			TT_WARN("Compression failed for file '%s'!", p_file->getPath());
			size = static_cast<u32>(strBuf.size());
		}
	}
	
	if (isCompressed == false)
	{
		// Copy uncompressed data
		tt::mem::copy8(compressBuffer.getData(), strBuf.c_str(), static_cast<mem::size_type>(strBuf.size()));
	}
	
	// Done with str Buffer.
	tt::code::helpers::freeContainer(strBuf);
	
	TT_ASSERT(size <= static_cast<u32>(compressBuffer.getSize()));
	
	tt::fs::writeBool(p_file, isCompressed);
	tt::fs::writeInteger(p_file, size);
	
	
	// Obfuscate the buffer a bit here.
	{
		u8* ptr = reinterpret_cast<u8*>(compressBuffer.getData());
		for (s32 i = 0; i < compressBuffer.getSize(); ++i)
		{
			ptr[i] = ~ptr[i];
		}
	}
	
	tt::fs::write(p_file, compressBuffer.getData(), size);
	tt::math::hash::CRC32 crc;
	crc.update(compressBuffer.getData(), size);
	crc.update(p_signBuf, p_signSize);
	tt::fs::writeInteger(p_file, crc.getCRC());
}


bool loadBinaryJson(Json::Value* p_root_OUT, const tt::fs::FilePtr& p_file,
                    const char* p_signBuf, s32 p_signSize, s32 p_saveVersion)
{
	if (p_root_OUT == 0)
	{
		TT_PANIC("Json value ptr is null!");
		return false;
	}
	
	if (p_file == 0)
	{
		TT_PANIC("File ptr is null!");
		return false;
	}
	
	
	{
		tt::code::Buffer signInBuf(p_signSize);
		if (tt::fs::read(p_file, signInBuf.getData(), p_signSize) == false)
		{
			TT_PANIC("Failed to load binary JSON signature (%d bytes).", p_signSize);
			return false;
		}
		
		const char* savedSignBuf = reinterpret_cast<const char*>(signInBuf.getData());
		for (s32 i = 0; i < p_signSize; ++i)
		{
			if (savedSignBuf[i] != p_signBuf[i])
			{
				TT_PANIC("Binary JSON signature mismatch.");
				return false;
			}
		}
	}
	
	s32 savedVersion = 0;
	tt::fs::readInteger(p_file, &savedVersion);
	if (savedVersion != p_saveVersion)
	{
		TT_PANIC("Incorrect binary JSON save version found. Found version %d, expected version %d.",
		         savedVersion, p_saveVersion);
		return false;
	}
	
	bool isCompressed = true;
	if (tt::fs::readBool(p_file, &isCompressed) == false)
	{
		TT_PANIC("Getting isCompressed bool failed");
		return false;
	}
	
	u32 size = 0;
	if (tt::fs::readInteger(p_file, &size) == false)
	{
		TT_PANIC("Getting size failed");
		return false;
	}
	
	if (size <= 0)
	{
		TT_PANIC("Size can't be 0 or smaller! (found: %u)", size);
		return false;
	}
	
	if (isCompressed && size > 1024 * 1024) // FIXME: Replace magic numbers.
	{
		TT_PANIC("Size too large! (found: %u)", size);
		return false;
	}
	
	tt::code::Buffer compressBuffer(size);
	
	if (tt::fs::read(p_file, compressBuffer.getData(), compressBuffer.getSize()) == false)
	{
		TT_PANIC("reading compressed data failed");
		return false;
	}
	
	u32 savedCRC = 0;
	if (tt::fs::readInteger(p_file, &savedCRC) == false)
	{
		TT_PANIC("Reading CRC failed!");
		return false;
	}
	
	tt::math::hash::CRC32 crc;
	crc.update(compressBuffer.getData(), compressBuffer.getSize());
	crc.update(p_signBuf, p_signSize);
	
	if (savedCRC != crc.getCRC())
	{
		TT_PANIC("CRC check failed! (saved: %u, calc: %u)", savedCRC, crc.getCRC());
		return false;
	}
	
	// Unobfuscate the buffer here.
	{
		u8* ptr = reinterpret_cast<u8*>(compressBuffer.getData());
		for (s32 i = 0; i < compressBuffer.getSize(); ++i)
		{
			ptr[i] = ~ptr[i];
		}
	}
	
	// Check we have at least enough data to get the uncompressedSize.
	if (isCompressed && size < 4)
	{
		TT_PANIC("Size too small to get uncompressed size!");
		return false;
	}
	
	u32 uncompresSize = (isCompressed) ? 
			tt::compression::getUncompressedSize(compressBuffer.getData()) :
			size;
	
	if (uncompresSize > 3 * 1024 * 1024) // FIXME: Replace magic numbers.
	{
		TT_PANIC("uncompressed size too large! (found: %u)", uncompresSize);
		return false;
	}
	
	tt::code::Buffer uncompressBuffer(uncompresSize);
	
	if (isCompressed)
	{
		tt::compression::uncompressLZ(compressBuffer.getData(), uncompressBuffer.getData());
	}
	else
	{
		tt::mem::copy8(uncompressBuffer.getData(), compressBuffer.getData(), uncompressBuffer.getSize());
	}
	
	const char* strPtr = reinterpret_cast<const char*>(uncompressBuffer.getData());
	Json::Reader reader;
	if (reader.parse(strPtr, &strPtr[uncompressBuffer.getSize()], (*p_root_OUT), false) == false)
	{
		TT_PANIC("parsing JSON failed!");
		return false;
	}
	
	return true;
}


// -------------------------------------------------------------------------------------------------
// Read Generic types.


u32 readU32(const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(u32, 0, "reading u32 with name: '" << p_name << "' from json");
	
	const Json::Value& jValue = p_value[p_name];
	TT_ERR_ASSERTMSG(jValue.empty() == false && jValue.isConvertibleTo(Json::uintValue),
	                 "'" << p_name <<  "' JSON data missing or incorrect type. (type needed: UInt). "
	                 "Found: '" << jValue.toStyledString() << "'");
	
	return jValue.asUInt();
}


s32 readS32(const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(s32, 0, "reading s32 with name: '" << p_name << "' from json");
	
	const Json::Value& jValue = p_value[p_name];
	TT_ERR_ASSERTMSG(jValue.empty() == false && jValue.isConvertibleTo(Json::intValue),
	                 "'" << p_name <<  "' JSON data missing or incorrect type. (type needed: Int). "
	                 "Found: '" << jValue.toStyledString() << "'");
	
	return jValue.asInt();
}


bool readBool(const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "reading bool with name: '" << p_name << "' from json");
	
	const Json::Value& jValue = p_value[p_name];
	TT_ERR_ASSERTMSG(jValue.empty() == false && jValue.isBool(),
	                 "'" << p_name <<  "' JSON data missing or incorrect type. (type needed: Bool). "
	                 "Found: '" << jValue.toStyledString() << "'");
	
	return jValue.asBool();
}


std::string readString(const Json::Value& p_value, const std::string& p_name,
                       tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::string, "", "reading string with name: '" << p_name << "' from json");
	return readString(p_value[p_name], &errStatus);
}


std::string readString(const Json::Value& p_value, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::string, "", "reading string from json");
	
	TT_ERR_ASSERTMSG(p_value.empty() == false && p_value.isString(),
	                 "JSON data missing or incorrect type. (type needed: String). "
	                 "Found: '" << p_value.toStyledString() << "'");
	
	return p_value.asString();
}


std::wstring readWstring(const Json::Value& p_value, const std::string& p_name,
                         tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(std::wstring, L"", "reading wstring with name: '" << p_name << "' from json");
	
	std::string narrowStr(readString(p_value, p_name, &errStatus));
	
	std::string::iterator endIt = utf8::find_invalid(narrowStr.begin(), narrowStr.end());
	
	TT_ERR_ASSERTMSG(endIt == narrowStr.end(),
	                 "Invalid UTF-8 encoding detected string ('" << narrowStr << "').");
	
	std::vector<u16> utf16str;
	utf8::utf8to16(narrowStr.begin(), endIt, std::back_inserter(utf16str));
	
	return std::wstring(utf16str.begin(), utf16str.end());
}


math::Point2 readPoint2(const Json::Value& p_value, const std::string& p_name,
                        tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(math::Point2, math::Point2(), "reading Point2 with name: '" << p_name << "' from json");
	
	const Json::Value& jValue = p_value[p_name];
	TT_ERR_ASSERTMSG(jValue.empty() == false && jValue.isObject(),
	                 "'" << p_name <<  "' JSON data missing or incorrect type. (type needed: Object). "
	                 "Found: '" << jValue.toStyledString() << "'");
	
	math::Point2 result(readS32(jValue, "x", &errStatus), readS32(jValue, "y", &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	return result;
}


u64 readU64(const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(u64, 0, "Reading u64 from with name: '" << p_name << "' json");
	return tt::str::parseU64(readString(p_value, p_name, &errStatus), &errStatus);
}


u64 readU64(const Json::Value& p_value, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(u64, 0, "Reading u64 from json");
	return tt::str::parseU64(readString(p_value, &errStatus), &errStatus);
}


s64 readS64(const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(s64, 0, "Reading s64 from with name: '" << p_name << "' json");
	return tt::str::parseS64(readString(p_value, p_name, &errStatus), &errStatus);
}


real readReal( const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(real, 0, "reading real with name: '" << p_name << "' from json");
	
	const Json::Value& jValue = p_value[p_name];
	TT_ERR_ASSERTMSG(jValue.empty() == false && jValue.isConvertibleTo(Json::realValue),
	                 "'" << p_name <<  "' JSON data missing or incorrect type. (type needed: Real). "
	                 "Found: '" << jValue.toStyledString() << "'");
	
	return static_cast<real>(jValue.asDouble());
}


const Json::Value& readObject( const Json::Value& p_value, const std::string& p_name, tt::code::ErrorStatus* p_errStatus )
{
	const Json::Value& jValue = p_value[p_name];
	
	TT_ERR_CHAIN(const Json::Value&, jValue, "reading object with name: '" << p_name << "' from json");
	
	TT_ERR_ASSERTMSG(jValue.empty() == false && jValue.isObject(),
	                 "'" << p_name <<  "' JSON data missing or incorrect type. (type needed: Object). "
	                 "Found: '" << jValue.toStyledString() << "'");
	
	return jValue;
}


// -------------------------------------------------------------------------------------------------
// Write Generic types.


Json::Value writeU32(u32 p_value)
{
	TT_STATIC_ASSERT(sizeof(Json::UInt) == sizeof(u32));
	return Json::Value(Json::UInt(p_value));
}


Json::Value writeS32(s32 p_value)
{
	TT_STATIC_ASSERT(sizeof(Json::Int) == sizeof(s32));
	return Json::Value(Json::Int(p_value));
}


Json::Value writeBool(bool p_value)
{
	return Json::Value(p_value);
}


Json::Value writeString(const std::string& p_value)
{
	return Json::Value(p_value);
}


Json::Value writeWstring(const std::wstring& p_value)
{
	return writeString(tt::str::utf16ToUtf8(p_value));
}


Json::Value writePoint2( const math::Point2& p_value)
{
	Json::Value value;
	value["x"] = writeS32(p_value.x);
	value["y"] = writeS32(p_value.y);
	return value;
}


Json::Value writeU64(u64 p_value)
{
	return writeString(tt::str::toStr(p_value));
}


Json::Value writeS64(s64 p_value)
{
	return writeString(tt::str::toStr(p_value));
}


Json::Value writeReal( real p_value )
{
	return Json::Value(realToFloat(p_value));
}


// namespace end
}
}
}
