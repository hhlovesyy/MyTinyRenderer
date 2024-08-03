#include "win32.h"

static LRESULT CALLBACK process_message(HWND hWnd, UINT uMsg,
    WPARAM wParam, LPARAM lParam)
{
    window_t* window = (window_t*)GetProp(hWnd, WINDOW_ENTRY_NAME);
    if (window == NULL)
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else
    {
        return 0;
    }
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
            unsigned char* src_pixel = &src->color_buffer[src_index];
            unsigned char* dst_pixel = &dst->color_buffer[dst_index];
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

    window->surface->color_buffer = NULL;
    //maybe we need image release: image_release(window->surface);
    free(window);
}