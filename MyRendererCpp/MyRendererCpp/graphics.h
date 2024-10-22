#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "maths.h"
#include <cfloat>

using vertex_shader_t = vec4_t(*)(void* attribs, void* varyings, void* uniforms);//函数指针。返回值为vec4_t，参数为void* attribs, void* varyings, void* uniforms
using fragment_shader_t = vec4_t(*)(void* varyings, void* uniforms, int* discard, int backface);

class Program {
public:
	Program(vertex_shader_t vertex_shader, fragment_shader_t fragment_shader,
		int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms);
	~Program();

	void* get_attribs(int nth_vertex);
	void* get_uniforms();
	int alpha_blend;

private:
	vertex_shader_t vertex_shader_;
	fragment_shader_t fragment_shader_;
	int sizeof_attribs_;
	int sizeof_varyings_;
	int sizeof_uniforms_;

	void* shader_attribs_[3];
	void* shader_varyings_;
	void* shader_uniforms_;
	void* in_varyings_[10];
	void* out_varyings_[10];
};


class framebuffer_t
{
public:
    int width, height;
    unsigned char* color_buffer;
	float* depth_buffer;
    framebuffer_t(int width, int height, int color_buffer_size) : width(width), height(height), color_buffer(new unsigned char[color_buffer_size]()) 
	{
		depth_buffer = new float[width * height];
		for (int i = 0; i < width * height; i++) 
		{
			//float max
			depth_buffer[i] = FLT_MAX;
		}
	}
	~framebuffer_t() { delete[] color_buffer; delete[] depth_buffer; }
};

struct bbox_t
{
    int min_x, min_y, max_x, max_y;
};

/* framebuffer management */
framebuffer_t* framebuffer_create(int width, int height);
void framebuffer_clear_color(framebuffer_t* framebuffer, vec4_t color);
void framebuffer_clear_depth(framebuffer_t* framebuffer, float depth);
vec3_t calculate_weights(vec2_t abc[3], vec2_t& p);
bbox_t find_bounding_box(vec2_t abc[3], int width, int height);
void draw_fragment(framebuffer_t* framebuffer, int index, vec4_t& color, Program* program=nullptr);
vec3_t viewport_transform(int width, int height, vec3_t ndc_coord);
float interpolate_depth(float screen_depths[3], vec3_t weights);
vec3_t interpolate_varyings_weights(vec3_t& weights, float recip_w[3]);
#endif