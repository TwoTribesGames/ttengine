#if !defined(INC_TT_AUDIO_HELPERS_H)
#define INC_TT_AUDIO_HELPERS_H

#include <tt/math/math.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace audio {
namespace helpers {

/*! \brief Converts decibels to a ratio.
    \param p_dB Decibels to convert.
    \return Ratio based on given decibels.*/
inline real dBToRatio(real p_dB)
{
	return math::pow(real(10.0f), p_dB / 20.0f);
}


/*! \brief Converts a ratio to decibels.
    \param p_ratio Ratio to convert.
    \return Decibels based on given ratio.*/
inline real ratioTodB(real p_ratio)
{
	return 20.0f * math::log10(p_ratio);
}


/*! \brief Converts semitones to a ratio.
           Please note that there are 12 semitones in one octave (C, C#, D, D#, E, F, F#, G, G#, A, A#, B).
           Going up one octave doubles the frequency, going down halves it.
    \param p_semiTones Semitones to convert.
    \return Ratio based on given semitones.*/
inline real semiTonesToRatio(real p_semiTones)
{
	return math::pow(real(2.0f), p_semiTones / 12.0f);
}


/*! \brief Converts a ratio to semitones.
    \param p_ratio Ratio to convert.
    \return Semitones based on given ratio.*/
inline real ratioToSemiTones(real p_ratio)
{
	// log2(p_ratio) == log10(p_ratio) / log10(2)
	return 12.0f * (math::log10(p_ratio) / math::log10(2.0f));
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_HELPERS_H)
