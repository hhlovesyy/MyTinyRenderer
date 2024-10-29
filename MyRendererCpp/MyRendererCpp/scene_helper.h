#ifndef SCENE_HELPER_H
#define SCENE_HELPER_H
#include "scene.h"
#include "shader_BlinnPhong.h"

Scene scene_from_file(const char* filename, mat4_t root);

#endif