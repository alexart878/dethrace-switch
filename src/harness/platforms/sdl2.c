#include <SDL.h>

#ifdef __SWITCH__
#include <switch.h>
#endif

#include "harness.h"
#include "harness/config.h"
#include "harness/hooks.h"
#include "harness/trace.h"
#include "sdl2_scancode_to_dinput.h"

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* screen_texture;
uint32_t converted_palette[256];
br_pixelmap* last_screen_src;

SDL_GLContext* gl_context;

int render_width, render_height;

Uint32 last_frame_time;

uint8_t directinput_key_state[SDL_NUM_SCANCODES];

struct {
    int x, y;
    float scale_x, scale_y;
} viewport;

// Callbacks back into original game code
extern void QuitGame(void);
extern uint32_t gKeyboard_bits[8];
extern br_pixelmap* gBack_screen;

#ifdef __SWITCH__

PadState pad;

uint32_t switch_buttons[] = {
	HidNpadButton_A,
    HidNpadButton_B,
    HidNpadButton_X,
    HidNpadButton_Y,
    HidNpadButton_StickL,
    HidNpadButton_StickR,
    HidNpadButton_L,
    HidNpadButton_R,
    HidNpadButton_ZL,
    HidNpadButton_ZR,
    HidNpadButton_Plus,
    HidNpadButton_Minus,
    HidNpadButton_Left,
    HidNpadButton_Up,
    HidNpadButton_Right,
    HidNpadButton_Down,
};

static int map_switch_key_to_sdl_scancode(uint32_t key) 
{
    switch (key) 
    {
        case HidNpadButton_A:       return SDL_SCANCODE_SPACE;
        case HidNpadButton_B:       return SDL_SCANCODE_B;
        case HidNpadButton_X:       return SDL_SCANCODE_X;
        case HidNpadButton_Y:       return SDL_SCANCODE_Y;
        case HidNpadButton_StickL:  return -1;
        case HidNpadButton_StickR:  return -1;
        case HidNpadButton_L:       return SDL_SCANCODE_L;
        case HidNpadButton_R:       return SDL_SCANCODE_R;
        case HidNpadButton_ZL:      return SDL_SCANCODE_KP_2;
        case HidNpadButton_ZR:      return SDL_SCANCODE_KP_8;
        case HidNpadButton_Plus:    return SDL_SCANCODE_RETURN;
        case HidNpadButton_Minus:   return SDL_SCANCODE_ESCAPE;
        case HidNpadButton_Left:    return SDL_SCANCODE_MINUS;
        case HidNpadButton_Up:      return SDL_SCANCODE_KP_8;
        case HidNpadButton_Right:   return SDL_SCANCODE_EQUALS;
        case HidNpadButton_Down:    return SDL_SCANCODE_KP_2;

        default: return -1;
    }
}

