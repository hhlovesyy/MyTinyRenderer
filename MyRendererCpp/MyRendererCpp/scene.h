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
	void (*draw)(Model* model, framebuffer_t* framebuffer);
	void (*release)(Model* model);
	Program* program;
	Skeleton* skeleton;
	int transparent;//是否是透明物体 用于渲染顺序
};

class Scene
{
public:
	vec4_t background;
	/* light intensity */
	float ambient_intensity;
	std::vector<Model*> models;
};

class FrameInfo
{
public:
	static float frame_time;
	static void set_frame_time(float time)
	{
		frame_time = time;
	}
	static float get_frame_time()
	{
		return frame_time;
	}
};

#endif