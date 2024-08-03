#include "test_rasterization.h"
#include "win32.h"
#include "graphics.h"
#include <iostream>
using namespace std;

typedef void tickfunc_t(framebuffer_t* framebuffer);

static const char* const WINDOW_TITLE = "Lesson1";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

int x_delta = 0;

void test_enter_mainloop(tickfunc_t* tickfunc)
{
	window_t* window;
	framebuffer_t* framebuffer;
	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
	while (1)
	{
		tickfunc(framebuffer);
		window_draw_buffer(window, framebuffer);
		++x_delta;
	}
	
}

void rasterization_triangle(framebuffer_t* framebuffer)
{
	vec4_t default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;
	vec2_t abc[3] = { vec2_new(100 + x_delta, 300), vec2_new(200 + x_delta, 600), vec2_new(300 + x_delta, 100) };
	bbox_t bbox = find_bounding_box(abc, width, height);
	vec4_t color1{ 1,0,0,1 };
	vec4_t color2{ 0,1,0,1 };
	vec4_t color3{ 0,0,1,1 };

	for (int i = bbox.min_x; i <= bbox.max_x; i++)
	{
		for (int j = bbox.min_y; j <= bbox.max_y; j++)
		{
			vec2_t p{ (float)(i + 0.5), (float)(j + 0.5) };
			vec3_t result = calculate_weights(abc, p);
			
			if (result.x > 0 && result.y > 0 && result.z > 0)
			{
				vec4_t color = vec4_new(
					color1.x * result.x + color2.x * result.y + color3.x * result.z,
					color1.y * result.x + color2.y * result.y + color3.y * result.z,
					color1.z * result.x + color2.z * result.y + color3.z * result.z,
					1
				);

				draw_fragment(framebuffer, j * width + i, color);
			}
		}
	}
}

void test_rasterization()
{
	test_enter_mainloop(rasterization_triangle);
}