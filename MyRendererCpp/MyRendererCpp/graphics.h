#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "maths.h"
class framebuffer_t
{
public:
    int width, height;
    unsigned char* color_buffer;
    framebuffer_t() : width(0), height(0), color_buffer(nullptr) {}
    framebuffer_t(int width, int height, int color_buffer_size) : width(width), height(height), color_buffer(new unsigned char[color_buffer_size]()) {}
};

struct bbox_t
{
    int min_x, min_y, max_x, max_y;
};

/* framebuffer management */
framebuffer_t* framebuffer_create(int width, int height);
void framebuffer_clear_color(framebuffer_t* framebuffer, vec4_t color);
vec3_t calculate_weights(vec2_t abc[3], vec2_t& p);
bbox_t find_bounding_box(vec2_t abc[3], int width, int height);
void draw_fragment(framebuffer_t* framebuffer, int index, vec4_t& color);
vec3_t viewport_transform(int width, int height, vec3_t ndc_coord);
float interpolate_depth(float screen_depths[3], vec3_t weights);
vec3_t interpolate_varyings_weights(vec3_t& weights, float recip_w[3]);
#endif