void process_switch_input(int* dinput_key)
{
    padUpdate(&pad);

    u64 kDown = padGetButtonsDown(&pad);
    u64 kUp = padGetButtonsUp(&pad);

    for (int i = 0; i < 16; ++i)
    {
        if (kDown & BIT(i)) 
        {
            int sdl_scancode = map_switch_key_to_sdl_scancode(switch_buttons[i]);
            
            if (sdl_scancode >= 0) 
            {
                *dinput_key = sdlScanCodeToDirectInputKeyNum[sdl_scancode];
                
                if (*dinput_key == 0) 
                {
                    LOG_WARN("unexpected scan code %s (%d)", SDL_GetScancodeName(sdl_scancode), sdl_scancode);
                    continue;
                }
                
                directinput_key_state[*dinput_key] = 0x80;
                gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
            }
        } 
        else if (kUp & BIT(i))
        {
            int sdl_scancode = map_switch_key_to_sdl_scancode(switch_buttons[i]);
            
            if (sdl_scancode >= 0) 
            {
                *dinput_key = sdlScanCodeToDirectInputKeyNum[sdl_scancode];
                
                if (*dinput_key != 0)
                {
                    directinput_key_state[*dinput_key] = 0x00;
                    gKeyboard_bits[*dinput_key >> 5] &= ~(1 << (*dinput_key & 0x1F));
                }
            }
        }
    }

    HidAnalogStickState analog_stick_l = padGetStickPos(&pad, 0);
    HidAnalogStickState analog_stick_r = padGetStickPos(&pad, 1);

    // Left stick X

    if (analog_stick_l.x > 0x3000)
    {
        // Right 
        *dinput_key = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_KP_6];
        directinput_key_state[*dinput_key] = 0x80;
        gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
    }
    else if (analog_stick_l.x < -0x3000)
    {
        // Left
        *dinput_key = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_KP_4];
        directinput_key_state[*dinput_key] = 0x80;
        gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
    }
    else
    {
        // Neutral
        
        int dinput_key_kp_4 = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_KP_4];
        int dinput_key_kp_6 = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_KP_6];

        directinput_key_state[dinput_key_kp_4] = 0x00;
        directinput_key_state[dinput_key_kp_6] = 0x00;

        gKeyboard_bits[dinput_key_kp_4 >> 5] &= ~(1 << (dinput_key_kp_4 & 0x1F));
        gKeyboard_bits[dinput_key_kp_6 >> 5] &= ~(1 << (dinput_key_kp_6 & 0x1F));
    }

    // Right stick X

    if (analog_stick_r.x > 0x3000) 
    {
        // Right
        *dinput_key = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_RIGHT];
        directinput_key_state[*dinput_key] = 0x80;
        gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
    } 
    else if (analog_stick_r.x < -0x3000) 
    {
        // Left
        *dinput_key = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_LEFT];
        directinput_key_state[*dinput_key] = 0x80;
        gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
    } 
    else 
    {
        // Neutral

        int dinput_key_right = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_RIGHT];
        int dinput_key_left = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_LEFT];

        directinput_key_state[dinput_key_right] = 0x00;
        directinput_key_state[dinput_key_left] = 0x00;

        gKeyboard_bits[dinput_key_right >> 5] &= ~(1 << (dinput_key_right & 0x1F));
        gKeyboard_bits[dinput_key_left >> 5] &= ~(1 << (dinput_key_left & 0x1F));
    }

    // Right stick Y

    if (analog_stick_r.y > 0x3000) 
    {
        // Up
        *dinput_key = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_UP];
        directinput_key_state[*dinput_key] = 0x80;
        gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
    } 
    else if (analog_stick_r.y < -0x3000) 
    {
        // Down
        *dinput_key = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_DOWN];
        directinput_key_state[*dinput_key] = 0x80;
        gKeyboard_bits[*dinput_key >> 5] |= (1 << (*dinput_key & 0x1F));
    } 
    else 
    {
        // Neutral

        int dinput_key_up = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_UP];
        int dinput_key_down = sdlScanCodeToDirectInputKeyNum[SDL_SCANCODE_DOWN];

        directinput_key_state[dinput_key_up] = 0x00;
        directinput_key_state[dinput_key_down] = 0x00;
        
        gKeyboard_bits[dinput_key_up >> 5] &= ~(1 << (dinput_key_up & 0x1F));
        gKeyboard_bits[dinput_key_down >> 5] &= ~(1 << (dinput_key_down & 0x1F));
    }
}

#endif

static void calculate_viewport(int window_width, int window_height) {
    int vp_width, vp_height;
    float target_aspect_ratio;
    float aspect_ratio;

    aspect_ratio = (float)window_width / window_height;
    target_aspect_ratio = (float)gBack_screen->width / gBack_screen->height;

    vp_width = window_width;
    vp_height = window_height;
    if (aspect_ratio != target_aspect_ratio) {
        if (aspect_ratio > target_aspect_ratio) {
            vp_width = window_height * target_aspect_ratio + .5f;
        } else {
            vp_height = window_width / target_aspect_ratio + .5f;
        }
    }
    viewport.x = (window_width - vp_width) / 2;
    viewport.y = (window_height - vp_height) / 2;
    viewport.scale_x = (float)vp_width / gBack_screen->width;
    viewport.scale_y = (float)vp_height / gBack_screen->height;
}

static int set_window_pos(void* hWnd, int x, int y, int nWidth, int nHeight) {
    // SDL_SetWindowPosition(hWnd, x, y);
    if (nWidth == 320 && nHeight == 200) {
        nWidth = 640;
        nHeight = 400;
    }
    SDL_SetWindowSize(hWnd, nWidth, nHeight);
    return 0;
}

static void destroy_window(void* hWnd) {
    // SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    window = NULL;
}

