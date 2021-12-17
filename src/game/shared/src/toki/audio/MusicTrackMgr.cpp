#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/fs.h>
#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>
#include <tt/code/helpers.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/audio/MusicTrackMgr.h>
#include <toki/constants.h>


namespace toki {
namespace audio {

//--------------------------------------------------------------------------------------------------
// Public member functions

MusicTrackMgr::MusicTrackMgr(s32 p_reserveCount)
:
tt::code::HandleArrayMgr<MusicTrack>(p_reserveCount),
m_availableMusicNames()
{
	gatherAvailableMusicNames();
	
	/*
	TT_Printf("MusicTrackMgr::MusicTrackMgr: Available music track names (%u):\n",
	          m_availableMusicNames.size());
	if (m_availableMusicNames.empty())
	{
		TT_Printf("MusicTrackMgr::MusicTrackMgr: { NONE }\n");
	}
	else
	{
		for (tt::str::StringSet::iterator it = m_availableMusicNames.begin();
		     it != m_availableMusicNames.end(); ++it)
		{
			TT_Printf("MusicTrackMgr::MusicTrackMgr: - '%s'\n", (*it).c_str());
		}
	}
	// */
}


MusicTrackMgr::~MusicTrackMgr()
{
}


MusicTrackHandle MusicTrackMgr::createTrack(const std::string& p_musicName)
{
	// Check if music exists
	if (m_availableMusicNames.find(p_musicName) == m_availableMusicNames.end())
	{
		TT_NONFATAL_PANIC("Cannot create track for music name '%s': this music file does not exist.",
		                  p_musicName.c_str());
		return MusicTrackHandle();
	}
	
	return create(MusicTrack::CreationParams(p_musicName));
}


void MusicTrackMgr::destroyTrack(const MusicTrackHandle& p_handle)
{
	destroy(p_handle);
	TT_ASSERT(getTrack(p_handle) == 0);
}


void MusicTrackMgr::update(real p_deltaTime)
{
	MusicTrack* track = getFirst();
	for (s32 i = getActiveCount(); i > 0; --i, ++track)
	{
		track->update(p_deltaTime);
	}
}


void MusicTrackMgr::pauseAllTracks()
{
	MusicTrack* track = getFirst();
	for (s32 i = getActiveCount(); i > 0; --i, ++track)
	{
		track->pause();
	}
}


void MusicTrackMgr::resumeAllTracks()
{
	MusicTrack* track = getFirst();
	for (s32 i = getActiveCount(); i > 0; --i, ++track)
	{
		track->resume();
	}
}


void MusicTrackMgr::handleMusicVolumeSettingChanged()
{
	MusicTrack* track = getFirst();
	for (s32 i = getActiveCount(); i > 0; --i, ++track)
	{
		track->handleMusicVolumeSettingChanged();
	}
}


void MusicTrackMgr::resetAll()
{
	reset();
}


void MusicTrackMgr::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	tt::code::serializeHandleArrayMgr(*this, p_context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	p_context->flush();
}


void MusicTrackMgr::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	tt::code::unserializeHandleArrayMgr(this, p_context);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void MusicTrackMgr::gatherAvailableMusicNames()
{
	TT_ASSERT(m_availableMusicNames.empty());
	
	if (tt::fs::dirExists(MusicTrack::ms_musicPath) == false)
	{
		TT_WARN("Music directory '%s' does not exist. No music is available for playback.",
		        MusicTrack::ms_musicPath.c_str());
		return;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(MusicTrack::ms_musicPath));
	if (dir == 0)
	{
		return;
	}
	
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		/* FIXME: Do we need recursion for music data?
		if (entry.isDirectory())
		{
			if (entry.getName() != "." && entry.getName() != "..")
			{
				gatherAvailableMusicNames(p_path + entry.getName() + "/");
			}
		}
		else
		// */
		if (tt::fs::utils::getExtension(entry.getName()) == MusicTrack::ms_musicExtension)
		{
			// Strip rootpath
			//const std::string path = p_path.substr(rootPathLength);
			//m_availableMusicNames.insert(path + tt::fs::utils::getFileTitle(entry.getName()));
			m_availableMusicNames.insert(tt::fs::utils::getFileTitle(entry.getName()));
		}
	}
}

// Namespace end
}
}
