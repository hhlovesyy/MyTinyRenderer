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
//获取材质
std::vector<TGAImage> img;
void preLoadModel()
{
	//相对路径
	const char* model_path = "combinePamu.obj";
	Mesh* mesh = Mesh::load(model_path);
	if (mesh == nullptr) 
	{
		std::cerr << "Failed to load model: " << model_path << std::endl;
		return;
	}

	//获取模型的顶点数据
	vertices = mesh->getVertices();
	num_faces = mesh->getNumFaces();

	
	mesh->load_texture("assets/combinePamu", "_diffuse.tga", img);
}

vec4_t sample2D(TGAImage& image, vec2_t uv)
{
	int width = image.width();
	int height = image.height();
	int x = uv.x * width;
	int y = uv.y * height;
	if (x < 0) x = 0;
	if (x >= width) x = width - 1;
	if (y < 0) y = 0;
	if (y >= height) y = height - 1;
	TGAColor color = image.get(x, y);
	/*std::cout << color.bgra[0] << " " << color.bgra[1] << " " << color.bgra[2] << " " << color.bgra[3] << std::endl;*/
	return vec4_new(color.bgra[2], color.bgra[1], color.bgra[0], color.bgra[3]);

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
		//纹理坐标
		vec2_t uv[3] = {
			vec2_new(vertices[index * 3].texcoord.x, vertices[index * 3].texcoord.y),
			vec2_new(vertices[index * 3 + 1].texcoord.x, vertices[index * 3 + 1].texcoord.y),
			vec2_new(vertices[index * 3 + 2].texcoord.x, vertices[index * 3 + 2].texcoord.y)
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

		
		for (int i = bbox.min_x; i <= bbox.max_x; i++)
		{
			for (int j = bbox.min_y; j <= bbox.max_y; j++)
			{
				vec2_t p{ (float)(i + 0.5), (float)(j + 0.5) };
				vec3_t result = calculate_weights(screen_coords, p);
				

				if (!(result.x > 0 && result.y > 0 && result.z > 0)) continue;
				
				//对UV做重心插值，而不是对颜色做重心插值
				vec2_t uv_p = vec2_add(vec2_add(vec2_mul(uv[0], result.x), vec2_mul(uv[1], result.y)), vec2_mul(uv[2], result.z));
				
				vec4_t color = sample2D(img[0], vec2_t{ uv_p.x, 1.0f - uv_p.y });//纹理坐标的Y轴是反的
				//vec4_t color = sample2D(img[0], uv[0]);
				vec4_t finalColor = vec4_mul(color, 1.0f / 255.0f);

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
				draw_fragment(framebuffer, j * width + i, finalColor);

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
