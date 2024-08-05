#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"
class Camera
{
public:
	vec3_t position;
	vec3_t target;
	float aspect;
	Camera(vec3_t position, vec3_t target, float aspect) : position(position), target(target), aspect(aspect) {}
};

mat4_t camera_get_view_matrix(Camera& camera);
mat4_t camera_get_proj_matrix(Camera& camera);
#endif
