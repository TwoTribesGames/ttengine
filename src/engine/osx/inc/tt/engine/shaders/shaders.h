#ifndef INC_TT_ENGINE_SHADERS_SHADERS_H
#define INC_TT_ENGINE_SHADERS_SHADERS_H

namespace tt {
namespace engine {
namespace shaders {

//--------------------------------------------------------------------------------------------------
// Vertex Shaders

// Map quad to entire screen (hardcoded for quad size = 4)
const char* fullscreenQuad();

// Gaussian Blur vertex shaders (compute sampling points)
const char* vertexFastGaussH();
const char* vertexFastGaussV();


//--------------------------------------------------------------------------------------------------
// Fragment Shaders

// Return unmodified texture color
const char* textured();

// Return unmodified texture color from filtered sampler
const char* texturedFiltered();

// Return brg instead of rgb (for debugging)
const char* texturedShift();

// Simple box blur filter
const char* boxBlur();
const char* boxBlurLite();

// Bright pass filter
const char* brightPass();

const char* colorPass();
const char* radialBlur();
const char* colorSeparation();
const char* colorSeparationLite();
const char* mixEffect();

// Gaussian Blur
const char* fastGaussH();
const char* fastGaussV();

}
}
}

#endif
