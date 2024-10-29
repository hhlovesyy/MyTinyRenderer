#ifndef SKELETON_H
#define SKELETON_H
#include "maths.h"
#include <vector>

class Joint
{
public:
	int joint_index;
	int parent_index;
	mat4_t inverse_bind_pose;
	//translations
	int num_translations;
	std::vector<float> translation_times; //时间,应该指的是关键帧的时间
	std::vector<vec3_t> translations;
	//rotations
	int num_rotations;
	std::vector<float> rotation_times;
	std::vector<quat_t> rotations;
	//scales
	int num_scales;
	std::vector<float> scale_times;
	std::vector<vec3_t> scales;  //这里认为可以接收非统一缩放
	/* interpolated */
	mat4_t transform;
};

class Skeleton
{
public:
	float min_time;
	float max_time;
	int num_joints;
	std::vector<Joint> joints;
	/* cached result */
	std::vector<mat4_t> joint_matrices;
	std::vector<mat3_t> normal_matrices;
	float last_time;

	static Skeleton* load(std::string filename);
};
static Joint load_joint(FILE* file);
static void read_scales(FILE* file, Joint* joint);
static void read_inverse_bind(FILE* file, Joint* joint);
static void read_translations(FILE* file, Joint* joint);
static void read_rotations(FILE* file, Joint* joint);
void skeleton_update_joints(Skeleton* skeleton, float frame_time);
static vec3_t get_translation(Joint* joint, float frame_time);
static quat_t get_rotation(Joint* joint, float frame_time);
static vec3_t get_scale(Joint* joint, float frame_time);
#endif
