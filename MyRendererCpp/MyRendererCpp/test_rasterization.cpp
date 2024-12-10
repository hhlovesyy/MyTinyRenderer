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
	vec4 default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;
	vec2 abc[3] = { vec2{100.0f + x_delta, 300}, vec2{200.0f + x_delta, 600}, vec2{300.0f + x_delta, 100} };
	bbox_t bbox = find_bounding_box(abc, width, height);
	vec4 color1{ 1,0,0,1 };
	vec4 color2{ 0,1,0,1 };
	vec4 color3{ 0,0,1,1 };

	for (int i = bbox.min_x; i <= bbox.max_x; i++)
	{
		for (int j = bbox.min_y; j <= bbox.max_y; j++)
		{
			vec2 p{ (float)(i + 0.5), (float)(j + 0.5) };
			vec3 result = calculate_weights(abc, p);
			
			if (result[0] > 0 && result[1] > 0 && result[2] > 0)
			{
				vec4 color = vec4{
					color1[0] * result[0] + color2[0] * result[1] + color3[0] * result[2],
					color1[1] * result[0] + color2[1] * result[1] + color3[1] * result[2],
					color1[2] * result[0] + color2[2] * result[1] + color3[2] * result[2],
					1
				};

				draw_fragment(framebuffer, j * width + i, color,nullptr);
			}
		}
	}
}

void test_rasterization()
{
	test_enter_mainloop(rasterization_triangle);
}