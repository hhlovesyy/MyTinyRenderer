#ifndef IMAGE_H
#define IMAGE_H

class image_t
{
public:
	int width, height, channels;
	unsigned char* color_buffer;
	image_t() : width(0), height(0), channels(0), color_buffer(nullptr) {}
	image_t(int width, int height, int channels) : width(width), height(height), channels(channels), color_buffer(new unsigned char[width * height * channels]()) {}
	~image_t() { delete[] color_buffer; }
};

/* image creating/releasing */
image_t* image_create(int width, int height, int channels);

#endif