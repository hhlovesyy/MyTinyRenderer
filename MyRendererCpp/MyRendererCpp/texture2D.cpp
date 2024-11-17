#include "texture2D.h"

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