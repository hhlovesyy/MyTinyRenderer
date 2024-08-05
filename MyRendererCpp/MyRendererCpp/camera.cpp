#include "camera.h"

static const float NEAR = 0.01f;
static const float FAR = 1000;
static const float FOVY = TO_RADIANS(60);
static const vec3_t UP = { 0, 1, 0 };

mat4_t camera_get_view_matrix(Camera& camera)
{
	return mat4_lookat(camera.position, camera.target, UP);
}

mat4_t camera_get_proj_matrix(Camera& camera) 
{
	return mat4_perspective(FOVY, camera.aspect, NEAR, FAR);
}