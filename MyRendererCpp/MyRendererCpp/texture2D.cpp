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
	textures.push_back(right);
	textures.push_back(left);
	textures.push_back(top);
	textures.push_back(bottom);
	textures.push_back(front);
	textures.push_back(back);
	std::cout<<front.use_count()<<std::endl;
	//确认一下 dump出来看看结果
	/*right->write_texture_to_file("right.tga");
	left->write_texture_to_file("left.tga");
	top->write_texture_to_file("top.tga");
	bottom->write_texture_to_file("bottom.tga");
	front->write_texture_to_file("front.tga");
	back->write_texture_to_file("back.tga");*/
}

/*
 * for cubemap sampling, see subsection 3.7.5 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
static int select_cubemap_face(vec3_t direction, vec2_t* texcoord) {
	float abs_x = (float)fabs(direction.x);
	float abs_y = (float)fabs(direction.y);
	float abs_z = (float)fabs(direction.z);
	float ma, sc, tc;
	int face_index;

	if (abs_x > abs_y && abs_x > abs_z) {   /* major axis -> x */
		ma = abs_x;
		if (direction.x > 0) {                  /* positive x */
			face_index = 0;
			sc = -direction.z;
			tc = direction.y;
		}
		else {                                /* negative x */
			face_index = 1;
			sc = +direction.z;
			tc = direction.y;
		}
	}
	else if (abs_y > abs_z) {             /* major axis -> y */
		ma = abs_y;
		if (direction.y > 0) {                  /* positive y */
			face_index = 2;
			sc = +direction.x;
			tc = -direction.z;
		}
		else {                                /* negative y */
			face_index = 3;
			sc = +direction.x;
			tc = +direction.z;
		}
	}
	else {                                /* major axis -> z */
		ma = abs_z;
		if (direction.z > 0) {                  /* positive z */
			face_index = 4;
			sc = +direction.x;
			tc = +direction.y;
		}
		else {                                /* negative z */
			face_index = 5;
			sc = -direction.x;
			tc = direction.y;
		}
	}

	texcoord->x = (sc / ma + 1) / 2;
	texcoord->y = (tc / ma + 1) / 2;
	return face_index;
}

vec4_t texture_repeat_sample(std::weak_ptr<Texture> face, vec2_t texcoord) 
{
	std::shared_ptr<Texture> texture = face.lock();
	float u = texcoord.x - (float)floor(texcoord.x);
	float v = texcoord.y - (float)floor(texcoord.y);
	int c = (int)((texture->width - 1) * u);
	int r = (int)((texture->height - 1) * v);
	int index = r * texture->width + c;
	return texture->buffer[index];
}

vec4_t cubemap_repeat_sample(std::shared_ptr<CubeMap> cubemap, vec3_t direction)
{
	vec2_t texcoord;
	int face_index = select_cubemap_face(direction, &texcoord);
	//texcoord.y = 1 - texcoord.y;
	return texture_repeat_sample(cubemap->textures[face_index], texcoord);
}

vec4_t cubemap_sample(std::shared_ptr<CubeMap> cubemap, vec3_t direction) 
{
	return cubemap_repeat_sample(cubemap, direction);
}