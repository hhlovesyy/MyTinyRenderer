#include "test_skeleton_animation.h"
#include "test_helper.h"
#include "scene_helper.h"
#include "rasterization.h"
#include "build_scene.h"
#include <iostream>
using namespace std;

vector<Model*> models_skel_anim;
SceneBuilder builder_skel_anim;
Scene scene_skel_anim;
typedef void tickfunc_t(framebuffer_t* framebuffer, Camera* camera);
float FrameInfo::frame_time = 0.0f; // 静态成员变量的定义与初始化
void preLoadModel_skeleton_anim()
{
	builder_skel_anim = SceneBuilder();
	//加载模型(新)
	char cwd[1024];
	_getcwd(cwd, sizeof(cwd));
	printf("Current working directory: %s\n", cwd);
	mat4_t translation = mat4_translate(376.905f, -169.495f, 0);
	mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
	mat4_t scale = mat4_scale(0.001f, 0.001f, 0.001f);
	mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
	scene_skel_anim = scene_from_file("skeleton_anim/phoenix/phoenix.scene", root);

	//获取模型的顶点数据
	models_skel_anim = scene_skel_anim.models;

	/*vertices = mesh->getVertices();
	num_faces = mesh->getNumFaces();*/


}

void skeleton_anim(framebuffer_t* framebuffer, Camera* camera)
{
	vec4_t default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //请注意，在每tick绘制之前，先清空一下framebuffer
	framebuffer_clear_depth(framebuffer, FLT_MAX); //请注意，在每tick绘制之前，先清空一下framebuffer
	//绘制三角形的主函数
	int width = framebuffer->width;
	int height = framebuffer->height;

	builder_skel_anim.test_draw_scene(scene_skel_anim, framebuffer, camera);
}

void test_enter_mainloop_skeleton_anim(tickfunc_t* tickfunc)
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
		FrameInfo::set_frame_time(curr_time);

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

		record.orbit_delta = vec2_new(0, 0);
		record.pan_delta = vec2_new(0, 0);
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;
		input_poll_events();
	}

}


void test_skeleton_animation()
{
	preLoadModel_skeleton_anim();
	test_enter_mainloop_skeleton_anim(skeleton_anim);
}