#include <tt/code/bufferutils.h>
#include <tt/code/DefaultValue.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/TriggerInfo.h>
#include <tt/pres/TriggerFactoryInterface.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlNode.h>

namespace tt {
namespace pres {


static const u16 s_version = 6;


//--------------------------------------------------------------------------------------------------
// Public member functions


TriggerInfo TriggerInfo::loadXml(const xml::XmlNode* p_node,
                                 const DataTags& p_applyTags,
                                 const Tags& p_acceptedTags,
                                 code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(TriggerInfo, TriggerInfo(), 
		 "Loading trigger info from XML");
	
	TriggerInfo info;
	
	info.type = xml::util::parseStr(p_node, "type", &errStatus);
	info.data = xml::util::parseStr(p_node, "data", &errStatus);
	
	
	info.loadXmlCommon(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	
	TT_ERR_RETURN_ON_ERROR();
	
	return info;
}


bool TriggerInfo::loadXmlCommon(const xml::XmlNode* p_node, 
                                const DataTags& p_applyTags, 
                                const Tags& p_acceptedTags,
                                code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, 
		 "Loading common trigger info from XML");
	
	const char* syncIdAttr = "sync_id";
	const char* endSyncIdAttr = "end_sync_id";
	
	TT_ERR_ASSERTMSG((p_node->hasAttribute(syncIdAttr) && p_node->hasAttribute(endSyncIdAttr)) == false,
	                 " Don't use both '" << syncIdAttr << "' and '" << endSyncIdAttr <<
	                 "' attributes in node '" << p_node->getName() << "'. Just use one of them.");
	
	code::DefaultValue<std::string> syncIdDefaultValue("");
	syncIdDefaultValue = xml::util::parseOptionalStr(p_node, syncIdAttr, &errStatus);
	
	code::DefaultValue<std::string> endSyncIdDefaultValue("");
	endSyncIdDefaultValue = xml::util::parseOptionalStr(p_node, endSyncIdAttr, &errStatus);
	
	endSync = p_node->hasAttribute("end_sync_id");
	if (endSync)
	{
		syncId = endSyncIdDefaultValue;
	}
	else
	{
		syncId = syncIdDefaultValue;
	}
	
	code::DefaultValue<bool> loopingDefaultValue(false);
	loopingDefaultValue = xml::util::parseOptionalBool(p_node, "looping", &errStatus);
	looping = loopingDefaultValue;
	
	delay    = parseOptionalPresentationValue(p_node, "delay",    0, &errStatus);
	duration = parseOptionalPresentationValue(p_node, "duration", 0, &errStatus);
	
	dataTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	TT_ERR_RETURN_ON_ERROR();
	return true;
}


TriggerInfo TriggerInfo::loadBin(const u8*& p_bufferOUT, size_t& p_sizeOUT,
                                 const DataTags& p_applyTags, const Tags& p_acceptedTags,
                                 code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(TriggerInfo, TriggerInfo(), 
		 "Loading trigger info Binary");
	
	using namespace code::bufferutils;
	
	TriggerInfo info;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	if (version != s_version)
	{
		TT_ERR_AND_RETURN("Invalid version, code '" << s_version << "', data '" << version <<
			"', Please update your converter");
	}
	
	info.type      = be_get<std::string>(p_bufferOUT, p_sizeOUT);
	info.data      = be_get<std::string>(p_bufferOUT, p_sizeOUT);
	info.syncId    = be_get<std::string>(p_bufferOUT, p_sizeOUT);
	info.endSync   = be_get<bool       >(p_bufferOUT, p_sizeOUT);
	info.looping   = be_get<bool       >(p_bufferOUT, p_sizeOUT);
	
	info.duration.load(p_bufferOUT, p_sizeOUT, &errStatus);
	info.delay   .load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	info.dataTags.load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	return info;
}


bool TriggerInfo::saveBin(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "Saving Trigger type:'" << type << "' data:'" << data << "'");
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	be_put(type,      p_bufferOUT, p_sizeOUT);
	be_put(data,      p_bufferOUT, p_sizeOUT);
	be_put(syncId,    p_bufferOUT, p_sizeOUT);
	be_put(endSync,   p_bufferOUT, p_sizeOUT);
	be_put(looping,   p_bufferOUT, p_sizeOUT);
	
	duration.save(p_bufferOUT, p_sizeOUT, &errStatus);
	delay   .save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	dataTags.save(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	return true;
}


size_t TriggerInfo::getBufferSize() const
{
	size_t size = 2; // version
	size += 2 + type.size();
	size += 2 + data.size();
	size += 2 + syncId.size();
	size += 1 + 1; // endsync + looping
	size += delay.getBufferSize() + duration.getBufferSize(); // delay + duration
	size += dataTags.getBufferSize();
	return size;
}


// Namespace end
}
}
