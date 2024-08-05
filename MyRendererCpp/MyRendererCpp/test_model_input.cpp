#include "test_model_input.h"
#include "win32.h"
#include "graphics.h"
#include <iostream>
#include "mesh.h"
#include "camera.h"
using namespace std;

typedef void tickfunc_t(framebuffer_t* framebuffer);

static const char* const WINDOW_TITLE = "Lesson1";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

int num_faces = 0;
std::vector<Mesh::Vertex> vertices;

void preLoadModel()
{
	//相对路径
	const char* model_path = "test.obj";
	Mesh* mesh = Mesh::load(model_path);
	if (mesh == nullptr) 
	{
		std::cerr << "Failed to load model: " << model_path << std::endl;
		return;
	}

	//获取模型的顶点数据
	vertices = mesh->getVertices();
	num_faces = mesh->getNumFaces();
}

//传入顶点数据
void model_input_transform(framebuffer_t* framebuffer)
{
	vec4_t default_color = { 0, 0, 0, 1 };
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
	vec3_t camera_pos = vec3_new(-1, 0, 1.5f);
	vec3_t target_pos = vec3_new(0, 0, 0);

	Camera camera(camera_pos, target_pos, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
	mat4_t view_matrix = camera_get_view_matrix(camera);
	mat4_t proj_matrix = camera_get_proj_matrix(camera);

	
	for (int index = 0; index < num_faces; index++)
	{
		vec3_t abc_3d[3] = {
			vec3_new(vertices[index * 3].position.x, vertices[index * 3].position.y, vertices[index * 3].position.z),
			vec3_new(vertices[index * 3 + 1].position.x, vertices[index * 3 + 1].position.y, vertices[index * 3 + 1].position.z),
			vec3_new(vertices[index * 3 + 2].position.x, vertices[index * 3 + 2].position.y, vertices[index * 3 + 2].position.z)
		};

		vec4_t clip_abc[3];
		vec3_t ndc_coords[3];

		vec2_t screen_coords[3];
		float screen_depths[3];

		for (int i = 0; i < 3; i++)
		{
			clip_abc[i] = mat4_mul_vec4(proj_matrix, mat4_mul_vec4(view_matrix, vec4_from_vec3(abc_3d[i], 1)));
			vec3_t clip_coord = vec3_from_vec4(clip_abc[i]);
			ndc_coords[i] = vec3_div(clip_coord, clip_abc[i].w);  //这一步是做透视除法
			vec3_t window_coord = viewport_transform(width, height, ndc_coords[i]); //这一步是做视口变换
			screen_coords[i] = vec2_new(window_coord.x, window_coord.y);
			screen_depths[i] = window_coord.z;
		}

		bbox_t bbox = find_bounding_box(screen_coords, width, height);

		//vec4_t color{ index,0.8,0.8,1 };
		vec4_t color{ 1,1,1,1 };
		for (int i = bbox.min_x; i <= bbox.max_x; i++)
		{
			for (int j = bbox.min_y; j <= bbox.max_y; j++)
			{
				vec2_t p{ (float)(i + 0.5), (float)(j + 0.5) };
				vec3_t result = calculate_weights(screen_coords, p);

				if (!(result.x > 0 && result.y > 0 && result.z > 0)) continue;

				//Zbuffer test
				float z = result.x * screen_depths[0] + result.y * screen_depths[1] + result.z * screen_depths[2];
				int screen_index = j * width + i;
				if (z < zbuffer[screen_index])
				{
					zbuffer[screen_index] = z;
				}
				else
				{
					continue;
				}
				draw_fragment(framebuffer, j * width + i, color);

			}
		}
	}

}
void test_enter_mainloop_model_input(tickfunc_t* tickfunc)
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

void test_model_input()
{
	preLoadModel();
	//进入主循环并渲染模型,传入顶点数据
	test_enter_mainloop_model_input(model_input_transform);
}
