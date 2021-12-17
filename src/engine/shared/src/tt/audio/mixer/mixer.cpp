#include <vector>

#include <tt/audio/mixer/mixer.h>
#include <tt/math/Random.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace audio {
namespace mixer {

struct Channel
{
	bool active; //!< Whether this channel is active
	
	// playback parameters
	freq_type freq;     //!< Frequency in Hz
	vol_type  vol;      //!< Volume (0 - 256)
	pan_type  pan;      //!< Panning (-128 (left) - 128 (right))
	s64       position; //!< Current position
	s64       delta;    //!< Position update per mixer frame
	
	// PCM parameters
	const void* sample;     //!< Sample data, 0 implies PSG
	SampleType  format;     //!< Sample format
	LoopType    loop;       //!< Loop type
	s64         loopStart;  //!< Loop start in samples
	s64         dataLength; //!< Data length in samples
	bool        direction;  //!< Current direction (true = forward, false = backward)
	
	// PSG parameters
	duty_type duty; //!< Duty ratio, 0 implies noise
	
	// Noise parameters
	NoiseType noise; //!< Noise type
	
	Channel()
	:
	active(false),
	freq(0),
	vol(0),
	pan(0),
	position(0),
	delta(0),
	sample(0),
	format(SampleType_PCM8),
	loop(LoopType_None),
	loopStart(0),
	dataLength(0),
	direction(true),
	duty(0),
	noise(NoiseType_White)
	{ }
};

typedef std::vector<Channel> Channels;

static Channels  s_channels;
static freq_type s_freq     = 44100; //!< Mixer sample rate
static vol_type  s_volume   = 256;   //!< Master volume
static pan_type  s_panning  = 0;     //!< Master panning
static callback  s_callback = 0;     //!< Mixer callback function
static void*     s_param    = 0;     //!< Mixer callback parameter
static size_type s_interval = 0;     //!< Mixer callback interval
static size_type s_count    = 0;     //!< Mixer callback interval counter

// Private functions
static void mixChannelInterleaved(channel_type p_channel, s16* p_buffer, size_type p_frames);


// External constants
const vol_type c_minVol = 0;   //!< Minimum volume
const vol_type c_maxVol = 256; //!< Maximum volume

const pan_type c_minPan    = -128; //!< Minimum panning
const pan_type c_maxPan    =  128; //!< Maximum panning
const pan_type c_leftPan   = -128; //!< Full left panning
const pan_type c_centerPan =    0; //!< Centered panning
const pan_type c_rightPan  =  128; //!< Full right panning

const duty_type c_minDuty =   1; //!< Minimum duty
const duty_type c_maxDuty = 255; //!< Maximum duty
const duty_type c_1_8Duty =  32; //!< 1/8th duty (12.5%)
const duty_type c_2_8Duty =  64; //!< 2/8th duty (25.0%)
const duty_type c_3_8Duty =  96; //!< 3/8th duty (37.5%)
const duty_type c_4_8Duty = 128; //!< 4/8th duty (50.0%)
const duty_type c_5_8Duty = 160; //!< 5/8th duty (62.5%)
const duty_type c_6_8Duty = 192; //!< 6/8th duty (75.0%)
const duty_type c_7_8Duty = 224; //!< 7/8th duty (87.5%)


bool initialize(channel_type p_channels, freq_type p_sampleRate)
{
	if (s_channels.empty() == false)
	{
		TT_PANIC("Mixer already initalized.");
		return false;
	}
	if (p_channels <= 0)
	{
		TT_PANIC("Channel count (%d) must be larger than 0.", p_channels);
		return false;
	}
	if (p_sampleRate <= 0)
	{
		TT_PANIC("Sample rate (%d) must be larger than 0.", p_sampleRate);
		return false;
	}
	
	s_channels.resize(static_cast<Channels::size_type>(p_channels));
	
	// reset internal state
	s_freq     = p_sampleRate;
	s_volume   = 256;
	s_panning  = 0;
	s_callback = 0;
	s_param    = 0;
	s_interval = 0;
	s_count    = 0;
	
	return true;
}


bool end()
{
	if (s_channels.empty())
	{
		TT_PANIC("Mixer already ended or never initialized.");
		return false;
	}
	
	// reset internal state
	s_channels.clear();
	s_freq     = 44100;
	s_volume   = 256;
	s_panning  = 0;
	s_callback = 0;
	s_param    = 0;
	s_interval = 0;
	s_count    = 0;
	
	return true;
}


vol_type setMasterVolume(vol_type p_vol)
{
	TT_ASSERTMSG(p_vol >= c_minVol && p_vol <= c_maxVol,
	             "Volume %d out of bounds [%d - %d].",
	             p_vol, c_minVol, c_maxVol);
	vol_type ret = s_volume;
	s_volume = p_vol;
	return ret;
}


pan_type setMasterPanning(pan_type p_pan)
{
	TT_ASSERTMSG(p_pan >= c_minPan && p_pan <= c_maxPan,
	             "Panning %d out of bounds [%d - %d].",
	             p_pan, c_minPan, c_maxPan);
	pan_type ret = s_panning;
	s_panning = p_pan;
	return ret;
}


callback setCallback(callback p_callback, void* p_param)
{
	callback ret = s_callback;
	s_callback = p_callback;
	s_param    = p_param;
	return ret;
}


bool setupPCM(channel_type p_channel,
              SampleType   p_format,
              const void*  p_data,
              LoopType     p_loop,
              size_type    p_loopStart,
              size_type    p_dataLength,
              freq_type    p_freq,
              vol_type     p_volume,
              pan_type     p_pan)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	TT_ASSERTMSG(p_data != 0, "No sample data specified.");
	
