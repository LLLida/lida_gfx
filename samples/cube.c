/* lida_gfx sample: cube.c

   This sample shows on how to allocate GPU resources(buffers, images)
   and work with renderpasses by rendering a rotating cube into a lowres image.

   Here what GPU does at each frame:
    1 pass: render a cube into low resolution image with depth applied.
    2 pass: read resulting image from previous pass and render it fullscreen.
*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>
#include "lida_gfx.h"
#include "util.h"

// Vertex data for our cube
static const float cube_vertex_data[] = {
  // -x               color
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
  0.0f, 0.0f, 1.0f,   0.9f, 0.8f, 0.0f,
  1.0f, 0.0f, 1.0f,   0.9f, 0.8f, 0.0f,
  1.0f, 1.0f, 1.0f,   0.9f, 0.8f, 0.0f,
  1.0f, 1.0f, 1.0f,   0.9f, 0.8f, 0.0f,
  0.0f, 1.0f, 1.0f,   0.9f, 0.8f, 0.0f,
  0.0f, 0.0f, 1.0f,   0.9f, 0.8f, 0.0f,
};

int main(int argc, char** argv) {

  int r = gfx_init(&(GFX_Init_Info) {
      .app_name = "lida_gfx_sample_cube",
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

  // Create render pass which will render into a lowres texture.
  GFX_Attachment_Info offscreen_attachments[2] = {
    // 1st attachment: color image with RGBA8 format.
    {
      .format         = GFX_FORMAT_R8G8B8A8_UNORM,
      .load_op        = GFX_ATTACHMENT_OP_CLEAR,
      .store_op       = GFX_ATTACHMENT_OP_STORE,
      .initial_layout = GFX_IMAGE_LAYOUT_UNDEFINED,
      .final_layout   = GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      .work_layout    = GFX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    },
    // 2nd attachment: depth image with D32 format.
    {
      .format         = GFX_FORMAT_D32_SFLOAT,
      .load_op        = GFX_ATTACHMENT_OP_CLEAR,
      .store_op       = GFX_ATTACHMENT_OP_NONE,
      .initial_layout = GFX_IMAGE_LAYOUT_UNDEFINED,
      .final_layout   = GFX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
      .work_layout    = GFX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    }
  };
  GFX_Render_Pass* offscreen_pass = gfx_render_pass(offscreen_attachments, 2);

  // Create images for offscreen pass. You might notice that there are
  // images and textures in this library. They're not the same. If
  // you're familiar with Vulkan then think of textures as of image
  // views. If not then consider them as slices of an image: one image
  // may have several slices for different mips, layers.
  GFX_Image color_image, depth_image;
  gfx_create_image(&color_image, GFX_IMAGE_USAGE_COLOR_ATTACHMENT|GFX_IMAGE_USAGE_SAMPLED,
                   1080/8, 720/8, 1,
                   GFX_FORMAT_R8G8B8A8_UNORM, 1, 1);
  gfx_create_image(&depth_image, GFX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT,
                   1080/8, 720/8, 1,
                   GFX_FORMAT_D32_SFLOAT, 1, 1);
  // Allocate memory for images, nothing special. Creation of images
  // doesn't allocate any GPU memory. Note that you can create
  // textures only after you've binded images to memory.
  GFX_Memory_Block image_memory;
  {
    GFX_Image images[2] = {color_image, depth_image};
    gfx_allocate_memory_for_images(&image_memory, images, 2,
                                   GFX_MEMORY_PROPERTY_DEVICE_LOCAL);
  }
  // Create textures. Notice how we specified that we'd want to only
  // touch the first mip and the first layer.
  GFX_Texture color_texture, depth_texture;
  gfx_create_texture(&color_texture, &color_image, 0, 0, 1, 1);
  gfx_create_texture(&depth_texture, &depth_image, 0, 0, 1, 1);

  // Create some buffers. NOTE: creation of buffers doesn't allocate
  // any GPU memory, memory management is done at user side. Don't
  // worry it's easier than in original Vulkan API.
  GFX_Buffer vertex_buffer, uniform_buffer;
  gfx_create_buffer(&vertex_buffer, GFX_BUFFER_USAGE_VERTEX, 2048);
  gfx_create_buffer(&uniform_buffer, GFX_BUFFER_USAGE_UNIFORM, 256);
  // Memory blocks combine several resources. You could create memory
  // block per object but it's not performant to do that.
  GFX_Memory_Block buffer_memory;
  {
    GFX_Buffer buffers[] = { vertex_buffer, uniform_buffer };
    gfx_allocate_memory_for_buffers(&buffer_memory, buffers, 2,
                                    // We'd like to write to buffers from CPU side, so we're allocating
                                    // a specific type of memory that can be accessed from CPU.
                                    GFX_MEMORY_PROPERTY_HOST_VISIBLE|GFX_MEMORY_PROPERTY_HOST_COHERENT);
    vertex_buffer = buffers[0];
    uniform_buffer = buffers[1];
  }
  // Copy vertex data to our vertex buffer. This just does a
  // memcpy to a mapped region.
  gfx_copy_to_buffer(&vertex_buffer, cube_vertex_data, 0, sizeof(cube_vertex_data));

  // It is possible that GPU may use several vertex buffers at a
  // time. So, input attributes are combined in 'bindings'. In our
  // case we have only 1 binding and 2 attributes.
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

  GFX_Pipeline pipelines[2];
  GFX_Pipeline_Desc descs[2] = {
    // pipeline which will render our cube.
    {
      .vertex_shader          = "shaders/cube.vert.spv",
      .fragment_shader        = "shaders/cube.frag.spv",
      .vertex_binding_count   = 1,
      .vertex_bindings        = &cube_vertex_binding,
      .vertex_attribute_count = 2,
      .vertex_attributes      = cube_vertex_attributes,
      .depth_test             = 1,
      .depth_write            = 1,
      .render_pass            = offscreen_pass,
    },
    // pipeline which renders an image.
    {
      .vertex_shader          = "shaders/offscreen.vert.spv",
      .fragment_shader        = "shaders/offscreen.frag.spv",
      .render_pass            = gfx_get_main_pass(&window),
    }
  };
  gfx_create_graphics_pipelines(pipelines, 2, descs);
  GFX_Pipeline cube_pipeline = pipelines[0], texture_pipeline = pipelines[1];

  // Allocate descriptor set for uniform buffer. Note how descriptor
  // set creation is done in 2 steps: allocation and update. This is
  // done for performance reasons and it's how descriptor sets are
  // done in Vulkan.
  GFX_Descriptor_Set uniform_ds;
  gfx_allocate_descriptor_sets(&uniform_ds, 1, &(GFX_Descriptor_Set_Binding) {
      .binding = 0,
      .type    = GFX_TYPE_UNIFORM_BUFFER,
      .stages  = GFX_STAGE_VERTEX
    }, 1,
    0);
  // Allocate descriptor set for offscreen texture.
  GFX_Descriptor_Set offscreen_ds;
  gfx_allocate_descriptor_sets(&offscreen_ds, 1, &(GFX_Descriptor_Set_Binding) {
      .binding = 0,
      .type    = GFX_TYPE_IMAGE_SAMPLER,
      .stages  = GFX_STAGE_FRAGMENT
    }, 1,
    0);
  // We specified buffer range as [0..sizeof(Mat4)] - that's the exact size of uniform buffer in shader.
  gfx_descriptor_buffer(uniform_ds, 0, GFX_TYPE_UNIFORM_BUFFER, &uniform_buffer, 0, sizeof(Mat4));
  // We specified is_linear=0 which means that we'd want to use nearest sampling.
  gfx_descriptor_sampled_texture(offscreen_ds, 0, GFX_TYPE_IMAGE_SAMPLER, &color_texture, 0, GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
  gfx_batch_update_descriptor_sets();

  int running = 1;
  SDL_Event event;
  // Why are you running? Why are you running?
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

    // Write the camera matrix to the uniform buffer. Nothing special,
    // this code makes camera rotate around the cube using some linear
    // algebra.
    static float phi = 0.0f;
    phi += 0.01f;
    Vec3 camera_pos    = { 1.0f + 3.0f * sinf(phi), 0.9f + 2.0f * sinf(phi*0.5f), -0.7f + -2.7f * cosf(phi) };
    Vec3 camera_target = { 0.5f, 0.5f, 0.5f };
    Vec3 camera_up     = { 0.0f, 1.0f, 0.0f };
    Mat4 view          = look_at_matrix(camera_pos, camera_target, camera_up);
    Mat4 proj          = perspective_matrix(radians(45.0f), 1080.0/720.0, 0.5f);
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

      // Bind our pipeline.
      gfx_bind_pipeline(&cube_pipeline, &uniform_ds, 1, NULL, 0);
      // It is possible that one big buffer is used for several
      // tasks. For that reason Vulkan API provides per buffer
      // offset. Vertices are read beginning from that offset.
      uint64_t offset = 0;
      gfx_bind_vertex_buffers(&vertex_buffer, 1, &offset);
      // 36 = 6 * 2 * 3. 6: number of faces in cube. 2: number of
      // triangles in face. 3: number of vertices per triangle.
      gfx_draw(36, 1, 0, 0);

      gfx_end_render_pass();
    }

    gfx_swap_buffers(&window);
    {
      gfx_begin_main_pass(&window);

      // Render the offscreen image.
      gfx_bind_pipeline(&texture_pipeline, &offscreen_ds, 1, NULL, 0);
      gfx_draw(6, 1, 0, 0);

      gfx_end_render_pass();
    }

    gfx_submit_and_present(&window);
  }

  gfx_wait_idle_gpu();

  // Don't forget to clean up!
  gfx_destroy_pipeline(&texture_pipeline);
  gfx_destroy_pipeline(&cube_pipeline);
  gfx_destroy_buffer(&uniform_buffer);
  gfx_destroy_buffer(&vertex_buffer);
  gfx_destroy_texture(&depth_texture);
  gfx_destroy_texture(&color_texture);
  gfx_destroy_image(&depth_image);
  gfx_destroy_image(&color_image);
  gfx_free_memory(&buffer_memory);
  gfx_free_memory(&image_memory);

  gfx_destroy_window(&window);

  gfx_free();
  printf("\nFreed vulkan successfully!\n");

  return 0;
}
