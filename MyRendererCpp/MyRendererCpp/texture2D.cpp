#include "texture2D.h"

vec4_t sample2D(TGAImage& image, vec2_t uv)
{
	int width = image.width();
	int height = image.height();
	int x = uv.x * width;
	int y = (1.0 - uv.y) * height;//纹理坐标的Y轴是反的
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	TGAColor color = image.get(x, y);
	/*std::cout << color.bgra[0] << " " << color.bgra[1] << " " << color.bgra[2] << " " << color.bgra[3] << std::endl;*/
	return vec4_new(color.bgra[2]/255.0f, color.bgra[1] / 255.0f, color.bgra[0] / 255.0f, color.bgra[3] / 255.0f);

}