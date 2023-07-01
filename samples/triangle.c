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

  // Initialise the library.
  int r = gfx_init(&(GFX_Init_Info) {
      .app_name = "test",
      .app_version = 0,
      // This loads the standard Khronos validation layer.
      .enable_debug_layers = 1,
      .gpu_id = 0,
      // Provide a function to log errors.
      .log_fn = log_func,
      // Provide functions for loading shaders. Here we're using SDL2
      // functions for file manipulations. However, it's not necessary
      // to load shaders from files. The 'load_shader_fn' just must
      // return a SPIR-V blob and the 'free_shader_fn' must free that
      // blob.
      .load_shader_fn = SDL_LoadFile,
      .free_shader_fn = SDL_free
    });
  if (r != 0) {
    printf("FATAL: error ocurred while initialising graphics module!\n");
  }

  printf("Initialised vulkan successfully!\n");

  // Create window.
  SDL_Window* handle = SDL_CreateWindow("window", SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_VULKAN);
  GFX_Window window;
  gfx_create_window_sdl(&window, handle, 1);

  // Create pipeline. This pipeline will render a triangle.
  GFX_Pipeline triangle_pipeline;
  GFX_Pipeline_Desc desc = {
    .vertex_shader   = "shaders/triangle.vert.spv",
    .fragment_shader = "shaders/triangle.frag.spv",
    .render_pass     = gfx_get_main_pass(&window),
  };
  // Note how this function accepts array of pipeline descs. It can
  // create multiple pipelines at a time for performance.
  r = gfx_create_graphics_pipelines(&triangle_pipeline, 1, &desc);
  if (r != 0) {
    printf("FATAL: failed to create triangle pipeline\n");
  }

  // main loop
  int running = 1;
  SDL_Event event;
  while (running) {

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = 0;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE)
          running = 0;
        break;
      }
    }

    // Start recording commands. Note that we can start recording
    // commands before acquiring next swapchain image. This is
    // basically double buffering as it is.
    gfx_begin_commands(&window);

    // Swap buffers and acquire next swapchain image. From this point
    // you can record commands for rendering to swapchain image which
    // will be shown on screen.
    gfx_swap_buffers(&window);
    {
      // Start render pass which will render to the swapchain image.
      gfx_begin_main_pass(&window);

      // Bind the triangle pipeline.
      gfx_bind_pipeline(&triangle_pipeline);
      // Draw our triangle. If you're questioning why we don't bind
      // any vertex buffer then I'll answer you: all 3 vertices are
      // inside the shader. Take a look at shaders/triangle.vert.
      gfx_draw(3, 1, 0, 0);

      // End the render pass.
      gfx_end_render_pass();
    }

    // Send recorded commands to GPU. After those commands are done
    // current swapchain image will be presented on screen.
    gfx_submit_and_present(&window);
  }

  // GPU might not be done with executing commands. Before destroying
  // GPU resources we need before GPU is done with execution.
  gfx_wait_idle_gpu();

  gfx_destroy_pipeline(&triangle_pipeline);

  gfx_destroy_window(&window);

  gfx_free();
  printf("\nFreed vulkan successfully!\n");

  return 0;
}
