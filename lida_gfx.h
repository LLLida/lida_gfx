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
   Author: Adil Mokhammad
   Mail: 0adilmohammad0@gmail.com

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

  // allows buffer to be the source in transfer operations
  GFX_BUFFER_USAGE_TRANSFER_SRC = 0x00000001,
  // allows buffer to be the destination in transfer operations
  GFX_BUFFER_USAGE_TRANSFER_DST = 0x00000002,
  // allows buffer to be readed from shaders(uniform buffer)
  GFX_BUFFER_USAGE_UNIFORM = 0x00000010,
  // allows buffer to be readed/written from shaders(storage buffer)
  GFX_BUFFER_USAGE_STORAGE = 0x00000020,
  // allows buffer to be used as index buffer
  GFX_BUFFER_USAGE_INDEX = 0x00000040,
  // allows buffer to be used as vertex buffer
  GFX_BUFFER_USAGE_VERTEX = 0x00000080,
  // allows buffer to used as indirect buffer
  GFX_BUFFER_USAGE_INDIRECT = 0x00000100,

} GFX_Buffer_Usage;

typedef enum {

  // allows image to be the source in transfer operations
  GFX_IMAGE_USAGE_TRANSFER_SRC = 0x00000001,
  // allows image to be the destination in transfer operations
  GFX_IMAGE_USAGE_TRANSFER_DST = 0x00000002,
  // allows image to be readed from shaders
  GFX_IMAGE_USAGE_SAMPLED = 0x00000004,
  // allows image to be written from compute shaders
  GFX_IMAGE_USAGE_STORAGE = 0x00000008,
  // allows image to be used as color attachment
  GFX_IMAGE_USAGE_COLOR_ATTACHMENT = 0x00000010,
  // allows image to be used as depth-stencil attachment
  GFX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT = 0x00000020,

} GFX_Image_Usage;

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
  GFX_FORMAT_R16_UNORM = 70,
  GFX_FORMAT_R16_SNORM = 71,
  GFX_FORMAT_R16_USCALED = 72,
  GFX_FORMAT_R16_SSCALED = 73,
  GFX_FORMAT_R16_UINT = 74,
  GFX_FORMAT_R16_SINT = 75,
  GFX_FORMAT_R16_SFLOAT = 76,
  GFX_FORMAT_R16G16_UNORM = 77,
  GFX_FORMAT_R16G16_SNORM = 78,
  GFX_FORMAT_R16G16_USCALED = 79,
  GFX_FORMAT_R16G16_SSCALED = 80,
  GFX_FORMAT_R16G16_UINT = 81,
  GFX_FORMAT_R16G16_SINT = 82,
  GFX_FORMAT_R16G16_SFLOAT = 83,
  GFX_FORMAT_R16G16B16_UNORM = 84,
  GFX_FORMAT_R16G16B16_SNORM = 85,
  GFX_FORMAT_R16G16B16_USCALED = 86,
  GFX_FORMAT_R16G16B16_SSCALED = 87,
  GFX_FORMAT_R16G16B16_UINT = 88,
  GFX_FORMAT_R16G16B16_SINT = 89,
  GFX_FORMAT_R16G16B16_SFLOAT = 90,
  GFX_FORMAT_R16G16B16A16_UNORM = 91,
  GFX_FORMAT_R16G16B16A16_SNORM = 92,
  GFX_FORMAT_R16G16B16A16_USCALED = 93,
  GFX_FORMAT_R16G16B16A16_SSCALED = 94,
  GFX_FORMAT_R16G16B16A16_UINT = 95,
  GFX_FORMAT_R16G16B16A16_SINT = 96,
  GFX_FORMAT_R16G16B16A16_SFLOAT = 97,
  GFX_FORMAT_R32_UINT = 98,
  GFX_FORMAT_R32_SINT = 99,
  GFX_FORMAT_R32_SFLOAT = 100,
  GFX_FORMAT_R32G32_UINT = 101,
  GFX_FORMAT_R32G32_SINT = 102,
  GFX_FORMAT_R32G32_SFLOAT = 103,
  GFX_FORMAT_R32G32B32_UINT = 104,
  GFX_FORMAT_R32G32B32_SINT = 105,
  GFX_FORMAT_R32G32B32_SFLOAT = 106,
  GFX_FORMAT_R32G32B32A32_UINT = 107,
  GFX_FORMAT_R32G32B32A32_SINT = 108,
  GFX_FORMAT_R32G32B32A32_SFLOAT = 109,
  GFX_FORMAT_D16_UNORM = 124,
  GFX_FORMAT_D32_SFLOAT = 126,
  GFX_FORMAT_D16_UNORM_S8_UINT = 128,
  GFX_FORMAT_D24_UNORM_S8_UINT = 129,
  GFX_FORMAT_D32_SFLOAT_S8_UINT = 130,

} GFX_Format;

typedef enum {

  GFX_STAGE_VERTEX = 0x00000001,
  GFX_STAGE_FRAGMENT = 0x00000010,
  GFX_STAGE_COMPUTE = 0x00000020,
  GFX_STAGE_ALL = 0x7FFFFFFF,

} GFX_Stage;

