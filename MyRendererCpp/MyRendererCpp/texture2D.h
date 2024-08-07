#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include "maths.h"
#include "tgaimage.h"
vec4_t sample2D(TGAImage& image, vec2_t uv);
#endif