	TT_ASSERTMSG(p_dataLength > 0, "Invalid data length %d.", p_dataLength);
	TT_ASSERTMSG(p_loopStart >= 0 && p_loopStart < p_dataLength,
	             "Loopstart %d out of bounds [0 - %u).",
	             p_loopStart, p_dataLength);
	
	TT_ASSERTMSG(p_freq > 0, "Invalid frequency %d.", p_freq);
	
	TT_ASSERTMSG(p_volume >= c_minVol && p_volume <= c_maxVol,
	             "Volume %d out of bounds [%d - %d].",
	             p_volume, c_minVol, c_maxVol);
	
	TT_ASSERTMSG(p_pan >= c_minPan && p_pan <= c_maxPan,
	             "Panning %d out of bounds [%d - %d].",
	             p_pan, c_minPan, c_maxPan);
	
	// all parameters verified
	
	Channel& chan = s_channels[channel];
	if (chan.active)
	{
		// stop channel first
		setActive(p_channel, false);
	}
	
	// common parameters
	chan.freq     = p_freq;
	chan.vol      = p_volume;
	chan.pan      = p_pan;
	chan.position = 0;
	chan.delta    = (s64(p_freq) << 32) / s_freq;
	
	// PCM parameters
	chan.sample     = p_data;
	chan.format     = p_format;
	chan.loop       = p_loop;
	chan.loopStart  = s64(p_loopStart) << 32;
	chan.dataLength = s64(p_dataLength) << 32;
	
	return true;
}


bool setupPSG(channel_type p_channel,
              duty_type    p_duty,
              freq_type    p_freq,
              vol_type     p_volume,
              pan_type     p_pan)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	TT_ASSERTMSG(p_freq > 0, "Invalid frequency %d.", p_freq);
	
	TT_ASSERTMSG(p_volume >= c_minVol && p_volume <= c_maxVol,
	             "Volume %d out of bounds [%d - %d].",
	             p_volume, c_minVol, c_maxVol);
	
	TT_ASSERTMSG(p_pan >= c_minPan && p_pan <= c_maxPan,
	             "Panning %d out of bounds [%d - %d].",
	             p_pan, c_minPan, c_maxPan);
	
	TT_ASSERTMSG(p_duty >= c_minDuty && p_duty <= c_maxDuty,
	             "Duty %d out of bounds [%d - %d].",
	             p_duty, c_minDuty, c_maxDuty);
	
	// all parameters verified
	
	Channel& chan = s_channels[channel];
	if (chan.active)
	{
		// stop channel first
		setActive(p_channel, false);
	}
	
	// common parameters
	chan.freq     = p_freq;
	chan.vol      = p_volume;
	chan.pan      = p_pan;
	chan.position = 0;
	chan.delta    = (s64(p_freq) << 32) / s_freq;
	
	// PSG parameters
	chan.duty = p_duty;
	
	return true;
}


