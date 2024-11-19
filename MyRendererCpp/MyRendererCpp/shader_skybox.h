#ifndef SHADER_SKYBOX_H
#define SHADER_SKYBOX_H
#include "mesh.h"
#include "scene.h"
#include "graphics.h"
#include "rasterization.h"
#include <memory>

struct attribs_skybox
{
	vec3_t position;
};

struct varyings_skybox
{
	vec3_t direction;
};

struct uniforms_skybox
{
	mat4_t vp_matrix;
	std::shared_ptr<CubeMap> skybox;
};

vec4_t skybox_vertex_shader(void* attribs, void* varyings, void* uniforms);
vec4_t skybox_fragment_shader(void* varyings, void* uniforms,
	int* discard, int backface);
std::shared_ptr<Model> skybox_create_model(std::string skybox_name, int blur_level);

#endif