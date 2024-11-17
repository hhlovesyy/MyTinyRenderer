#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include "maths.h"
#include "tgaimage.h"
#include "graphics.h"
#include <iostream>

//Enum
enum SAMPLE_TYPE
{
	SAMPLE_NEAREST = 0,
	SAMPLE_BILINEAR = 1,
	SAMPLE_TRILINEAR = 2,
	SAMPLE_ANISOTROPIC = 3,
	DEFAULT = 4
};

class Texture
{
public:
	int width;
	int height;
	std::vector<vec4_t> buffer; //´æ´¢ÎÆÀíÊý¾Ý

	void set_texture(TGAImage& image)
	{
		width = image.width();
		height = image.height();
		buffer.resize(width * height);
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				TGAColor color = image.get(i, j);
				buffer[i + j * width] = vec4_new(color.bgra[2] / 255.0f, color.bgra[1] / 255.0f, color.bgra[0] / 255.0f, color.bgra[3] / 255.0f);
			}
		}
	}

	void set_texture_from_depth_buffer(framebuffer_t* buffer)
	{
		width = buffer->width;
		height = buffer->height;
		this->buffer.resize(width * height);
		int num_pixels = width * height;
		for (int i = 0; i < num_pixels; i++) {
			float depth = buffer->depth_buffer[i];
			this->buffer[i] = vec4_new(depth, depth, depth, 1);
		}
	}

	vec4_t getColor(float x, float y, SAMPLE_TYPE type=DEFAULT)
	{
		vec4_t result;
		switch (type)
		{
		case DEFAULT:
			result = buffer[(int)x + (int)y * width];
			break;
		case SAMPLE_BILINEAR:
			int x0 = x;
			int x1 = x + 1;
			int y0 = y;
			int y1 = y + 1;
			if (x0 < 0) x0 = 0;
			if (x1 >= width) x1 = width - 1;
			if (y0 < 0) y0 = 0;
			if (y1 >= height) y1 = height - 1;
			vec4_t color00 = buffer[x0 + y0 * width];
			vec4_t color01 = buffer[x0 + y1 * width];
			vec4_t color10 = buffer[x1 + y0 * width];
			vec4_t color11 = buffer[x1 + y1 * width];
			float u = x - x0;
			float v = y - y0;
			vec4_t part1 = vec4_mul(color00, (1 - u) * (1 - v));
			vec4_t part2 = vec4_mul(color01, (1 - u) * v);
			vec4_t part3 = vec4_mul(color10, u * (1 - v));
			vec4_t part4 = vec4_mul(color11, u * v);
			result = vec4_add(vec4_add(part1, part2), vec4_add(part3, part4));
			break;
		}
		return result;
	}

	void write_texture_to_file(const char* filename)
	{
		TGAImage image(width, height, TGAImage::RGB);
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				vec4_t color = buffer[i + j * width];
				TGAColor tga_color{ color.z * 255, color.y * 255, color.x * 255, color.w * 255 };
				image.set(i, j, tga_color);
			}
		}
		//image.flip_vertically();
		image.write_tga_file(filename);
	}
};

vec4_t sample2D(Texture& texture, vec2_t uv);
vec4_t sample2D(Texture* texture, vec2_t uv, SAMPLE_TYPE type);

#endif