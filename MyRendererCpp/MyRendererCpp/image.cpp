#include "image.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

image_t* image_create(int width, int height, int channels)
{
	assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);
	image_t* image = new image_t(width, height, channels);
	return image;
}