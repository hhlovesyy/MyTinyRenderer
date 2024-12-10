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
	std::vector<vec4> buffer; //存储纹理数据
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
			//load hdr image, 读取hdr格式的图片，具体的细节暂时不需要关心
			std::shared_ptr<image_t> image = load_hdr_image(filename);

			set_texture(image, image->format, usage);
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
			vec4 texel = { 0, 0, 0, 1 };
			if (image->channels == 1) {             /* GL_LUMINANCE */
				texel[0] = texel[1] = texel[2] = pixel[0];
			}
			else if (image->channels == 2) {      /* GL_LUMINANCE_ALPHA */
				texel[0] = texel[1] = texel[2] = pixel[0];
				texel[3] = pixel[1];
			}
			else if (image->channels == 3) {      /* GL_RGB */
				texel[0] = pixel[0];
				texel[1] = pixel[1];
				texel[2] = pixel[2];
			}
			else {                                /* GL_RGBA */
				texel[0] = pixel[0];
				texel[1] = pixel[1];
				texel[2] = pixel[2];
				texel[3] = pixel[3];
			}
			buffer[i] = texel;
		}
	}

	void linear_to_srgb() 
	{
		int num_pixels = width * height;
		for (int i = 0; i < num_pixels; i++)
		{
			vec4& pixel = buffer[i];
			pixel[0] = float_linear2srgb(float_aces(pixel[0]));
			pixel[1] = float_linear2srgb(float_aces(pixel[1]));
			pixel[2] = float_linear2srgb(float_aces(pixel[2]));
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
			hdr_image_to_texture(image); //如果是hdr空间的图像，那么其实是线性空间的
			if (usage == USAGE_LDR_COLOR) //不在HDR空间做着色，就要把线性空间的颜色进行gamma校正
			{
				linear_to_srgb(); 
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
				buffer[i + j * width] = vec4{ color.bgra[2] / 255.0f, color.bgra[1] / 255.0f, color.bgra[0] / 255.0f, color.bgra[3] / 255.0f };
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
				buffer[i + j * width] = vec4{ color.bgra[2] / 255.0f, color.bgra[1] / 255.0f, color.bgra[0] / 255.0f, color.bgra[3] / 255.0f };
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
			this->buffer[i] = vec4{ depth, depth, depth, 1 };
		}
	}

	vec4 getColor(float x, float y, SAMPLE_TYPE type=DEFAULT)
	{
		vec4 result;
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
			vec4 color00 = buffer[x0 + y0 * width];
			vec4 color01 = buffer[x0 + y1 * width];
			vec4 color10 = buffer[x1 + y0 * width];
			vec4 color11 = buffer[x1 + y1 * width];
			float u = x - x0;
			float v = y - y0;
			result = color00 * (1 - u) * (1 - v) + color01 * (1 - u) * v + color10 * u * (1 - v) + color11 * u * v;
			break;
		}
		return result;
	}

	void write_texture_to_file(const char* filename)
	{
		//todo：如果buffer中此时存储的是gamma校正之后的颜色，则写入gamma校正之后的颜色即可；
		//否则如果存储的是线性空间的颜色，则进行gamma校正之后再写入，比如那张shadowmap
		TGAImage image(width, height, TGAImage::RGB);
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				vec4 color = buffer[i + j * width];
				TGAColor tga_color{ color[2] * 255, color[1] * 255, color[0] * 255, color[3] * 255 };
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
	std::vector<std::shared_ptr<Texture>> textures;
	CubeMap() = default;
	CubeMap(std::string name, int blur_level)
	{
		load_skybox(name, blur_level);
	}

	int blur_level;
	void load_skybox(std::string name, int blur_level);
};

vec4 sample2D(Texture& texture, vec2 uv);
vec4 sample2D(Texture* texture, vec2 uv, SAMPLE_TYPE type);
vec4 cubemap_sample(std::shared_ptr<CubeMap> cubemap, vec3 direction);
vec4 cubemap_repeat_sample(std::shared_ptr<CubeMap> cubemap, vec3 direction);
static int select_cubemap_face(vec3 direction, vec2* texcoord);
#endif