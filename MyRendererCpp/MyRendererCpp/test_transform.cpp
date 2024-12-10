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

void matrix_translate(vec2* abc)
{
	mat4_t translate = mat4_translate(l2_x_delta_trans, l2_y_delta_trans, 0);
	vec4 a{ abc[0][0], abc[0][1], 0, 1};
	vec4 b = vec4{abc[1][0], abc[1][1], 0, 1};
	vec4 c = vec4{ abc[2][0], abc[2][1], 0, 1 };
	a = mat4_mul_vec4(translate, a);
	b = mat4_mul_vec4(translate, b);
	c = mat4_mul_vec4(translate, c);
	abc[0] = vec2{ a[0], a[1]};
	abc[1] = vec2{ b[0], b[1]};
	abc[2] = vec2{ c[0], c[1]};
}

void matrix_rotateZ(vec2* abc)
{
	float radian = TO_RADIANS(l2_x_delta_trans);
	mat4_t rotate = mat4_rotate_z(-radian);
	vec4 a = vec4{ abc[0][0], abc[0][1], 0, 1};
	vec4 b = vec4{ abc[1][0], abc[1][1], 0, 1 };
	vec4 c = vec4{abc[2][0], abc[2][1], 0, 1};
	a = mat4_mul_vec4(rotate, a);
	b = mat4_mul_vec4(rotate, b);
	c = mat4_mul_vec4(rotate, c);
	abc[0] = vec2{a[0], a[1]};
	abc[1] = vec2{ b[0], b[1]};
	abc[2] = vec2{ c[0], c[1] };
}



void rasterization_triangle2(framebuffer_t* framebuffer)
{
	vec4 default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;
	vec2 abc[3] = { vec2{100 , 300}, vec2{200 , 600}, vec2{300, 100} };
	//matrix_translate(abc);
	matrix_rotateZ(abc);
	//matrix_scale(abc);
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

void test_transform2()
{
	test_enter_mainloop2(rasterization_triangle2);
}