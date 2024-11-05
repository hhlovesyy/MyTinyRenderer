#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "macro.h"
#include "mesh.h"
#include "private.h"
#include "skeleton.h"

static void read_inverse_bind(FILE* file, Joint* joint)
{
	char line[LINE_SIZE];
	int items;
	int i;
	items = fscanf(file, " %s", line);
	assert(items == 1 && strcmp(line, "inverse-bind:") == 0);
	for (i = 0; i < 4; i++) {
		items = fscanf(file, " %f %f %f %f",
			&joint->inverse_bind_pose.m[i][0],
			&joint->inverse_bind_pose.m[i][1],
			&joint->inverse_bind_pose.m[i][2],
			&joint->inverse_bind_pose.m[i][3]);
		assert(items == 4);
	}
}

static void read_translations(FILE* file, Joint* joint) 
{
    int items;
    int i;
    items = fscanf(file, " translations %d:", &joint->num_translations);
    assert(items == 1 && joint->num_translations >= 0);
    if (joint->num_translations > 0) 
    {
		joint->translations.resize(joint->num_translations);
		joint->translation_times.resize(joint->num_translations);
        //std::vector<vec3_t> translations;
        //std::vector<float> translation_times;
        for (i = 0; i < joint->num_translations; i++)
        {
			items = fscanf(file, " time: %f, value: [%f, %f, %f]",
                				&joint->translation_times[i],
                				&joint->translations[i].x,
                				&joint->translations[i].y,
                				&joint->translations[i].z);
			assert(items == 4);
		}
    }
    
    UNUSED_VAR(items);
}

static void read_rotations(FILE* file, Joint* joint) 
{
    int items;
    int i;
    items = fscanf(file, " rotations %d:", &joint->num_rotations);
    assert(items == 1 && joint->num_rotations >= 0);
    if (joint->num_rotations > 0)
    {
        //std::vector<quat_t> rotations;
		//std::vector<float> rotation_times;
		joint->rotations.resize(joint->num_rotations);
		joint->rotation_times.resize(joint->num_rotations);
        for (i = 0; i < joint->num_rotations; i++)
        {
            items = fscanf(file, " time: %f, value: [%f, %f, %f, %f]",
                				&joint->rotation_times[i],
                				&joint->rotations[i].x,
                				&joint->rotations[i].y,
                				&joint->rotations[i].z,
                				&joint->rotations[i].w);
			assert(items == 5);
        }
    }
    UNUSED_VAR(items);
}

static void read_scales(FILE* file, Joint* joint) 
{
    int items;
    int i;
    items = fscanf(file, " scales %d:", &joint->num_scales);
    assert(items == 1 && joint->num_scales >= 0);
    if (joint->num_scales > 0) 
    {
		joint->scales.resize(joint->num_scales);
		joint->scale_times.resize(joint->num_scales);
		for (i = 0; i < joint->num_scales; i++)
		{
			items = fscanf(file, " time: %f, value: [%f, %f, %f]",
												&joint->scale_times[i],
												&joint->scales[i].x,
												&joint->scales[i].y,
												&joint->scales[i].z);
			assert(items == 4);
		}
    }
    UNUSED_VAR(items);
}

static vec3_t get_translation(Joint* joint, float frame_time)
{
    int num_translations = joint->num_translations;
    std::vector<float> translation_times = joint->translation_times;
    std::vector<vec3_t> translation_values = joint->translations;

    if (num_translations == 0) {
        return vec3_new(0, 0, 0);
    }
    else if (frame_time <= translation_times[0]) {
        return translation_values[0];
    }
    else if (frame_time >= translation_times[num_translations - 1]) {
        return translation_values[num_translations - 1];
    }
    else {
        int i;
        for (i = 0; i < num_translations - 1; i++) {
            float curr_time = translation_times[i];
            float next_time = translation_times[i + 1];
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                vec3_t curr_translation = translation_values[i];
                vec3_t next_translation = translation_values[i + 1];
                return vec3_lerp(curr_translation, next_translation, t);
            }
        }
        assert(0);
        return vec3_new(0, 0, 0);
    }
}

static quat_t get_rotation(Joint* joint, float frame_time) 
{
    int num_rotations = joint->num_rotations;
    std::vector<float> rotation_times = joint->rotation_times;
    std::vector<quat_t> rotation_values = joint->rotations;

    if (num_rotations == 0) {
        return quat_new(0, 0, 0, 1);
    }
    else if (frame_time <= rotation_times[0]) {
        return rotation_values[0];
    }
    else if (frame_time >= rotation_times[num_rotations - 1]) {
        return rotation_values[num_rotations - 1];
    }
    else {
        int i;
        for (i = 0; i < num_rotations - 1; i++) {
            float curr_time = rotation_times[i];
            float next_time = rotation_times[i + 1];
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                quat_t curr_rotation = rotation_values[i];
                quat_t next_rotation = rotation_values[i + 1];
                return quat_slerp(curr_rotation, next_rotation, t);
            }
        }
        assert(0);
        return quat_new(0, 0, 0, 1);
    }
}

