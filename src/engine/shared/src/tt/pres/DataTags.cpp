#include <sstream>

#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/DataTags.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlNode.h>

namespace tt {
namespace pres {


static const u16 s_version = 0;

//--------------------------------------------------------------------------------------------------
// Public member functions


bool DataTags::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, const DataTags& p_applyTags, 
                    const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading Tags");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version,
	                 "Invalid version: code " << s_version << ", "
	                 "data " << version << " -- please update your converter.");
	
	const std::string name = be_get<std::string>(p_bufferOUT, p_sizeOUT);
	m_nameTag = Tag(name);
	if (name.empty())
	{
		m_nameTag.invalidate();
	}

#if !defined(TT_BUILD_FINAL)
	m_name = name;
#endif
	
	const u16 mustHaveTagGroupCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 tagGroupIndex = 0; tagGroupIndex < mustHaveTagGroupCount; ++tagGroupIndex)
	{
		const u16 tagCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
		Tags mustHaveGroup;
		
		for (u16 tagIndex = 0; tagIndex < tagCount; ++tagIndex)
		{
			std::string tagstr(be_get<std::string>(p_bufferOUT, p_sizeOUT));
			Tag tag(tagstr);
			
			// check if the tag is accepted. If the acceptedTags is empty all tags are allowed.
			TT_ERR_ASSERTMSG(p_acceptedTags.empty() || p_acceptedTags.find(tag) != p_acceptedTags.end(),
			                 "Unsupported Tag: " << tagstr);
			
			mustHaveGroup.insert(tag);
		}
		
		m_mustHaves.push_back(mustHaveGroup);
	}
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
	const u16 mustNotHaveTagGroupCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 tagGroupIndex = 0; tagGroupIndex < mustNotHaveTagGroupCount; ++tagGroupIndex)
	{
		TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
		const u16 tagCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
		Tags mustNotHaveGroup;
		
		for (u16 tagIndex = 0; tagIndex < tagCount; ++tagIndex)
		{
			TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
			std::string tagstr(be_get<std::string>(p_bufferOUT, p_sizeOUT));
			Tag tag(tagstr);
			
			// check if the tag is accepted. If the acceptedTags is empty all tags are allowed.
			TT_ERR_ASSERTMSG(p_acceptedTags.empty() || p_acceptedTags.find(tag) != p_acceptedTags.end(),
			                 "Unsupported Tag: " << tagstr);
			
			mustNotHaveGroup.insert(tag);
		}
		
		m_mustNotHaves.push_back(mustNotHaveGroup);
	}
	
	addDataTags(p_applyTags);
	
	return true;
}


bool DataTags::shouldPlay(const Tags& p_startTags, const std::string& p_name)
{
	// if the name from data is empty, check the tags. If the names don't match don't play.
	// when the start name is empty only play the empty named animations.
	if (m_nameTag.isValid() && m_nameTag != Tag(p_name)) return false;
	
	// First check the must not have groups
	for(TagGroup::const_iterator tagGroup(m_mustNotHaves.begin()); tagGroup != m_mustNotHaves.end(); ++tagGroup)
	{
		bool foundAllMustNotHaveTags = true;
		for(Tags::const_iterator it(tagGroup->begin()) ; it != tagGroup->end() ; ++it)
		{
			// check that all the tags in ths group are in the start tags
			if (p_startTags.find(*it) == p_startTags.end())
			{
				foundAllMustNotHaveTags = false;
				break;
			}
		}
		// if all the tags of a group are in the start tags, don't play.
		if (foundAllMustNotHaveTags)
		{
			return false;
		}
	}
	
	// Check the must have groups
	for(TagGroup::const_iterator tagGroup(m_mustHaves.begin()); tagGroup != m_mustHaves.end(); ++tagGroup)
	{
		bool foundAllMustHaveTags = true;
		for(Tags::const_iterator it(tagGroup->begin()) ; it != tagGroup->end() ; ++it)
		{
			// check that all the tags in ths group are in the start tags
			if (p_startTags.find(*it) == p_startTags.end())
			{
				foundAllMustHaveTags = false;
				break;
			}
		}
		// All the tags in this group are in the start tags, so play.
		if (foundAllMustHaveTags)
		{
			return true;
		}
	}
	
	// If we reach this part. Either there are musthaves and they did not match, which means
	// we should not play. Or there are no musthaves which means we can play.
	return m_mustHaves.empty();
}


