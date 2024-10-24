#ifndef TEST_HELPER_H
#define TEST_HELPER_H
#include "win32.h"
#include "camera.h"

static const char* const WINDOW_TITLE = "test";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = { -0.3, 0.3, 1.4 };
static const vec3_t CAMERA_TARGET = { 0, 0, 0 };

static const float CLICK_DELAY = 0.25f;


class record_t
{
public:
    /* orbit */
    int is_orbiting;
    vec2_t orbit_pos;
    vec2_t orbit_delta;
    /* pan */
    int is_panning;
    vec2_t pan_pos;
    vec2_t pan_delta;
    /* zoom */
    float dolly_delta;

    /* click */
    float press_time;
    float release_time;
    vec2_t press_pos;
    vec2_t release_pos;
    int single_click;
    int double_click;
    vec2_t click_pos;
};

void button_callback(window_t* window, button_t button, int pressed);
void update_camera(window_t* window, Camera* camera,
    record_t* record);
void update_click(float curr_time, record_t* record);
void scroll_callback(window_t* window, float offset);
#endif