#include "camera.h"
#include "macro.h"
#include <assert.h>
static const float NEAR = 0.1f;
static const float FAR = 10000;
static const float FOVY = TO_RADIANS(60);
static const vec3 UP = { 0, 1, 0 };

mat4_t camera_get_view_matrix(Camera& camera)
{
	return mat4_lookat(camera.position, camera.target, UP);
}

mat4_t camera_get_proj_matrix(Camera& camera) 
{
	return mat4_perspective(FOVY, camera.aspect, NEAR, FAR);
}

/* camera updating */

void camera_set_transform(Camera* camera, vec3 position, vec3 target)
{
    assert((pos - target).length() > EPSILON);
    camera->position = position;
    camera->target = target;
}

static vec3 calculate_pan(vec3& from_camera, std::shared_ptr<motion_t> motion)
{
    vec3 forward = from_camera.normalized();
    vec3 left = cross(UP, forward);
    vec3 up = cross(forward, left);

    float distance = from_camera.length();
    float factor = distance * (float)tan(FOVY / 2) * 2;
    vec3 delta_x = left * (motion->pan[0] * factor);
    vec3 delta_y = up *(motion->pan[1] * factor);
    return delta_x + delta_y;
}

static vec3 calculate_offset(vec3 from_target, std::shared_ptr<motion_t> motion)
{
    float radius = from_target.length();
    float theta = (float)atan2(from_target[0], from_target[2]);  /* azimuth */
    float phi = (float)acos(from_target[1] / radius);           /* polar */
    float factor = PI * 2;
    vec3 offset;

    radius *= (float)pow(0.95, motion->dolly);
    theta -= motion->orbit[0] * factor;
    phi -= motion->orbit[1] * factor;
    phi = float_clamp(phi, EPSILON, PI - EPSILON);

    offset[0] = radius * (float)sin(phi) * (float)sin(theta);
    offset[1] = radius * (float)cos(phi);
    offset[2] = radius * (float)sin(phi) * (float)cos(theta);

    return offset;
}

void camera_update_transform(Camera* camera, std::shared_ptr<motion_t> motion)
{
    vec3 from_target = camera->position - camera->target;
    vec3 from_camera = camera->target- camera->position;
    //std::cout<<motion.use_count()<<std::endl;
    vec3 pan = calculate_pan(from_camera, motion);
    //std::cout << motion.use_count() << std::endl;
    vec3 offset = calculate_offset(from_target, motion);
    //std::cout << motion.use_count() << std::endl;
    camera->target = camera->target + pan;
    camera->position = camera->target + offset;
}