static vec3_t get_scale(Joint* joint, float frame_time) 
{
    int num_scales = joint->num_scales;
    std::vector<float> scale_times = joint->scale_times;
    std::vector<vec3_t> scale_values = joint->scales;

    if (num_scales == 0) {
        return vec3_new(1, 1, 1);
    }
    else if (frame_time <= scale_times[0]) {
        return scale_values[0];
    }
    else if (frame_time >= scale_times[num_scales - 1]) {
        return scale_values[num_scales - 1];
    }
    else {
        int i;
        for (i = 0; i < num_scales - 1; i++) {
            float curr_time = scale_times[i];
            float next_time = scale_times[i + 1];
            if (frame_time >= curr_time && frame_time < next_time) {
                float t = (frame_time - curr_time) / (next_time - curr_time);
                vec3_t curr_scale = scale_values[i];
                vec3_t next_scale = scale_values[i + 1];
                return vec3_lerp(curr_scale, next_scale, t);
            }
        }
        assert(0);
        return vec3_new(1, 1, 1);
    }
}

void skeleton_update_joints(Skeleton* skeleton, float frame_time) 
{
    frame_time = (float)fmod(frame_time, skeleton->max_time);
    if (frame_time != skeleton->last_time) {
        int i;
        for (i = 0; i < skeleton->num_joints; i++) {
            Joint* joint = &skeleton->joints[i];
            vec3_t translation = get_translation(joint, frame_time);
            quat_t rotation = get_rotation(joint, frame_time);
            vec3_t scale = get_scale(joint, frame_time);
            mat4_t joint_matrix;
            mat3_t normal_matrix;

            //这个joint->transform指的是相对于父节点的变换
            joint->transform = mat4_from_trs(translation, rotation, scale);
            if (joint->parent_index >= 0) 
            {
                Joint* parent = &skeleton->joints[joint->parent_index];
                //在算到子joint的时候，父joint的transform已经算好了，这个是记录文件的时候要约定的内容
                joint->transform = mat4_mul_mat4(parent->transform,
                    joint->transform);
            }

            joint_matrix = mat4_mul_mat4(joint->transform, joint->inverse_bind_pose);
            normal_matrix = mat3_inverse_transpose(mat3_from_mat4(joint_matrix));
            //如果数组不够大，扩容
            if (skeleton->joint_matrices.size() < skeleton->num_joints)
            {
				skeleton->joint_matrices.resize(skeleton->num_joints);
				skeleton->normal_matrices.resize(skeleton->num_joints);
			}
            skeleton->joint_matrices[i] = joint_matrix;
            skeleton->normal_matrices[i] = normal_matrix;
        }
        skeleton->last_time = frame_time;
    }
}

static Joint load_joint(FILE* file)
{
	Joint joint; //Joint* joint = new Joint(); 
	int items;
	items = fscanf(file, " joint %d:", &joint.joint_index);
	assert(items == 1);
	items = fscanf(file, " parent-index: %d", &joint.parent_index);
	assert(items == 1);
	read_inverse_bind(file, &joint);
	read_translations(file, &joint);
	read_rotations(file, &joint);
	read_scales(file, &joint);
	return joint;
}

//load函数用于加载模型文件，返回一个Mesh对象。
Skeleton* Skeleton::load(std::string filename)
{
	Skeleton* skeleton = new Skeleton();
	FILE* file = fopen(filename.c_str(), "rb");
	if (file == nullptr)
	{
		std::cerr << "Failed to open skeleton file: " << filename << std::endl;
		return nullptr;
	}
	int items = fscanf(file, " joint-size: %d", &skeleton->num_joints);
	assert(items == 1 && skeleton->num_joints > 0);
	items = fscanf(file, " time-range: [%f, %f]",
		&skeleton->min_time, &skeleton->max_time);
	assert(items == 2 && skeleton->min_time < skeleton->max_time);
	skeleton->joints.resize(skeleton->num_joints);
	for (int i = 0; i < skeleton->num_joints; i++)
	{
		Joint joint = load_joint(file);
		assert(joint.joint_index == i);
		skeleton->joints[i] = joint;
	}
	fclose(file);
	return skeleton;
}