bool setupNoise(channel_type p_channel,
                NoiseType    p_noise,
                freq_type    p_freq,
                vol_type     p_volume,
                pan_type     p_pan)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	TT_ASSERTMSG(p_freq > 0, "Invalid frequency %d.", p_freq);
	
	TT_ASSERTMSG(p_volume >= c_minVol && p_volume <= c_maxVol,
	             "Volume %d out of bounds [%d - %d].",
	             p_volume, c_minVol, c_maxVol);
	
	TT_ASSERTMSG(p_pan >= c_minPan && p_pan <= c_maxPan,
	             "Panning %d out of bounds [%d - %d].",
	             p_pan, c_minPan, c_maxPan);
	
	// all parameters verified
	
	Channel& chan = s_channels[channel];
	if (chan.active)
	{
		// stop channel first
		setActive(p_channel, false);
	}
	
	// common parameters
	chan.freq     = p_freq;
	chan.vol      = p_volume;
	chan.pan      = p_pan;
	chan.position = 0;
	chan.delta    = (s64(p_freq) << 32) / s_freq;
	
	// Noise parameters
	chan.noise = p_noise;
	
	return true;
}


bool setFrequency(channel_type p_channel, freq_type p_freq)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	TT_ASSERTMSG(p_freq > 0, "Invalid frequency %d.", p_freq);
	
	// all parameters verified
	
	Channel& chan = s_channels[channel];
	chan.freq = p_freq;
	
	return true;
}


bool setVolume(channel_type p_channel, vol_type p_vol)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	TT_ASSERTMSG(p_vol >= c_minVol && p_vol <= c_maxVol,
	             "Volume %d out of bounds [%d - %d].",
	             p_vol, c_minVol, c_maxVol);
	
	// all parameters verified
	
	Channel& chan = s_channels[channel];
	chan.vol = p_vol;
	
	return true;
}


bool setPanning(channel_type p_channel, pan_type p_pan)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	TT_ASSERTMSG(p_pan >= c_minPan && p_pan <= c_maxPan,
	             "Panning %d out of bounds [%d - %d].",
	             p_pan, c_minPan, c_maxPan);
	
	// all parameters verified
	
	Channel& chan = s_channels[channel];
	chan.pan = p_pan;
	
	return true;
}


bool setActive(channel_type p_channel, bool p_active)
{
	Channels::size_type channel(p_channel);
	TT_ASSERTMSG(p_channel >= 0 && channel < s_channels.size(),
	             "Channel %d out of bounds [0 - %u).",
	             p_channel, s_channels.size());
	
	Channel& chan = s_channels[channel];
	bool ret = chan.active;
	/*
	if (ret != p_active)
	{
		if (p_active)
		{
			// TODO: dunno
		}
		else
		{
			// TODO: declicker
		}
	}
	*/
	chan.active = p_active;
	return ret;
}


size_type mixInterleaved(size_type p_frames,
                         s16*      p_buffer,
                         bool      p_clear)
{
	TT_ASSERTMSG(p_buffer != 0, "No buffer specified.");
	TT_ASSERTMSG(p_frames >= 0, "Invalid frame count %d.", p_frames);
	
	if (p_clear)
	{
		mem::zero8(p_buffer, sizeof(u16) * p_frames * 2);
	}
	
	size_type todo = p_frames;
	
	while (todo != 0)
	{
		size_type toMix = todo;
		
		if (s_callback != 0 && s_count == 0)
		{
			(*s_callback)(s_param);
			s_count = s_interval;
			toMix = s_interval > todo ? todo : s_interval;
		}
		
		for (channel_type i = 0; i < static_cast<channel_type>(s_channels.size()); ++i)
		{
			if (s_channels[i].active)
			{
				mixChannelInterleaved(i, p_buffer, toMix);
			}
		}
		
		// TODO: Declicker
		
		p_buffer += toMix * 2;
		todo -= toMix;
	}
	
	return p_frames;
}


// Private functions

