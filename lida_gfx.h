/* -*- mode: c -*-
              ___         ____   ________      ________
             |   |       |    | |        \    /        \
             |   |       |    | |    __   \  /    ___   \
             |   |       |    | |   |  \   | |   /   \   |
             |   |       |    | |   |  |   | |   |___|   |
             |   |       |    | |   |  |   | |           |
             |   |       |    | |   |  |   | |    ___    |
             |   |_____  |    | |   |__/   | |   |   |   |
             |         | |    | |         |  |   |   |   |
             |_________| |____| |________/   |___|   |___|

   Graphics abstraction layer for Vulkan.

   INSTALLATION. This library is bundled as two files: lida_gfx.h and
   lida_gfx_vulkan.c. Just copy them inside your project. You'd need
   to add lida_gfx_vulkan.c to your built system. IMPORTANT NOTE:
   don't link to any Vulkan library; all needed Vulkan functions are
   loaded by this library automatically.

   INFO. This library is developed by a single person, who is a math
   student in Moscow as of 2023. My name is Adil.

   Vulkan is hard. To me it's hard not because it has so many features
   but because it requires you to write thousands line of cumbersome
   code. By using this library you may make your Vulkan experience
   easier; it allows to not write so much of unnecessary code but still
   utilize the GPU's power.

   Currently we implement following things for user:
   - function loading;
   - VkInstance creation;
   - validation layers;
   - VkDevice creation, currently there's no way for user to specify
     which extensions will be used;
   - Swaphain creation/management;
   - Seamless render pass caching;

   ALLOCATIONS. This library does no memory allocations. You heard it
   right. All memory is managed inside one statically allocated
   buffer. LRU caches are used to manage variable amount of
   objects. All memory allocations are done either by user or by the
   Vulkan driver.

 */

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GFX_Log_Callback)(int severity, const char* fmt, ...);

typedef void* (*GFX_Load_Shader_Module_Callback)(const char* tag, size_t* bytes);
typedef void  (*GFX_Free_Shader_Module_Callback)(void* data);

typedef struct {

  const char*      app_name;
  uint32_t         app_version;
  int              enable_debug_layers;
  uint32_t         gpu_id;

  GFX_Log_Callback                log_fn;
  GFX_Load_Shader_Module_Callback load_shader_fn;
  GFX_Free_Shader_Module_Callback free_shader_fn;

} GFX_Init_Info;

typedef enum {

  /* memory flags */
  GFX_MEMORY_HEAP_GPU = 1<<0,
  GFX_MEMORY_HEAP_CPU = 1<<1,
  GFX_MEMORY_HEAP_GPU_CPU = 1<<2,

  /* buffer flags */
  // allows buffer to be the source in transfer operations
  GFX_BUFFER_TRANSFER_SRC = 1<<3,
  // allows buffer to be the destination in transfer operations
  GFX_BUFFER_TRANSFER_DST = 1<<4,
  // allows buffer to be readed from shaders(uniform buffer)
  GFX_BUFFER_UNIFORM = 1<<4,
  // allows buffer to be readed/written from shaders(storage buffer)
  GFX_BUFFER_STORAGE = 1<<5,
  // allows buffer to be used as index buffer
  GFX_BUFFER_INDEX = 1<<6,
  // allows buffer to be used as vertex buffer
  GFX_BUFFER_VERTEX = 1<<7,
  // allows buffer to used as indirect buffer
  GFX_BUFFER_INDIRECT = 1<<8,

  /* image flags */
  // allows image to be the source in transfer operations
  GFX_IMAGE_TRANSFER_SRC = 1<<3,
  // allows image to be the destination in transfer operations
  GFX_IMAGE_TRANSFER_DST = 1<<4,
  // allows image to be readed from shaders
  GFX_IMAGE_SAMPLED = 1<<5,
  // allows image to be written from compute shaders
  GFX_IMAGE_STORAGE = 1<<6,
  // allows image to be used as color attachment
  GFX_IMAGE_COLOR_ATTACHMENT = 1<<7,
  // allows image to be used as depth-stencil attachment
  GFX_IMAGE_DEPTH_STENCIL_ATTACHMENT = 1<<8,

} GFX_Bits;

