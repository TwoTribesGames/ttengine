#if defined(TT_PLATFORM_OSX_IPHONE)

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#include <tt/audio/player/AVAudioMusicPlayer.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>



// -------------------------------------------------------------------------------------------------
// Obj-C code:

@interface TTdevObjCAVAudioMusicPlayerDelegate : NSObject<AVAudioPlayerDelegate>
{
	tt::audio::player::AVAudioMusicPlayer* m_player;
	
	bool m_songReachedEnd;
}

@property bool songReachedEnd;

- (id)init:(tt::audio::player::AVAudioMusicPlayer*)p_player;

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer*)p_player successfully:(BOOL)p_successful;
- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer*)p_player error:(NSError*)p_error;
- (void)audioPlayerBeginInterruption:(AVAudioPlayer*)p_player;
- (void)audioPlayerEndInterruption:(AVAudioPlayer*)p_player;
- (void)audioPlayerEndInterruption:(AVAudioPlayer*)p_player withFlags:(NSUInteger)p_flags;
@end


@implementation TTdevObjCAVAudioMusicPlayerDelegate
@synthesize songReachedEnd = m_songReachedEnd;

- (id)init:(tt::audio::player::AVAudioMusicPlayer*)p_player
{
	self = [super init];
	if (self != nil)
	{
		m_player = p_player;
		TT_NULL_ASSERT(m_player);
		if (m_player == 0)
		{
			[self release];
			self = 0;
		}
		
		m_songReachedEnd = false;
	}
	return self;
}


- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer*)p_player successfully:(BOOL)p_successful
{
	// Called when a sound has finished playing.
	//NSLog(@"TTdevObjCAVAudioMusicPlayerDelegate::audioPlayerDidFinishPlaying: Player finished playing %s.",
	//      p_successful ? "successfully" : "with ERROR");
	(void)p_player;
	(void)p_successful;
	TT_ASSERTMSG(p_successful, "Music playback finished with an error.");
	m_songReachedEnd = true;
}


- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer*)p_player error:(NSError*)p_error
{
	(void)p_player;
	(void)p_error;
	// Called when an audio player encounters a decoding error during playback.
	TT_PANIC("AVAudioMusicPlayer playback encountered an audio decoding error: '%s'",
	         [[p_error localizedDescription] UTF8String]);
}


- (void)audioPlayerBeginInterruption:(AVAudioPlayer*)p_player
{
	// Called when an audio player is interrupted, such as by an incoming phone call.
	// TODO: Probably nothing to do here. Double check.
	(void)p_player;
}


- (void)audioPlayerEndInterruption:(AVAudioPlayer*)p_player
{
	// Called after your audio session interruption ends.
	// TODO: Probably nothing to do here. Double check.
	(void)p_player;
}


- (void)audioPlayerEndInterruption:(AVAudioPlayer*)p_player withFlags:(NSUInteger)p_flags
{
	// Called after your audio session interruption ends, with flags indicating the state of the audio session.
	// TODO: Probably nothing to do here. Double check.
	(void)p_player;
	(void)p_flags;
}

@end



// -------------------------------------------------------------------------------------------------
// CPP code:

namespace tt {
namespace audio {
namespace player {



// -------------------------------------------------------------------------------------------------
// Public functions

AVAudioMusicPlayer::AVAudioMusicPlayer()
:
m_player(0),
m_playerDelegate(0),
m_paused(false),
m_looping(false),
m_isPlayingJingle(false),
m_songResumePosition(0),
m_songResumeLooping(false)
{
}


AVAudioMusicPlayer::~AVAudioMusicPlayer()
{
	// stop playing
	if (isPlaying())
	{
		stop();
	}
	
	TT_ASSERT(m_player == 0);
	TT_ASSERT(m_playerDelegate == 0);
}


// Playback functions

bool AVAudioMusicPlayer::play(const std::string& p_song, bool p_looping)
{
	m_isPlayingJingle = false;
	return playImpl(p_song, p_looping, 0);
}


bool AVAudioMusicPlayer::stop()
{
	if (m_player != 0)
	{
		AVAudioPlayer* player = static_cast<AVAudioPlayer*>(m_player);
		[player stop];
		[player release];
		m_player = 0;
		
		// FIXME: Does the delegate need to be released separately?
		m_playerDelegate = 0;
	}
	
	m_paused = false;
	m_currentSong.clear();
	
	return true;
}


bool AVAudioMusicPlayer::pause()
{
	if (m_currentSong.empty())
	{
		return false;
	}
	if (m_paused)
	{
		return true;
	}
	
	m_paused = true;
	
	if (m_player != 0)
	{
		AVAudioPlayer* player = static_cast<AVAudioPlayer*>(m_player);
		[player pause];
	}
	
	return true;
}


bool AVAudioMusicPlayer::resume()
{
	if (m_currentSong.empty())
	{
		return false;
	}
	if (m_paused == false)
	{
		return true;
	}
	
	m_paused = false;
	
	if (m_player != 0)
	{
		AVAudioPlayer* player = static_cast<AVAudioPlayer*>(m_player);
		[player play];
	}
	
	
	return true;
}


bool AVAudioMusicPlayer::playJingle(const std::string& p_jingle)
{
	if (m_isPlayingJingle == false)
	{
		// Remember the current position of the song so we can restore it.
		m_songResumePosition = 0;
		
		if (m_player != 0)
		{
			AVAudioPlayer* player = static_cast<AVAudioPlayer*>(m_player);
			m_songResumePosition = [player currentTime];
		}
		
		m_songResumeName       = m_currentSong;
		m_songResumeLooping    = m_looping;
		
		m_isPlayingJingle    = true;
	}
	
	return playImpl(p_jingle, false, 0);
}


void AVAudioMusicPlayer::update()
{
	// TODO: Player update?
	// Check for end of jingle
	if (m_isPlayingJingle &&
	    static_cast<TTdevObjCAVAudioMusicPlayerDelegate*>(m_playerDelegate).songReachedEnd)
	{
		if (m_songResumeName.empty() == false)
		{
			playImpl(m_songResumeName, m_songResumeLooping, m_songResumePosition);
			m_songResumeName.clear();
		}
		m_isPlayingJingle = false;
	}
}


// Status functions

const std::string& AVAudioMusicPlayer::getCurrentSong() const
{
	return m_currentSong;
}


bool AVAudioMusicPlayer::isPlaying() const
{
	return m_currentSong.empty() == false;
}


bool AVAudioMusicPlayer::isPaused() const
{
	return m_paused;
}


bool AVAudioMusicPlayer::isLooping() const
{
	return m_looping;
}


// -------------------------------------------------------------------------------------------------
// Private functions

void AVAudioMusicPlayer::updateVolume(real p_volume)
{
	if (m_player != 0)
	{
		AVAudioPlayer* player = static_cast<AVAudioPlayer*>(m_player);
		player.volume = realToFloat(p_volume);
	}
}


bool AVAudioMusicPlayer::playImpl(const std::string& p_song, bool p_looping, u8 p_resumePosition)
{
	if (stop() == false)
	{
		return false;
	}
	
	m_currentSong = p_song;
	m_looping     = p_looping;
	m_paused      = false;
	
	if (tt::fs::fileExists(p_song) == false)
	{
		TT_PANIC("Can't find file: '%s'", p_song.c_str());
		return false;
	}
	
	NSString* nsAbsPath = [[NSBundle mainBundle] bundlePath];
	nsAbsPath = [nsAbsPath stringByAppendingString:@"/"];
	nsAbsPath = [nsAbsPath stringByAppendingString:[NSString stringWithUTF8String:p_song.c_str()]];
	
	NSURL* fileURL      = [[[NSURL alloc] initFileURLWithPath:nsAbsPath isDirectory: NO] autorelease];
	
	/* // Can't use this. It's iOS 4.0 only! (And doesn't work in simulator.)
	NSError* urlError = nil;
	if ([fileURL checkResourceIsReachableAndReturnError: &urlError] == NO)
	{
		TT_PANIC("Couldn't reach resource: '%s' (bundle: '%s'). Error: '%s'", 
		         p_song.c_str(), [nsAbsPath UTF8String],
		         [[urlError localizedDescription] UTF8String]);
		return false;
	}
	*/
	
	if (m_player == 0)
	{
		AVAudioPlayer* player = [AVAudioPlayer alloc];
		m_player = static_cast<void*>(player);
		
		TTdevObjCAVAudioMusicPlayerDelegate* playerDelegate =
			[[TTdevObjCAVAudioMusicPlayerDelegate alloc] init:this];
		m_playerDelegate = playerDelegate;
	}
	TT_NULL_ASSERT(m_player);
	TT_NULL_ASSERT(m_playerDelegate);
	
	AVAudioPlayer* player = static_cast<AVAudioPlayer*>(m_player);
	NSError* initError = nil;
	[player initWithContentsOfURL: fileURL error: &initError];
	
	TT_ASSERTMSG(initError == nil,
	             "Error in init of AVAudioPlayer. '%s' reason: '%s'",
			     [[initError localizedDescription]   UTF8String],
				 [[initError localizedFailureReason] UTF8String]);
	
	TTdevObjCAVAudioMusicPlayerDelegate* playerDelegate =
		static_cast<TTdevObjCAVAudioMusicPlayerDelegate*>(m_playerDelegate);
	playerDelegate.songReachedEnd = false;
	player.delegate = playerDelegate;
	
	[player setCurrentTime: p_resumePosition];
	BOOL result = [player prepareToPlay];
	TT_ASSERTMSG(result == YES, 
	             "AVAudioMusicPlayer failed to prepareToPlay - '%s'. Will try to play anyway.", 
	             p_song.c_str());
	
	// Set the current volume to the new player. (Otherwise the default volume is used.)
	player.volume = realToFloat(getPlaybackVolume());
	
	player.numberOfLoops = (m_looping) ? -1 : 0;
	
	result = [player play];
	TT_ASSERTMSG(result == YES, 
	             "AVAudioMusicPlayer failed to play - '%s'!", 
	             p_song.c_str());
	
	return true;
}

// Namespace end
}
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