// Checks whether the `flag_check` is the only modifier applied.
// e.g. is_only_modifier(event.key.keysym.mod, KMOD_ALT) returns true when only the ALT key was pressed
static int is_only_key_modifier(int modifier_flags, int flag_check) {
    return (modifier_flags & flag_check) && (modifier_flags & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI)) == (modifier_flags & flag_check);
}

static void get_and_handle_message(MSG_* msg) {
    SDL_Event event;
    int dinput_key;

#ifdef __SWITCH__

    process_switch_input(&dinput_key);

#endif

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        
#ifndef __SWITCH__
        
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (event.key.windowID != SDL_GetWindowID(window)) {
                continue;
            }
            if (event.key.keysym.sym == SDLK_RETURN) {
                if (event.key.type == SDL_KEYDOWN) {
                    if ((event.key.keysym.mod & (KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI))) {
                        // Ignore keydown of RETURN when used together with some modifier
                        return;
                    }
                } else if (event.key.type == SDL_KEYUP) {
                    if (is_only_key_modifier(event.key.keysym.mod, KMOD_ALT)) {
                        SDL_SetWindowFullscreen(window, (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                }
            }

            // Map incoming SDL scancode to DirectInput DIK_* key code.
            // https://github.com/DanielGibson/Snippets/blob/master/sdl2_scancode_to_dinput.h
            dinput_key = sdlScanCodeToDirectInputKeyNum[event.key.keysym.scancode];
            if (dinput_key == 0) {
                LOG_WARN("unexpected scan code %s (%d)", SDL_GetScancodeName(event.key.keysym.scancode), event.key.keysym.scancode);
                return;
            }
            // DInput expects high bit to be set if key is down
            // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/ee418261(v=vs.85)
            directinput_key_state[dinput_key] = (event.type == SDL_KEYDOWN ? 0x80 : 0);
            if (event.type == SDL_KEYDOWN) {
                gKeyboard_bits[dinput_key >> 5] |= (1 << (dinput_key & 0x1F));
            } else {
                gKeyboard_bits[dinput_key >> 5] &= ~(1 << (dinput_key & 0x1F));
            }
            break;

#endif

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                calculate_viewport(event.window.data1, event.window.data2);
            }
            break;

        case SDL_QUIT:
            QuitGame();
        }
    }
}

static void get_keyboard_state(unsigned int count, uint8_t* buffer) {
    memcpy(buffer, directinput_key_state, count);
}

static int get_mouse_buttons(int* pButton1, int* pButton2) {
    if (SDL_GetMouseFocus() != window) {
        *pButton1 = 0;
        *pButton2 = 0;
        return 0;
    }
    int state = SDL_GetMouseState(NULL, NULL);
    *pButton1 = state & SDL_BUTTON_LMASK;
    *pButton2 = state & SDL_BUTTON_RMASK;
    return 0;
}

static int get_mouse_position(int* pX, int* pY) {
    int window_width, window_height;
    float lX, lY;

    if (SDL_GetMouseFocus() != window) {
        return 0;
    }
    SDL_GetWindowSize(window, &window_width, &window_height);

    SDL_GetMouseState(pX, pY);
    if (renderer != NULL) {
        // software renderer
        SDL_RenderWindowToLogical(renderer, *pX, *pY, &lX, &lY);
    } else {
        // hardware renderer
        // handle case where window is stretched larger than the pixel size
        lX = *pX * (640.0f / window_width);
        lY = *pY * (480.0f / window_height);
    }
    *pX = (int)lX;
    *pY = (int)lY;
    return 0;
}

static void limit_fps(void) {
    Uint32 now = SDL_GetTicks();
    if (last_frame_time != 0) {
        unsigned int frame_time = now - last_frame_time;
        last_frame_time = now;
        if (frame_time < 100) {
            int sleep_time = (1000 / harness_game_config.fps) - frame_time;
            if (sleep_time > 5) {
                gHarness_platform.Sleep(sleep_time);
            }
        }
    }
    last_frame_time = SDL_GetTicks();
}

int show_error_message(void* window, char* text, char* caption) {
    fprintf(stderr, "%s", text);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, caption, text, window);
    return 0;
}

