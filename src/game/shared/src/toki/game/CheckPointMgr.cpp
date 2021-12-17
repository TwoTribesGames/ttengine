#include <tt/code/Buffer.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/CheckPointMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions


CheckPointMgrPtr CheckPointMgr::create(ProgressType p_progressType)
{
	switch (p_progressType)
	{
	                                                              // The total is 24.5 MB
	case ProgressType_Main:      return create(24 * 1024 * 1024); // 24 MB
	case ProgressType_UserLevel: return create(1 * 512 * 1024);   // 0.5 MB
	default:
		TT_PANIC("Unknown ProgressType: %d", p_progressType);
		return CheckPointMgrPtr();
	}
}


CheckPointMgrPtr CheckPointMgr::create(u32 p_sizeLimitCheckpoints)
{
	return CheckPointMgrPtr(new CheckPointMgr(p_sizeLimitCheckpoints));
}


void CheckPointMgr::setCheckPoint(const serialization::SerializationMgrPtr& p_checkPointData,
                                  const std::string&                        p_id)
{
	// Check if checkpoint with this ID already exists
	CheckPoints::iterator existingCheckpointIt = m_checkPoints.find(p_id);
	
	if (existingCheckpointIt == m_checkPoints.end() && m_totalSize >= m_sizeLimitCheckpoints)
	{
		TT_PANIC("setCheckPointData called for checkpoint '%s' while CheckPointMgr is at its size limit."
		         "Total size %d, size limit %d. Please remove checkpoint(s).", 
		         p_id.c_str(), m_totalSize, m_sizeLimitCheckpoints);
		return;
	}
	
	TT_NULL_ASSERT(p_checkPointData);
	if (p_checkPointData != 0)
	{
		tt::code::BufferPtr compressedPtr = p_checkPointData->compress();
		TT_NULL_ASSERT(compressedPtr);
		if (compressedPtr != 0)
		{
			if (existingCheckpointIt != m_checkPoints.end())
			{
				// Overwriting old one
				m_totalSize -= existingCheckpointIt->second->getSize();
				existingCheckpointIt->second = compressedPtr;
				
				TT_Printf("# Replaced checkpoint with ID '%s'. Size: %d KB. Compressed size: %d KB. ", 
					p_id.c_str(),
					p_checkPointData->getTotalSize() / 1024,
					compressedPtr->getSize() / 1024);
			}
			else
			{
				// Adding new checkpoint
				m_checkPoints[p_id] = compressedPtr;
				
				TT_Printf("# Added checkpoint '%s'. Size: %d KB. Compressed size: %d KB. ", 
					p_id.c_str(),
					p_checkPointData->getTotalSize() / 1024,
					compressedPtr->getSize() / 1024);
			}
			m_totalSize += compressedPtr->getSize();
			
			TT_Printf("Total checkpoints: %d. Size left: %d KB\n",
				m_checkPoints.size(), static_cast<s32>(m_sizeLimitCheckpoints - m_totalSize) / 1024);
		}
	}
}


serialization::SerializationMgrPtr CheckPointMgr::getCheckPoint(const std::string& p_id) const
{
	CheckPoints::const_iterator it = m_checkPoints.find(p_id);
	
	if(it != m_checkPoints.end())
	{
		return serialization::SerializationMgr::createFromCompressedBuffer(it->second);
	}
	
	TT_PANIC("Checkpoint ID '%s' not found", p_id.c_str());
	return serialization::SerializationMgrPtr();
}


tt::str::Strings CheckPointMgr::getAllCheckPointIDs() const
{
	tt::str::Strings result;
	for(CheckPoints::const_iterator it = m_checkPoints.begin(); it != m_checkPoints.end(); ++it)
	{
		result.push_back(it->first);
	}
	return result;
}


bool CheckPointMgr::hasCheckPoint(const std::string& p_id) const
{
	return m_checkPoints.find(p_id) != m_checkPoints.end();
}


bool CheckPointMgr::renameCheckPoint(const std::string& p_oldID, const std::string& p_newID)
{
	CheckPoints::iterator it = m_checkPoints.find(p_oldID);
	if (it != m_checkPoints.end())
	{
		const tt::code::BufferPtr bufferPtr(it->second);
		m_checkPoints.erase(it);
		
		it = m_checkPoints.find(p_newID);
		if (it != m_checkPoints.end())
		{
			m_totalSize -= (*it).second->getSize();
			m_checkPoints.erase(it);
			TT_Printf("# Renaming (replace) checkpoint '%s' to '%s'. Size left: %d KB.\n", p_oldID.c_str(), p_newID.c_str(),
				static_cast<s32>(m_sizeLimitCheckpoints - m_totalSize) / 1024);
		}
		else
		{
			TT_Printf("# Renaming checkpoint '%s' to '%s'. Size left: %d KB.\n", p_oldID.c_str(), p_newID.c_str(),
				static_cast<s32>(m_sizeLimitCheckpoints - m_totalSize) / 1024);
		}
		
		m_checkPoints[p_newID] = bufferPtr;
		
		return true;
	}
	return false;
}


