#ifndef TEST_HELPER_H
#define TEST_HELPER_H
#include "win32.h"
#include "camera.h"

static const char* const WINDOW_TITLE = "blinn_phong";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3 CAMERA_POSITION = { 2, 2, 1.5f };
static const vec3 CAMERA_TARGET = { 0, 0, 0 };

static const float CLICK_DELAY = 0.25f;


class record_t
{
public:
    /* orbit */
    int is_orbiting;
    vec2 orbit_pos;
    vec2 orbit_delta;
    /* pan */
    int is_panning;
    vec2 pan_pos;
    vec2 pan_delta;
    /* zoom */
    float dolly_delta;

    /* click */
    float press_time;
    float release_time;
    vec2 press_pos;
    vec2 release_pos;
    int single_click;
    int double_click;
    vec2 click_pos;
};

void button_callback(window_t* window, button_t button, int pressed);
void update_camera(window_t* window, Camera* camera,
    record_t* record);
void update_click(float curr_time, record_t* record);
void scroll_callback(window_t* window, float offset);
#endif