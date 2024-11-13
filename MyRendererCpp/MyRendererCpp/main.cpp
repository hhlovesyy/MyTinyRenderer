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
#include "win32.h"
#include "scene.h"
#include "config.h"


typedef void testfunc_t();
typedef struct { const char* testname; testfunc_t* testfunc; } testcase_t;
float FrameInfo::frame_time = 0.0f; // ��̬��Ա�����Ķ������ʼ��
float FrameInfo::ambient_intensity = 0.0f; // ��̬��Ա�����Ķ������ʼ��
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
};

int main(int argc, char* argv[])
{
    platform_initialize();
    //now we test rasterization
    const char* testname = g_testcases[0].testname;
    testfunc_t* testfunc = g_testcases[0].testfunc;
    //ʹ�ú���ָ����ú��������磺����test_model_input��������Ϊ������test_model_input.h��test_model_input.h�а���test_model_input������
    if (testfunc)
    {
        testfunc();
    }
}