#include "win32.h"
#include <iostream>

void* window_get_userdata(window_t* window) 
{
    return window->userdata;
}

static void register_class(void)
{
    ATOM class_atom;
    WNDCLASS window_class;
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = process_message;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = GetModuleHandle(NULL);
    window_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = WINDOW_CLASS_NAME;
    class_atom = RegisterClass(&window_class);
    assert(class_atom != 0);
    UNUSED_VAR(class_atom);
}

static void initialize_path(void)
{
#ifdef UNICODE
    wchar_t path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    *wcsrchr(path, L'\\') = L'\0';
    _wchdir(path);
    _wchdir(L"assets");
#else
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    *strrchr(path, '\\') = '\0';
    _chdir(path);
    _chdir("assets");
#endif
}

void platform_initialize(void)
{
    assert(g_initialized == 0);
    register_class();
    initialize_path();
    g_initialized = 1;
}

static HWND create_window(const char* title_, int width, int height) {
    DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT rect;
    HWND handle;

#ifdef UNICODE
    wchar_t title[LINE_SIZE];
    mbstowcs(title, title_, LINE_SIZE);
#else
    const char* title = title_;
#endif

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    AdjustWindowRect(&rect, style, 0);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    handle = CreateWindow(WINDOW_CLASS_NAME, title, style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    assert(handle != NULL);
    return handle;
}

/*
 * for memory device context, see
 * https://docs.microsoft.com/en-us/windows/desktop/gdi/memory-device-contexts
 */
static void create_surface(HWND handle, int width, int height,
    image_t** out_surface, HDC* out_memory_dc) {
    BITMAPINFOHEADER bi_header;
    HBITMAP dib_bitmap;
    HBITMAP old_bitmap;
    HDC window_dc;
    HDC memory_dc;
    image_t* surface;

    surface = image_create(width, height, 4);

    window_dc = GetDC(handle);
    memory_dc = CreateCompatibleDC(window_dc);
    ReleaseDC(handle, window_dc);

    memset(&bi_header, 0, sizeof(BITMAPINFOHEADER));
    bi_header.biSize = sizeof(BITMAPINFOHEADER);
    bi_header.biWidth = width;
    bi_header.biHeight = -height;  /* top-down */
    bi_header.biPlanes = 1;
    bi_header.biBitCount = 32;
    bi_header.biCompression = BI_RGB;
    dib_bitmap = CreateDIBSection(memory_dc, (BITMAPINFO*)&bi_header,
        DIB_RGB_COLORS, (void**)&surface->color_buffer,
        NULL, 0);
    assert(dib_bitmap != NULL);
    old_bitmap = (HBITMAP)SelectObject(memory_dc, dib_bitmap);
    DeleteObject(old_bitmap);

    *out_surface = surface;
    *out_memory_dc = memory_dc;
}

void private_blit_bgr(framebuffer_t* src, image_t* dst)
{
    int width = dst->width;
    int height = dst->height;
    int r, c;

    assert(src->width == dst->width && src->height == dst->height);
    assert(dst->channels == 4);

    for (r = 0; r < height; r++)
    {
        for (c = 0; c < width; c++)
        {
            int flipped_r = height - 1 - r;
            int src_index = (r * width + c) * 4;
            int dst_index = (flipped_r * width + c) * 4;
            char* src_pixel = &src->color_buffer[src_index];
            char* dst_pixel = &dst->color_buffer[dst_index];
            dst_pixel[0] = src_pixel[2];  /* blue */
            dst_pixel[1] = src_pixel[1];  /* green */
            dst_pixel[2] = src_pixel[0];  /* red */
        }
    }
}

static void present_surface(window_t* window)
{
    HDC window_dc = GetDC(window->handle);
    HDC memory_dc = window->memory_dc;
    image_t* surface = window->surface;
    int width = surface->width;
    int height = surface->height;
    BitBlt(window_dc, 0, 0, width, height, memory_dc, 0, 0, SRCCOPY);
    ReleaseDC(window->handle, window_dc);
}


void window_draw_buffer(window_t* window, framebuffer_t* buffer)
{
    private_blit_bgr(buffer, window->surface);
    present_surface(window);
}

window_t* window_create(const char* title, int width, int height)
{
    window_t* window;
    HWND handle;
    image_t* surface;
    HDC memory_dc;

    assert(width > 0 && height > 0);
    handle = create_window(title, width, height);
    create_surface(handle, width, height, &surface, &memory_dc);
    window = new window_t();
    window->handle = handle;
    window->memory_dc = memory_dc;
    window->surface = surface;
    SetProp(handle, WINDOW_ENTRY_NAME, window);
    ShowWindow(handle, SW_SHOW);
    return window;
}

void window_destroy(window_t* window)
{
    ShowWindow(window->handle, SW_HIDE);
    RemoveProp(window->handle, WINDOW_ENTRY_NAME);

    DeleteDC(window->memory_dc);
    DestroyWindow(window->handle);

    window->surface->color_buffer.clear();
    //maybe we need image release: image_release(window->surface);
    free(window);
}

/* misc platform functions */

static double get_native_time(void) 
{
    static double period = -1;
    LARGE_INTEGER counter;
    if (period < 0) {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        period = 1 / (double)frequency.QuadPart;
    }
    QueryPerformanceCounter(&counter);
    return counter.QuadPart * period;
}

float platform_get_time(void)
{
    static double initial = -1;
    if (initial < 0) {
        initial = get_native_time();
    }
    return (float)(get_native_time() - initial);
}

/* input related functions */

/*
 * for virtual-key codes, see
 * https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes
 */
static void handle_key_message(window_t* window, WPARAM virtual_key,
    char pressed) 
{
    std::cout<<"key message"<<std::endl;
    keycode_t key;
    switch (virtual_key) 
    {
        case 'A':      key = KEY_A;     break;
        case 'D':      key = KEY_D;     break;
        case 'S':      key = KEY_S;     break;
        case 'W':      key = KEY_W;     break;
        case VK_SPACE: key = KEY_SPACE; break;
        case VK_ESCAPE: window->should_close = 1; return;
        default:       key = KEY_NUM;   break;
    }
    if (key < KEY_NUM) 
    {
        window->keys[key] = pressed;
        if (window->callbacks.key_callback)
        {
            window->callbacks.key_callback(window, key, pressed);
        }
    }
}

static void handle_button_message(window_t* window, button_t button,
    char pressed) 
{
    window->buttons[button] = pressed;
    if (window->callbacks.button_callback)
    {
        window->callbacks.button_callback(window, button, pressed);
    }
}

static void handle_scroll_message(window_t* window, float offset) 
{
    if (window->callbacks.scroll_callback) 
    {
        window->callbacks.scroll_callback(window, offset);
    }
}

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam) 
{
    window_t* window = (window_t*)GetProp(hWnd, WINDOW_ENTRY_NAME);
    if (window == NULL) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else if (uMsg == WM_KEYDOWN) {
        handle_key_message(window, wParam, 1);
        return 0;
    }
    else if (uMsg == WM_KEYUP) {
        handle_key_message(window, wParam, 0);
        return 0;
    }
    else if (uMsg == WM_LBUTTONDOWN) {
        handle_button_message(window, BUTTON_L, 1);
        return 0;
    }
    else if (uMsg == WM_RBUTTONDOWN) {
        handle_button_message(window, BUTTON_R, 1);
        return 0;
    }
    else if (uMsg == WM_LBUTTONUP) {
        handle_button_message(window, BUTTON_L, 0);
        return 0;
    }
    else if (uMsg == WM_RBUTTONUP) {
        handle_button_message(window, BUTTON_R, 0);
        return 0;
    }
    else if (uMsg == WM_MOUSEWHEEL) {
        float offset = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        handle_scroll_message(window, offset);
        return 0;
    }
    else {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}


void input_poll_events(void) 
{
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

int input_key_pressed(window_t* window, keycode_t key) 
{
    assert(key >= 0 && key < KEY_NUM);
    return window->keys[key];
}