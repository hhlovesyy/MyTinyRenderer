#ifndef SCENE_H
#define SCENE_H
#include "maths.h"
#include "mesh.h"
#include "camera.h"
#include "graphics.h"
#include "skeleton.h"

class Model
{
public:
	Mesh* mesh;
	mat4_t transform;
	float distance;//中心点到摄像机的距离，可用于深度排序渲染顺序
	void (*update)(Model* model, Camera* camera);
	void (*draw)(Model* model, framebuffer_t* framebuffer,bool isDrawShadowMap);
	void (*release)(Model* model);

	void (*update_new)(std::shared_ptr<Model> model, Camera* camera);
	void (*draw_new)(std::shared_ptr<Model> model, framebuffer_t* framebuffer,bool isDrawShadowMap);

	Program* program;
	Skeleton* skeleton;
	int transparent;//是否是透明物体 用于渲染顺序
};

class Scene
{
public:
	vec4 background;
	/* light intensity */
	float ambient_intensity;
	framebuffer_t* shadowmap_buffer;
	Texture* shadowmap;
	//framebuffer_t* shadowmap;//暂时
	std::vector<Model*> models;
	std::shared_ptr<Model> skybox;
};

class FrameInfo
{
public:
	static float frame_time;
	static float ambient_intensity;

	static vec3 light_dir;
	//mat4_t light_view_matrix; //关照视角矩阵 用于比如ShadowMap

	static void set_frame_time(float time)
	{
		frame_time = time;
	}
	static float get_frame_time()
	{
		return frame_time;
	}
	static mat4_t get_light_view_matrix()
	{
		//如果光源旋转等，就需要每帧更新，否则不用。这里暂时是每帧更新
		vec3 light_pos = light_dir;
		vec3 light_target{ 0, 0, 0 };
		vec3 light_up{ 0, 1, 0 };
		return mat4_lookat(light_pos, light_target, light_up);
	}
	static mat4_t get_light_proj_matrix(float half_w, float half_h,
		float z_near, float z_far)
	{
		return mat4_orthographic(half_w, half_h, z_near, z_far);
	}
	static mat4_t get_light_proj_matrix()
	{
		return mat4_orthographic(1, 1, 0, 5);
	}
};

#endif