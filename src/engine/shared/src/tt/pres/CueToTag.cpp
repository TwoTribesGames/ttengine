#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/pres/CueToTag.h>
#include <tt/str/parse.h>
#include <tt/xml/fwd.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>



namespace tt {
namespace pres {


const Tags CueToTag::ms_emptyTags = Tags();


CueToTag::CueToTag()
:
m_cueMapping(),
m_loaded(false)
{
}


bool CueToTag::load(const std::string& p_file, const Cues& p_requiredCues, 
                    Tags* p_usedTagsOUT, code::ErrorStatus* p_errStatus)
{
	m_loaded = false;
	m_cueMapping.clear();
	if(p_usedTagsOUT != 0) p_usedTagsOUT->clear();
	
	TT_ERR_CHAIN(bool, false, "Loading CueToTag file " << p_file);
	TT_ERR_ASSERTMSG(fs::fileExists(p_file), "CueToTag file '" << p_file << "' does not exist.");
	xml::XmlDocumentPtr doc(new xml::XmlDocument(p_file));
	TT_ERR_ASSERTMSG(doc->getRootNode() != 0, "Loading of " << p_file << " failed.");
	
	const xml::XmlNode* rootNode(doc->getRootNode());
	TT_ERR_ASSERTMSG(rootNode->getName() == "cuetotag", 
	                 "Rootnode of cue to tag should be 'cuetotag' and not '" << rootNode->getName()
	                 << "'. In file " << p_file); 
	
	// hold a string set of the tags to check for tags with the same hash
	typedef std::set<std::string> StringSet;
	StringSet strTags;
	
	// parse cues
	for(const xml::XmlNode* cueNode(rootNode->getChild()) ; cueNode != 0 ; cueNode = cueNode->getSibling())
	{
		TT_ERR_ASSERTMSG(cueNode->getName() == "cue", 
		                "childs of cuetotag elements should be 'cue' and not '" << cueNode->getName()
		                << "'. In file " << p_file); 
		
		// parse cue name
		TT_ERR_ASSERTMSG(cueNode->getAttribute("name").empty() == false, 
		                "cue elements should have a name atribute. In file " << p_file);
		Cue cue(cueNode->getAttribute("name"));
		
		TT_ERR_ASSERTMSG(p_requiredCues.find(cue) != p_requiredCues.end(), 
		                 "the cue name '" << cueNode->getAttribute("name") << 
		                 "' in file '" << p_file << "' is not an accepted cue");
		
		// parse tagsets
		TagSets tagSets;
		
		for(const xml::XmlNode* tagsetNode(cueNode->getChild()) ; tagsetNode != 0 ; 
		    tagsetNode = tagsetNode->getSibling())
		{
			TT_ERR_ASSERTMSG(tagsetNode->getName() == "tagset", 
			                 "childs of cue elements should be 'tagset' and not '" 
			                 << tagsetNode->getName() << "'. In file " << p_file); 
			
			TagSet tagset;
			// parse tagset weight
			TT_ERR_ASSERTMSG(tagsetNode->getAttribute("weight").empty() == false, 
			                "tagset elements should have a weight atribute. In file " << p_file);
			tagset.weight = str::parseS32(tagsetNode->getAttribute("weight"), &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			
			// parse tags
			for(const xml::XmlNode* tagNode(tagsetNode->getChild()) ; tagNode != 0 ; 
			    tagNode = tagNode->getSibling())
			{
				TT_ERR_ASSERTMSG(tagNode->getName() == "tag", 
				                 "childs of tagset elements should be 'tag' and not '" 
				                 << tagNode->getName() << "'. In file " << p_file);
				
				// parse tag name
				TT_ERR_ASSERTMSG(tagNode->getAttribute("name").empty() == false, 
				                "tag elements should have a name atribute. In file " << p_file);
				Tag tag(tagNode->getAttribute("name"));
				
				if(p_usedTagsOUT != 0)
				{
					// check for duplicate hashes
					TT_ERR_ASSERTMSG(p_usedTagsOUT->find(tag) == p_usedTagsOUT->end() || 
					                 strTags.find(tag.getName()) != strTags.end(),
							"Two tag strings have the same hash value, choose a different"
							" name for the tag: " << tag.getName());
					// hold string verion of tag
					strTags.insert(tag.getName());
					// add to accepted tags
					p_usedTagsOUT->insert(tag);
				}
				
				tagset.tags.insert(tag);
			}
			
			tagSets.push_back(tagset);
		}
		
		m_cueMapping.insert(std::make_pair(cue, tagSets));
	}
	// check if all required cues are present
	if(m_cueMapping.size() != p_requiredCues.size())
	{
		for(Cues::const_iterator it(p_requiredCues.begin()) ; it != p_requiredCues.end() ; ++it)
		{
			if(m_cueMapping.find(*it) == m_cueMapping.end())
			{
				TT_PANIC("The cue '%s' was not found in the cue to tag file '%s'", 
				         it->getName().c_str(), p_file.c_str());
			}
		}
		TT_PANIC("CueToTag file '%s' does not have all tags or has not existing tags", p_file.c_str());
	}
	
	m_loaded = true;
	return true;
}


const Tags& CueToTag::cueToTag(const Cue& p_cue, math::Random& p_random) const
{
	TT_ASSERTMSG(m_loaded, "Cue to tag file failed loading or is missing. Or load did not get called");
	if(m_loaded == false) 
	{
		return ms_emptyTags;
	}
	
	// can't show the name of the que. it is only available in debug builds.
	TT_ASSERTMSG(m_cueMapping.find(p_cue) != m_cueMapping.end(), "Cue not found in Cue to tag file.");
	
	const TagSets& tagSets(m_cueMapping.find(p_cue)->second);
	
	// early out (90% of all cues only contains 1 tagSet)
	if (tagSets.size() == 1)
	{
		return tagSets[0].tags;
	}
	
	s32 totalWeight = 0;
	
	for(TagSets::const_iterator it(tagSets.begin()); it != tagSets.end(); ++it)
	{
		totalWeight += it->weight;
	}
	
	s32 randomWeight(static_cast<s32>(p_random.getNext(static_cast<u32>(totalWeight))));
	
	for(TagSets::const_iterator it(tagSets.begin()); it != tagSets.end(); ++it)
	{
		if(randomWeight < it->weight)
		{
			return it->tags;
		}
		
		randomWeight -= it->weight;
	}
	
	TT_PANIC("Error calculating weighted random tagset");
	return ms_emptyTags;
}

//namespace end
}
}
