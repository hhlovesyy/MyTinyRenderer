#include "test_model_input.h"
#include "win32.h"
#include "graphics.h"
#include <iostream>
#include "mesh.h"
#include "camera.h"
#include "test_helper.h"
using namespace std;

typedef void tickfunc_t(framebuffer_t* framebuffer, Camera* camera);

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
void model_input_transform(framebuffer_t* framebuffer,Camera* camera)
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

	mat4_t view_matrix = camera_get_view_matrix(*camera);
	mat4_t proj_matrix = camera_get_proj_matrix(*camera);

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
	delete[] zbuffer;
}
void test_enter_mainloop_model_input(tickfunc_t* tickfunc)
{
	window_t* window;
	framebuffer_t* framebuffer;

	float prev_time;
	float print_time;
	int num_frames;

	window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);

	Camera* camera = new Camera(CAMERA_POSITION, CAMERA_TARGET, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
	callbacks_t callbacks;
	record_t record;
	memset(&callbacks, 0, sizeof(callbacks_t));
	memset(&record, 0, sizeof(record_t));
	callbacks.button_callback = button_callback;
	callbacks.scroll_callback = scroll_callback;
	window->userdata = &record;
	window->callbacks = callbacks;

	num_frames = 0;
	prev_time = platform_get_time();
	print_time = prev_time;

	while (!window->should_close)
	{
		float curr_time = platform_get_time();
		float delta_time = curr_time - prev_time; //与帧率有关的值
		update_camera(window, camera, &record);
		update_click(curr_time, &record);

		tickfunc(framebuffer, camera);
		window_draw_buffer(window, framebuffer);
		num_frames += 1;
		if (curr_time - print_time >= 1) {
			int sum_millis = (int)((curr_time - print_time) * 1000);
			int avg_millis = sum_millis / num_frames;
			printf("fps: %3d, avg: %3d ms\n", num_frames, avg_millis);
			num_frames = 0;
			print_time = curr_time;
		}
		prev_time = curr_time;

		record.orbit_delta = vec2_new(0, 0);
		record.pan_delta = vec2_new(0, 0);
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;
		input_poll_events();
	}
	
}

void test_model_input()
{
	preLoadModel();
	//进入主循环并渲染模型,传入顶点数据
	test_enter_mainloop_model_input(model_input_transform);
}
