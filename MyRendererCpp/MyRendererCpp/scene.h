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
	float distance;//���ĵ㵽������ľ��룬���������������Ⱦ˳��
	void (*update)(Model* model, Camera* camera);
	void (*draw)(Model* model, framebuffer_t* framebuffer);
	void (*release)(Model* model);
	Program* program;
	Skeleton* skeleton;
	int transparent;//�Ƿ���͸������ ������Ⱦ˳��
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