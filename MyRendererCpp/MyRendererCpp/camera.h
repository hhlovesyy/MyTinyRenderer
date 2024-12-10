#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"
#include <memory>
#include <iostream>
class Camera
{
public:
	vec3 position;
	vec3 target;
	float aspect;
	Camera(vec3 position, vec3 target, float aspect) : position(position), target(target), aspect(aspect) {}
};
class motion_t
{ 
public:
	vec2 orbit; 
	vec2 pan;
	float dolly;
} ;

mat4_t camera_get_view_matrix(Camera& camera);
mat4_t camera_get_proj_matrix(Camera& camera);
void camera_set_transform(Camera* camera, vec3 position, vec3 target);
void camera_update_transform(Camera* camera, std::shared_ptr<motion_t> motion);
#endif