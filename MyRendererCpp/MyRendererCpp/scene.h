#ifndef SCENE_H
#define SCENE_H
#include "maths.h"
#include "mesh.h"
#include "camera.h"
#include "graphics.h"

class Model
{
public:
	Mesh* mesh;
	mat4_t transform;
	void (*update)(Model* model, Camera* camera);
	void (*draw)(Model* model, framebuffer_t* framebuffer);
	void (*release)(Model* model);
};

class Scene
{
public:
	vec4_t background;
	std::vector<Model*> models;
};

#endif