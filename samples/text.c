/**
   A simple text editor.

   This sample demonstrates how to efficiently render text with the
   library.

*/
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>
#include "lida_gfx.h"
#include "util.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "data/font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

typedef struct {
  Vec2 pos;
  Vec2 uv;
  uint32_t color;
} Vertex;

typedef struct {
  Vec2 advance;
  Vec2 bearing;
  Vec2 size;
  Vec2 uv_offset;
  Vec2 uv_size;
} Glyph_Info;

struct {
  GFX_Memory_Block gpu_memory;
  GFX_Memory_Block cpu_memory;
  GFX_Image font_atlas;
  GFX_Texture atlas_texture;
  GFX_Buffer vertex_buffer;
  GFX_Buffer index_buffer;
  GFX_Descriptor_Set atlas_ds;
  Glyph_Info glyphs[128];
  uint32_t pixel_size;
  uint32_t num_vertices;
  uint32_t num_indices;
} text;

void init_text();
void free_text();
void upload_font(FT_Face face);
void draw_str(const char* text, uint32_t num_chars, Vec2 pos, uint32_t color, uint32_t window_width, uint32_t window_height);

int main(int argc, char** argv)
{
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
    return -1;
  }

  SDL_Window* handle = SDL_CreateWindow("lida_gfx sample: equalizer", SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED|SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_VULKAN|SDL_WINDOW_RESIZABLE);
  GFX_Window window;
  gfx_create_window_sdl(&window, handle, 0);

  init_text();

  FT_Library freetype;
  FT_Error err = FT_Init_FreeType(&freetype);
  if (err != 0) {
    printf("failed to initialize the freetype library: %s", FT_Error_String(err));
    return -2;
  }

  FT_Face face;
  err = FT_New_Memory_Face(freetype, pixel_font, sizeof(pixel_font), 0, &face);
  if (err != 0) {
    printf("failed to create face from font: %s", FT_Error_String(err));
    return -3;
  }

  GFX_Vertex_Binding vertex_binding = {
    .stride = sizeof(Vertex),
  };
  GFX_Vertex_Attribute vertex_attributes[3] = {
    // position: vec2
    { .location = 0,
      .format   = GFX_FORMAT_R32G32_SFLOAT,
      .offset   = offsetof(Vertex, pos) },
    // uv: vec2
    { .location = 1,
      .format   = GFX_FORMAT_R32G32_SFLOAT,
      .offset   = offsetof(Vertex, uv) },
    // color: uint32_t
    { .location = 2,
      .format   = GFX_FORMAT_R32_UINT,
      .offset   = offsetof(Vertex, color) }
  };
  GFX_Pipeline text_pipeline;
  gfx_create_graphics_pipelines(&text_pipeline, 1, &(GFX_Pipeline_Desc) {
      .vertex_shader          = "shaders/text.vert.spv",
      .fragment_shader        = "shaders/text.frag.spv",
      .vertex_binding_count   = 1,
      .vertex_bindings        = &vertex_binding,
      .vertex_attribute_count = 3,
      .vertex_attributes      = vertex_attributes,
      .render_pass            = gfx_get_main_pass(&window)
    });

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

    static int font_is_ready = 0;

    uint32_t window_width, window_height;
    if (gfx_resize_window(&window, &window_width, &window_height)) {
      // resize window
    }

    gfx_begin_commands(&window);

    int draw_text = font_is_ready;

    if (!font_is_ready) {
      upload_font(face);
      font_is_ready = 1;
    }

    gfx_swap_buffers(&window);

    if (draw_text) {
#if 1
      text.num_vertices = 0;
      text.num_indices = 0;
      const char* str = "Vulkan is not hard";
      draw_str(str, strlen(str), (Vec2) { 0.2, 0.3 }, 0xffffffff, window_width, window_height);
#else
      text.num_vertices = 0;
      text.num_indices = 0;
      const char* str = "A";
      draw_str(str, strlen(str), (Vec2) { 0.2, 0.3 }, 0xffffffff, window_width, window_height);
#endif
    }

    {
      gfx_begin_main_pass(&window);

      GFX_Clear_Color clear_color = { 0.0745f, 0.0745f, 0.0745f, 0.0f };
      gfx_clear_attachment(&clear_color, 0, 0, window_width, window_height);

      if (draw_text) {
        gfx_bind_pipeline(&text_pipeline);
        gfx_bind_descriptor_sets(&text.atlas_ds, 1);
        uint64_t offset = 0;
        gfx_bind_index_buffer(&text.index_buffer, offset);
        gfx_bind_vertex_buffers(&text.vertex_buffer, 1, &offset);
        gfx_draw_indexed(text.num_indices, 1, 0, 0, 0);
      }

      gfx_end_render_pass();
    }

    gfx_submit_and_present(&window);

  }

  FT_Done_Face(face);
  FT_Done_FreeType(freetype);

  gfx_wait_idle_gpu();

  gfx_destroy_pipeline(&text_pipeline);
  free_text();
  gfx_destroy_window(&window);

  gfx_free();

  return 0;
}

