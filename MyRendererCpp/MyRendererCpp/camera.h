#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"
#include <memory>
#include <iostream>
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
	vec2<float> orbit; 
	vec2<float> pan;
	float dolly;
} ;

mat4_t camera_get_view_matrix(Camera& camera);
mat4_t camera_get_proj_matrix(Camera& camera);
void camera_set_transform(Camera* camera, vec3_t position, vec3_t target);
void camera_update_transform(Camera* camera, std::shared_ptr<motion_t> motion);
#endif