#ifndef TEXTURE2D_H
#define TEXTURE2D_H

#include "maths.h"
#include "tgaimage.h"
#include "graphics.h"
#include <iostream>
#include "image.h"

//Enum
enum SAMPLE_TYPE
{
	SAMPLE_NEAREST = 0,
	SAMPLE_BILINEAR = 1,
	SAMPLE_TRILINEAR = 2,
	SAMPLE_ANISOTROPIC = 3,
	DEFAULT = 4
};

typedef enum 
{
	USAGE_LDR_COLOR,
	USAGE_LDR_DATA,
	USAGE_HDR_COLOR,
	USAGE_HDR_DATA
} usage_t;

class Texture
{
public:
	int width;
	int height;
	std::vector<vec4_t> buffer; //存储纹理数据
	Texture() = default;

	Texture(std::string filename, usage_t usage) : width(0), height(0)
	{
		std::string extension = filename.substr(filename.find_last_of(".") + 1);
		if(extension == "tga")
		{
			std::unique_ptr<TGAImage> image = std::make_unique<TGAImage>();
			if (!image->read_tga_file(filename))
			{
				std::cerr << "Failed to read texture file: " << filename << std::endl;
				return;
			}
			set_texture(std::move(image));
		}
		else if (extension == "hdr")
		{
			//load hdr image, 读取hdr格式的图片，具体的细节不需要关心
			std::shared_ptr<image_t> image = load_hdr_image(filename);
			set_texture(image, image->format,usage);
		}
		
	}

	void hdr_image_to_texture(std::shared_ptr<image_t> image)
	{
		width = image->width;
		height = image->height;
		int num_pixels = image->width * image->height;
		buffer.resize(num_pixels);
		for (int i = 0; i < num_pixels; i++)
		{
			float* pixel = &image->hdr_buffer[i * image->channels];
			vec4_t texel = { 0, 0, 0, 1 };
			if (image->channels == 1) {             /* GL_LUMINANCE */
				texel.x = texel.y = texel.z = pixel[0];
			}
			else if (image->channels == 2) {      /* GL_LUMINANCE_ALPHA */
				texel.x = texel.y = texel.z = pixel[0];
				texel.w = pixel[1];
			}
			else if (image->channels == 3) {      /* GL_RGB */
				texel.x = pixel[0];
				texel.y = pixel[1];
				texel.z = pixel[2];
			}
			else {                                /* GL_RGBA */
				texel.x = pixel[0];
				texel.y = pixel[1];
				texel.z = pixel[2];
				texel.w = pixel[3];
			}
			buffer[i] = texel;
		}
	}

	void linear_to_srgb() 
	{
		int num_pixels = width * height;
		for (int i = 0; i < num_pixels; i++)
		{
			vec4_t* pixel = &buffer[i];
			pixel->x = float_linear2srgb(float_aces(pixel->x));
			pixel->y = float_linear2srgb(float_aces(pixel->y));
			pixel->z = float_linear2srgb(float_aces(pixel->z));
		}
	}

	void set_texture(std::shared_ptr<image_t> image, format_t format, usage_t usage)
	{
		//todo:目前的cubemap支持的image是HDR的，但是usage是LDR的
		if (image->format == FORMAT_LDR) 
		{
			/*ldr_image_to_texture(image, texture);
			if (usage == USAGE_HDR_COLOR) 
			{
				srgb_to_linear(texture);
			}*/
		}
		else 
		{
			hdr_image_to_texture(image);
			if (usage == USAGE_LDR_COLOR)
			{
				linear_to_srgb();  //todo:演示一下不开这个的后果是什么
			}
		}
	}

	void set_texture(std::unique_ptr<TGAImage> image)
	{
		width = image->width();
		height = image->height();
		buffer.resize(width * height);
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				TGAColor color = image->get(i, j);
				buffer[i + j * width] = vec4_new(color.bgra[2] / 255.0f, color.bgra[1] / 255.0f, color.bgra[0] / 255.0f, color.bgra[3] / 255.0f);
			}
		}
	}

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

class CubeMap
{
public:
	//shared_ptr
	std::shared_ptr<Texture> front;
	std::shared_ptr<Texture> back;
	std::shared_ptr<Texture> left;
	std::shared_ptr<Texture> right;
	std::shared_ptr<Texture> top;
	std::shared_ptr<Texture> bottom;
	CubeMap() = default;
	CubeMap(std::string name, int blur_level)
	{
		load_skybox(name, blur_level);
	}

	int blur_level;
	void load_skybox(std::string name, int blur_level);
};

vec4_t sample2D(Texture& texture, vec2_t uv);
vec4_t sample2D(Texture* texture, vec2_t uv, SAMPLE_TYPE type);

#endif