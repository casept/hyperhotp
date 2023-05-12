#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GLES2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gles2.h"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE  "Hyperhotp"

#define MAX_VERTEX_MEMORY  512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#include "../core/hyperhotp.h"
#include "../core/log.h"

static bool disableable_button(struct nk_context *ctx, const char *label, nk_bool enabled) {
    if (!enabled) {
        struct nk_style_button button;
        button = ctx->style.button;
        ctx->style.button.normal = nk_style_item_color(nk_rgb(40, 40, 40));
        ctx->style.button.hover = nk_style_item_color(nk_rgb(40, 40, 40));
        ctx->style.button.active = nk_style_item_color(nk_rgb(40, 40, 40));
        ctx->style.button.border_color = nk_rgb(60, 60, 60);
        ctx->style.button.text_background = nk_rgb(60, 60, 60);
        ctx->style.button.text_normal = nk_rgb(60, 60, 60);
        ctx->style.button.text_hover = nk_rgb(60, 60, 60);
        ctx->style.button.text_active = nk_rgb(60, 60, 60);
        nk_button_label(ctx, label);
        ctx->style.button = button;
        return false;
    } else {
        return nk_button_label(ctx, label);
    }
}

static char NewSerial[HYPERHOTP_SERIAL_LEN + 1] = {0};
static char NewSeed[HYPERHOTP_SEED_LEN_ASCII + 1] = {0};

static void main_loop(struct nk_context *ctx, nk_bool *running, SDL_Window **win, libusb_device_handle *handle,
                      FIDOCID cid) {
    /* Input */
    SDL_Event evt;
    nk_input_begin(ctx);
    while (SDL_PollEvent(&evt)) {
        if (evt.type == SDL_QUIT) *running = nk_false;
        nk_sdl_handle_event(&evt);
    }
    nk_input_end(ctx);

    /* GUI */
    if (nk_begin(ctx, WINDOW_TITLE, nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT), 0)) {
        // Display information about the device
        nk_layout_row_dynamic(ctx, 0, 2);
        char serial[HYPERHOTP_SERIAL_LEN] = {0};
        const int programmed = hyperhotp_check_programmed(handle, cid, serial);
        if (programmed) {
            nk_label(ctx, "Programmed: YES", NK_LEFT);
        } else {
            nk_label(ctx, "Programmed: NO", NK_LEFT);
        }
        char serial_str[HYPERHOTP_SERIAL_LEN + 16];
        memset(serial_str, '\0', HYPERHOTP_SERIAL_LEN + 16);
        snprintf(serial_str, HYPERHOTP_SERIAL_LEN + 16, "Serial: %.8s", serial);
        nk_label(ctx, serial_str, NK_LEFT);

        // Button for clearing the device
        nk_layout_row_dynamic(ctx, 0, 1);
        if (disableable_button(ctx, "Reset", programmed)) {
            if (hyperhotp_reset(handle, cid) == 0) {
                printf("Reset complete!\n");
            } else {
                char *err_str = log_get_last_error_string();
                fprintf(stderr, "Failed to reset device, error message: %s\n", err_str);
                log_free_error_string(err_str);
                exit(EXIT_FAILURE);
            }
        }

        // Button and associated input elements for programming the device
        nk_layout_row_dynamic(ctx, 0, 2);
        nk_label(ctx, "New serial:", NK_LEFT);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX | NK_EDIT_AUTO_SELECT, NewSerial, sizeof(NewSerial),
                                       nk_filter_decimal);
        nk_layout_row_dynamic(ctx, 0, 2);
        nk_label(ctx, "New hex seed:", NK_LEFT);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX | NK_EDIT_AUTO_SELECT, NewSeed, sizeof(NewSeed), nk_filter_hex);

        // TODO: Sanitize inputs
        nk_bool programming_enabled = (strnlen(NewSerial, HYPERHOTP_SERIAL_LEN) == HYPERHOTP_SERIAL_LEN) &&
                                      (strnlen(NewSeed, HYPERHOTP_SEED_LEN_ASCII) == HYPERHOTP_SEED_LEN_ASCII);
        // TODO: 8 vs 6 char code check
        nk_layout_row_dynamic(ctx, 0, 1);
        if (disableable_button(ctx, "Program", programming_enabled)) {
            const int err = hyperhotp_program(handle, cid, true, NewSerial, NewSeed);
            if (err != 0) {
                char *err_str = log_get_last_error_string();
                fprintf(stderr, "Failed to program device, error message: %s\n", err_str);
                log_free_error_string(err_str);
                exit(EXIT_FAILURE);
            } else {
                printf("Programming complete!\n");
            }
        }
    }
    nk_end(ctx);

    /* Draw */
    {
        float bg[4];
        int win_width, win_height;
        nk_color_fv(bg, nk_rgb(0xaa, 0xaa, 0xaa));
        SDL_GetWindowSize(*win, &win_width, &win_height);
        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg[0], bg[1], bg[2], bg[3]);
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        SDL_GL_SwapWindow(*win);
    }
}

int main(int argc, char *argv[]) {
    SDL_Window *win;
    SDL_GLContext glContext;

    nk_bool running = nk_true;
    struct nk_context *ctx;

    NK_UNUSED(argc);
    NK_UNUSED(argv);

    // SDL Init
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    // OpenGL init
    glContext = SDL_GL_CreateContext(win);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Nuklear init
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font *proggy = nk_font_atlas_add_default(atlas, 32.0, NULL);
    assert(proggy != NULL);
    nk_sdl_font_stash_end();
    ctx = nk_sdl_init(win);
    assert(ctx != NULL);
    nk_style_set_font(ctx, &proggy->handle);

    // USB device init
    libusb_device_handle *handle = NULL;
    FIDOCID cid;
    // TODO: Notify user graphically of error
    int err = hyperhotp_init(&handle, cid);
    if (err != 0) {
        char *msg = log_get_last_error_string();
        log_fatal(msg);
        log_free_error_string(msg);
    }

    while (running) main_loop(ctx, &running, &win, handle, cid);

    nk_font_atlas_cleanup(atlas);
    nk_sdl_shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
