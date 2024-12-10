#include "test_model_input.h"
#include "shader_BlinnPhong.h"
#include "win32.h"
#include "graphics.h"
#include <iostream>
#include "mesh.h"
#include "camera.h"
#include "test_helper.h"
#include "scene_helper.h"
#include "rasterization.h"
#include "build_scene.h"
using namespace std;

typedef void tickfunc_t(framebuffer_t* framebuffer, Camera* camera);

int num_faces = 0;
std::vector<Vertex> vertices;
//获取材质
std::vector<TGAImage> img;
vector<Model*> models;
SceneBuilder builder;
Scene scene;
void preLoadModel()
{
	builder = SceneBuilder();
	//加载模型(新)
	//Scene* scene = scene_from_file("combinePamu.scene");
	mat4_t root= mat4_identity();
	scene = scene_from_file("combinePamu.scene", root);

	//获取模型的顶点数据
	 models = scene.models;
	
	/*vertices = mesh->getVertices();
	num_faces = mesh->getNumFaces();*/

	
}

//传入顶点数据
void model_input_transform(framebuffer_t* framebuffer,Camera* camera)
{
	vec4 default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	framebuffer_clear_depth(framebuffer, FLT_MAX); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;

	builder.test_draw_scene(scene, framebuffer,camera);

	/*mat4_t view_matrix = camera_get_view_matrix(*camera);
	mat4_t proj_matrix = camera_get_proj_matrix(*camera);

	for(int index=0;index<models.size();index++)
	{
		Mesh* mesh = models[index]->mesh;
		

		Program* program=models[index]->program;
		uniforms_blinnphong* uniforms = (uniforms_blinnphong*)program->get_uniforms();
		uniforms->light_dir = vec3_new(0.5f, 0.8f, 0.9f);
		uniforms->camera_pos = camera->position;
		
		uniforms->camera_vp_matrix = mat4_mul_mat4(camera_get_proj_matrix(*camera), camera_get_view_matrix(*camera));
		
	
		rasterization_tri(mesh, program, framebuffer);	
	}
	*/
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
		FrameInfo::ambient_intensity = scene.ambient_intensity;
		//调用传入的函数，main中将这个函数设置为此类下的model_input_transform
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

		record.pan_delta.reset();
		record.orbit_delta.reset();
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;
		input_poll_events();
	}
	
}

void test_model_input()
{
	//创建模型
	preLoadModel();
	//进入主循环并渲染模型,传入顶点数据
	test_enter_mainloop_model_input(model_input_transform);
}