static void mixChannelInterleaved(channel_type p_channel, s16* p_buffer, size_type p_frames)
{
	Channels::size_type channel(p_channel);
	Channel& chan = s_channels[channel];
	
	vol_type leftVol  = chan.pan > 0 ? 256 - chan.pan : 256;
	vol_type rightVol = chan.pan < 0 ? 256 + chan.pan : 256;
	
	leftVol  *= chan.vol;
	rightVol *= chan.vol;
	
	if (chan.sample)
	{
		TT_Printf("Render %d samples of PCM\n", p_frames);
		// render sample
		switch (chan.format)
		{
		case SampleType_PCM8:
			{
				const s8* buffer = reinterpret_cast<const s8*>(chan.sample);
				for (size_type frame = 0; frame < p_frames; ++frame)
				{
					s64 curr = buffer[chan.position >> 32];
					s64 next = buffer[(chan.position >> 32) + 1];
					
					s8 sample = static_cast<s8>(
						((curr * (0x100000000LL - (chan.position & 0xFFFFFFFF))) + 
						(next * (chan.position & 0xFFFFFFFF))) / 0x100000000LL);
					
					*p_buffer += static_cast<s16>((sample * leftVol) >> 8);
					++p_buffer;
					*p_buffer += static_cast<s16>((sample * rightVol) >> 8);
					++p_buffer;
					
					// update position
					chan.position += chan.delta;
					if (chan.delta > 0)
					{
						if (chan.position >= chan.dataLength)
						{
							switch (chan.loop)
							{
							case LoopType_None:
								chan.active = false;
								break;
								
							case LoopType_Loop:
								chan.position -= chan.dataLength - chan.loopStart;
								break;
								
							case LoopType_PingPong:
								chan.position = chan.dataLength - (chan.position - chan.dataLength);
								chan.delta = -chan.delta;
								break;
								
							default:
								break;
							}
						}
					}
					else if (chan.position <= chan.loopStart)
					{
						// loop type is always ping pong
						chan.position = chan.loopStart + (chan.loopStart - chan.position);
						chan.delta = -chan.delta;
					}
				}
			}
			break;
			
		case SampleType_PCM16:
			{
				const s16* buffer = reinterpret_cast<const s16*>(chan.sample);
				for (size_type frame = 0; frame < p_frames; ++frame)
				{
					s64 curr = buffer[chan.position >> 32];
					s64 next = buffer[(chan.position >> 32) + 1];
					
					s16 sample = static_cast<s16>(
						((curr * (0x100000000LL - (chan.position & 0xFFFFFFFF))) + 
						(next * (chan.position & 0xFFFFFFFF))) / 0x100000000LL);
					
					
					*p_buffer += static_cast<s16>((sample * leftVol) >> 16);
					++p_buffer;
					*p_buffer += static_cast<s16>((sample * rightVol) >> 16);
					++p_buffer;
					
					
					// update position
					chan.position += chan.delta;
					if (chan.delta > 0)
					{
						if (chan.position >= chan.dataLength)
						{
							switch (chan.loop)
							{
							case LoopType_None:
								chan.active = false;
								break;
								
							case LoopType_Loop:
								chan.position -= chan.dataLength - chan.loopStart;
								break;
								
							case LoopType_PingPong:
								chan.position = chan.dataLength - (chan.position - chan.dataLength);
								chan.delta = -chan.delta;
								break;
								
							default:
								break;
							}
						}
					}
					else if (chan.position <= chan.loopStart)
					{
						// loop type is always ping pong
						chan.position = chan.loopStart + (chan.loopStart - chan.position);
						chan.delta = -chan.delta;
					}
				}
			}
			break;
			
		default:
			break;
		}
	}
	else if (chan.duty > 0)
	{
		TT_Printf("Render %d samples of PSG\n", p_frames);
		TT_Printf("Left vol: %d\n", leftVol);
		TT_Printf("Right vol: %d\n", rightVol);
		TT_Printf("Chan vol: %d\n", chan.vol);
		// render PSG
		for (size_type frame = 0; frame < p_frames; ++frame)
		{
			// ignore frequency for now
			vol_type vol = chan.position < chan.duty ? chan.vol : -chan.vol;
			--vol;
			++chan.position;
			if (chan.position >= 256)
			{
				chan.position = 0;
			}
			vol >>= 1;
			
			*p_buffer += static_cast<s16>(vol * leftVol);
			++p_buffer;
			*p_buffer += static_cast<s16>(vol * rightVol);
			++p_buffer;
		}
	}
	else
	{
		// render noise
		
		// Gaussian white noise
		{
			static const s32 q = 15;
			static const s32 c1 = (1 << q) - 1;
			static const s32 c2 = (c1 / 3) + 1;
			math::Random& rand = math::Random::getEffects();
			for (size_type frame = 0; frame < p_frames; ++frame)
			{
				s32 noise = static_cast<s32>(2 * (rand.getNext(c2) +
				                                  rand.getNext(c2) +
				                                  rand.getNext(c2))) - c1;
				
				*p_buffer += static_cast<s16>((noise * leftVol) >> 8);
				++p_buffer;
				*p_buffer += static_cast<s16>((noise * rightVol) >> 8);
				++p_buffer;
			}
		}
	}
}


// namespace end
}
}
}