typedef enum {

  GFX_FORMAT_R8_UNORM = 9,
  GFX_FORMAT_R8_SNORM = 10,
  GFX_FORMAT_R8_USCALED = 11,
  GFX_FORMAT_R8_SSCALED = 12,
  GFX_FORMAT_R8_UINT = 13,
  GFX_FORMAT_R8_SINT = 14,
  GFX_FORMAT_R8_SRGB = 15,
  GFX_FORMAT_R8G8_UNORM = 16,
  GFX_FORMAT_R8G8_SNORM = 17,
  GFX_FORMAT_R8G8_USCALED = 18,
  GFX_FORMAT_R8G8_SSCALED = 19,
  GFX_FORMAT_R8G8_UINT = 20,
  GFX_FORMAT_R8G8_SINT = 21,
  GFX_FORMAT_R8G8_SRGB = 22,
  GFX_FORMAT_R8G8B8_UNORM = 23,
  GFX_FORMAT_R8G8B8_SNORM = 24,
  GFX_FORMAT_R8G8B8_USCALED = 25,
  GFX_FORMAT_R8G8B8_SSCALED = 26,
  GFX_FORMAT_R8G8B8_UINT = 27,
  GFX_FORMAT_R8G8B8_SINT = 28,
  GFX_FORMAT_R8G8B8_SRGB = 29,
  GFX_FORMAT_R8G8B8A8_UNORM = 37,
  GFX_FORMAT_R8G8B8A8_SNORM = 38,
  GFX_FORMAT_R8G8B8A8_USCALED = 39,
  GFX_FORMAT_R8G8B8A8_SSCALED = 40,
  GFX_FORMAT_R8G8B8A8_UINT = 41,
  GFX_FORMAT_R8G8B8A8_SINT = 42,
  GFX_FORMAT_R8G8B8A8_SRGB = 43,

} GFX_Format;

typedef enum {

  GFX_STAGE_VERTEX = 1<<0,
  GFX_STAGE_FRAGMENT = 1<<1,
  GFX_STAGE_COMPUTE = 1<<2,
  GFX_STAGE_ALL = 0x7FFFFFFF,

} GFX_Stage;

typedef enum {

  GFX_ATTACHMENT_OP_NONE,
  GFX_ATTACHMENT_OP_LOAD,
  GFX_ATTACHMENT_OP_CLEAR,
  GFX_ATTACHMENT_OP_STORE,

} GFX_Attachment_Op;

typedef enum {

  GFX_IMAGE_LAYOUT_UNDEFINED = 0,
  GFX_IMAGE_LAYOUT_GENERAL = 1,
  GFX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
  GFX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
  GFX_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
  GFX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
  GFX_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
  GFX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
  GFX_IMAGE_LAYOUT_PREINITIALIZED = 8,
  GFX_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
  GFX_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
  GFX_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
  GFX_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL = 1000241001,
  GFX_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
  GFX_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL = 1000241003,
  GFX_IMAGE_LAYOUT_READ_ONLY_OPTIMAL = 1000314000,
  GFX_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL = 1000314001,
  GFX_IMAGE_LAYOUT_PRESENT_SRC_KHR = 1000001002,

} GFX_Image_Layout;

typedef struct {

  GFX_Format format;
  GFX_Attachment_Op load_op;
  GFX_Attachment_Op store_op;
  GFX_Image_Layout initial_layout;
  GFX_Image_Layout final_layout;
  GFX_Image_Layout work_layout;

} GFX_Attachment_Info;

typedef struct GFX_Render_Pass GFX_Render_Pass;

typedef struct {

  const char* tag;
  uint32_t bytes;
  GFX_Bits bits;

} GFX_Buffer;

typedef struct {

  const char* tag;
  uint32_t width, height;
  GFX_Format format;
  GFX_Bits bits;

} GFX_Image;

typedef enum {

  GFX_TYPE_SAMPLER,
  GFX_TYPE_IMAGE_SAMPLER,
  GFX_TYPE_STORAGE_IMAGE,
  GFX_TYPE_UNIFORM_BUFFER,
  GFX_TYPE_STORAGE_BUFFER,

} GFX_Binding_Type;

typedef struct {

  union {
    GFX_Buffer* buffer;
    GFX_Image* image;
  } data;
  uint32_t index;
  GFX_Binding_Type type;
  GFX_Stage stages;

} GFX_Binding;

typedef uint32_t GFX_Descriptor_Set;

typedef struct {
  char data[512];
} GFX_Window;

/**
   Initialise the graphics library.

   This creates all necessary Vulkan objects for further job.
 */
int gfx_init(const GFX_Init_Info* info);

/**
   Free the graphics library.

   This destroys all Vulkan objects created by 'gfx_init()'.
 */
void gfx_free();

/**
   Check whether the graphics library was initialised.
 */
int gfx_is_initialised();

void gfx_wait_idle_gpu();

typedef struct SDL_Window SDL_Window;

/**
   Create a window.
   @param vsync - whether to use vsync
 */
int gfx_create_window_sdl(GFX_Window* window, SDL_Window* handle, int vsync);

/**
   Destroy a window.
 */
void gfx_destroy_window(GFX_Window* window);

void gfx_begin_commands(GFX_Window* window);
/**
   This also starts render pass.
 */
int gfx_swap_buffers(GFX_Window* window);
/**
   This ends render pass and presents window.
 */
int gfx_submit_and_present(GFX_Window* window);

void gfx_begin_main_pass(GFX_Window* window);

/**
   Render passes are cached, no worries about calling twice with same arguments.
 */
GFX_Render_Pass* gfx_render_pass(const GFX_Attachment_Info* attachments, uint32_t count);

void gfx_end_render_pass();

#ifdef __cplusplus
}
#endif
