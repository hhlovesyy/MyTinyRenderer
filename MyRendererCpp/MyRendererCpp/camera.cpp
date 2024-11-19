#include "camera.h"
#include "macro.h"
#include <assert.h>
static const float NEAR = 0.1f;
static const float FAR = 10000;
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

/* camera updating */

void camera_set_transform(Camera* camera, vec3_t position, vec3_t target)
{
    assert(vec3_length(vec3_sub(position, target)) > EPSILON);
    camera->position = position;
    camera->target = target;
}

static vec3_t calculate_pan(vec3_t from_camera, std::shared_ptr<motion_t> motion)
{
    vec3_t forward = vec3_normalize(from_camera);
    vec3_t left = vec3_cross(UP, forward);
    vec3_t up = vec3_cross(forward, left);

    float distance = vec3_length(from_camera);
    float factor = distance * (float)tan(FOVY / 2) * 2;
    vec3_t delta_x = vec3_mul(left, motion->pan.x * factor);
    vec3_t delta_y = vec3_mul(up, motion->pan.y * factor);
    return vec3_add(delta_x, delta_y);
}

static vec3_t calculate_offset(vec3_t from_target, std::shared_ptr<motion_t> motion)
{
    float radius = vec3_length(from_target);
    float theta = (float)atan2(from_target.x, from_target.z);  /* azimuth */
    float phi = (float)acos(from_target.y / radius);           /* polar */
    float factor = PI * 2;
    vec3_t offset;

    radius *= (float)pow(0.95, motion->dolly);
    theta -= motion->orbit.x * factor;
    phi -= motion->orbit.y * factor;
    phi = float_clamp(phi, EPSILON, PI - EPSILON);

    offset.x = radius * (float)sin(phi) * (float)sin(theta);
    offset.y = radius * (float)cos(phi);
    offset.z = radius * (float)sin(phi) * (float)cos(theta);

    return offset;
}

void camera_update_transform(Camera* camera, std::shared_ptr<motion_t> motion)
{
    vec3_t from_target = vec3_sub(camera->position, camera->target);
    vec3_t from_camera = vec3_sub(camera->target, camera->position);
    //std::cout<<motion.use_count()<<std::endl;
    vec3_t pan = calculate_pan(from_camera, motion);
    //std::cout << motion.use_count() << std::endl;
    vec3_t offset = calculate_offset(from_target, motion);
    //std::cout << motion.use_count() << std::endl;
    camera->target = vec3_add(camera->target, pan);
    camera->position = vec3_add(camera->target, offset);
}