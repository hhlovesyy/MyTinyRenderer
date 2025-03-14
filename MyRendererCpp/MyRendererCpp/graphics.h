#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "maths.h"
#include <cfloat>
#include <vector>

using vertex_shader_t = vec4(*)(void* attribs, void* varyings, void* uniforms);//函数指针。返回值为vec4_t，参数为void* attribs, void* varyings, void* uniforms
using fragment_shader_t = vec4(*)(void* varyings, void* uniforms, int* discard, int backface);

#define MAX_VARYINGS 10
class Program {
public:
	Program(vertex_shader_t vertex_shader, fragment_shader_t fragment_shader,
		int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms, int alpha_blend = 0);
	~Program();

	void* get_attribs(int nth_vertex);
	void* get_uniforms();
	int alpha_blend;
	void* shader_attribs_[3];
	void* shader_varyings_;
	void* shader_uniforms_;
	void* in_varyings_[10];
	void* out_varyings_[10];
	bool double_sided;
	int sizeof_attribs_;
	int sizeof_varyings_;
	int sizeof_uniforms_;

	vec4 in_coords[MAX_VARYINGS];
	vec4 out_coords[MAX_VARYINGS];

	vertex_shader_t vertex_shader_;
	fragment_shader_t fragment_shader_;
	
};


class framebuffer_t
{
public:
    int width, height;
	std::vector<char> color_buffer;
	std::vector<float> depth_buffer;
    framebuffer_t(int width, int height, int color_buffer_size) : width(width), height(height)
	{
		depth_buffer.resize(width * height);
		color_buffer.resize(color_buffer_size, 0);
		for (int i = 0; i < width * height; i++) 
		{
			//float max
			depth_buffer[i] = FLT_MAX;
		}
	}
	~framebuffer_t() {}
};

struct bbox_t
{
    int min_x, min_y, max_x, max_y;
};

/* framebuffer management */
framebuffer_t* framebuffer_create(int width, int height);
void framebuffer_clear_color(framebuffer_t* framebuffer, vec4 color);
void framebuffer_clear_depth(framebuffer_t* framebuffer, float depth);
vec3 calculate_weights(vec2 abc[3], vec2& p);
bbox_t find_bounding_box(vec2 abc[3], int width, int height);
vec3 viewport_transform(int width, int height, vec3 ndc_coord);
float interpolate_depth(float screen_depths[3], vec3 weights);
vec3 interpolate_varyings_weights(vec3& weights, float recip_w[3]);

void* program_get_attribs(Program* program, int nth_vertex);
void interpolate_varyings(
	void* src_varyings[3], void* dst_varyings,
	int sizeof_varyings, vec3 weights, float recip_w[3]);
void draw_fragment(framebuffer_t* framebuffer, int index, vec4& color, Program* program);
void draw_fragment_new(framebuffer_t* framebuffer, Program* program,
	int backface, int index, float depth);
#endif