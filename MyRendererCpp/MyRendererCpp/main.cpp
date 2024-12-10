#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include "test_rasterization.h"
#include "test_transform.h"
#include "test_rasterization.h"
#include "test_model_input.h"
#include "test_space_transform.h"
#include "test_camera_move.h"
#include "test_skeleton_animation.h"
#include "test_blinn_phong_light.h"
#include "test_maths.h"
#include "win32.h"
#include "scene.h"
#include "config.h"


typedef void testfunc_t();
typedef struct { const char* testname; testfunc_t* testfunc; } testcase_t;
float FrameInfo::frame_time = 0.0f; // 静态成员变量的定义与初始化
float FrameInfo::ambient_intensity = 0.0f; // 静态成员变量的定义与初始化
vec3_t FrameInfo::light_dir = vec3_new(0.5f, 0.8f, 0.9f); // 静态成员变量的定义与初始化
bool Global_Config::should_flip_uv_y = false;
static testcase_t g_testcases[] = 
{
    {"test_rasterization", test_rasterization},
    {"test_transform2", test_transform2},
    {"test_model_input",test_model_input},
    {"test_space_transform", test_space_transform},
    {"test_camera_move", test_camera_move},
    {"test_skeleton_animation", test_skeleton_animation},
    {"test_blinn_phong_light", test_blinn_phong_light},
    {"test_maths", test_all_maths},
};

int main(int argc, char* argv[])
{
    platform_initialize();
    int test_index = 5;
    const char* testname = g_testcases[test_index].testname;
    testfunc_t* testfunc = g_testcases[test_index].testfunc;
    //使用函数指针调用函数
    if (testfunc)
    {
        testfunc();
    }
}