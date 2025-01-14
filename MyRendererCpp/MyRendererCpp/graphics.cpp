#include "graphics.h"
//#include "maths.h"
#include <assert.h>
#include <math.h>
#include <vcruntime_string.h>
#include "rasterization.h"

/*
 * for depth interpolation, see subsection 3.5.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */

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



float interpolate_depth(float screen_depths[3], vec3 weights) 
{
	float depth0 = screen_depths[0] * weights[0];
	float depth1 = screen_depths[1] * weights[1];
	float depth2 = screen_depths[2] * weights[2];
	return depth0 + depth1 + depth2;
}

bbox_t find_bounding_box(vec2 abc[3], int width, int height)
{
	vec2 min = vec_min(vec_min(abc[0], abc[1]), abc[2]);
	vec2 max = vec_max(vec_max(abc[0], abc[1]), abc[2]);
	bbox_t bbox;
	bbox.min_x = max_integer((int)floor(min[0]), 0);
	bbox.min_y = max_integer((int)floor(min[1]), 0);
	bbox.max_x = min_integer((int)ceil(max[0]), width - 1);
	bbox.max_y = min_integer((int)ceil(max[1]), height - 1);
	return bbox;
}

/*
[Obselete] ��Ҫ���������������ǰ���ο�ʱ��Ҫ�Ĳ��Ժ��������ڲ�������߼���
*/
//Ŀǰֻ�Ǽ򵥵Ľ���ɫд�뵽framebuffer�У�����͸���Ȼ��
void draw_fragment(framebuffer_t* framebuffer, int index, vec4& color,Program* program)
{
	//start ����͸���Ȼ��
	int transparent = 0;
	if (program != nullptr)
	{
		transparent = program->alpha_blend;
	}
	 
	if (transparent == 1)
	{
		//float alpha = color.w;
		float alpha = 0.6;
		if (alpha < 1)
		{
			unsigned char dst_r = framebuffer->color_buffer[index * 4 + 0];
			unsigned char dst_g = framebuffer->color_buffer[index * 4 + 1];
			unsigned char dst_b = framebuffer->color_buffer[index * 4 + 2];
			color[0] = color[0] * alpha + float_from_uchar(dst_r) * (1 - alpha);
			color[1] = color[1] * alpha + float_from_uchar(dst_g) * (1 - alpha);
			color[2] = color[2] * alpha + float_from_uchar(dst_b) * (1 - alpha);
		}
	}
	
	color = vec4_saturate(color);
	//end
	framebuffer->color_buffer[index * 4 + 0] = float_to_uchar(color[0]);
	framebuffer->color_buffer[index * 4 + 1] = float_to_uchar(color[1]);
	framebuffer->color_buffer[index * 4 + 2] = float_to_uchar(color[2]);
	framebuffer->color_buffer[index * 4 + 3] = float_to_uchar(color[3]);
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
vec3 calculate_weights(vec2 abc[3], vec2& p)
{
	vec2 a = abc[0];
	vec2 b = abc[1];
	vec2 c = abc[2];
	vec2 ab = b - a;
	vec2 ac = c - a;
	vec2 pa = a - p;
	float factor = 1 / (ab[0] * ac[1] - ab[1] * ac[0]);
	float s = (ac[0] * pa[1] - ac[1] * pa[0]) * factor;
	float t = (ab[1] * pa[0] - ab[0] * pa[1]) * factor;
	vec3 weights{ 1 - s - t, s, t };
	return weights;
}

void framebuffer_clear_color(framebuffer_t* framebuffer, vec4 color)
{
	int num_pixels = framebuffer->width * framebuffer->height;
	for (int i = 0; i < num_pixels; i++)
	{
		framebuffer->color_buffer[i * 4 + 0] = float_to_uchar(color[0]);
		framebuffer->color_buffer[i * 4 + 1] = float_to_uchar(color[1]);
		framebuffer->color_buffer[i * 4 + 2] = float_to_uchar(color[2]);
		framebuffer->color_buffer[i * 4 + 3] = float_to_uchar(color[3]);
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
	vec4 default_color{ 0, 0, 0, 1 };
	framebuffer_t* framebuffer;

	assert(width > 0 && height > 0);
	framebuffer = new framebuffer_t(width, height, color_buffer_size);

	//�����framebuffer��ÿ�����ص㶼���Ĭ����ɫ
	framebuffer_clear_color(framebuffer, default_color);

	return framebuffer;
}

/*
 * for viewport transformation, see subsection 2.12.1 of
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 */
vec3 viewport_transform(int width, int height, vec3 ndc_coord)
{
	float x = (ndc_coord[0] + 1) * 0.5f * (float)width;   /* [-1, 1] -> [0, w] */
	float y = (ndc_coord[1] + 1) * 0.5f * (float)height;  /* [-1, 1] -> [0, h] */
	float z = (ndc_coord[2] + 1) * 0.5f;                  /* [-1, 1] -> [0, 1] */
	return vec3{ x, y, z };
}

vec3 interpolate_varyings_weights(vec3& weights, float recip_w[3])
{
	float weight0 = recip_w[0] * weights[0];
	float weight1 = recip_w[1] * weights[1];
	float weight2 = recip_w[2] * weights[2];
	float normalizer = 1 / (weight0 + weight1 + weight2);
	return vec3{ weight0 * normalizer, weight1 * normalizer, weight2 * normalizer };
}

void* program_get_attribs(Program* program, int nth_vertex) 
{
	assert(nth_vertex >= 0 && nth_vertex < 3);
	return program->shader_attribs_[nth_vertex];
}

/*
 * for perspective correct interpolation, see
 * https://www.comp.nus.edu.sg/~lowkl/publications/lowk_persp_interp_techrep.pdf
 * https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf
 *
 * equation 15 in reference 1 (page 2) is a simplified 2d version of
 * equation 3.5 in reference 2 (page 58) which uses barycentric coordinates
 */
void interpolate_varyings(
	void* src_varyings[3], void* dst_varyings,
	int sizeof_varyings, vec3 weights, float recip_w[3])
{
	int num_floats = sizeof_varyings / sizeof(float);
	float* src0 = (float*)src_varyings[0];
	float* src1 = (float*)src_varyings[1];
	float* src2 = (float*)src_varyings[2];
	float* dst = (float*)dst_varyings;
	//weights对应α'，β'，γ'，是屏幕空间的重心坐标，recip_w是w值，也就是裁剪空间透视除法前的za，zb，zc
	float weight0 = recip_w[0] * weights[0]; 
	float weight1 = recip_w[1] * weights[1];
	float weight2 = recip_w[2] * weights[2];
	float normalizer = 1 / (weight0 + weight1 + weight2); //裁剪空间的Z
	int i;
	for (i = 0; i < num_floats; i++) {
		float sum = src0[i] * weight0 + src1[i] * weight1 + src2[i] * weight2;
		dst[i] = sum * normalizer;
	}
}

void draw_fragment_new(framebuffer_t* framebuffer, Program* program,
	int backface, int index, float depth)
{
	vec4 color;
	int discard;
	/* execute fragment shader */
	discard = 0;
	color = program->fragment_shader_(program->shader_varyings_,
		program->get_uniforms(),
		&discard,
		backface);
	if (discard) 
	{
		return;
	}
	color = vec4_saturate(color);
	int transparent = program->alpha_blend;
	if (transparent == 1)
	{
		//float alpha = color.w;
		float alpha = 0.6;
		if (alpha < 1)
		{
			unsigned char dst_r = framebuffer->color_buffer[index * 4 + 0];
			unsigned char dst_g = framebuffer->color_buffer[index * 4 + 1];
			unsigned char dst_b = framebuffer->color_buffer[index * 4 + 2];
			color[0] = color[0] * alpha + float_from_uchar(dst_r) * (1 - alpha);
			color[1] = color[1] * alpha + float_from_uchar(dst_g) * (1 - alpha);
			color[2] = color[2] * alpha + float_from_uchar(dst_b) * (1 - alpha);
		}
	}
	/* write color and depth */
	framebuffer->color_buffer[index * 4 + 0] = float_to_uchar(color[0]);
	framebuffer->color_buffer[index * 4 + 1] = float_to_uchar(color[1]);
	framebuffer->color_buffer[index * 4 + 2] = float_to_uchar(color[2]);
	framebuffer->depth_buffer[index] = depth;
}