typedef enum {
  GFX_PIPELINE_STAGE_TOP_OF_PIPE = 0x00000001,
  GFX_PIPELINE_STAGE_DRAW_INDIRECT = 0x00000002,
  GFX_PIPELINE_STAGE_VERTEX_INPUT = 0x00000004,
  GFX_PIPELINE_STAGE_VERTEX_SHADER = 0x00000008,
  GFX_PIPELINE_STAGE_GEOMETRY_SHADER = 0x00000040,
  GFX_PIPELINE_STAGE_FRAGMENT_SHADER = 0x00000080,
  GFX_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS = 0x00000100,
  GFX_PIPELINE_STAGE_LATE_FRAGMENT_TESTS = 0x00000200,
  GFX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT = 0x00000400,
  GFX_PIPELINE_STAGE_COMPUTE_SHADER = 0x00000800,
  GFX_PIPELINE_STAGE_TRANSFER = 0x00001000,
  GFX_PIPELINE_STAGE_BOTTOM_OF_PIPE = 0x00002000,
  GFX_PIPELINE_STAGE_HOST = 0x00004000,
  GFX_PIPELINE_STAGE_ALL_GRAPHICS = 0x00008000,
  GFX_PIPELINE_STAGE_ALL_COMMANDS = 0x00010000,
  GFX_PIPELINE_STAGE_NONE = 0,
} GFX_Pipeline_Stage;

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

typedef enum {
    GFX_MEMORY_PROPERTY_DEVICE_LOCAL = 0x00000001,
    GFX_MEMORY_PROPERTY_HOST_VISIBLE = 0x00000002,
    GFX_MEMORY_PROPERTY_HOST_COHERENT = 0x00000004,
    GFX_MEMORY_PROPERTY_HOST_CACHED = 0x00000008,
    // Following flags are unsupported at the moment
    // GFX_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 0x00000010,
    // GFX_MEMORY_PROPERTY_PROTECTED_BIT = 0x00000020,
} GFX_Memory_Properties;

typedef enum {
  GFX_SAMPLER_ADDRESS_MODE_REPEAT = 0,
  GFX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
  GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2,
  GFX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = 3,
  GFX_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
} GFX_Sampler_Address_Mode;

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
    uint32_t binding;
    uint32_t stride;
    int      per_instance;
} GFX_Vertex_Binding;

typedef struct {
    uint32_t    location;
    uint32_t    binding;
    GFX_Format  format;
    uint32_t    offset;
} GFX_Vertex_Attribute;

typedef struct {

  const char* vertex_shader;
  const char* fragment_shader;
  uint32_t vertex_binding_count;
  const GFX_Vertex_Binding* vertex_bindings;
  uint32_t vertex_attribute_count;
  const GFX_Vertex_Attribute* vertex_attributes;
  // if set to 0 than disable depth test.
  int depth_test;
  // if set to 0 than disable writing to depth buffer.
  int depth_write;
  // TODO: primitive topology
  // TODO: viewport
  // TODO: scissor
  // TODO: polygonMode
  // TODO: cullMode
  // TODO: blend_logic
  // TODO: attachments
  GFX_Render_Pass* render_pass;
  // uint32_t subpass; // no support for subpasses yet

} GFX_Pipeline_Desc;

typedef struct {
  char data[24];
} GFX_Pipeline;

typedef struct {
  char data[48];
} GFX_Memory_Block;

typedef struct {
  char data[512];
} GFX_Window;

typedef struct {
  char data[32];
} GFX_Buffer;

typedef struct {
  char data[32];
} GFX_Image;

typedef struct {
  char data[32];
} GFX_Texture;

typedef enum {

  GFX_TYPE_SAMPLER = 0,
  GFX_TYPE_IMAGE_SAMPLER = 1,
  GFX_TYPE_STORAGE_IMAGE = 3,
  GFX_TYPE_UNIFORM_BUFFER = 6,
  GFX_TYPE_STORAGE_BUFFER = 7,

} GFX_Descriptor_Type;

typedef uint64_t GFX_Descriptor_Set;

typedef struct {
  uint32_t binding;
  GFX_Descriptor_Type type;
  GFX_Stage stages;
} GFX_Descriptor_Set_Binding;

typedef float GFX_Clear_Color[4];

typedef struct {
  GFX_Image* image;
  GFX_Image_Layout new_layout;
  uint32_t mip_level;
  uint32_t mip_count;
  uint32_t array_layer;
  uint32_t layer_count;
} GFX_Image_Barrier;

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

int gfx_create_graphics_pipelines(GFX_Pipeline* pipelines, uint32_t count, const GFX_Pipeline_Desc* descs);
int gfx_create_compute_pipelines(GFX_Pipeline* pipelines, uint32_t count, const char** tags);
void gfx_destroy_pipeline(GFX_Pipeline* pipeline);

typedef struct SDL_Window SDL_Window;

/**
   Create a window.
   @param vsync - whether to use vsync
   @note the SDL_WINDOW_VULKAN flag must be passed to SDL_CreateWindow
 */
