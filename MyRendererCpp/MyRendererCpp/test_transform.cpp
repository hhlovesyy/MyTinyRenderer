#include "test_transform.h"
#include "win32.h"
#include "graphics.h"
#include <iostream>
using namespace std;

typedef void tickfunc_t(framebuffer_t* framebuffer);

static const char* const WINDOW_TITLE = "Lesson2";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

int l2_x_delta_trans = 0;
int l2_y_delta_trans = 0;

void test_enter_mainloop2(tickfunc_t* tickfunc)
{
	window_t* window;
	framebuffer_t* framebuffer;
	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
	while (1)
	{
		tickfunc(framebuffer);
		window_draw_buffer(window, framebuffer);
		++l2_x_delta_trans;
		++l2_y_delta_trans;
	}

}

void matrix_translate(vec2_t* abc)
{
	mat4_t translate = mat4_translate(l2_x_delta_trans, l2_y_delta_trans, 0);
	vec4_t a = vec4_new(abc[0].x, abc[0].y, 0, 1);
	vec4_t b = vec4_new(abc[1].x, abc[1].y, 0, 1);
	vec4_t c = vec4_new(abc[2].x, abc[2].y, 0, 1);
	a = mat4_mul_vec4(translate, a);
	b = mat4_mul_vec4(translate, b);
	c = mat4_mul_vec4(translate, c);
	abc[0] = vec2_new(a.x, a.y);
	abc[1] = vec2_new(b.x, b.y);
	abc[2] = vec2_new(c.x, c.y);
}

void matrix_rotateZ(vec2_t* abc)
{
	float radian = TO_RADIANS(l2_x_delta_trans);
	mat4_t rotate = mat4_rotate_z(-radian);
	vec4_t a = vec4_new(abc[0].x, abc[0].y, 0, 1);
	vec4_t b = vec4_new(abc[1].x, abc[1].y, 0, 1);
	vec4_t c = vec4_new(abc[2].x, abc[2].y, 0, 1);
	a = mat4_mul_vec4(rotate, a);
	b = mat4_mul_vec4(rotate, b);
	c = mat4_mul_vec4(rotate, c);
	abc[0] = vec2_new(a.x, a.y);
	abc[1] = vec2_new(b.x, b.y);
	abc[2] = vec2_new(c.x, c.y);
}

void matrix_scale(vec2_t* abc)
{
	mat4_t scale = mat4_scale(0.2 + l2_x_delta_trans /100.0f, 0.2 + l2_x_delta_trans / 100.0f, 0.2 + l2_x_delta_trans / 100.0f);
	vec4_t a = vec4_new(abc[0].x, abc[0].y, 0, 1);
	vec4_t b = vec4_new(abc[1].x, abc[1].y, 0, 1);
	vec4_t c = vec4_new(abc[2].x, abc[2].y, 0, 1);
	a = mat4_mul_vec4(scale, a);
	b = mat4_mul_vec4(scale, b);
	c = mat4_mul_vec4(scale, c);
	abc[0] = vec2_new(a.x, a.y);
	abc[1] = vec2_new(b.x, b.y);
	abc[2] = vec2_new(c.x, c.y);
}

void rasterization_triangle2(framebuffer_t* framebuffer)
{
	vec4_t default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;
	vec2_t abc[3] = { vec2_new(100 , 300), vec2_new(200 , 600), vec2_new(300, 100) };
	//matrix_translate(abc);
	matrix_rotateZ(abc);
	//matrix_scale(abc);
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

void test_transform2()
{
	test_enter_mainloop2(rasterization_triangle2);
}