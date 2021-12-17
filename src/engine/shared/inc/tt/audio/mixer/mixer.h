#if !defined(INC_TT_AUDIO_MIXER_MIXER_H)
#define INC_TT_AUDIO_MIXER_MIXER_H

#include <tt/audio/mixer/types.h>


namespace tt {
namespace audio {
namespace mixer {

/*! \brief Initializes software mixer.
    \param p_channels Amount of channels.
    \param p_sampleRate Sample rate of mixer output.
    \return True on success, false on invalid params or when already initialized.*/
bool initialize(channel_type p_channels, freq_type p_sampleRate);

/*! \brief Ends the software mixer.
    \return True on success, false when already ended or never initialized.*/
bool end();

/*! \brief Sets mixer's master volume.
    \param p_vol Volume (0 - 256).
    \return Previous master volume.*/
vol_type setMasterVolume(vol_type p_vol);

/*! \brief Sets mixer's master panning.
    \param p_pan Panning (-128 - 128).
    \return Previous master panning.*/
vol_type setMasterPanning(vol_type p_vol);

/*! \brief Sets mixer callback function.
    \param p_callback Function to call during periodic intervals.
    \param p_param Parameter for callback function.
    \return Previous callback function.*/
callback setCallback(callback p_callback, void* p_param);

/*! \brief Sets mixer callback interval.
    \param p_frames Callback interval in frames.*/
void setCallbackInterval(size_type p_frames);

/*! \brief Sets up a channel for PCM playback.
    \param p_channel Channel to set up.
    \param p_format Sample format.
    \param p_data Sample data.
    \param p_loop Looping type.
    \param p_loopStart Starting position of loop in samples.
    \param p_dataLength Sample length in samples.
    \param p_freq Playback rate in samples per second.
    \param p_volume Volume ratio (0 - 256).
    \param p_pan Panning (-128 - 128).
    \return True on success, false on fail.*/
bool setupPCM(channel_type p_channel,
              SampleType   p_format,
              const void*  p_data,
              LoopType     p_loop,
              size_type    p_loopStart,
              size_type    p_dataLength,
              freq_type    p_freq,
              vol_type     p_volume,
              pan_type     p_pan);


/*! \brief Sets up a channel for PSG rectangular waves.
    \param p_channel Channel to set up.
    \param p_duty Duty ratio (1 - 255).
    \param p_pitch Pitch in Hz.
    \param p_volume Volume ratio (0 - 256).
    \param p_pan Panning (-128 - 128).
    \return True on success, false on fail.*/
bool setupPSG(channel_type p_channel,
              duty_type    p_duty,
              freq_type    p_freq,
              vol_type     p_volume,
              pan_type     p_pan);


/*! \brief Sets up a channel for playing noise.
    \param p_channel Channel to set up.
    \param p_noise Noise type.
    \param p_pitch Pitch in Hz.
    \param p_volume Volume ratio (0 - 256).
    \param p_pan Panning (-128 - 128).
    \return True on success, false on fail.*/
bool setupNoise(channel_type p_channel,
                NoiseType    p_noise,
                freq_type    p_freq,
                vol_type     p_volume,
                pan_type     p_pan);


/*! \brief Changes the frequency of a channel.
    \param p_channel Channel to set up.
    \param p_freq Frequency in Hz.
    \return True on success, false on fail.*/
bool setFrequency(channel_type p_channel,
                  freq_type    p_freq);


/*! \brief Changes the volume of a channel.
    \param p_channel Channel to set up.
    \param p_vol Volume (0 - 256).
    \return True on success, false on fail.*/
bool setVolume(channel_type p_channel,
               vol_type     p_vol);


/*! \brief Changes the panning of a channel.
    \param p_channel Channel to set up.
    \param p_pan Panning (-128 - 128).
    \return True on success, false on fail.*/
bool setPanning(channel_type p_channel,
                pan_type     p_pan);


/*! \brief (De)Activates a channel.
    \param p_channel Channel to (de)activate.
    \param p_active Whether the channel should be active.
    \return The previous state.*/
bool setActive(channel_type p_channel,
                bool         p_active);


/*! \brief Performs mixing to an interleaved buffer.
    \param p_frames Amount of frames to mix.
    \param p_buffer Buffer to mix to.
    \param p_clear Whether the buffer should be cleared before mixing.
    \return The amount of mixed frames.*/
size_type mixInterleaved(size_type p_frames,
                         s16*      p_buffer,
                         bool      p_clear = false);


/*! \brief Performs mixing to separate left and right buffers.
    \param p_frames Amount of frames to mix.
    \param p_leftBuffer Buffer to mix left channel to.
    \param p_rightBuffer Buffer to mix right channel to.
    \param p_clear Whether the buffers should be cleared before mixing.
    \return The amount of mixed frames.*/
size_type mix(size_type p_frames,
              s16*      p_leftBuffer,
              s16*      p_rightBuffer,
              bool      p_clear = false);

// namespace end
}
}
}

#endif // !defined(INC_TT_AUDIO_MIXER_MIXER_H)
