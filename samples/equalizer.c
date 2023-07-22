/**
   Equalizer using Fast Fourier Transform (hopefully).
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>
#include "lida_gfx.h"
#include "util.h"

uint8_t* audio_pos;
uint32_t audio_len;

void
my_audio_callback(void *userdata, Uint8 *stream, int len)
{
  if (audio_len ==0)
    return;
  printf("len=%d audio_len=%u\n", len, audio_len);

  len = ( len > audio_len ? audio_len : len );
  //SDL_memcpy (stream, audio_pos, len);                                  // simply copy from one buffer into the other
  SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);// mix from one buffer into another

  audio_pos += len;
  audio_len -= len;
}


int main(int argc, char** argv)
{
  if (argc != 2) {
    printf("usage: %s <WAV music file>\n", argv[0]);
    return -1;
  }

  int r = gfx_init(&(GFX_Init_Info) {
      .app_name = "lida_gfx_sample_equalizer",
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

  SDL_Window* handle = SDL_CreateWindow("lida_gfx sample: equalizer", SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_VULKAN);
  GFX_Window window;
  gfx_create_window_sdl(&window, handle, 1);

  // Some SDL2 code to load and play music.
  SDL_Init(SDL_INIT_AUDIO);

  uint32_t wav_length;
  uint8_t* wav_buffer;
  SDL_AudioSpec wav_spec;
  if (SDL_LoadWAV(argv[1], &wav_spec, &wav_buffer, &wav_length) == NULL) {
    printf("failed to load music file form '%s' with error '%s'\n", argv[1], SDL_GetError());
    return -2;
  }

  printf("Loaded '%s' successfully:\n", argv[1]);
  printf("  frequency: %d\n", wav_spec.freq);
  printf("  format: %x\n", wav_spec.format);
  printf("  channels: %d\n", (int)wav_spec.channels);
  printf("  samples: %d\n", (int)wav_spec.samples);
  printf("  size: %u\n", wav_spec.size);

  wav_spec.callback = my_audio_callback;
  wav_spec.userdata = NULL;
  audio_pos = wav_buffer;
  audio_len = wav_length;

  if ( SDL_OpenAudio(&wav_spec, NULL) < 0 ){
    printf("Couldn't open audio: %s\n", SDL_GetError());
    exit(-1);
  }

  SDL_PauseAudio(0);

  // Create buffers.
  GFX_Buffer vertex_buffer, sample_buffer;
  gfx_create_buffer(&vertex_buffer, GFX_BUFFER_USAGE_VERTEX|GFX_BUFFER_USAGE_STORAGE, 1024*1024);
  gfx_create_buffer(&sample_buffer, GFX_BUFFER_USAGE_STORAGE, 256*1024);
  GFX_Memory_Block buffer_memory;
  {
    GFX_Buffer buffers[2] = {vertex_buffer, sample_buffer};
    gfx_allocate_memory_for_buffers(&buffer_memory, buffers, 2,
                                    GFX_MEMORY_PROPERTY_HOST_VISIBLE|GFX_MEMORY_PROPERTY_HOST_COHERENT);
    vertex_buffer = buffers[0];
    sample_buffer = buffers[1];
  }

  // Fill the sample buffer.
  const int num_samples = 256;
  {
    float* samples = gfx_get_buffer_data(&sample_buffer);
    for (int i = 0; i < num_samples; i++) {
      const float pi = 3.14159265358979;
      float t = (float)i / num_samples;
      // samples[i] = 1.5 * sin(2*pi*2*t) + 1.6 * sin(2*pi*8*t);
      samples[i] = 1.5 * sin(2*pi*2*t) + 0.7 * sin(2*pi*8*t) + 1.2 * cos(2*pi*32.3*t);
    }
  }

  // Create pipeline for rect rendering.
  GFX_Vertex_Binding vertex_binding = {
    .stride = sizeof(float)*8,
  };
  GFX_Vertex_Attribute vertex_attributes[2] = {
    // position: vec2
    { .location = 0,
      .format = GFX_FORMAT_R32G32_SFLOAT,
      .offset = 0 },
    // color: vec3
    { .location = 1,
      .format = GFX_FORMAT_R32G32B32_SFLOAT,
      .offset = sizeof(float)*4 }
  };

  GFX_Pipeline rect_pipeline;
  gfx_create_graphics_pipelines(&rect_pipeline, 1, &(GFX_Pipeline_Desc) {
      .vertex_shader = "shaders/colored2d.vert.spv",
      .fragment_shader = "shaders/colored2d.frag.spv",
      .vertex_binding_count = 1,
      .vertex_bindings = &vertex_binding,
      .vertex_attribute_count = 2,
      .vertex_attributes = vertex_attributes,
      .render_pass = gfx_get_main_pass(&window)
    });

  GFX_Pipeline fourier_pipeline;
  {
    const char* shaders[] = { "shaders/fourier_transform.comp.spv" };
    gfx_create_compute_pipelines(&fourier_pipeline, 1, shaders);
  }

  GFX_Descriptor_Set fourier_ds;
  {
    GFX_Descriptor_Set_Binding bindings[2] = {
      { .binding = 0,
        .type = GFX_TYPE_STORAGE_BUFFER,
        .stages = GFX_STAGE_COMPUTE, },
      { .binding = 1,
        .type = GFX_TYPE_STORAGE_BUFFER,
        .stages = GFX_STAGE_COMPUTE, },
    };
    gfx_allocate_descriptor_sets(&fourier_ds, 1, bindings, 2, 0);
    gfx_descriptor_buffer(fourier_ds, 0, GFX_TYPE_STORAGE_BUFFER, &sample_buffer, 0, 0);
    gfx_descriptor_buffer(fourier_ds, 1, GFX_TYPE_STORAGE_BUFFER, &vertex_buffer, 0, 0);
  }
  gfx_batch_update_descriptor_sets();

  int running = 1;
  SDL_Event event;
  while (running) {

    while (SDL_PollEvent(&event)) {
      switch (event.type)
        {
        case SDL_QUIT:
          running = 0;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym)
            {
            case SDLK_q:
            case SDLK_ESCAPE:
              running = 0;
              break;
            }
          break;
        }
    }

    uint32_t window_width, window_height;
    if (gfx_resize_window(&window, &window_width, &window_height)) {
      // resize window
    }

    gfx_begin_commands(&window);

    struct {
      uint32_t samples;
      uint32_t freqs;
    } transform_info;
    transform_info.samples = num_samples;
    transform_info.freqs = num_samples>>1;

    gfx_bind_pipeline(&fourier_pipeline);
    gfx_bind_descriptor_sets(&fourier_ds, 1);
    gfx_push_constants(&transform_info, sizeof(transform_info));
    gfx_dispatch((transform_info.freqs+63)/64, 1, 1);

    gfx_swap_buffers(&window);

    // Main pass.
    {
      gfx_begin_main_pass(&window);

      gfx_bind_pipeline(&rect_pipeline);
      uint64_t offset = 0;
      gfx_bind_vertex_buffers(&vertex_buffer, 1, &offset);
      gfx_draw(6*transform_info.freqs, 1, 0, 0);

      gfx_end_render_pass();
    }

    gfx_submit_and_present(&window);

  }

  gfx_wait_idle_gpu();

  gfx_destroy_pipeline(&rect_pipeline);
  gfx_destroy_pipeline(&fourier_pipeline);
  gfx_destroy_buffer(&vertex_buffer);
  gfx_destroy_buffer(&sample_buffer);
  gfx_free_memory(&buffer_memory);

  gfx_destroy_window(&window);

  gfx_free();

  SDL_CloseAudio();
  SDL_FreeWAV(wav_buffer);

  return 0;
}
