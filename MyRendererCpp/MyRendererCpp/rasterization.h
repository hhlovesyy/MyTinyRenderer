#ifndef RASTERIZATION_H
#define RASTERIZATION_H

#include "maths.h"
#include "scene.h"
#include "shader_BlinnPhong.h"
#include "macro.h"
#include "test_helper.h"

void rasterization_tri(Mesh* mesh, Program* program, framebuffer_t* framebuffer);
#endif // !RASTERIZATION_H
