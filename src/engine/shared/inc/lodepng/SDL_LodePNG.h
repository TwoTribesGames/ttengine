#ifndef _SDL_LODEPNG_
#define _SDL_LODEPNG_

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

SDL_Surface *SDL_LodePNG(const char* filename);

#ifndef NO_LOADPNG
#define SDL_LoadPNG SDL_LodePNG
#endif

#ifdef __cplusplus
}
#endif

#endif
