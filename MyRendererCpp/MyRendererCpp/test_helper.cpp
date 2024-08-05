#include "test_helper.h"
#include "win32.h"
#include <iostream>
#include "camera.h"

void input_query_cursor(window_t* window, float* xpos, float* ypos)
{
    POINT point;
    GetCursorPos(&point);
    ScreenToClient(window->handle, &point);
    *xpos = (float)point.x;
    *ypos = (float)point.y;
}

static vec2_t get_cursor_pos(window_t* window)
{
    float xpos, ypos;
    input_query_cursor(window, &xpos, &ypos);
    return vec2_new(xpos, ypos);
}

static vec2_t get_pos_delta(vec2_t old_pos, vec2_t new_pos)
{
    vec2_t delta = vec2_sub(new_pos, old_pos);
    return vec2_div(delta, (float)WINDOW_HEIGHT);
}

void update_click(float curr_time, record_t* record) 
{
    float last_time = record->release_time;
    if (last_time && curr_time - last_time > CLICK_DELAY) {
        vec2_t pos_delta = vec2_sub(record->release_pos, record->press_pos);
        if (vec2_length(pos_delta) < 5) {
            record->single_click = 1;
        }
        record->release_time = 0;
    }
    if (record->single_click || record->double_click) {
        float click_x = record->release_pos.x / WINDOW_WIDTH;
        float click_y = record->release_pos.y / WINDOW_HEIGHT;
        record->click_pos = vec2_new(click_x, 1 - click_y);
    }
}

void update_camera(window_t* window, Camera* camera,
    record_t* record)
{
    vec2_t cursor_pos = get_cursor_pos(window);
    if (record->is_orbiting) 
    {
        vec2_t pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
        record->orbit_delta = vec2_add(record->orbit_delta, pos_delta);
        record->orbit_pos = cursor_pos;
        //std::cout<<record->orbit_delta.x<<" "<<record->orbit_delta.y<<std::endl;
    }
    if (record->is_panning)
    {
        vec2_t pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
        record->pan_delta = vec2_add(record->pan_delta, pos_delta);
        record->pan_pos = cursor_pos;
    }
    if (input_key_pressed(window, KEY_SPACE))
    {
        camera_set_transform(camera, CAMERA_POSITION, CAMERA_TARGET);
    }
    else
    {
        motion_t motion;
        motion.orbit = record->orbit_delta;
        motion.pan = record->pan_delta;
        motion.dolly = record->dolly_delta;
        camera_update_transform(camera, motion);
    }
}

void scroll_callback(window_t* window, float offset) 
{
    record_t* record = (record_t*)window_get_userdata(window);
    record->dolly_delta += offset;
}

void button_callback(window_t* window, button_t button, int pressed)
{
	record_t* record = (record_t*)window_get_userdata(window);
	vec2_t cursor_pos = get_cursor_pos(window);
    if (button == BUTTON_L)
    {
        float curr_time = platform_get_time();
        if (pressed)
        {
            record->is_orbiting = 1;
            record->orbit_pos = cursor_pos;
            record->press_time = curr_time;
            record->press_pos = cursor_pos;
        }
        else
        {
            float prev_time = record->release_time;
            vec2_t pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
            record->is_orbiting = 0;
            record->orbit_delta = vec2_add(record->orbit_delta, pos_delta);
            if (prev_time && curr_time - prev_time < CLICK_DELAY) 
            {
                record->double_click = 1;
                record->release_time = 0;
            }
            else 
            {
                record->release_time = curr_time;
                record->release_pos = cursor_pos;
            }
		}
    }
    else if (button == BUTTON_R) 
    {
        if (pressed) 
        {
            record->is_panning = 1;
            record->pan_pos = cursor_pos;
        }
        else
        {
            vec2_t pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
            record->is_panning = 0;
            record->pan_delta = vec2_add(record->pan_delta, pos_delta);
        }
    }
}