#define _CRT_SECURE_NO_WARNINGS
#include "texture2D.h"
#include <cassert>

vec4 sample2D(Texture& texture, vec2 uv)
{
	int width = texture.width;
	int height = texture.height;
	if (width <= 0 || height <= 0)
	{
		//说明贴图没有，或者出了问题
		return vec4{ 0, 0, 0, 0 };
	}
	int x = uv[0] * width;
	int y = (1.0 - uv[1]) * height;//纹理坐标的Y轴是反的
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	vec4 color = texture.buffer[x + y * width];
	/*std::cout << color.bgra[0] << " " << color.bgra[1] << " " << color.bgra[2] << " " << color.bgra[3] << std::endl;*/
	return color;

}

vec4 sample2D(Texture* texture, vec2 uv, SAMPLE_TYPE type=DEFAULT)
{
	int width = texture->width;
	int height = texture->height;
	if (width <= 0 || height <= 0)
	{
		//说明贴图没有，或者出了问题
		return vec4{ 0, 0, 0, 0 };
	}
	float x = uv[0] * width;
	float y = (1.0 - uv[1]) * height;//纹理坐标的Y轴是反的
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	vec4 color = texture->getColor(x, y, type);
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
	textures.push_back(std::make_shared<Texture>(paths[0], USAGE_LDR_COLOR)); //positive X, right
	textures.push_back(std::make_shared<Texture>(paths[1], USAGE_LDR_COLOR)); //negative X, left
	textures.push_back(std::make_shared<Texture>(paths[2], USAGE_LDR_COLOR)); //positive Y, top
	textures.push_back(std::make_shared<Texture>(paths[3], USAGE_LDR_COLOR)); //negative Y, bottom
	textures.push_back(std::make_shared<Texture>(paths[4], USAGE_LDR_COLOR)); //positive Z, front
	textures.push_back(std::make_shared<Texture>(paths[5], USAGE_LDR_COLOR)); //negative Z, back
	//std::cout<<front.use_count()<<std::endl;
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
static int select_cubemap_face(vec3 direction, vec2* texcoord) {
	float abs_x = (float)fabs(direction[0]);
	float abs_y = (float)fabs(direction[1]);
	float abs_z = (float)fabs(direction[2]);
	float ma, sc, tc;
	int face_index;

	if (abs_x > abs_y && abs_x > abs_z) {   /* major axis -> x */
		ma = abs_x;
		if (direction[0] > 0) {                  /* positive x */
			face_index = 0;
			sc = -direction[2];
			tc = direction[1];
		}
		else {                                /* negative x */
			face_index = 1;
			sc = +direction[2];
			tc = direction[1];
		}
	}
	else if (abs_y > abs_z) {             /* major axis -> y */
		ma = abs_y;
		if (direction[1] > 0) {                  /* positive y */
			face_index = 2;
			sc = +direction[0];
			tc = -direction[2];
		}
		else {                                /* negative y */
			face_index = 3;
			sc = +direction[0];
			tc = +direction[2];
		}
	}
	else {                                /* major axis -> z */
		ma = abs_z;
		if (direction[2] > 0) {                  /* positive z */
			face_index = 4;
			sc = +direction[0];
			tc = +direction[1];
		}
		else {                                /* negative z */
			face_index = 5;
			sc = -direction[0];
			tc = direction[1];
		}
	}

	texcoord->data[0] = (sc / ma + 1) / 2;
	texcoord->data[1] = (tc / ma + 1) / 2;
	return face_index;
}

vec4 texture_repeat_sample(std::shared_ptr<Texture> tex, vec2 texcoord) 
{
	std::shared_ptr<Texture> texture = tex;
	float u = texcoord[0] - (float)floor(texcoord[0]);
	float v = texcoord[1] - (float)floor(texcoord[1]);
	int c = (int)((texture->width - 1) * u);
	int r = (int)((texture->height - 1) * v);
	int index = r * texture->width + c;
	if (texture->buffer.size() == 0)
	{
		//std::cout << "buffer size is 0" << std::endl;
		return vec4{ 0, 0, 0, 0 };
	}
	return texture->buffer[index];
}

vec4 cubemap_repeat_sample(std::shared_ptr<CubeMap> cubemap, vec3 direction)
{
	vec2 texcoord;
	int face_index = select_cubemap_face(direction, &texcoord);
	//texcoord.y = 1 - texcoord.y;
	return texture_repeat_sample(cubemap->textures[face_index], texcoord);
}

vec4 cubemap_sample(std::shared_ptr<CubeMap> cubemap, vec3 direction) 
{
	return cubemap_repeat_sample(cubemap, direction);
}