#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
#include "image.h"
#include "graphics.h"
#include "macro.h"

#ifdef UNICODE
static const wchar_t* const WINDOW_CLASS_NAME = L"Class";
static const wchar_t* const WINDOW_ENTRY_NAME = L"Entry";
#else
static const char* const WINDOW_CLASS_NAME = "Class";
static const char* const WINDOW_ENTRY_NAME = "Entry";
#endif
typedef enum { KEY_A, KEY_D, KEY_S, KEY_W, KEY_SPACE, KEY_NUM } keycode_t;
typedef enum { BUTTON_L, BUTTON_R, BUTTON_NUM } button_t;

struct window_t;

typedef struct
{
	void (*key_callback)(window_t* window, keycode_t key, int pressed);
	void (*button_callback)(window_t* window, button_t button, int pressed);
	void (*scroll_callback)(window_t* window, float offset);
} callbacks_t;

struct window_t
{
	HWND handle;
	HDC memory_dc;
	image_t* surface;
	/* common data */
	int should_close;
	char keys[KEY_NUM];
    char buttons[BUTTON_NUM];
	callbacks_t callbacks;
	void* userdata;
};

void* window_get_userdata(window_t* window);


/* window related functions */
static int g_initialized = 0;

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam);

static void register_class(void);

static void initialize_path(void);

void platform_initialize(void);

static HWND create_window(const char* title_, int width, int height);

/*
 * for memory device context, see
 * https://docs.microsoft.com/en-us/windows/desktop/gdi/memory-device-contexts
 */
static void create_surface(HWND handle, int width, int height,
	image_t** out_surface, HDC* out_memory_dc);

void private_blit_bgr(framebuffer_t* src, image_t* dst);

static void present_surface(window_t* window);

void window_draw_buffer(window_t* window, framebuffer_t* buffer);

window_t* window_create(const char* title, int width, int height);

void window_destroy(window_t* window);

float platform_get_time(void);

void input_poll_events(void);

int input_key_pressed(window_t* window, keycode_t key);