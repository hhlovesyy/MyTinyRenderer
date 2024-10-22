#include "graphics.h"
//#include "maths.h"
#include <assert.h>
#include <math.h>
#include <vcruntime_string.h>

/*
 * for depth interpolation, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */

/* program management */

/* program management */

#define MAX_VARYINGS 10

Program::Program(vertex_shader_t vertex_shader, fragment_shader_t fragment_shader,
	int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms, int alpha_blend)
	: vertex_shader_(vertex_shader), fragment_shader_(fragment_shader),
	sizeof_attribs_(sizeof_attribs), sizeof_varyings_(sizeof_varyings),
	sizeof_uniforms_(sizeof_uniforms), alpha_blend(alpha_blend)
{
	assert(sizeof_attribs > 0 && sizeof_varyings > 0 && sizeof_uniforms > 0);
	assert(sizeof_varyings % sizeof(float) == 0);

	for (int i = 0; i < 3; i++) {
		shader_attribs_[i] = new char[sizeof_attribs];
		memset(shader_attribs_[i], 0, sizeof_attribs);
	}
	shader_varyings_ = new char[sizeof_varyings];
	memset(shader_varyings_, 0, sizeof_varyings);
	shader_uniforms_ = new char[sizeof_uniforms];
	memset(shader_uniforms_, 0, sizeof_uniforms);
	for (int i = 0; i < MAX_VARYINGS; i++) {
		in_varyings_[i] = new char[sizeof_varyings];
		memset(in_varyings_[i], 0, sizeof_varyings);
		out_varyings_[i] = new char[sizeof_varyings];
		memset(out_varyings_[i], 0, sizeof_varyings);
	}
}

Program::~Program() {
	for (int i = 0; i < 3; i++) {
		delete[] shader_attribs_[i];
	}
	delete[] shader_varyings_;
	delete[] shader_uniforms_;
	for (int i = 0; i < MAX_VARYINGS; i++) {
		delete[] in_varyings_[i];
		delete[] out_varyings_[i];
	}
}

void* Program::get_attribs(int nth_vertex) {
	assert(nth_vertex >= 0 && nth_vertex < 3);
	return shader_attribs_[nth_vertex];
}

void* Program::get_uniforms() 
{
	return shader_uniforms_;
}



float interpolate_depth(float screen_depths[3], vec3_t weights) 
{
	float depth0 = screen_depths[0] * weights.x;
	float depth1 = screen_depths[1] * weights.y;
	float depth2 = screen_depths[2] * weights.z;
	return depth0 + depth1 + depth2;
}

bbox_t find_bounding_box(vec2_t abc[3], int width, int height)
{
	vec2_t min = vec2_min(vec2_min(abc[0], abc[1]), abc[2]);
	vec2_t max = vec2_max(vec2_max(abc[0], abc[1]), abc[2]);
	bbox_t bbox;
	bbox.min_x = max_integer((int)floor(min.x), 0);
	bbox.min_y = max_integer((int)floor(min.y), 0);
	bbox.max_x = min_integer((int)ceil(max.x), width - 1);
	bbox.max_y = min_integer((int)ceil(max.y), height - 1);
	return bbox;
}

//目前只是简单的将颜色写入到framebuffer中，并做透明度混合
void draw_fragment(framebuffer_t* framebuffer, int index, vec4_t& color,Program* program)
{
	//start 增加透明度混合
	int transparent = program->alpha_blend;
	if (transparent == 1)
	{
		//float alpha = color.w;
		float alpha = 0.5;
		if (alpha < 1)
		{
			unsigned char dst_r = framebuffer->color_buffer[index * 4 + 0];
			unsigned char dst_g = framebuffer->color_buffer[index * 4 + 1];
			unsigned char dst_b = framebuffer->color_buffer[index * 4 + 2];
			color.x = color.x * alpha + float_from_uchar(dst_r) * (1 - alpha);
			color.y = color.y * alpha + float_from_uchar(dst_g) * (1 - alpha);
			color.z = color.z * alpha + float_from_uchar(dst_b) * (1 - alpha);
		}
	}
	
	//end
	framebuffer->color_buffer[index * 4 + 0] = float_to_uchar(color.x);
	framebuffer->color_buffer[index * 4 + 1] = float_to_uchar(color.y);
	framebuffer->color_buffer[index * 4 + 2] = float_to_uchar(color.z);
	framebuffer->color_buffer[index * 4 + 3] = float_to_uchar(color.w);
}

/*
 * for barycentric coordinates, see
 * http://blackpawn.com/texts/pointinpoly/
 *
 * solve
 *     P = A + s * AB + t * AC  -->  AP = s * AB + t * AC
 * then
 *     s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
 *     t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
 *
 * notice
 *     P = A + s * AB + t * AC
 *       = A + s * (B - A) + t * (C - A)
 *       = (1 - s - t) * A + s * B + t * C
 * then
 *     weight_A = 1 - s - t
 *     weight_B = s
 *     weight_C = t
 */
vec3_t calculate_weights(vec2_t abc[3], vec2_t& p)
{
	vec2_t a = abc[0];
	vec2_t b = abc[1];
	vec2_t c = abc[2];
	vec2_t ab = vec2_sub(b, a);
	vec2_t ac = vec2_sub(c, a);
	vec2_t pa = vec2_sub(a, p);
	float factor = 1 / (ab.x * ac.y - ab.y * ac.x);
	float s = (ac.x * pa.y - ac.y * pa.x) * factor;
	float t = (ab.y * pa.x - ab.x * pa.y) * factor;
	vec3_t weights = vec3_new(1 - s - t, s, t);
	return weights;
}

void framebuffer_clear_color(framebuffer_t* framebuffer, vec4_t color)
{
	int num_pixels = framebuffer->width * framebuffer->height;
	for (int i = 0; i < num_pixels; i++)
	{
		framebuffer->color_buffer[i * 4 + 0] = float_to_uchar(color.x);
		framebuffer->color_buffer[i * 4 + 1] = float_to_uchar(color.y);
		framebuffer->color_buffer[i * 4 + 2] = float_to_uchar(color.z);
		framebuffer->color_buffer[i * 4 + 3] = float_to_uchar(color.w);
	}
}

void framebuffer_clear_depth(framebuffer_t* framebuffer, float depth)
{
	int num_pixels = framebuffer->width * framebuffer->height;
	int i;
	for (i = 0; i < num_pixels; i++) 
	{
		framebuffer->depth_buffer[i] = depth;
	}
}

framebuffer_t* framebuffer_create(int width, int height)
{
	int color_buffer_size = width * height * 4;
	vec4_t default_color = { 0, 0, 0, 1 };
	framebuffer_t* framebuffer;

	assert(width > 0 && height > 0);
	framebuffer = new framebuffer_t(width, height, color_buffer_size);

	//给这个framebuffer的每个像素点都填充默认颜色
	framebuffer_clear_color(framebuffer, default_color);

	return framebuffer;
}

/*
 * for viewport transformation, see subsection 2.12.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
vec3_t viewport_transform(int width, int height, vec3_t ndc_coord)
{
	float x = (ndc_coord.x + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
	float y = (ndc_coord.y + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
	float z = (ndc_coord.z + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return vec3_new(x, y, z);
}

vec3_t interpolate_varyings_weights(vec3_t& weights, float recip_w[3])
{
	float weight0 = recip_w[0] * weights.x;
	float weight1 = recip_w[1] * weights.y;
	float weight2 = recip_w[2] * weights.z;
	float normalizer = 1 / (weight0 + weight1 + weight2);
	return vec3_t{ weight0 * normalizer, weight1 * normalizer, weight2 * normalizer };
}