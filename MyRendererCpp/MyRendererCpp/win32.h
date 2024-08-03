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

struct window_t
{
	HWND handle;
	HDC memory_dc;
	image_t* surface;
};

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