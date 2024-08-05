#include "graphics.h"
#include "maths.h"
#include <assert.h>
#include <math.h>

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

void draw_fragment(framebuffer_t* framebuffer, int index, vec4_t& color)
{
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