void DataTags::addDataTags(const DataTags& p_applyTags)
{
	
#if !defined(TT_BUILD_FINAL)
	if (m_name.empty() == false) m_allUsedNames.insert(m_name);
	if (p_applyTags.m_name.empty() == false) m_allUsedNames.insert(p_applyTags.m_name);
	m_allUsedNames.insert(p_applyTags.m_allUsedNames.begin(), p_applyTags.m_allUsedNames.end());

	m_mustHavesStr   .insert(m_mustHavesStr   .end(), p_applyTags.m_mustHavesStr   .begin(), p_applyTags.m_mustHavesStr   .end());
	m_mustNotHavesStr.insert(m_mustNotHavesStr.end(), p_applyTags.m_mustNotHavesStr.begin(), p_applyTags.m_mustNotHavesStr.end());
	
	if (m_name.empty()) m_name = p_applyTags.m_name;
#endif
	
	if(m_nameTag.isValid() == false) m_nameTag = p_applyTags.m_nameTag;
	m_mustHaves   .insert(m_mustHaves   .end(), p_applyTags.m_mustHaves   .begin(), p_applyTags.m_mustHaves   .end());
	m_mustNotHaves.insert(m_mustNotHaves.end(), p_applyTags.m_mustNotHaves.begin(), p_applyTags.m_mustNotHaves.end());
}


Tags DataTags::getAllUsedTags() const
{
	Tags allTags;
	
	for(TagGroup::const_iterator tagGroup(m_mustNotHaves.begin()); tagGroup != m_mustNotHaves.end(); ++tagGroup)
	{
		allTags.insert(tagGroup->begin(), tagGroup->end());
	}
	
	for(TagGroup::const_iterator tagGroup(m_mustHaves.begin()); tagGroup != m_mustHaves.end(); ++tagGroup)
	{
		allTags.insert(tagGroup->begin(), tagGroup->end());
	}
	
	return allTags;
}

#if !defined(TT_BUILD_FINAL)

bool DataTags::load(const xml::XmlNode* p_node, const DataTags& p_applyTags, const Tags& p_acceptedTags,
                    code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading DataTags from node '" << p_node->getName() << "'");
	
	m_name = p_node->getAttribute("name");
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "musthave")
		{
			Tags mustHaveGroup;
			str::Strings mustHaveStrGroup;
			
			loadTags(child, p_acceptedTags, &mustHaveGroup, &mustHaveStrGroup, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			TT_ERR_ASSERT(mustHaveGroup.size() == mustHaveStrGroup.size());
			
			m_mustHaves.push_back(mustHaveGroup);
			m_mustHavesStr.push_back(mustHaveStrGroup);
		}
		else if(child->getName() == "mustnothave")
		{
			Tags mustnotHaveGroup;
			str::Strings mustnotHaveStrGroup;
			
			loadTags(child, p_acceptedTags, &mustnotHaveGroup, &mustnotHaveStrGroup, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			TT_ERR_ASSERT(mustnotHaveGroup.size() == mustnotHaveStrGroup.size());
			
			m_mustNotHaves.push_back(mustnotHaveGroup);
			m_mustNotHavesStr.push_back(mustnotHaveStrGroup);
		}
		else
		{
			TT_ERR_ASSERTMSG(child->getName() != "tag", "Found unexpected 'tag' element as child of '" <<
			                 p_node->getName() << "' element. "
			                 "Tag elements should be child to 'musthave' or 'mustnothave' elements.");
		}
	}
	
	addDataTags(p_applyTags);
	
	TT_ERR_ASSERT(m_mustHaves.size() == m_mustHavesStr.size() && 
	              m_mustNotHaves.size() == m_mustNotHavesStr.size());
	return true;
}


bool DataTags::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN(bool, false, "Saving DataTags");
	TT_ERR_ASSERT(m_mustHaves.size() == m_mustHavesStr.size() && m_mustNotHaves.size() == m_mustNotHavesStr.size());
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	be_put(m_name, p_bufferOUT, p_sizeOUT);
	
	// must have
	be_put(static_cast<u16>(m_mustHavesStr.size()), p_bufferOUT, p_sizeOUT); // count of taggroups
	
	for(TagStrGroup::const_iterator tagGroup(m_mustHavesStr.begin()) ; tagGroup != m_mustHavesStr.end() ; ++tagGroup)
	{
		be_put(static_cast<u16>(tagGroup->size()), p_bufferOUT, p_sizeOUT); // count of tags
		
		for(str::Strings::const_iterator tag(tagGroup->begin()) ; tag != tagGroup->end() ; ++tag)
		{
			be_put(*tag, p_bufferOUT, p_sizeOUT); // tag
		}
	}
	
	// must not have
	be_put(static_cast<u16>(m_mustNotHavesStr.size()), p_bufferOUT, p_sizeOUT); // count of taggroups
	
	for(TagStrGroup::const_iterator tagGroup(m_mustNotHavesStr.begin()) ; tagGroup != m_mustNotHavesStr.end() ; ++tagGroup)
	{
		be_put(static_cast<u16>(tagGroup->size()), p_bufferOUT, p_sizeOUT); // count of tags
		
		for(str::Strings::const_iterator tag(tagGroup->begin()) ; tag != tagGroup->end() ; ++tag)
		{
			be_put(*tag, p_bufferOUT, p_sizeOUT); // tag
		}
	}
	
	return true;
}


