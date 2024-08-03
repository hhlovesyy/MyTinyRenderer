#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "test_rasterization.h"
#include "win32.h"

typedef void testfunc_t();
typedef struct { const char* testname; testfunc_t* testfunc; } testcase_t;

static testcase_t g_testcases[] = 
{
    {"blinn", test_rasterization},
};

int main(int argc, char* argv[])
{
    platform_initialize();
    //now we test rasterization
    const char* testname = g_testcases[0].testname;
    testfunc_t* testfunc = g_testcases[0].testfunc;
    if (testfunc)
    {
        testfunc();
    }
}