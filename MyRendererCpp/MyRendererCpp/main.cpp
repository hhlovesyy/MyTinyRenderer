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
#include "win32.h"

typedef void testfunc_t();
typedef struct { const char* testname; testfunc_t* testfunc; } testcase_t;

static testcase_t g_testcases[] = 
{
    {"test_rasterization", test_rasterization},
    {"test_transform2", test_transform2},
    {"test_model_input",test_model_input},
    {"test_space_transform", test_space_transform},
    {"test_camera_move", test_camera_move},
};

int main(int argc, char* argv[])
{
    platform_initialize();
    //now we test rasterization
    const char* testname = g_testcases[2].testname;
    testfunc_t* testfunc = g_testcases[2].testfunc;
    //使用函数指针调用函数，比如：调用test_model_input函数（因为引用了test_model_input.h，test_model_input.h中包含test_model_input函数）
    if (testfunc)
    {
        testfunc();
    }
}