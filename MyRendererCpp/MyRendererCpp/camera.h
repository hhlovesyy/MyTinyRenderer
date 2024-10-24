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
class motion_t
{ 
public:
	vec2_t orbit; 
	vec2_t pan; 
	float dolly;
} ;

mat4_t camera_get_view_matrix(Camera& camera);
mat4_t camera_get_proj_matrix(Camera& camera);
void camera_set_transform(Camera* camera, vec3_t position, vec3_t target);
void camera_update_transform(Camera* camera, motion_t motion);
#endif