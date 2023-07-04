/* lida_gfx sample: bloom.c

   This sample shows on how to use compute shaders and do some basic
   synchronization.  Starting from this sample the app window is
   resizable.

*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>
#include "lida_gfx.h"
#include "util.h"

typedef struct {
  Vec3 position;
  Vec3 normal;
} Vertex;
#include "data/teapot.h"

const GFX_Format color_format = GFX_FORMAT_R8G8B8A8_UNORM;
const GFX_Format depth_format = GFX_FORMAT_D32_SFLOAT;

static void load_mesh(GFX_Buffer* buffer);
static GFX_Render_Pass* create_offscreen_pass();
static void create_offscreen_pass_attachments(GFX_Window* window, GFX_Memory_Block* memory,
                                              GFX_Image* color_image, GFX_Image* depth_image,
                                              GFX_Texture* color_texture, GFX_Texture* depth_texture);
static void destroy_offscreen_pass_attachments(GFX_Memory_Block* memory,
                                              GFX_Image* color_image, GFX_Image* depth_image,
                                              GFX_Texture* color_texture, GFX_Texture* depth_texture);

int main(int argc, char** argv) {

  int r = gfx_init(&(GFX_Init_Info) {
      .app_name = "lida_gfx_sample_bloom",
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

  const uint32_t num_vertices = sizeof(teapot_model_indices) / sizeof(uint32_t);

  // Allocate buffers.
  GFX_Buffer vertex_buffer, uniform_buffer;
  gfx_create_buffer(&vertex_buffer, GFX_BUFFER_USAGE_VERTEX, num_vertices * sizeof(Vertex));
  gfx_create_buffer(&uniform_buffer, GFX_BUFFER_USAGE_UNIFORM, 2048);
  GFX_Memory_Block buffer_memory;
  {
    GFX_Buffer buffers[2] = { vertex_buffer, uniform_buffer };
    gfx_allocate_memory_for_buffers(&buffer_memory, buffers, 2,
                                    GFX_MEMORY_PROPERTY_HOST_VISIBLE|GFX_MEMORY_PROPERTY_HOST_COHERENT);
    vertex_buffer = buffers[0];
    uniform_buffer = buffers[1];
  }

  // Load mesh to vertex buffer.
  load_mesh(&vertex_buffer);

  SDL_Window* handle = SDL_CreateWindow("lida_gfx sample: bloom", SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_VULKAN|SDL_WINDOW_RESIZABLE);
  GFX_Window window;
  gfx_create_window_sdl(&window, handle, 1);

  GFX_Render_Pass* offscreen_pass = create_offscreen_pass();
  GFX_Memory_Block image_memory;
  GFX_Image color_image, depth_image;
  GFX_Texture color_texture, depth_texture;
  create_offscreen_pass_attachments(&window, &image_memory, &color_image, &depth_image, &color_texture, &depth_texture);

  GFX_Pipeline model_pipeline, display_pipeline;
  {
    GFX_Vertex_Binding vertex_binding = {
      .stride = sizeof(float)*6,
    };
    GFX_Vertex_Attribute vertex_attributes[2] = {
      // position: vec3
      { .location = 0,
        .format = GFX_FORMAT_R32G32B32_SFLOAT,
        .offset = 0 },
      // normal: vec3
      { .location = 1,
        .format = GFX_FORMAT_R32G32B32_SFLOAT,
        .offset = sizeof(float)*3 }
    };
    GFX_Pipeline pipelines[2];
    GFX_Pipeline_Desc descs[2] = {
      {
        .vertex_shader          = "shaders/model.vert.spv",
        .fragment_shader        = "shaders/model.frag.spv",
        .vertex_binding_count   = 1,
        .vertex_bindings        = &vertex_binding,
        .vertex_attribute_count = 2,
        .vertex_attributes      = vertex_attributes,
        .depth_test = 1,
        .depth_write = 1,
        .render_pass            = offscreen_pass,
      },
      {
        .vertex_shader          = "shaders/offscreen.vert.spv",
        .fragment_shader        = "shaders/offscreen.frag.spv",
        .render_pass            = gfx_get_main_pass(&window),
      }
    };
    gfx_create_graphics_pipelines(pipelines, 2, descs);
    model_pipeline = pipelines[0];
    display_pipeline = pipelines[1];
  }

  GFX_Descriptor_Set uniform_ds;
  gfx_allocate_descriptor_sets(&uniform_ds, 1, &(GFX_Descriptor_Set_Binding) {
      .binding = 0,
      .type = GFX_TYPE_UNIFORM_BUFFER,
      .stages = GFX_STAGE_VERTEX
    }, 1,
    0);
  GFX_Descriptor_Set offscreen_ds;
  gfx_allocate_descriptor_sets(&offscreen_ds, 1, &(GFX_Descriptor_Set_Binding) {
      .binding = 0,
      .type = GFX_TYPE_IMAGE_SAMPLER,
      .stages = GFX_STAGE_FRAGMENT
    }, 1,
    1);
  gfx_descriptor_buffer(uniform_ds, 0, GFX_TYPE_UNIFORM_BUFFER, &uniform_buffer, 0, sizeof(Mat4));
  gfx_descriptor_sampled_texture(offscreen_ds, 0, GFX_TYPE_IMAGE_SAMPLER, &color_texture, 0, GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
  gfx_batch_update_descriptor_sets();

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

    uint32_t window_width, window_height;
    if (gfx_resize_window(&window, &window_width, &window_height)) {
      destroy_offscreen_pass_attachments(&image_memory, &color_image, &depth_image, &color_texture, &depth_texture);
      create_offscreen_pass_attachments(&window, &image_memory, &color_image, &depth_image, &color_texture, &depth_texture);
    }

    static float phi = 0.0f;
    phi += 0.01f;
    Vec3 camera_pos = { 1.0f + 3.0f * sinf(phi), 0.9f + 2.0f * sinf(phi*0.5f), -0.7f + -2.7f * cosf(phi) };
    Vec3 camera_target = { 0.5f, 0.5f, 0.5f };
    Vec3 camera_up = { 0.0f, 1.0f, 0.0f };
    Mat4 view = look_at_matrix(camera_pos, camera_target, camera_up);
    Mat4 proj = perspective_matrix(radians(80.0f), (float)window_width/(float)window_height, 0.5f);
    Mat4 camera_matrix = mat4_mul(proj, view);
    gfx_copy_to_buffer(&uniform_buffer, &camera_matrix, 0, sizeof(camera_matrix));

    gfx_begin_commands(&window);

    {
      float clear_colors[][4] = {
        { 0.7f, 0.5f, 0.2f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 0.0f },
      };
      GFX_Texture textures[2] = {color_texture, depth_texture};
      gfx_begin_render_pass(offscreen_pass, textures, 2, clear_colors);

      gfx_bind_pipeline(&model_pipeline, &uniform_ds, 1);
      uint64_t offset = 0;
      gfx_bind_vertex_buffers(&vertex_buffer, 1, &offset);
      gfx_draw(num_vertices, 1, 0, 0);

      gfx_end_render_pass();
    }

    gfx_swap_buffers(&window);

    {
      gfx_begin_main_pass(&window);

      gfx_bind_pipeline(&display_pipeline, &offscreen_ds, 1);
      gfx_draw(6, 1, 0, 0);

      gfx_end_render_pass();
    }

    gfx_submit_and_present(&window);
  }

  gfx_wait_idle_gpu();

  destroy_offscreen_pass_attachments(&image_memory, &color_image, &depth_image, &color_texture, &depth_texture);

  gfx_destroy_pipeline(&display_pipeline);
  gfx_destroy_pipeline(&model_pipeline);
  gfx_destroy_buffer(&uniform_buffer);
  gfx_destroy_buffer(&vertex_buffer);
  gfx_free_memory(&buffer_memory);

  gfx_destroy_window(&window);

  gfx_free();

  return 0;
}

void
load_mesh(GFX_Buffer* buffer)
{
  Vertex* vertices = gfx_get_buffer_data(buffer);

  for (uint32_t i = 0; i < sizeof(teapot_model_indices)/sizeof(uint32_t); i++) {
    memcpy(&vertices[i], &teapot_model_vertices[teapot_model_indices[i]], sizeof(Vertex));
  }
}

GFX_Render_Pass*
create_offscreen_pass()
{
  GFX_Attachment_Info offscreen_attachments[2] = {
    {
      .format = color_format,
      .load_op = GFX_ATTACHMENT_OP_CLEAR,
      .store_op = GFX_ATTACHMENT_OP_STORE,
      .initial_layout = GFX_IMAGE_LAYOUT_UNDEFINED,
      .final_layout = GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .work_layout = GFX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    },
    {
      .format = depth_format,
      .load_op = GFX_ATTACHMENT_OP_CLEAR,
      .store_op = GFX_ATTACHMENT_OP_NONE,
      .initial_layout = GFX_IMAGE_LAYOUT_UNDEFINED,
      .final_layout = GFX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .work_layout = GFX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    }
  };
  return gfx_render_pass(offscreen_attachments, 2);
}

void
create_offscreen_pass_attachments(GFX_Window* window, GFX_Memory_Block* memory,
                                  GFX_Image* color_image, GFX_Image* depth_image,
                                  GFX_Texture* color_texture, GFX_Texture* depth_texture)
{
  uint32_t width, height;
  gfx_get_window_size(window, &width, &height);
  GFX_Image images[2];
  gfx_create_image(&images[0], GFX_IMAGE_USAGE_COLOR_ATTACHMENT|GFX_IMAGE_USAGE_SAMPLED,
                   width, height, 1, color_format, 1, 1);
  gfx_create_image(&images[1], GFX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
                   width, height, 1, depth_format, 1, 1);
  gfx_allocate_memory_for_images(memory, images, 2,
                                 GFX_MEMORY_PROPERTY_DEVICE_LOCAL);
  memcpy(color_image, &images[0], sizeof(GFX_Image));
  memcpy(depth_image, &images[1], sizeof(GFX_Image));
  gfx_create_texture(color_texture, color_image, 0, 0, 1, 1);
  gfx_create_texture(depth_texture, depth_image, 0, 0, 1, 1);
}

 void
 destroy_offscreen_pass_attachments(GFX_Memory_Block* memory,
                                    GFX_Image* color_image, GFX_Image* depth_image,
                                    GFX_Texture* color_texture, GFX_Texture* depth_texture)
{
  gfx_destroy_texture(color_texture);
  gfx_destroy_texture(depth_texture);
  gfx_destroy_image(color_image);
  gfx_destroy_image(depth_image);
  gfx_free_memory(memory);
}
