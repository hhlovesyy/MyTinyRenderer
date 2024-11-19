#include "image.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include "macro.h"
std::shared_ptr<image_t> load_hdr_image(std::string filename)
{
    int width, height;
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file)
    {
        fprintf(stderr, "Failed to open file: %s\n", filename.c_str());
        return nullptr;
    }
    read_hdr_header(file, &width, &height);
    std::shared_ptr<image_t> image = image_create(width, height, 3, FORMAT_HDR);
    for (int i = 0; i < height; i++)
    {
        int row = height - 1 - i;
        read_hdr_scanline(file, image, row);
    }
    fclose(file);
    //std::cout << "load_hdr_image function" << image.use_count() << std::endl;
    return image;
}

image_t* image_create(int width, int height, int channels)
{
	assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);
	image_t* image = new image_t(width, height, channels);
	return image;
}

std::shared_ptr<image_t> image_create(int width, int height, int channels, format_t format)
{
    int num_elems = width * height * channels;
    assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);
    assert(format == FORMAT_LDR || format == FORMAT_HDR);
    std::shared_ptr<image_t> image = std::make_shared<image_t>(width, height, channels, format);
    return image;
}

/* hdr codec */

static void read_line(FILE* file, char line[256]) 
{
    if (fgets(line, 256, file) == NULL) {
        assert(0);
    }
}

static int starts_with(const char* string, const char* prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

static void read_hdr_header(FILE* file, int* width, int* height) 
{
    char line[256];
    int header_found = 0;
    int format_found = 0;
    int items;

    read_line(file, line);
    assert(starts_with(line, "#?"));

    while (1) {
        read_line(file, line);
        if (strlen(line) == 1 && line[0] == '\n') {
            header_found = 1;
            break;
        }
        else if (starts_with(line, "FORMAT=")) {
            assert(starts_with(line, "FORMAT=32-bit_rle_rgbe"));
            format_found = 1;
        }
        else if (starts_with(line, "GAMMA=")) {
            /* ignore gamma */
        }
        else if (starts_with(line, "EXPOSURE=")) {
            /* ignore exposure */
        }
        else if (starts_with(line, "#")) {
            /* ignore comments */
        }
        else {
            assert(0);
        }
    }
    assert(header_found != 0);
    assert(format_found != 0);
    UNUSED_VAR(header_found);
    UNUSED_VAR(format_found);

    read_line(file, line);
    items = sscanf(line, "-Y %d +X %d", height, width);
    assert(items == 2 && *width > 0 && *height > 0);
    UNUSED_VAR(items);
}

static float* get_hdr_pixel(std::shared_ptr<image_t> image, int row, int col)
{
    int index = (row * image->width + col) * image->channels;
    return &image->hdr_buffer[index];
}

static void read_bytes(FILE* file, void* buffer, int size)
{
    int count = (int)fread(buffer, 1, size, file);
    assert(count == size);
    UNUSED_VAR(count);
}

static void rgbe_to_floats(unsigned char rgbe[4], float floats[3])
{
    float rm = rgbe[0];                             /* red mantissa */
    float gm = rgbe[1];                             /* green mantissa */
    float bm = rgbe[2];                             /* blue mantissa */
    int eb = rgbe[3];                               /* exponent biased */
    if (eb == 0) {
        floats[0] = floats[1] = floats[2] = 0;
    }
    else {
        int ev = eb - 128;                          /* exponent value */
        float factor = (float)((1.0 / 256) * pow(2, ev));
        floats[0] = rm * factor;                    /* red value */
        floats[1] = gm * factor;                    /* green value */
        floats[2] = bm * factor;                    /* blue value */
    }
}

static void read_hdr_flat_scanline(FILE* file, std::shared_ptr<image_t> image, int row) 
{
    int i;
    for (i = 0; i < image->width; i++) {
        float* pixel = get_hdr_pixel(image, row, i);
        unsigned char rgbe[4];
        read_bytes(file, rgbe, 4);
        rgbe_to_floats(rgbe, pixel);
    }
}

static unsigned char read_byte(FILE* file)
{
    int byte = fgetc(file);
    assert(byte != EOF);
    return (unsigned char)byte;
}

static void read_hdr_rle_scanline(FILE* file, std::shared_ptr<image_t> image, int row) 
{
    unsigned char* channels[4];
    int i, j;

    for (i = 0; i < 4; i++) {
        channels[i] = (unsigned char*)malloc(image->width);
    }
    for (i = 0; i < 4; i++) {
        int size = 0;
        while (size < image->width) {
            unsigned char byte = read_byte(file);
            if (byte > 128) {
                int count = byte - 128;
                unsigned char value = read_byte(file);
                assert(count > 0 && size + count <= image->width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = value;
                }
            }
            else {
                int count = byte;
                assert(count > 0 && size + count <= image->width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = read_byte(file);
                }
            }
        }
        assert(size == image->width);
    }

    for (i = 0; i < image->width; i++) {
        float* pixel = get_hdr_pixel(image, row, i);
        unsigned char rgbe[4];
        for (j = 0; j < 4; j++) {
            rgbe[j] = channels[j][i];
        }
        rgbe_to_floats(rgbe, pixel);
    }
    for (i = 0; i < 4; i++) {
        free(channels[i]);
    }
}

static void read_hdr_scanline(FILE* file, std::shared_ptr<image_t> image, int row)
{
    if (image->width < 8 || image->width > 0x7fff) {
        read_hdr_flat_scanline(file, image, row);
    }
    else {
        unsigned char bytes[4];
        read_bytes(file, bytes, 4);
        if (bytes[0] != 2 || bytes[1] != 2 || bytes[2] & 0x80) {
            fseek(file, -4, SEEK_CUR);
            read_hdr_flat_scanline(file, image, row);
        }
        else {
            assert(bytes[2] * 256 + bytes[3] == image->width);
            read_hdr_rle_scanline(file, image, row);
        }
    }
}
