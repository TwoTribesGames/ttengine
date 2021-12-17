#if !defined(INC_TT_AUDIO_CODEC_TTADPCM_IMAADPCM_H)
#define INC_TT_AUDIO_CODEC_TTADPCM_IMAADPCM_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace audio {
namespace codec {
namespace ttadpcm {

struct ADPCMState
{
	int stepIndex;
	int predictor;
};

/*! \brief decodes a single ADPCM nibble to a signed 16 bit sample.
    \param p_state Current decoding state.
    \param p_nibble ADPCM nibble to decode.
    \return Decoded signed 16 bit sample. */
s16 decode(ADPCMState& p_state, u8 p_nibble);

/*! \brief Encodes a single signed 16 bit sample to an ADPCM nibble.
    \param p_state Current encoding state.
    \param p_sample signed 16 bit sample to encode.
    \return Encoded ADPCM nibble. */
u8 encode(ADPCMState& p_state, s16 p_sample);

// namespace end
}
}
}
}

#endif // !defined(INC_TT_AUDIO_CODEC_TTADPCM_IMAADPCM_H)
