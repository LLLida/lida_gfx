/* lida_gfx sample: cube.c

   This sample shows on how to allocate GPU resources and work with
   renderpasses by rendering a rotating cube.
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>
#include "lida_gfx.h"
#include "util.h"

static void log_func(int sev, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
}

static const float cube_vertex_data[] = {
  // -x
  0.0f, 1.0f, 1.0f,   0.8f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f,   0.8f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f,   0.8f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f,   0.8f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f,   0.8f, 0.0f, 0.0f,
  0.0f, 1.0f, 1.0f,   0.8f, 0.0f, 0.0f,
  // +x
  1.0f, 1.0f, 1.0f,   0.0f, 0.8f, 0.0f,
  1.0f, 0.0f, 0.0f,   0.0f, 0.8f, 0.0f,
  1.0f, 1.0f, 0.0f,   0.0f, 0.8f, 0.0f,
  1.0f, 0.0f, 0.0f,   0.0f, 0.8f, 0.0f,
  1.0f, 1.0f, 1.0f,   0.0f, 0.8f, 0.0f,
  1.0f, 0.0f, 1.0f,   0.0f, 0.8f, 0.0f,
  // -y
  0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.8f,
  1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.8f,
  1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.8f,
  1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.8f,
  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 0.8f,
  0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.8f,
  // +y
  0.0f, 1.0f, 0.0f,   0.8f, 0.0f, 0.8f,
  1.0f, 1.0f, 1.0f,   0.8f, 0.0f, 0.8f,
  1.0f, 1.0f, 0.0f,   0.8f, 0.0f, 0.8f,
  1.0f, 1.0f, 1.0f,   0.8f, 0.0f, 0.8f,
  0.0f, 1.0f, 0.0f,   0.8f, 0.0f, 0.8f,
  0.0f, 1.0f, 1.0f,   0.8f, 0.0f, 0.8f,
  // -z
  0.0f, 0.0f, 0.0f,   0.0f, 0.8f, 0.8f,
  1.0f, 1.0f, 0.0f,   0.0f, 0.8f, 0.8f,
  1.0f, 0.0f, 0.0f,   0.0f, 0.8f, 0.8f,
  1.0f, 1.0f, 0.0f,   0.0f, 0.8f, 0.8f,
  0.0f, 0.0f, 0.0f,   0.0f, 0.8f, 0.8f,
  0.0f, 1.0f, 0.0f,   0.0f, 0.8f, 0.8f,
  // +z
  0.0f, 0.0f, 1.0f,   0.3f, 0.3f, 0.3f,
  1.0f, 0.0f, 1.0f,   0.3f, 0.3f, 0.3f,
  1.0f, 1.0f, 1.0f,   0.3f, 0.3f, 0.3f,
  1.0f, 1.0f, 1.0f,   0.3f, 0.3f, 0.3f,
  0.0f, 1.0f, 1.0f,   0.3f, 0.3f, 0.3f,
  0.0f, 0.0f, 1.0f,   0.3f, 0.3f, 0.3f,
};

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
    return -1;
  }

  printf("Initialised vulkan successfully!\n");

  SDL_Window* handle = SDL_CreateWindow("lida_gfx sample: cube", SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_VULKAN);
  GFX_Window window;
  gfx_create_window_sdl(&window, handle, 1);

  GFX_Render_Pass* offscreen_pass = gfx_render_pass(&(GFX_Attachment_Info) {
      .format = GFX_FORMAT_R8G8B8A8_UNORM,
      .load_op = GFX_ATTACHMENT_OP_CLEAR,
      .store_op = GFX_ATTACHMENT_OP_STORE,
      .initial_layout = GFX_IMAGE_LAYOUT_UNDEFINED,
      .final_layout = GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .work_layout = GFX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    }, 1);

  GFX_Memory_Block vertex_memory;
  GFX_Buffer vertex_buffer, uniform_buffer;
  gfx_create_buffer(&vertex_buffer, GFX_BUFFER_USAGE_VERTEX, 2048);
  gfx_create_buffer(&uniform_buffer, GFX_BUFFER_USAGE_UNIFORM, 256);
  {
    GFX_Buffer buffers[] = { vertex_buffer, uniform_buffer };
    gfx_allocate_memory_for_buffers(&vertex_memory, buffers, 2,
                                    GFX_MEMORY_PROPERTY_HOST_VISIBLE|GFX_MEMORY_PROPERTY_HOST_COHERENT);
    vertex_buffer = buffers[0];
    uniform_buffer = buffers[1];
  }
  // Copy vertex data to our vertex buffer. This just does a
  // memcpy to a mapped region.
  gfx_copy_to_buffer(&vertex_buffer, cube_vertex_data, 0, sizeof(cube_vertex_data));

  GFX_Vertex_Binding cube_vertex_binding = {
    .stride = sizeof(float)*6,
  };
  GFX_Vertex_Attribute cube_vertex_attributes[2] = {
    // position: vec3
    { .location = 0,
      .format = GFX_FORMAT_R32G32B32_SFLOAT,
      .offset = 0 },
    // color: vec3
    { .location = 1,
      .format = GFX_FORMAT_R32G32B32_SFLOAT,
      .offset = sizeof(float)*3 }
  };
  GFX_Pipeline cube_pipeline;
  gfx_create_graphics_pipelines(&cube_pipeline, 1, &(GFX_Pipeline_Desc) {
      .vertex_shader          = "shaders/cube.vert.spv",
      .fragment_shader        = "shaders/cube.frag.spv",
      .vertex_binding_count   = 1,
      .vertex_bindings        = &cube_vertex_binding,
      .vertex_attribute_count = 2,
      .vertex_attributes      = cube_vertex_attributes,
      // .render_pass            = offscreen_pass,
      .render_pass            = gfx_get_main_pass(&window),
    });

  // allocate descriptor set for uniform buffer
  GFX_Descriptor_Set uniform_ds;
  gfx_allocate_descriptor_sets(&uniform_ds, 1, &(GFX_Descriptor_Set_Binding) {
      .binding = 0,
      .type = GFX_TYPE_UNIFORM_BUFFER,
      .stages = GFX_STAGE_VERTEX
    }, 1,
    0);
  gfx_descriptor_buffer(uniform_ds, 0, GFX_TYPE_UNIFORM_BUFFER, &uniform_buffer, 0, sizeof(Mat4));
  gfx_batch_update_descriptor_sets();

  int i = 0;

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

    // write the camera matrix to the uniform buffer
#if 1
    float phi = i++ * 0.01f;
    Vec3 camera_pos = { 1.0f * sinf(phi), 0.0f, -2.0f * cosf(phi) };
    Vec3 camera_target = { 0.5f, 0.5f, 0.5f };
    Vec3 camera_up = { 0.0f, 1.0f, 0.0f };
    Mat4 view = look_at_matrix(camera_pos, camera_target, camera_up);
    Mat4 proj = perspective_matrix(radians(100.0f), 1080.0/720.0, 0.5f);
    Mat4 camera_matrix = mat4_mul(proj, view);
#else
    Mat4 camera_matrix = {
      0.5, 0.1, 0.0, 0.0,
      0.0, 0.5, 0.1, -0.3,
      0.1, 0.0, 0.5, 0.0,
      0.0, 0.0, 0.0, 1.0,
    };
#endif
    gfx_copy_to_buffer(&uniform_buffer, &camera_matrix, 0, sizeof(camera_matrix));

    gfx_begin_commands(&window);

    gfx_swap_buffers(&window);
    {
      gfx_begin_main_pass(&window);

      gfx_bind_pipeline(&cube_pipeline, &uniform_ds, 1);
      uint64_t offset = 0;
      gfx_bind_vertex_buffers(&vertex_buffer, 1, &offset);
      gfx_draw(36, 1, 0, 0);

      gfx_end_render_pass();
    }

    gfx_submit_and_present(&window);
  }

  gfx_wait_idle_gpu();

  // Don't forget to clean up!
  gfx_destroy_pipeline(&cube_pipeline);
  gfx_destroy_buffer(&uniform_buffer);
  gfx_destroy_buffer(&vertex_buffer);
  gfx_free_memory(&vertex_memory);

  gfx_destroy_window(&window);

  gfx_free();
  printf("\nFreed vulkan successfully!\n");

  return 0;
}
