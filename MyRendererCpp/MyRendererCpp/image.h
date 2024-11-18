#define _CRT_SECURE_NO_WARNINGS
#ifndef IMAGE_H
#define IMAGE_H
#include <memory>
#include <string>

typedef enum {
	FORMAT_LDR,
	FORMAT_HDR
} format_t;

class image_t
{
public:
	format_t format;
	int width, height, channels;
	unsigned char* color_buffer;
	float* hdr_buffer;
	image_t() : width(0), height(0), channels(0), color_buffer(nullptr) {}
	image_t(int width, int height, int channels) : width(width), height(height), channels(channels), color_buffer(new unsigned char[width * height * channels]()) {}
	image_t(int width, int height, int channels, format_t format): width(width), height(height), channels(channels)
	{
		if (format == FORMAT_LDR)
		{
			color_buffer = new unsigned char[width * height * channels]();
		}
		else
		{
			hdr_buffer = new float[width * height * channels]();
		}
	}
	~image_t() { 
		/*if(color_buffer) delete[] color_buffer;
		if(hdr_buffer) delete[] hdr_buffer;*/
	}
};

/* image creating/releasing */
image_t* image_create(int width, int height, int channels);
std::shared_ptr<image_t> image_create(int width, int height, int channels, format_t format);
static void read_hdr_header(FILE* file, int* width, int* height);
std::shared_ptr<image_t> load_hdr_image(std::string filename);
static void read_line(FILE* file, char line[256]);
static int starts_with(const char* string, const char* prefix);
static void read_hdr_scanline(FILE* file, std::shared_ptr<image_t> image, int row);
static void read_hdr_flat_scanline(FILE* file, std::shared_ptr<image_t> image, int row);
static float* get_hdr_pixel(std::shared_ptr<image_t> image, int row, int col);
static void read_bytes(FILE* file, void* buffer, int size);
static void rgbe_to_floats(unsigned char rgbe[4], float floats[3]);
static unsigned char read_byte(FILE* file);
static void read_hdr_rle_scanline(FILE* file, std::shared_ptr<image_t> image, int row);
#endif