void
init_text()
{
  gfx_create_image(&text.font_atlas, GFX_IMAGE_USAGE_SAMPLED|GFX_IMAGE_USAGE_TRANSFER_DST,
                   512, 256, 1, GFX_FORMAT_R8G8B8A8_UNORM, 1, 1);
  gfx_allocate_memory_for_images(&text.gpu_memory, &text.font_atlas, 1, GFX_MEMORY_PROPERTY_DEVICE_LOCAL);
  gfx_create_texture(&text.atlas_texture, &text.font_atlas, 0, 0, 1, 1);

  const int max_glyphs = 16*1024;
  GFX_Buffer buffers[2];
  gfx_create_buffer(&buffers[0], GFX_BUFFER_USAGE_VERTEX|GFX_BUFFER_USAGE_TRANSFER_SRC, max_glyphs * 4 * sizeof(Vertex));
  gfx_create_buffer(&buffers[1], GFX_BUFFER_USAGE_INDEX,  max_glyphs * 6 * sizeof(uint32_t));
  gfx_allocate_memory_for_buffers(&text.cpu_memory, buffers, 2,
                                  GFX_MEMORY_PROPERTY_HOST_VISIBLE|GFX_MEMORY_PROPERTY_HOST_COHERENT);
  text.vertex_buffer = buffers[0];
  text.index_buffer  = buffers[1];

  // TODO: reset descriptor pool on resizes.
  gfx_allocate_descriptor_sets(&text.atlas_ds, 1, &(GFX_Descriptor_Set_Binding) {
      .binding = 0,
      .type    = GFX_TYPE_IMAGE_SAMPLER,
      .stages  = GFX_STAGE_FRAGMENT
    }, 1,
    1);
  gfx_descriptor_sampled_texture(text.atlas_ds, 0, &text.atlas_texture,
                                 GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
  gfx_batch_update_descriptor_sets();

  text.pixel_size = 16;
}

void
free_text()
{
  gfx_destroy_buffer(&text.vertex_buffer);
  gfx_destroy_buffer(&text.index_buffer);
  gfx_destroy_texture(&text.atlas_texture);
  gfx_destroy_image(&text.font_atlas);
  gfx_free_memory(&text.cpu_memory);
  gfx_free_memory(&text.gpu_memory);
}

void
upload_font(FT_Face face)
{
  FT_Set_Pixel_Sizes(face, 0, text.pixel_size);
  FT_GlyphSlot glyph_slot = face->glyph;
  stbrp_rect rects[96];

  uint32_t atlas_width, atlas_height;
  gfx_get_image_extent(&text.font_atlas, &atlas_width, &atlas_height, NULL);

  float inv_size          = 1.0f / text.pixel_size;
  float inv_extent_width  = 1.0f / (float)atlas_width;
  float inv_extent_height = 1.0f / (float)atlas_height;

  for (int i = 32; i < 128; i++) {
    FT_Error err = FT_Load_Char(face, i, FT_LOAD_RENDER);
    if (err) {
      printf("freetype: failed to load char '%c' with error(%d) %s\n",
             (char)i, err, FT_Error_String(err));
      continue;
    }

    text.glyphs[i].advance.x = (glyph_slot->advance.x >> 6) * inv_size;
    text.glyphs[i].advance.y = (glyph_slot->advance.y >> 6) * inv_size;
    text.glyphs[i].bearing.x = glyph_slot->bitmap_left * inv_size;
    text.glyphs[i].bearing.y = glyph_slot->bitmap_top * inv_size;
    text.glyphs[i].size.x    = glyph_slot->bitmap.width * inv_size;
    text.glyphs[i].size.y    = glyph_slot->bitmap.rows * inv_size;
    text.glyphs[i].uv_size.x = glyph_slot->bitmap.width * inv_extent_width;
    text.glyphs[i].uv_size.y = glyph_slot->bitmap.rows * inv_extent_height;

    rects[i-32].id = i;
    rects[i-32].w  = glyph_slot->bitmap.width;
    rects[i-32].h  = glyph_slot->bitmap.rows;
  }

  // pack rects
  stbrp_context rect_packing;
  stbrp_node rect_nodes[1024];
  stbrp_init_target(&rect_packing, atlas_width, atlas_height, rect_nodes, 1024);
  stbrp_setup_heuristic(&rect_packing, STBRP_HEURISTIC_Skyline_default);
  if (stbrp_pack_rects(&rect_packing, rects, 96) == 0) {
    printf("failed to pack glyphs to bitmap :( maybe try to pick smaller font size?\n");
    return;
  }

  // load glyphs to staging buffer. (we use vertex buffer as staging)
  uint8_t* tmp = gfx_get_buffer_data(&text.vertex_buffer);
  uint32_t max_height = 0;
  for (uint32_t i = 0; i < 128-32; i++) {
    int c = rects[i].id;
    FT_Error err = FT_Load_Char(face, c, FT_LOAD_RENDER);
    if (err != 0) {
      printf("freetype: failed to load char '%c' with error %s\n", i, FT_Error_String(err));
      continue;
    }
    if (rects[i].y + rects[i].h > max_height)
      max_height = rects[i].y + rects[i].h;
    if (max_height > atlas_height) {
      printf("not enough space in font atlas; required extent is at least [%u, %u]\n",
             atlas_width, max_height);
      return;
    }
    // NOTE: we multiply here by 4 because format is RGBA8 - 4 bytes
    uint32_t offset = (rects[i].x + rects[i].y * atlas_width) << 2;
    uint8_t* data = tmp + offset;
    for (uint32_t y = 0; y < glyph_slot->bitmap.rows; y++) {
      for (uint32_t x = 0; x < glyph_slot->bitmap.width; x++) {
        uint32_t pos = (y * atlas_width + x) << 2;
        // for now we fill everything with 1: every glyph will be white
        data[pos + 0] = 255;
        data[pos + 1] = 255;
        data[pos + 2] = 255;
        data[pos + 3] = glyph_slot->bitmap.buffer[y * glyph_slot->bitmap.width + x];
      }
    }
    text.glyphs[c].uv_offset.x = rects[i].x * inv_extent_width;
    text.glyphs[c].uv_offset.y = rects[i].y * inv_extent_height;
  }

  // copy vertex buffer contents to font atlas
  GFX_Image_Barrier barrier = {
    .image = &text.font_atlas,
    .new_layout = GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    .mip_level = 0,
    .mip_count = 1,
    .array_layer = 0,
    .layer_count = 1
  };
  gfx_barrier(GFX_PIPELINE_STAGE_TOP_OF_PIPE, GFX_PIPELINE_STAGE_TRANSFER, &barrier, 1);

  gfx_copy_buffer_to_image(&text.vertex_buffer, &text.font_atlas,
                           0, 0, 0,
                           atlas_width, max_height, 1);

  barrier.new_layout = GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  gfx_barrier(GFX_PIPELINE_STAGE_TRANSFER, GFX_PIPELINE_STAGE_FRAGMENT_SHADER, &barrier, 1);
}

void
draw_str(const char* str, uint32_t num_chars, Vec2 pos, uint32_t color, uint32_t window_width, uint32_t window_height)
{
  Vertex* out_vertices = gfx_get_buffer_data(&text.vertex_buffer);
  uint32_t* out_indices = gfx_get_buffer_data(&text.index_buffer);
  Vec2 size = {
    (float)text.pixel_size / window_width,
    (float)text.pixel_size / window_height,
  };
  while (num_chars--) {
    // upload 6 indices and 4 vertices to buffer
    Glyph_Info* glyph = &text.glyphs[(int)*str];
    Vec2 pos;
    pos.x = pos.x + glyph->bearing.x * size.x;
    pos.y = pos.y - glyph->bearing.y * size.y;
    Vec2 offset;
    offset.x = glyph->size.x * size.x;
    offset.y = glyph->size.y * size.y;
    const Vec2 muls[] = {
      { 0.0f, 0.0f },
      { 1.0f, 0.0f },
      { 0.0f, 1.0f },
      { 1.0f, 1.0f }
    };
    const uint32_t indices[6] = { /*1st triangle*/0, 1, 3, /*2nd triangle*/3, 2, 0 };
    for (int i = 0; i < 6; i++) {
      out_indices[text.num_indices++] = text.num_vertices + indices[i];
    }
    for (int i = 0; i < 4; i++) {
      out_vertices[text.num_vertices++] = (Vertex) {
        .pos.x = pos.x + offset.x * muls[i].x,
        .pos.y = pos.y + offset.y * muls[i].y,
        .uv.x = glyph->uv_offset.x + glyph->uv_size.x * muls[i].x,
        .uv.y = glyph->uv_offset.y + glyph->uv_size.y * muls[i].y,
        .color = color,
      };
    }
    pos.x += glyph->advance.x * size.x;
    pos.y += glyph->advance.y * size.y;
    str++;
  }
}
