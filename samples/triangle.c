/* -*- compile-command: "./build.sh"; -*- */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>
#include "lida_gfx.h"

void log_func(int sev, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
}

int main(int argc, char** argv) {

  int r = gfx_init(&(GFX_Init_Info) {
      .app_name = "test",
      .app_version = 0,
      .enable_debug_layers = 1,
      .gpu_id = 0,
      .log_fn = log_func,
      .load_shader_fn = SDL_LoadFile,
      .free_shader_fn = SDL_free
    });
  if (r != 0) {
    printf("FATAL: error ocurred while initialising graphics module!\n");
  }

  printf("Initialised vulkan successfully!\n");

  SDL_Window* handle = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_VULKAN);
  GFX_Window window;
  gfx_create_window_sdl(&window, handle, 1);

  GFX_Pipeline triangle_pipeline;
  GFX_Pipeline_Desc desc = {
    .vertex_shader   = "shaders/triangle.vert.spv",
    .fragment_shader = "shaders/triangle.frag.spv",
    .render_pass     = gfx_get_main_pass(&window),
  };
  r = gfx_create_graphics_pipelines(&triangle_pipeline, 1, &desc);
  if (r != 0) {
    printf("FATAL: failed to create triangle pipeline\n");
  }

  int running = 1;
  SDL_Event event;
  while (running) {

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = 0;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_q)
          running = 0;
        break;
      }
    }

    gfx_begin_commands(&window);

    gfx_swap_buffers(&window);
    {
      gfx_begin_main_pass(&window);
      gfx_bind_pipeline(&triangle_pipeline);
      gfx_draw(3, 1, 0, 0);
      gfx_end_render_pass();
    }

    gfx_submit_and_present(&window);
  }

  gfx_wait_idle_gpu();

  gfx_destroy_pipeline(&triangle_pipeline);

  gfx_destroy_window(&window);

  gfx_free();
  printf("\nFreed vulkan successfully!\n");

  return 0;
}
