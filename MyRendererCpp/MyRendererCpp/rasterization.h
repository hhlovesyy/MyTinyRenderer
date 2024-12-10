#ifndef RASTERIZATION_H
#define RASTERIZATION_H

#include "maths.h"
#include "scene.h"
#include "shader_BlinnPhong.h"
#include "macro.h"
#include "test_helper.h"

void rasterization_tri(Mesh* mesh, Program* program, framebuffer_t* framebuffer,bool isDrawShadowMap);
void graphics_draw_triangle(framebuffer_t* framebuffer, Program* program);
int rasterize_triangle(framebuffer_t* framebuffer, Program* program,
	vec4 clip_coords[3], void* varyings[3]);
#endif // !RASTERIZATION_H
