#include "test_space_transform.h"
#include "win32.h"
#include "graphics.h"
#include "camera.h"
#include <iostream>
#include "test_helper.h"
//static const char* const WINDOW_TITLE = "Lesson3";
//static const int WINDOW_WIDTH = 800;
//static const int WINDOW_HEIGHT = 600;

typedef void tickfunc_t(framebuffer_t* framebuffer);

void space_transform(framebuffer_t* framebuffer)
{
	vec4 default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;
	//生成Zbuffer
	float* zbuffer = new float[width * height];
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = FLT_MAX;
	}
	float vertices[] = 
	{
		-0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f, 
		-0.5f,  0.5f, -0.5f,  
		-0.5f, -0.5f, -0.5f,  

		-0.5f, -0.5f,  0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f,  
		-0.5f, -0.5f,  0.5f,  

		-0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f, -0.5f,  
		-0.5f, -0.5f, -0.5f,  
		-0.5f, -0.5f, -0.5f,  
		-0.5f, -0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f, 

		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  

		-0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f, -0.5f,  0.5f,  
		-0.5f, -0.5f,  0.5f,  
		-0.5f, -0.5f, -0.5f,  

		-0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f, -0.5f,  
	};

	vec3 camera_pos = vec3{ -1, 0, 1.5f };
	vec3 target_pos = vec3{0, 0, 0};
	//Camera *camera = new Camera(camera_pos, target_pos, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
	Camera camera(camera_pos, target_pos, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
	mat4_t view_matrix = camera_get_view_matrix(camera);
	mat4_t proj_matrix = camera_get_proj_matrix(camera);

	//每三个顶点赋值给abc_3d,一共绘制12个三角形
	for (int index = 0; index < 12; index++)
	{
		vec3 abc_3d[3] = { vec3{vertices[index * 9], vertices[index * 9 + 1], vertices[index * 9 + 2]},
			vec3{vertices[index * 9 + 3], vertices[index * 9 + 4], vertices[index * 9 + 5]},
			vec3{vertices[index * 9 + 6], vertices[index * 9 + 7], vertices[index * 9 + 8]} };
		
		vec4 clip_abc[3];
		vec3 ndc_coords[3];

		vec2 screen_coords[3];
		float screen_depths[3];

		float recip_w[3];

		for (int i = 0; i < 3; i++)
		{
			clip_abc[i] = mat4_mul_vec4(proj_matrix, mat4_mul_vec4(view_matrix, vec4_from_vec3(abc_3d[i], 1)));
			recip_w[i] = 1 / clip_abc[i][3];
			vec3 clip_coord = vec3_from_vec4(clip_abc[i]);
			ndc_coords[i] = clip_coord / clip_abc[i][3];  //这一步是做透视除法
			vec3 window_coord = viewport_transform(width, height, ndc_coords[i]); //这一步是做视口变换
			screen_coords[i] = vec2{ window_coord[0], window_coord[1] };
			screen_depths[i] = window_coord[2];
		}

		//vec2_t abc[3] = { vec2_new(100 , 300), vec2_new(200 , 600), vec2_new(300, 100) };

		bbox_t bbox = find_bounding_box(screen_coords, width, height);
		//random 0-1 color
		//vec4_t color1{ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1 };
		vec4 color1{ 1,0,0,1 };
		vec4 color2{ 0,1,0,1 };
		vec4 color3{ 0,0,1,1 };
		for (int i = bbox.min_x; i <= bbox.max_x; i++)
		{
			for (int j = bbox.min_y; j <= bbox.max_y; j++)
			{
				vec2 p{ (float)(i + 0.5), (float)(j + 0.5) };
				vec3 result = calculate_weights(screen_coords, p);
				
				if (!(result[0] > -EPSILON && result[1] > -EPSILON && result[2] > -EPSILON)) continue; //考虑一下浮点数精度可能带来的问题
				float depth = interpolate_depth(screen_depths, result);
				//Zbuffer test
				int screen_index = j * width + i;
				if (depth < zbuffer[screen_index])
				{
					zbuffer[screen_index] = depth;
				}
				else
				{
					continue;
				}
				vec3 new_weights = interpolate_varyings_weights(result, recip_w);
				vec4 color = color1 * new_weights[0] + color2 * new_weights[1] + color3 * new_weights[2];
				vec4 zBufferValue{ zbuffer[screen_index],zbuffer[screen_index], zbuffer[screen_index], zbuffer[screen_index] };
				//std::cout << zbuffer[screen_index] << std::endl;
				draw_fragment(framebuffer, j* width + i, color, nullptr);
		
			}
		}
	}
	
}

void test_enter_mainloop3(tickfunc_t* tickfunc)
{
	window_t* window;
	framebuffer_t* framebuffer;
	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
	while (1)
	{
		tickfunc(framebuffer);
		window_draw_buffer(window, framebuffer);
	}

}

void test_space_transform()
{
	test_enter_mainloop3(space_transform);
}