int gfx_create_window_sdl(GFX_Window* window, SDL_Window* handle, int vsync);

/**
   Destroy a window.
 */
void gfx_destroy_window(GFX_Window* window);

int gfx_resize_window(GFX_Window* window, uint32_t* width, uint32_t* height);

void gfx_get_window_size(const GFX_Window* window, uint32_t* width, uint32_t* height);

void gfx_begin_commands(GFX_Window* window);
/**
   Acquire next swapchain image. Return 0 on success, 1 if resized and other value on error.
 */
int gfx_swap_buffers(GFX_Window* window);
/**
   This ends render pass and presents window.
 */
int gfx_submit_and_present(GFX_Window* window);

GFX_Render_Pass* gfx_get_main_pass(GFX_Window* window);

void gfx_begin_main_pass(GFX_Window* window);

/**
   Render passes are cached, no worries about calling twice with same arguments.
 */
GFX_Render_Pass* gfx_render_pass(const GFX_Attachment_Info* attachments, uint32_t count);

/**
   Begin a render pass. This function may create a framebuffer to
   render.
 */
void gfx_begin_render_pass(GFX_Render_Pass* render_pass, const GFX_Texture* attachments, uint32_t num_attachments, const GFX_Clear_Color* clear_colors);
void gfx_end_render_pass();

void gfx_bind_pipeline(GFX_Pipeline* pipeline);

/**
   NOTE: this function must be called after gfx_bind_pipeline()!
 */
void gfx_bind_descriptor_sets(const GFX_Descriptor_Set* descriptor_sets, uint32_t ds_count);
/**
   NOTE: this function must be called after gfx_bind_pipeline()!
 */
void gfx_push_constants(const void* push_constant, uint32_t push_constant_size);

void gfx_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
void gfx_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);

void gfx_dispatch(uint32_t x, uint32_t y, uint32_t z);

void gfx_barrier(GFX_Pipeline_Stage src_stage, GFX_Pipeline_Stage dst_stage, const GFX_Image_Barrier* barriers, uint32_t count);
#define gfx_compute_to_compute_barrier(barriers, count) gfx_barrier(GFX_PIPELINE_STAGE_COMPUTE_SHADER, GFX_PIPELINE_STAGE_COMPUTE_SHADER, barriers, count)

int gfx_allocate_memory_for_buffers(GFX_Memory_Block* memory, GFX_Buffer* buffers, uint32_t count, GFX_Memory_Properties properties);
int gfx_allocate_memory_for_images(GFX_Memory_Block* memory, GFX_Image* images, uint32_t count, GFX_Memory_Properties properties);
void gfx_free_memory(GFX_Memory_Block* memory);

int gfx_create_buffer(GFX_Buffer* buffer, GFX_Buffer_Usage usage, uint32_t size);
void gfx_destroy_buffer(GFX_Buffer* buffer);
void* gfx_get_buffer_data(GFX_Buffer* buffer);
int gfx_copy_to_buffer(GFX_Buffer* buffer, const void* src, uint32_t offset, uint32_t size);

void gfx_bind_vertex_buffers(GFX_Buffer* buffers, uint32_t count, const uint64_t* offsets);
void gfx_bind_index_buffer(GFX_Buffer* buffer, const uint64_t offset);

int gfx_create_image(GFX_Image* image, GFX_Image_Usage usage,
                     uint32_t width, uint32_t height, uint32_t depth,
                     GFX_Format format, uint32_t mips, uint32_t levels);
void gfx_destroy_image(GFX_Image* image);
void gfx_get_image_extent(GFX_Image* image, uint32_t* width, uint32_t* height, uint32_t* depth);
int gfx_create_texture(GFX_Texture* texture, const GFX_Image* image,
                       uint32_t first_mip, uint32_t first_layer,
                       uint32_t num_mips, uint32_t num_layers);
void gfx_destroy_texture(GFX_Texture* texture);

/**
   Allocate descriptor sets that have the same layout.  After
   allocation it is necessary to update descriptor sets with the
   desired resources.
   */
int gfx_allocate_descriptor_sets(GFX_Descriptor_Set* sets, uint32_t num_sets,
                                 const GFX_Descriptor_Set_Binding* bindings, uint32_t num_bindings,
                                 int resetable);
int gfx_free_descriptor_sets(GFX_Descriptor_Set* sets, uint32_t num_sets);
void gfx_descriptor_buffer(GFX_Descriptor_Set set, uint32_t binding, GFX_Descriptor_Type type,
                           const GFX_Buffer* buffer, uint32_t offset, uint32_t range);
void gfx_descriptor_sampled_texture(GFX_Descriptor_Set set, uint32_t binding,
                                    const GFX_Texture* texture, GFX_Image_Layout layout, int is_linear_filter, GFX_Sampler_Address_Mode mode);
void gfx_descriptor_storage_texture(GFX_Descriptor_Set set, uint32_t binding, const GFX_Texture* texture);
void gfx_batch_update_descriptor_sets();

#ifdef __cplusplus
}
#endif