void CheckPointMgr::resetCheckPoint(const std::string& p_id)
{
	CheckPoints::iterator it = m_checkPoints.find(p_id);
	if (it != m_checkPoints.end())
	{
		m_totalSize -= (*it).second->getSize();
		m_checkPoints.erase(it);
		TT_Printf("# Removed checkpoint '%s'. Total checkpoints: %d. Size left: %d KB\n",
			p_id.c_str(),
			m_checkPoints.size(),
			static_cast<s32>(m_sizeLimitCheckpoints - m_totalSize) / 1024);
	}
}


void CheckPointMgr::resetAllCheckPoints()
{
	m_checkPoints.clear();
	m_totalSize = 0;
	TT_Printf("# All checkpoints removed.\n");
}


bool CheckPointMgr::save(tt::fs::FilePtr& p_file) const
{
	// Write number of checkpoints
	if(tt::fs::writeInteger<u32>(p_file, static_cast<u32>(m_checkPoints.size())) == false)
	{
		TT_PANIC("Couldn't write number of checkpoints to savedata.");
		return false;
	}
	
	for(CheckPoints::const_iterator it = m_checkPoints.begin(); it != m_checkPoints.end(); ++it)
	{
		// Write name
		if(tt::fs::writeNarrowString(p_file, it->first) == false)
		{
			TT_PANIC("Couldn't write name '%s' of checkpoint to savedata.",
				it->first.c_str());
			return false;
		}
		
		// Write checkpoint compressed size
		if(tt::fs::writeInteger<u32>(p_file, it->second->getSize()) == false)
		{
			TT_PANIC("Couldn't write compressed size '%d' of checkpoint '%s' to savedata.",
				it->second->getSize(), it->first.c_str());
			return false;
		}
		
		// Write checkpoint compressed data
		if(tt::fs::write(p_file, it->second->getData(), it->second->getSize()) != it->second->getSize())
		{
			TT_PANIC("Couldn't write compressed data of checkpoint '%s' to savedata.",
				it->first.c_str());
			return false;
		}
	}
	
	return true;
}


bool CheckPointMgr::load(const tt::fs::FilePtr& p_file)
{
	m_checkPoints.clear();
	m_totalSize = 0;
	
	u32 checkPointCount(0);
	if(tt::fs::readInteger(p_file, &checkPointCount) == false)
	{
		TT_PANIC("Couldn't read number of checkpoints from savedata.");
		return false;
	}
	
	for(u32 i = 0; i < checkPointCount; ++i)
	{
		std::string id;
		if(tt::fs::readNarrowString(p_file, &id) == false)
		{
			TT_PANIC("Couldn't read name of checkpoint from savedata.");
			return false;
		}
		
		u32 compressedSize = 0;
		if(tt::fs::readInteger<u32>(p_file, &compressedSize) == false)
		{
			TT_PANIC("Couldn't read compressed size of checkpoint '%s' from savedata.",
				id.c_str());
			return false;
		}
		
		tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(compressedSize));
		
		if(tt::fs::read(p_file, buffer->getData(), compressedSize) !=
		   static_cast<tt::fs::size_type>(compressedSize))
		{
			TT_PANIC("Couldn't read compressed data of checkpoint '%s' from savedata.",
				id.c_str());
			return false;
		}
		
		m_checkPoints[id] = buffer;
		m_totalSize += buffer->getSize();
	}
	
	return true;
}


CheckPointMgrPtr CheckPointMgr::clone() const
{
	return CheckPointMgrPtr(new CheckPointMgr(*this));
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CheckPointMgr::CheckPointMgr(u32 p_sizeLimitCheckpoints)
:
m_checkPoints(),
m_totalSize(0),
m_sizeLimitCheckpoints(p_sizeLimitCheckpoints)
{
}


CheckPointMgr::CheckPointMgr(const CheckPointMgr& p_rhs)
:
m_checkPoints(), // NOTE: Will be copied in the constructor body
m_totalSize           (p_rhs.m_totalSize),
m_sizeLimitCheckpoints(p_rhs.m_sizeLimitCheckpoints)
{
	// Clone all checkpoints
	for (CheckPoints::const_iterator it = p_rhs.m_checkPoints.begin();
	     it != p_rhs.m_checkPoints.end(); ++it)
	{
		m_checkPoints[(*it).first] = (*it).second->clone();
	}
}


// Namespace end
}
}