size_t DataTags::getBufferSize() const
{
	// version + name size + name + musthaves count + 
	size_t size = 2 + 2 + m_name.size() + 2;
	for(TagStrGroup::const_iterator tagGroup(m_mustHavesStr.begin()) ; tagGroup != m_mustHavesStr.end() ; ++tagGroup)
	{
		size += 2; // group count
		for(str::Strings::const_iterator tag(tagGroup->begin()) ; tag != tagGroup->end() ; ++tag)
		{
			// tag size + tag
			size += 2 + tag->size();
		}
	}
	// mustNothaves count + 
	size += 2;
	for(TagStrGroup::const_iterator tagGroup(m_mustNotHavesStr.begin()) ; tagGroup != m_mustNotHavesStr.end() ; ++tagGroup)
	{
		size += 2; // group count
		for(str::Strings::const_iterator tag(tagGroup->begin()) ; tag != tagGroup->end() ; ++tag)
		{
			// tag count + tag
			size += 2 + tag->size();
		}
	}
	
	return size;
}


std::string DataTags::getDebugString() const
{
	std::stringstream out;
	if (m_name.empty())
	{
		out << "no name \n";
	}
	else
	{
		out << "name: '" << m_name << "'\n";
	}
	out << m_mustHaves.size() << " Must have Tag Groups: \n";
	for(TagGroup::const_iterator tagGroup(m_mustHaves.begin()); tagGroup != m_mustHaves.end(); ++tagGroup)
	{
		out << " - " << tagGroup->size() << " tags: ";
		for(Tags::const_iterator it(tagGroup->begin()) ; it != tagGroup->end() ; ++it)
		{
			out << it->getName() << ", ";
		}
		out << " \n ";
	}
	
	out << m_mustNotHaves.size() << " Must Not Have Tag Groups: \n";
	for(TagGroup::const_iterator tagGroup(m_mustNotHaves.begin()); tagGroup != m_mustNotHaves.end(); ++tagGroup)
	{
		out << " - " << tagGroup->size() << " tags: ";
		for(Tags::const_iterator it(tagGroup->begin()) ; it != tagGroup->end() ; ++it)
		{
			out << it->getName() << ", ";
			
		}
		out << " \n ";
	}
	
	return out.str();
}

str::StringSet DataTags::getAllUsedNames()const
{
	return m_allUsedNames;
}

#endif // !defined TT_BUILD_FINAL


//--------------------------------------------------------------------------------------------------
// Private member functions


bool DataTags::loadTags(const xml::XmlNode* p_node, const Tags& p_acceptedTags, Tags* p_tagsOut,
                        str::Strings* p_strTagsOut, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Loading DataTags from node '" << p_node->getName() << "'");
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "tag")
		{
			std::string tagstr(xml::util::parseStr(child, "name", &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			
			Tag tag(tagstr);
			
			// check if the tag is accepted. If the acceptedTags is empty all tags are allowed.
			TT_ERR_ASSERTMSG(p_acceptedTags.empty() || p_acceptedTags.find(tag) != p_acceptedTags.end(),
			                 "Unsupported Tag: '" << tagstr << "'");
			
			if (p_tagsOut->find(tag) != p_tagsOut->end())
			{
				std::string tagSet;
				for (tt::str::Strings::const_iterator it = p_strTagsOut->begin(); it != p_strTagsOut->end(); ++it)
				{
					if (it != p_strTagsOut->begin())
					{
						tagSet += ", ";
					}
					tagSet += (*it);
				}
				
				TT_ERR_AND_RETURN("Duplicate tag '" << tagstr << "' found in tagset '" << tagSet << "'.");
			}
			
			p_tagsOut->insert(tag);
			p_strTagsOut->push_back(tagstr);
		}
		else
		{
			TT_ERR_AND_RETURN("Expected 'tag' element in '" << p_node->getName() << "'. And recieved '" << child->getName() << "' element.");
		}
	}
	
	return true;
}


//namespace end
}
}
