#define _CRT_SECURE_NO_WARNINGS
#include "texture2D.h"
#include <cassert>

vec4_t sample2D(Texture& texture, vec2_t uv)
{
	int width = texture.width;
	int height = texture.height;
	if (width <= 0 || height <= 0)
	{
		//说明贴图没有，或者出了问题
		return vec4_new(0, 0, 0, 0);
	}
	int x = uv.x * width;
	int y = (1.0 - uv.y) * height;//纹理坐标的Y轴是反的
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	vec4_t color = texture.buffer[x + y * width];
	/*std::cout << color.bgra[0] << " " << color.bgra[1] << " " << color.bgra[2] << " " << color.bgra[3] << std::endl;*/
	return color;

}

vec4_t sample2D(Texture* texture, vec2_t uv, SAMPLE_TYPE type=DEFAULT)
{
	int width = texture->width;
	int height = texture->height;
	if (width <= 0 || height <= 0)
	{
		//说明贴图没有，或者出了问题
		return vec4_new(0, 0, 0, 0);
	}
	float x = uv.x * width;
	float y = (1.0 - uv.y) * height;//纹理坐标的Y轴是反的
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	vec4_t color = texture->getColor(x, y, type);
	//std::cout << color.x << color.y << color.z << std::endl;
	return color;

}

void CubeMap::load_skybox(std::string name, int blur_level)
{
	const char* filename = name.c_str();
	const char* faces[6] = { "px", "nx", "py", "ny", "pz", "nz" };
	char paths[6][256];
	for (int i = 0; i < 6; i++)  //读取六个面
	{
		const char* format;
		if (blur_level == -1) {
			format = "%s/i_%s.hdr";
		}
		else if (blur_level == 1) {
			format = "%s/m1_%s.hdr";
		}
		else {
			assert(blur_level == 0);
			format = "%s/m0_%s.hdr";
		}
		sprintf(paths[i], format, filename, faces[i]);
	}
	right = std::make_shared<Texture>(paths[0], USAGE_LDR_COLOR); //positive X
	left = std::make_shared<Texture>(paths[1], USAGE_LDR_COLOR); //negative X
	top = std::make_shared<Texture>(paths[2], USAGE_LDR_COLOR); //positive Y
	bottom = std::make_shared<Texture>(paths[3], USAGE_LDR_COLOR); //negative Y
	front = std::make_shared<Texture>(paths[4], USAGE_LDR_COLOR); //positive Z
	back = std::make_shared<Texture>(paths[5], USAGE_LDR_COLOR); //negative Z
	//确认一下 dump出来看看结果
	right->write_texture_to_file("right.tga");
	left->write_texture_to_file("left.tga");
	top->write_texture_to_file("top.tga");
	bottom->write_texture_to_file("bottom.tga");
	front->write_texture_to_file("front.tga");
	back->write_texture_to_file("back.tga");
}