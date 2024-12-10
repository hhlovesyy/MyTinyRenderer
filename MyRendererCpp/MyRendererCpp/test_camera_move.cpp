#include "test_camera_move.h"
#include "win32.h"
#include "graphics.h"
#include "camera.h"
#include "test_helper.h"
#include <iostream>

typedef void tickfunc_t(framebuffer_t* framebuffer, Camera* camera);

void move_camera_and_draw(framebuffer_t* framebuffer, Camera* camera)
{
	vec4_t default_color = { 0, 0, 0, 1 };
	framebuffer_clear_color(framebuffer, default_color); //��ע�⣬��ÿtick����֮ǰ�������һ��framebuffer
	//���������ε�������
	int width = framebuffer->width;
	int height = framebuffer->height;
	//����Zbuffer
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

	//Camera *camera = new Camera(camera_pos, target_pos, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT);
	mat4_t view_matrix = camera_get_view_matrix(*camera);
	mat4_t proj_matrix = camera_get_proj_matrix(*camera);

	//ÿ�������㸳ֵ��abc_3d,һ������12��������
	for (int index = 0; index < 12; index++)
	{
		vec3_t abc_3d[3] = { vec3_new(vertices[index * 9], vertices[index * 9 + 1], vertices[index * 9 + 2]),
			vec3_new(vertices[index * 9 + 3], vertices[index * 9 + 4], vertices[index * 9 + 5]),
			vec3_new(vertices[index * 9 + 6], vertices[index * 9 + 7], vertices[index * 9 + 8]) };

		vec4_t clip_abc[3];
		vec3_t ndc_coords[3];

		vec2_t screen_coords[3];
		float screen_depths[3];

		float recip_w[3];

		for (int i = 0; i < 3; i++)
		{
			clip_abc[i] = mat4_mul_vec4(proj_matrix, mat4_mul_vec4(view_matrix, vec4_from_vec3(abc_3d[i], 1)));
			recip_w[i] = 1 / clip_abc[i].w;
			vec3_t clip_coord = vec3_from_vec4(clip_abc[i]);
			ndc_coords[i] = vec3_div(clip_coord, clip_abc[i].w);  //��һ������͸�ӳ���
			vec3_t window_coord = viewport_transform(width, height, ndc_coords[i]); //��һ�������ӿڱ任
			screen_coords[i] = vec2_new(window_coord.x, window_coord.y);
			screen_depths[i] = window_coord.z;
		}

		//vec2_t abc[3] = { vec2_new(100 , 300), vec2_new(200 , 600), vec2_new(300, 100) };

		bbox_t bbox = find_bounding_box(screen_coords, width, height);
		//random 0-1 color
		//vec4_t color1{ (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1 };
		vec4_t color1{ 1,0,0,1 };
		vec4_t color2{ 0,1,0,1 };
		vec4_t color3{ 0,0,1,1 };
		for (int i = bbox.min_x; i <= bbox.max_x; i++)
		{
			for (int j = bbox.min_y; j <= bbox.max_y; j++)
			{
				vec2_t p{ (float)(i + 0.5), (float)(j + 0.5) };
				vec3_t result = calculate_weights(screen_coords, p);

				if (!(result.x > -EPSILON && result.y > -EPSILON && result.z > -EPSILON)) continue; //����һ�¸��������ȿ��ܴ���������
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
				vec3_t new_weights = interpolate_varyings_weights(result, recip_w);
				vec4_t color = vec4_add(vec4_mul(color1, new_weights.x), vec4_add(vec4_mul(color2, new_weights.y), vec4_mul(color3, new_weights.z)));
				//vec4_t zBufferValue{ zbuffer[screen_index],zbuffer[screen_index], zbuffer[screen_index], zbuffer[screen_index] };
				//std::cout << zbuffer[screen_index] << std::endl;
				draw_fragment(framebuffer, j * width + i, color, nullptr);

			}
		}
	}
	delete[] zbuffer;
}

void test_enter_mainloop5(tickfunc_t* tickfunc)
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
		float delta_time = curr_time - prev_time; //��֡���йص�ֵ
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

		record.orbit_delta.reset();
		record.pan_delta.reset();
		record.dolly_delta = 0;
		record.single_click = 0;
		record.double_click = 0;
		input_poll_events();
	}
	window_destroy(window);
	delete(framebuffer);
	delete(camera);
}


void test_camera_move() 
{
	test_enter_mainloop5(move_camera_and_draw);
}
