#include "SDL_LodePNG.h"

#include "lodepng.h"
#include <stdlib.h>

SDL_Surface *SDL_LodePNG(const char* filename)
{
    SDL_Surface *temp = NULL;
    unsigned char *data = NULL;
    unsigned int w, h;

    unsigned ret = lodepng_decode32_file(&data, &w, &h, filename);
    if (ret == 0)
    {
        temp = SDL_CreateRGBSurfaceFrom((void*)data, w, h, 32, 4 * w,
                                        0x000000ff,
                                        0x0000ff00,
                                        0x00ff0000,
                                        0xff000000);
        if (temp) {
            SDL_Surface *t2 = SDL_ConvertSurface(temp, temp->format, 0);
            SDL_FreeSurface(temp);
            temp = t2;
        }
        free(data);
    }

    return temp;
}