static void create_window(char* title, int width, int height, tHarness_window_type window_type) {
    
#ifdef __SWITCH__

    padInitializeDefault(&pad);

#endif
    
    int window_width, window_height;

    render_width = width;
    render_height = height;

    window_width = width;
    window_height = height;

    // special case lores and make a bigger window
    if (width == 320 && height == 200) {
        window_width = 640;
        window_height = 480;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_PANIC("SDL_INIT_VIDEO error: %s", SDL_GetError());
    }

    if (window_type == eWindow_type_opengl) {

#ifndef __SWITCH__
    
        window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width, window_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

#else

        window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width, window_height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);

#endif

        if (window == NULL) {
            LOG_PANIC("Failed to create window: %s", SDL_GetError());
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        gl_context = SDL_GL_CreateContext(window);

        if (gl_context == NULL) {
            LOG_WARN("Failed to create OpenGL core profile: %s. Trying OpenGLES...", SDL_GetError());
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            gl_context = SDL_GL_CreateContext(window);
        }
        if (gl_context == NULL) {
            LOG_PANIC("Failed to create OpenGL context: %s", SDL_GetError());
        }
        SDL_GL_SetSwapInterval(1);

    } else {

#ifndef __SWITCH__

        window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width, window_height,
            SDL_WINDOW_RESIZABLE);

#else

        window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width, window_height,
            SDL_WINDOW_FULLSCREEN);

#endif

        if (window == NULL) {
            LOG_PANIC("Failed to create window: %s", SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
        if (renderer == NULL) {
            LOG_PANIC("Failed to create renderer: %s", SDL_GetError());
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderSetLogicalSize(renderer, render_width, render_height);

        screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (screen_texture == NULL) {
            SDL_RendererInfo info;
            SDL_GetRendererInfo(renderer, &info);
            for (Uint32 i = 0; i < info.num_texture_formats; i++) {
                LOG_INFO("%s\n", SDL_GetPixelFormatName(info.texture_formats[i]));
            }
            LOG_PANIC("Failed to create screen_texture: %s", SDL_GetError());
        }
    }

    SDL_ShowCursor(SDL_DISABLE);

    viewport.x = 0;
    viewport.y = 0;
    viewport.scale_x = 1;
    viewport.scale_y = 1;

    if (harness_game_config.start_full_screen) {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

static void swap(br_pixelmap* back_buffer) {
    uint8_t* src_pixels = back_buffer->pixels;
    uint32_t* dest_pixels;
    int dest_pitch;

    get_and_handle_message(NULL);

    if (gl_context != NULL) {
        SDL_GL_SwapWindow(window);
    } else {
        SDL_LockTexture(screen_texture, NULL, (void**)&dest_pixels, &dest_pitch);
        for (int i = 0; i < back_buffer->height * back_buffer->width; i++) {
            *dest_pixels = converted_palette[*src_pixels];
            dest_pixels++;
            src_pixels++;
        }
        SDL_UnlockTexture(screen_texture);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        last_screen_src = back_buffer;
    }

    if (harness_game_config.fps != 0) {
        limit_fps();
    }
}

static void palette_changed(br_colour entries[256]) {
    for (int i = 0; i < 256; i++) {
        converted_palette[i] = (0xff << 24 | BR_RED(entries[i]) << 16 | BR_GRN(entries[i]) << 8 | BR_BLU(entries[i]));
    }
    if (last_screen_src != NULL) {
        swap(last_screen_src);
    }
}

static void get_viewport(int* x, int* y, float* width_multipler, float* height_multiplier) {
    *x = viewport.x;
    *y = viewport.y;
    *width_multipler = viewport.scale_x;
    *height_multiplier = viewport.scale_y;
}

void Harness_Platform_Init(tHarness_platform* platform) {
    platform->ProcessWindowMessages = get_and_handle_message;
    platform->Sleep = SDL_Delay;
    platform->GetTicks = SDL_GetTicks;
    platform->ShowCursor = SDL_ShowCursor;
    platform->SetWindowPos = set_window_pos;
    platform->DestroyWindow = destroy_window;
    platform->GetKeyboardState = get_keyboard_state;
    platform->GetMousePosition = get_mouse_position;
    platform->GetMouseButtons = get_mouse_buttons;
    platform->ShowErrorMessage = show_error_message;

    platform->CreateWindow_ = create_window;
    platform->Swap = swap;
    platform->PaletteChanged = palette_changed;
    platform->GL_GetProcAddress = SDL_GL_GetProcAddress;
    platform->GetViewport = get_viewport;
}
