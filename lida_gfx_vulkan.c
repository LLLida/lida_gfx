/*
  lida_gfx_vulkan.c - the Vulkan abstraction layer for lida_gfx
*/

/* --Configuration */

#ifndef LIDA_GFX_VERSION
#define LIDA_GFX_VERSION 1
#endif

// NOTE: configure which window library you want to use
#define LIDA_GFX_USE_SDL
// #define LIDA_USE_GLFW

#define LIDA_GFX_SHADER_MAX_SETS 4
#define LIDA_GFX_SHADER_MAX_BINDINGS_PER_SET 8
#define LIDA_GFX_SHADER_MAX_RANGES 1

#include <assert.h>             // TODO: make assert macro customizable
#include <alloca.h>
#include <string.h>

#include "lida_gfx.h"

#ifdef LIDA_GFX_USE_SDL
#include <SDL_vulkan.h>
#endif
// TODO: GLFW support

#ifdef __GNUC__
// https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes
#define ATTRIBUTE_NONNULL(...) __attribute__((nonnull (__VA_ARGS__)))
#define ATTRIBUTE_FALLTHROUGH() __attribute__((fallthrough))
#define ATTRIBUTE_ALIGNED(a) __attribute__((aligned(a)))
#else
#define ATTRIBUTE_PRINTF(i)
#define ATTRIBUTE_NONNULL(...)
#define ATTRIBUTE_FALLTHROUGH()
#define ATTRIBUTE_ALIGNED(a)
#endif


/* --Vulkan */

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
static PFN_vkCreateInstance vkCreateInstance;
static PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;

#define FOR_ALL_FUNCTIONS()                             \
  X(vkAllocateCommandBuffers);                          \
  X(vkAllocateDescriptorSets);                          \
  X(vkAllocateMemory);                                  \
  X(vkBeginCommandBuffer);                              \
  X(vkBindBufferMemory);                                \
  X(vkBindImageMemory);                                 \
  X(vkCmdBeginQuery);                                   \
  X(vkCmdBeginRenderPass);                              \
  X(vkCmdBindDescriptorSets);                           \
  X(vkCmdBindIndexBuffer);                              \
  X(vkCmdBindPipeline);                                 \
  X(vkCmdBindVertexBuffers);                            \
  X(vkCmdBlitImage);                                    \
  X(vkCmdClearAttachments);                             \
  X(vkCmdClearColorImage);                              \
  X(vkCmdClearDepthStencilImage);                       \
  X(vkCmdCopyBuffer);                                   \
  X(vkCmdCopyBufferToImage);                            \
  X(vkCmdCopyImage);                                    \
  X(vkCmdCopyImageToBuffer);                            \
  X(vkCmdCopyQueryPoolResults);                         \
  X(vkCmdDispatch);                                     \
  X(vkCmdDispatchIndirect);                             \
  X(vkCmdDraw);                                         \
  X(vkCmdDrawIndexed);                                  \
  X(vkCmdDrawIndexedIndirect);                          \
  X(vkCmdDrawIndirect);                                 \
  X(vkCmdEndQuery);                                     \
  X(vkCmdEndRenderPass);                                \
  X(vkCmdExecuteCommands);                              \
  X(vkCmdFillBuffer);                                   \
  X(vkCmdNextSubpass);                                  \
  X(vkCmdPipelineBarrier);                              \
  X(vkCmdPushConstants);                                \
  X(vkCmdResetEvent);                                   \
  X(vkCmdResetQueryPool);                               \
  X(vkCmdResolveImage);                                 \
  X(vkCmdSetBlendConstants);                            \
  X(vkCmdSetDepthBias);                                 \
  X(vkCmdSetDepthBounds);                               \
  X(vkCmdSetEvent);                                     \
  X(vkCmdSetLineWidth);                                 \
  X(vkCmdSetScissor);                                   \
  X(vkCmdSetStencilCompareMask);                        \
  X(vkCmdSetStencilReference);                          \
  X(vkCmdSetStencilWriteMask);                          \
  X(vkCmdSetViewport);                                  \
  X(vkCmdUpdateBuffer);                                 \
  X(vkCmdWaitEvents);                                   \
  X(vkCmdWriteTimestamp);                               \
  X(vkCreateBuffer);                                    \
  X(vkCreateBufferView);                                \
  X(vkCreateCommandPool);                               \
  X(vkCreateComputePipelines);                          \
  X(vkCreateDescriptorPool);                            \
  X(vkCreateDescriptorSetLayout);                       \
  X(vkCreateDevice);                                    \
  X(vkCreateEvent);                                     \
  X(vkCreateFence);                                     \
  X(vkCreateFramebuffer);                               \
  X(vkCreateGraphicsPipelines);                         \
  X(vkCreateImage);                                     \
  X(vkCreateImageView);                                 \
  X(vkCreatePipelineCache);                             \
  X(vkCreatePipelineLayout);                            \
  X(vkCreateQueryPool);                                 \
  X(vkCreateRenderPass);                                \
  X(vkCreateSampler);                                   \
  X(vkCreateSemaphore);                                 \
  X(vkCreateShaderModule);                              \
  X(vkDestroyBuffer);                                   \
  X(vkDestroyBufferView);                               \
  X(vkDestroyCommandPool);                              \
  X(vkDestroyDescriptorPool);                           \
  X(vkDestroyDescriptorSetLayout);                      \
  X(vkDestroyDevice);                                   \
  X(vkDestroyEvent);                                    \
  X(vkDestroyFence);                                    \
  X(vkDestroyFramebuffer);                              \
  X(vkDestroyImage);                                    \
  X(vkDestroyImageView);                                \
  X(vkDestroyInstance);                                 \
  X(vkDestroyPipeline);                                 \
  X(vkDestroyPipelineCache);                            \
  X(vkDestroyPipelineLayout);                           \
  X(vkDestroyQueryPool);                                \
  X(vkDestroyRenderPass);                               \
  X(vkDestroySampler);                                  \
  X(vkDestroySemaphore);                                \
  X(vkDestroyShaderModule);                             \
  X(vkDeviceWaitIdle);                                  \
  X(vkEndCommandBuffer);                                \
  X(vkEnumerateDeviceExtensionProperties);              \
  X(vkEnumerateDeviceLayerProperties);                  \
  X(vkEnumeratePhysicalDevices);                        \
  X(vkFlushMappedMemoryRanges);                         \
  X(vkFreeCommandBuffers);                              \
  X(vkFreeDescriptorSets);                              \
  X(vkFreeMemory);                                      \
  X(vkGetBufferMemoryRequirements);                     \
  X(vkGetDeviceMemoryCommitment);                       \
  X(vkGetDeviceProcAddr);                               \
  X(vkGetDeviceQueue);                                  \
  X(vkGetEventStatus);                                  \
  X(vkGetFenceStatus);                                  \
  X(vkGetImageMemoryRequirements);                      \
  X(vkGetImageSparseMemoryRequirements);                \
  X(vkGetImageSubresourceLayout);                       \
  X(vkGetPhysicalDeviceFeatures);                       \
  X(vkGetPhysicalDeviceFormatProperties);               \
  X(vkGetPhysicalDeviceImageFormatProperties);          \
  X(vkGetPhysicalDeviceMemoryProperties);               \
  X(vkGetPhysicalDeviceProperties);                     \
  X(vkGetPhysicalDeviceQueueFamilyProperties);          \
  X(vkGetPhysicalDeviceSparseImageFormatProperties);    \
  X(vkGetPipelineCacheData);                            \
  X(vkGetQueryPoolResults);                             \
  X(vkGetRenderAreaGranularity);                        \
  X(vkInvalidateMappedMemoryRanges);                    \
  X(vkMapMemory);                                       \
  X(vkMergePipelineCaches);                             \
  X(vkQueueBindSparse);                                 \
  X(vkQueueSubmit);                                     \
  X(vkQueueWaitIdle);                                   \
  X(vkResetCommandBuffer);                              \
  X(vkResetCommandPool);                                \
  X(vkResetDescriptorPool);                             \
  X(vkResetEvent);                                      \
  X(vkResetFences);                                     \
  X(vkSetEvent);                                        \
  X(vkUnmapMemory);                                     \
  X(vkUpdateDescriptorSets);                            \
  X(vkWaitForFences);                                   \
  X(vkCmdDebugMarkerBeginEXT);                          \
  X(vkCmdDebugMarkerEndEXT);                            \
  X(vkCmdDebugMarkerInsertEXT);                         \
  X(vkDebugMarkerSetObjectNameEXT);                     \
  X(vkDebugMarkerSetObjectTagEXT);                      \
  X(vkCreateDebugReportCallbackEXT);                    \
  X(vkDebugReportMessageEXT);                           \
  X(vkDestroyDebugReportCallbackEXT);                   \
  X(vkDestroySurfaceKHR);                               \
  X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);         \
  X(vkGetPhysicalDeviceSurfaceFormatsKHR);              \
  X(vkGetPhysicalDeviceSurfacePresentModesKHR);         \
  X(vkGetPhysicalDeviceSurfaceSupportKHR);              \
  X(vkAcquireNextImageKHR);                             \
  X(vkCreateSwapchainKHR);                              \
  X(vkDestroySwapchainKHR);                             \
  X(vkGetSwapchainImagesKHR);                           \
  X(vkQueuePresentKHR)

// define Vulkan API functions
#define X(name) static PFN_##name name
FOR_ALL_FUNCTIONS();
#undef X


/* --platform specific stuff */

#ifdef _WIN32
typedef const char* LPCSTR;
typedef struct HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
#ifdef _WIN64
typedef __int64 (__stdcall* FARPROC)(void);
#else
typedef int (__stdcall* FARPROC)(void);
#endif
#else
# include <dlfcn.h>
#endif

#ifdef _WIN32
__declspec(dllimport) HMODULE __stdcall LoadLibraryA(LPCSTR);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE, LPCSTR);
#endif


/* --LRU Cache */

typedef struct {
  uint32_t hash;
  // reference to next node in hash table bucket
  int32_t list;
  // reference to previous node in linked list
  int32_t prev;
  // reference to next node in linked list
  int32_t next;
} Node_Header;

typedef uint32_t(*hash_function_t)(const void* obj);
typedef int(*equal_function_t)(const void* lhs, const void* rhs);
typedef void(*destructor_function_t)(void* obj);

typedef struct {

  hash_function_t hash_fn;
  equal_function_t eq_fn;
  destructor_function_t des_fn;
  const char* typename;
  uint32_t sizeof_;

  int32_t* ht_data;
  uint32_t ht_mask;
  char* node_data;
  int32_t first;
  int32_t last;
  int32_t free;

} LRU_Cache;

static uint32_t
nearest_pow2(uint32_t v)
{
  // https://stackoverflow.com/questions/466204/rounding-up-to-next-power-of-2
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}

static Node_Header*
lru_cache_ith(const LRU_Cache* lru, uint32_t i)
{
  void* data = &lru->node_data[i * (lru->sizeof_ + sizeof(Node_Header))];
  return data;
}

/**
   Create a LRU cache. This function does not any allocations, instead
   it takes a pointer to a chunk of memory it will be using. When
   cache runs out of space it, it clears space by destroying a Least
   Recently Used value.

   'hash_fn', 'eq_fn', 'des_fn', 'sizeof_' and 'typename' are
   necessary type information for objects to be stored.
*/
static LRU_Cache
lru_cache_init(uint32_t bytes, void* data, hash_function_t hash_fn, equal_function_t eq_fn, destructor_function_t des_fn, uint32_t sizeof_, const char* typename)
{
  uint32_t c1 = bytes / (sizeof_ + sizeof(Node_Header));
  uint32_t ht_size = nearest_pow2(c1>>1);
  LRU_Cache ret = {
    .hash_fn   = hash_fn,
    .eq_fn     = eq_fn,
    .des_fn    = des_fn,
    .typename  = typename,
    .sizeof_   = sizeof_,
    .ht_data   = data,
    .ht_mask   = ht_size-1,
    .node_data = (char*)data + ht_size * sizeof(int32_t),
    .first     = -1,
    .last      = -1,
    .free      = 0,
  };
  for (size_t i = 0; i < ret.ht_mask+1; i++) {
    ret.ht_data[i] = -1;
  }
  uint32_t capacity = (bytes - ht_size * sizeof(int32_t)) / (sizeof_ + sizeof(Node_Header));
  for (size_t i = 0; i < capacity; i++) {
    Node_Header* node = lru_cache_ith(&ret, i);
    node->prev        = -1;
    node->next        = -1;
    node->list        = i+1;
  }
  lru_cache_ith(&ret, capacity-1)->list = -1;
  // printf("ht_size=%u capacity=%u\n", ht_size, capacity);
  return ret;
}

/**
   Check whether a value is in cache. Returns pointer to that value in
   cache if it's found and NULL otherwise.

   NOTE: this does not update timestamp of value in queue.
*/
static void*
lru_cache_search(LRU_Cache* lru, const void* obj)
{
  uint32_t hash = lru->hash_fn(obj);
  uint32_t id   = hash & lru->ht_mask;

  int32_t next = lru->ht_data[id];
  while (next != -1) {
    Node_Header* node = lru_cache_ith(lru, next);
    if (hash == node->hash && lru->eq_fn(obj, node+1)) {
      return node+1;
    }
    next = node->list;
  }
  return NULL;
}

/**
   Check if an object in cache. If yes then return pointer to the
   object in cache. If not then insert this object to cache possibly
   by destroying the Least Recently Used one.

   'flag' is set to 1 when a new object was created. Otherwise it's set to 0.
*/
static void*
lru_cache_get(LRU_Cache* lru, const void* obj, int* flag)
{
  uint32_t hash = lru->hash_fn(obj);
  uint32_t id = hash & lru->ht_mask;
  Node_Header* node;

  int32_t* next = &lru->ht_data[id];
  while (*next != -1) {
    node = lru_cache_ith(lru, *next);
    if (hash == node->hash && lru->eq_fn(obj, node+1)) {
      // remove from linked list
      if (node->prev != -1) {
        Node_Header* left = lru_cache_ith(lru, node->prev);
        left->next = node->next;
        if (*next == lru->last)
          lru->last = left->prev;
      }
      if (node->next != -1) {
        Node_Header* right = lru_cache_ith(lru, node->next);
        right->prev = node->prev;
      }
      // move to front
      node->prev = -1;          // debug
      node->next = lru->first;
      lru_cache_ith(lru, lru->first)->prev = *next;
      lru->first = *next;

      if (flag) *flag = 0;
      return node+1;
    }
    next = &node->list;
  }
  if (lru->free == -1) {
    // delete recently used value
    Node_Header* last = lru_cache_ith(lru, lru->last);
    lru->des_fn(last+1);
    id = last->hash & lru->ht_mask;
    // remove from bucket( O(1) amortized )
    node = lru_cache_ith(lru, lru->ht_data[id]);
    Node_Header* prev = NULL;
    while (node != last) {
      prev = node;
      node = lru_cache_ith(lru, node->list);
    }
    if (prev) {
      prev->list = node->list;
      next = &prev->list;
    } else {
      lru->ht_data[id] = node->list;
      while (node->list != -1) {
        node = lru_cache_ith(lru, node->list);
      }
      next = &node->list;
    }
    last->list = -1;
    lru->free = lru->last;
    lru->last = last->prev;
    lru_cache_ith(lru, lru->last)->next = -1;
  }
  // insert new element
  node = lru_cache_ith(lru, lru->free);
  node->next = lru->first;
  *next = lru->free;
  if (lru->first != -1) lru_cache_ith(lru, lru->first)->prev = *next;
  lru->first = lru->free;
  if (lru->last == -1) lru->last = lru->first;
  lru->free = node->list;

  node->hash = hash;
  node->list = -1;
  node->prev = -1;              // debug
  memcpy(node+1, obj, lru->sizeof_);

  if (flag) *flag = 1;
  return node+1;
}

/**
   Call destructors for objects in cache.

   NOTE: destructors are called in order. Objects that were accessed
   more recently will be destroyed sooner.
*/
static void
lru_cache_destroy(LRU_Cache* lru)
{
  int32_t it = lru->first;
  while (it != -1) {
    Node_Header* node = lru_cache_ith(lru, it);
    lru->des_fn(node+1);
    it = node->next;
  }
  lru->ht_data = NULL;
  lru->node_data = NULL;
}

// for iteration
static void*
lru_cache_first_element(const LRU_Cache* lru)
{
  if (lru->first != -1) {
    return lru_cache_ith(lru, lru->first)+1;
  }
  return NULL;
}

// for iteration
static void*
lru_cache_next_element(const LRU_Cache* lru, void* obj)
{
  Node_Header* node = (Node_Header*)obj - 1;
  if (node->next != -1) {
    return lru_cache_ith(lru, node->next)+1;
  }
  return NULL;
}

#define LRU_CACHE_FOREACH(lru, Type, it) for (Type* it = lru_cache_first_element(lru); it; it = lru_cache_next_element(lru, it))

static uint32_t
hash_memory(const void* key, uint32_t bytes)
{
  // based on MurmurHash2: https://sites.google.com/site/murmurhash/
  const uint32_t seed = 909713;
  const uint32_t m = 0x5bd1e995;
  const int r = 24;
  uint32_t h = seed ^ bytes;
  const uint32_t* data = key;
  while (bytes >= 4) {
    uint32_t k = *(data++);

    k *= m;
    k ^= k >> r;
    k *= m;

    h *= m;
    h ^= k;

    bytes -= 4;
  }
  // handle remaining bytes
  const unsigned char* chars = (const unsigned char*)data;
  switch (bytes) {
  case 3: h ^= chars[2] << 16;
    ATTRIBUTE_FALLTHROUGH();
  case 2: h ^= chars[1] << 8;
    ATTRIBUTE_FALLTHROUGH();
  case 1: h ^= chars[0];
    h *= m;
  }
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

static uint32_t
hash_string(const char* str)
{
  // https://cp-algorithms.com/string/string-hashing.html
  uint32_t hash_value = 0;
  uint32_t p_pow = 1;
  char c;
#define HASH_P 31
#define HASH_M 1000009
  while ((c = *str)) {
    hash_value = (hash_value + (c - 'a' + 1) * p_pow) % HASH_M;
    p_pow = (p_pow * HASH_P) % HASH_M;
    str++;
  }
#undef HASH_P
#undef HASH_M
  return hash_value;
}


/* --global */

static struct {
  uint32_t membuf[4096];
  uint32_t memptr;
  uint32_t memright;
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice logical_device;
  // @TODO: utilize more device queues
  uint32_t graphics_queue_family;
  VkQueue graphics_queue;
  VkDebugReportCallbackEXT debug_report_callback;
  VkCommandPool command_pool;

  VkCommandBuffer current_cmd;

  GFX_Log_Callback                log_fn;
  GFX_Load_Shader_Module_Callback load_shader_fn;
  GFX_Free_Shader_Module_Callback free_shader_fn;

  uint32_t num_enabled_instance_extensions;
  const char** enabled_instance_extensions;

  VkQueueFamilyProperties* queue_families;
  uint32_t num_queue_families;

  // TODO: store those in hash table or smth
  const char** enabled_device_extensions;
  uint32_t num_enabled_device_extensions;

  LRU_Cache render_pass_cache;
  LRU_Cache shader_cache;
  LRU_Cache ds_layout_cache;
  LRU_Cache pipeline_layout_cache;
  LRU_Cache sampler_cache;

  VkPhysicalDeviceProperties device_properties;
  VkPhysicalDeviceFeatures device_features;
  VkPhysicalDeviceMemoryProperties memory_properties;

} g;

#define LOG_DEBUG(...) g.log_fn(0, __VA_ARGS__)
#define LOG_INFO(...) g.log_fn(1, __VA_ARGS__)
#define LOG_WARN(...) g.log_fn(2, __VA_ARGS__)
#define LOG_ERROR(...) g.log_fn(3, __VA_ARGS__)

#define ARR_SIZE(arr) sizeof(arr) / sizeof(arr[0])

static void*
push_mem(uint32_t bytes)
{
  void* ret = &g.membuf[g.memptr];
  g.memptr += (bytes+3)>>2;
  assert(g.memptr <= g.memright);
  return ret;
}

static void
pop_mem(void* ptr)
{
  g.memptr -= (uint32_t*)ptr - g.membuf;
}

static void*
push_mem_right(uint32_t bytes)
{
  assert(g.memright >= g.memptr + ((bytes+3)>>2));
  g.memright -= (bytes+3)>>2;
  void* ret = &g.membuf[g.memright];
  return ret;
}


/* --SPIR-V */
// https://github.com/KhronosGroup/SPIRV-Headers/blob/main/include/spirv/1.0/spirv.h

typedef unsigned int SpvId;

#define SPV_VERSION 0x10000
#define SPV_REVISION 12

static const unsigned int SpvMagicNumber = 0x07230203;
static const unsigned int SpvVersion = 0x00010000;
static const unsigned int SpvRevision = 12;
static const unsigned int SpvOpCodeMask = 0xffff;
static const unsigned int SpvWordCountShift = 16;

typedef enum SpvSourceLanguage_ {
  SpvSourceLanguageUnknown = 0,
  SpvSourceLanguageESSL = 1,
  SpvSourceLanguageGLSL = 2,
  SpvSourceLanguageOpenCL_C = 3,
  SpvSourceLanguageOpenCL_CPP = 4,
  SpvSourceLanguageHLSL = 5,
  SpvSourceLanguageMax = 0x7fffffff,
} SpvSourceLanguage;

typedef enum SpvExecutionModel_ {
  SpvExecutionModelVertex = 0,
  SpvExecutionModelTessellationControl = 1,
  SpvExecutionModelTessellationEvaluation = 2,
  SpvExecutionModelGeometry = 3,
  SpvExecutionModelFragment = 4,
  SpvExecutionModelGLCompute = 5,
  SpvExecutionModelKernel = 6,
  SpvExecutionModelMax = 0x7fffffff,
} SpvExecutionModel;

typedef enum SpvAddressingModel_ {
  SpvAddressingModelLogical = 0,
  SpvAddressingModelPhysical32 = 1,
  SpvAddressingModelPhysical64 = 2,
  SpvAddressingModelMax = 0x7fffffff,
} SpvAddressingModel;

typedef enum SpvMemoryModel_ {
  SpvMemoryModelSimple = 0,
  SpvMemoryModelGLSL450 = 1,
  SpvMemoryModelOpenCL = 2,
  SpvMemoryModelMax = 0x7fffffff,
} SpvMemoryModel;

typedef enum SpvExecutionMode_ {
  SpvExecutionModeInvocations = 0,
  SpvExecutionModeSpacingEqual = 1,
  SpvExecutionModeSpacingFractionalEven = 2,
  SpvExecutionModeSpacingFractionalOdd = 3,
  SpvExecutionModeVertexOrderCw = 4,
  SpvExecutionModeVertexOrderCcw = 5,
  SpvExecutionModePixelCenterInteger = 6,
  SpvExecutionModeOriginUpperLeft = 7,
  SpvExecutionModeOriginLowerLeft = 8,
  SpvExecutionModeEarlyFragmentTests = 9,
  SpvExecutionModePointMode = 10,
  SpvExecutionModeXfb = 11,
  SpvExecutionModeDepthReplacing = 12,
  SpvExecutionModeDepthGreater = 14,
  SpvExecutionModeDepthLess = 15,
  SpvExecutionModeDepthUnchanged = 16,
  SpvExecutionModeLocalSize = 17,
  SpvExecutionModeLocalSizeHint = 18,
  SpvExecutionModeInputPoints = 19,
  SpvExecutionModeInputLines = 20,
  SpvExecutionModeInputLinesAdjacency = 21,
  SpvExecutionModeTriangles = 22,
  SpvExecutionModeInputTrianglesAdjacency = 23,
  SpvExecutionModeQuads = 24,
  SpvExecutionModeIsolines = 25,
  SpvExecutionModeOutputVertices = 26,
  SpvExecutionModeOutputPoints = 27,
  SpvExecutionModeOutputLineStrip = 28,
  SpvExecutionModeOutputTriangleStrip = 29,
  SpvExecutionModeVecTypeHint = 30,
  SpvExecutionModeContractionOff = 31,
  SpvExecutionModePostDepthCoverage = 4446,
  SpvExecutionModeStencilRefReplacingEXT = 5027,
  SpvExecutionModeMax = 0x7fffffff,
} SpvExecutionMode;

typedef enum SpvStorageClass_ {
    SpvStorageClassUniformConstant = 0,
    SpvStorageClassInput = 1,
    SpvStorageClassUniform = 2,
    SpvStorageClassOutput = 3,
    SpvStorageClassWorkgroup = 4,
    SpvStorageClassCrossWorkgroup = 5,
    SpvStorageClassPrivate = 6,
    SpvStorageClassFunction = 7,
    SpvStorageClassGeneric = 8,
    SpvStorageClassPushConstant = 9,
    SpvStorageClassAtomicCounter = 10,
    SpvStorageClassImage = 11,
    SpvStorageClassStorageBuffer = 12,
    SpvStorageClassMax = 0x7fffffff,
} SpvStorageClass;

typedef enum SpvDim_ {
    SpvDim1D = 0,
    SpvDim2D = 1,
    SpvDim3D = 2,
    SpvDimCube = 3,
    SpvDimRect = 4,
    SpvDimBuffer = 5,
    SpvDimSubpassData = 6,
    SpvDimMax = 0x7fffffff,
} SpvDim;

typedef enum SpvSamplerAddressingMode_ {
    SpvSamplerAddressingModeNone = 0,
    SpvSamplerAddressingModeClampToEdge = 1,
    SpvSamplerAddressingModeClamp = 2,
    SpvSamplerAddressingModeRepeat = 3,
    SpvSamplerAddressingModeRepeatMirrored = 4,
    SpvSamplerAddressingModeMax = 0x7fffffff,
} SpvSamplerAddressingMode;

typedef enum SpvSamplerFilterMode_ {
    SpvSamplerFilterModeNearest = 0,
    SpvSamplerFilterModeLinear = 1,
    SpvSamplerFilterModeMax = 0x7fffffff,
} SpvSamplerFilterMode;

typedef enum SpvImageFormat_ {
    SpvImageFormatUnknown = 0,
    SpvImageFormatRgba32f = 1,
    SpvImageFormatRgba16f = 2,
    SpvImageFormatR32f = 3,
    SpvImageFormatRgba8 = 4,
    SpvImageFormatRgba8Snorm = 5,
    SpvImageFormatRg32f = 6,
    SpvImageFormatRg16f = 7,
    SpvImageFormatR11fG11fB10f = 8,
    SpvImageFormatR16f = 9,
    SpvImageFormatRgba16 = 10,
    SpvImageFormatRgb10A2 = 11,
    SpvImageFormatRg16 = 12,
    SpvImageFormatRg8 = 13,
    SpvImageFormatR16 = 14,
    SpvImageFormatR8 = 15,
    SpvImageFormatRgba16Snorm = 16,
    SpvImageFormatRg16Snorm = 17,
    SpvImageFormatRg8Snorm = 18,
    SpvImageFormatR16Snorm = 19,
    SpvImageFormatR8Snorm = 20,
    SpvImageFormatRgba32i = 21,
    SpvImageFormatRgba16i = 22,
    SpvImageFormatRgba8i = 23,
    SpvImageFormatR32i = 24,
    SpvImageFormatRg32i = 25,
    SpvImageFormatRg16i = 26,
    SpvImageFormatRg8i = 27,
    SpvImageFormatR16i = 28,
    SpvImageFormatR8i = 29,
    SpvImageFormatRgba32ui = 30,
    SpvImageFormatRgba16ui = 31,
    SpvImageFormatRgba8ui = 32,
    SpvImageFormatR32ui = 33,
    SpvImageFormatRgb10a2ui = 34,
    SpvImageFormatRg32ui = 35,
    SpvImageFormatRg16ui = 36,
    SpvImageFormatRg8ui = 37,
    SpvImageFormatR16ui = 38,
    SpvImageFormatR8ui = 39,
    SpvImageFormatMax = 0x7fffffff,
} SpvImageFormat;

typedef enum SpvImageChannelOrder_ {
    SpvImageChannelOrderR = 0,
    SpvImageChannelOrderA = 1,
    SpvImageChannelOrderRG = 2,
    SpvImageChannelOrderRA = 3,
    SpvImageChannelOrderRGB = 4,
    SpvImageChannelOrderRGBA = 5,
    SpvImageChannelOrderBGRA = 6,
    SpvImageChannelOrderARGB = 7,
    SpvImageChannelOrderIntensity = 8,
    SpvImageChannelOrderLuminance = 9,
    SpvImageChannelOrderRx = 10,
    SpvImageChannelOrderRGx = 11,
    SpvImageChannelOrderRGBx = 12,
    SpvImageChannelOrderDepth = 13,
    SpvImageChannelOrderDepthStencil = 14,
    SpvImageChannelOrdersRGB = 15,
    SpvImageChannelOrdersRGBx = 16,
    SpvImageChannelOrdersRGBA = 17,
    SpvImageChannelOrdersBGRA = 18,
    SpvImageChannelOrderABGR = 19,
    SpvImageChannelOrderMax = 0x7fffffff,
} SpvImageChannelOrder;

typedef enum SpvImageChannelDataType_ {
    SpvImageChannelDataTypeSnormInt8 = 0,
    SpvImageChannelDataTypeSnormInt16 = 1,
    SpvImageChannelDataTypeUnormInt8 = 2,
    SpvImageChannelDataTypeUnormInt16 = 3,
    SpvImageChannelDataTypeUnormShort565 = 4,
    SpvImageChannelDataTypeUnormShort555 = 5,
    SpvImageChannelDataTypeUnormInt101010 = 6,
    SpvImageChannelDataTypeSignedInt8 = 7,
    SpvImageChannelDataTypeSignedInt16 = 8,
    SpvImageChannelDataTypeSignedInt32 = 9,
    SpvImageChannelDataTypeUnsignedInt8 = 10,
    SpvImageChannelDataTypeUnsignedInt16 = 11,
    SpvImageChannelDataTypeUnsignedInt32 = 12,
    SpvImageChannelDataTypeHalfFloat = 13,
    SpvImageChannelDataTypeFloat = 14,
    SpvImageChannelDataTypeUnormInt24 = 15,
    SpvImageChannelDataTypeUnormInt101010_2 = 16,
    SpvImageChannelDataTypeMax = 0x7fffffff,
} SpvImageChannelDataType;

typedef enum SpvImageOperandsShift_ {
    SpvImageOperandsBiasShift = 0,
    SpvImageOperandsLodShift = 1,
    SpvImageOperandsGradShift = 2,
    SpvImageOperandsConstOffsetShift = 3,
    SpvImageOperandsOffsetShift = 4,
    SpvImageOperandsConstOffsetsShift = 5,
    SpvImageOperandsSampleShift = 6,
    SpvImageOperandsMinLodShift = 7,
    SpvImageOperandsMax = 0x7fffffff,
} SpvImageOperandsShift;

typedef enum SpvImageOperandsMask_ {
    SpvImageOperandsMaskNone = 0,
    SpvImageOperandsBiasMask = 0x00000001,
    SpvImageOperandsLodMask = 0x00000002,
    SpvImageOperandsGradMask = 0x00000004,
    SpvImageOperandsConstOffsetMask = 0x00000008,
    SpvImageOperandsOffsetMask = 0x00000010,
    SpvImageOperandsConstOffsetsMask = 0x00000020,
    SpvImageOperandsSampleMask = 0x00000040,
    SpvImageOperandsMinLodMask = 0x00000080,
} SpvImageOperandsMask;

typedef enum SpvFPFastMathModeShift_ {
    SpvFPFastMathModeNotNaNShift = 0,
    SpvFPFastMathModeNotInfShift = 1,
    SpvFPFastMathModeNSZShift = 2,
    SpvFPFastMathModeAllowRecipShift = 3,
    SpvFPFastMathModeFastShift = 4,
    SpvFPFastMathModeMax = 0x7fffffff,
} SpvFPFastMathModeShift;

typedef enum SpvFPFastMathModeMask_ {
    SpvFPFastMathModeMaskNone = 0,
    SpvFPFastMathModeNotNaNMask = 0x00000001,
    SpvFPFastMathModeNotInfMask = 0x00000002,
    SpvFPFastMathModeNSZMask = 0x00000004,
    SpvFPFastMathModeAllowRecipMask = 0x00000008,
    SpvFPFastMathModeFastMask = 0x00000010,
} SpvFPFastMathModeMask;

typedef enum SpvFPRoundingMode_ {
    SpvFPRoundingModeRTE = 0,
    SpvFPRoundingModeRTZ = 1,
    SpvFPRoundingModeRTP = 2,
    SpvFPRoundingModeRTN = 3,
    SpvFPRoundingModeMax = 0x7fffffff,
} SpvFPRoundingMode;

typedef enum SpvLinkageType_ {
    SpvLinkageTypeExport = 0,
    SpvLinkageTypeImport = 1,
    SpvLinkageTypeMax = 0x7fffffff,
} SpvLinkageType;

typedef enum SpvAccessQualifier_ {
    SpvAccessQualifierReadOnly = 0,
    SpvAccessQualifierWriteOnly = 1,
    SpvAccessQualifierReadWrite = 2,
    SpvAccessQualifierMax = 0x7fffffff,
} SpvAccessQualifier;

typedef enum SpvFunctionParameterAttribute_ {
    SpvFunctionParameterAttributeZext = 0,
    SpvFunctionParameterAttributeSext = 1,
    SpvFunctionParameterAttributeByVal = 2,
    SpvFunctionParameterAttributeSret = 3,
    SpvFunctionParameterAttributeNoAlias = 4,
    SpvFunctionParameterAttributeNoCapture = 5,
    SpvFunctionParameterAttributeNoWrite = 6,
    SpvFunctionParameterAttributeNoReadWrite = 7,
    SpvFunctionParameterAttributeMax = 0x7fffffff,
} SpvFunctionParameterAttribute;

typedef enum SpvDecoration_ {
    SpvDecorationRelaxedPrecision = 0,
    SpvDecorationSpecId = 1,
    SpvDecorationBlock = 2,
    SpvDecorationBufferBlock = 3,
    SpvDecorationRowMajor = 4,
    SpvDecorationColMajor = 5,
    SpvDecorationArrayStride = 6,
    SpvDecorationMatrixStride = 7,
    SpvDecorationGLSLShared = 8,
    SpvDecorationGLSLPacked = 9,
    SpvDecorationCPacked = 10,
    SpvDecorationBuiltIn = 11,
    SpvDecorationNoPerspective = 13,
    SpvDecorationFlat = 14,
    SpvDecorationPatch = 15,
    SpvDecorationCentroid = 16,
    SpvDecorationSample = 17,
    SpvDecorationInvariant = 18,
    SpvDecorationRestrict = 19,
    SpvDecorationAliased = 20,
    SpvDecorationVolatile = 21,
    SpvDecorationConstant = 22,
    SpvDecorationCoherent = 23,
    SpvDecorationNonWritable = 24,
    SpvDecorationNonReadable = 25,
    SpvDecorationUniform = 26,
    SpvDecorationSaturatedConversion = 28,
    SpvDecorationStream = 29,
    SpvDecorationLocation = 30,
    SpvDecorationComponent = 31,
    SpvDecorationIndex = 32,
    SpvDecorationBinding = 33,
    SpvDecorationDescriptorSet = 34,
    SpvDecorationOffset = 35,
    SpvDecorationXfbBuffer = 36,
    SpvDecorationXfbStride = 37,
    SpvDecorationFuncParamAttr = 38,
    SpvDecorationFPRoundingMode = 39,
    SpvDecorationFPFastMathMode = 40,
    SpvDecorationLinkageAttributes = 41,
    SpvDecorationNoContraction = 42,
    SpvDecorationInputAttachmentIndex = 43,
    SpvDecorationAlignment = 44,
    SpvDecorationExplicitInterpAMD = 4999,
    SpvDecorationOverrideCoverageNV = 5248,
    SpvDecorationPassthroughNV = 5250,
    SpvDecorationViewportRelativeNV = 5252,
    SpvDecorationSecondaryViewportRelativeNV = 5256,
    SpvDecorationHlslCounterBufferGOOGLE = 5634,
    SpvDecorationHlslSemanticGOOGLE = 5635,
    SpvDecorationMax = 0x7fffffff,
} SpvDecoration;

typedef enum SpvBuiltIn_ {
    SpvBuiltInPosition = 0,
    SpvBuiltInPointSize = 1,
    SpvBuiltInClipDistance = 3,
    SpvBuiltInCullDistance = 4,
    SpvBuiltInVertexId = 5,
    SpvBuiltInInstanceId = 6,
    SpvBuiltInPrimitiveId = 7,
    SpvBuiltInInvocationId = 8,
    SpvBuiltInLayer = 9,
    SpvBuiltInViewportIndex = 10,
    SpvBuiltInTessLevelOuter = 11,
    SpvBuiltInTessLevelInner = 12,
    SpvBuiltInTessCoord = 13,
    SpvBuiltInPatchVertices = 14,
    SpvBuiltInFragCoord = 15,
    SpvBuiltInPointCoord = 16,
    SpvBuiltInFrontFacing = 17,
    SpvBuiltInSampleId = 18,
    SpvBuiltInSamplePosition = 19,
    SpvBuiltInSampleMask = 20,
    SpvBuiltInFragDepth = 22,
    SpvBuiltInHelperInvocation = 23,
    SpvBuiltInNumWorkgroups = 24,
    SpvBuiltInWorkgroupSize = 25,
    SpvBuiltInWorkgroupId = 26,
    SpvBuiltInLocalInvocationId = 27,
    SpvBuiltInGlobalInvocationId = 28,
    SpvBuiltInLocalInvocationIndex = 29,
    SpvBuiltInWorkDim = 30,
    SpvBuiltInGlobalSize = 31,
    SpvBuiltInEnqueuedWorkgroupSize = 32,
    SpvBuiltInGlobalOffset = 33,
    SpvBuiltInGlobalLinearId = 34,
    SpvBuiltInSubgroupSize = 36,
    SpvBuiltInSubgroupMaxSize = 37,
    SpvBuiltInNumSubgroups = 38,
    SpvBuiltInNumEnqueuedSubgroups = 39,
    SpvBuiltInSubgroupId = 40,
    SpvBuiltInSubgroupLocalInvocationId = 41,
    SpvBuiltInVertexIndex = 42,
    SpvBuiltInInstanceIndex = 43,
    SpvBuiltInSubgroupEqMaskKHR = 4416,
    SpvBuiltInSubgroupGeMaskKHR = 4417,
    SpvBuiltInSubgroupGtMaskKHR = 4418,
    SpvBuiltInSubgroupLeMaskKHR = 4419,
    SpvBuiltInSubgroupLtMaskKHR = 4420,
    SpvBuiltInBaseVertex = 4424,
    SpvBuiltInBaseInstance = 4425,
    SpvBuiltInDrawIndex = 4426,
    SpvBuiltInDeviceIndex = 4438,
    SpvBuiltInViewIndex = 4440,
    SpvBuiltInBaryCoordNoPerspAMD = 4992,
    SpvBuiltInBaryCoordNoPerspCentroidAMD = 4993,
    SpvBuiltInBaryCoordNoPerspSampleAMD = 4994,
    SpvBuiltInBaryCoordSmoothAMD = 4995,
    SpvBuiltInBaryCoordSmoothCentroidAMD = 4996,
    SpvBuiltInBaryCoordSmoothSampleAMD = 4997,
    SpvBuiltInBaryCoordPullModelAMD = 4998,
    SpvBuiltInFragStencilRefEXT = 5014,
    SpvBuiltInViewportMaskNV = 5253,
    SpvBuiltInSecondaryPositionNV = 5257,
    SpvBuiltInSecondaryViewportMaskNV = 5258,
    SpvBuiltInPositionPerViewNV = 5261,
    SpvBuiltInViewportMaskPerViewNV = 5262,
    SpvBuiltInMax = 0x7fffffff,
} SpvBuiltIn;

typedef enum SpvSelectionControlShift_ {
    SpvSelectionControlFlattenShift = 0,
    SpvSelectionControlDontFlattenShift = 1,
    SpvSelectionControlMax = 0x7fffffff,
} SpvSelectionControlShift;

typedef enum SpvSelectionControlMask_ {
    SpvSelectionControlMaskNone = 0,
    SpvSelectionControlFlattenMask = 0x00000001,
    SpvSelectionControlDontFlattenMask = 0x00000002,
} SpvSelectionControlMask;

typedef enum SpvLoopControlShift_ {
    SpvLoopControlUnrollShift = 0,
    SpvLoopControlDontUnrollShift = 1,
    SpvLoopControlMax = 0x7fffffff,
} SpvLoopControlShift;

typedef enum SpvLoopControlMask_ {
    SpvLoopControlMaskNone = 0,
    SpvLoopControlUnrollMask = 0x00000001,
    SpvLoopControlDontUnrollMask = 0x00000002,
} SpvLoopControlMask;

typedef enum SpvFunctionControlShift_ {
    SpvFunctionControlInlineShift = 0,
    SpvFunctionControlDontInlineShift = 1,
    SpvFunctionControlPureShift = 2,
    SpvFunctionControlConstShift = 3,
    SpvFunctionControlMax = 0x7fffffff,
} SpvFunctionControlShift;

typedef enum SpvFunctionControlMask_ {
    SpvFunctionControlMaskNone = 0,
    SpvFunctionControlInlineMask = 0x00000001,
    SpvFunctionControlDontInlineMask = 0x00000002,
    SpvFunctionControlPureMask = 0x00000004,
    SpvFunctionControlConstMask = 0x00000008,
} SpvFunctionControlMask;

typedef enum SpvMemorySemanticsShift_ {
    SpvMemorySemanticsAcquireShift = 1,
    SpvMemorySemanticsReleaseShift = 2,
    SpvMemorySemanticsAcquireReleaseShift = 3,
    SpvMemorySemanticsSequentiallyConsistentShift = 4,
    SpvMemorySemanticsUniformMemoryShift = 6,
    SpvMemorySemanticsSubgroupMemoryShift = 7,
    SpvMemorySemanticsWorkgroupMemoryShift = 8,
    SpvMemorySemanticsCrossWorkgroupMemoryShift = 9,
    SpvMemorySemanticsAtomicCounterMemoryShift = 10,
    SpvMemorySemanticsImageMemoryShift = 11,
    SpvMemorySemanticsMax = 0x7fffffff,
} SpvMemorySemanticsShift;

typedef enum SpvMemorySemanticsMask_ {
    SpvMemorySemanticsMaskNone = 0,
    SpvMemorySemanticsAcquireMask = 0x00000002,
    SpvMemorySemanticsReleaseMask = 0x00000004,
    SpvMemorySemanticsAcquireReleaseMask = 0x00000008,
    SpvMemorySemanticsSequentiallyConsistentMask = 0x00000010,
    SpvMemorySemanticsUniformMemoryMask = 0x00000040,
    SpvMemorySemanticsSubgroupMemoryMask = 0x00000080,
    SpvMemorySemanticsWorkgroupMemoryMask = 0x00000100,
    SpvMemorySemanticsCrossWorkgroupMemoryMask = 0x00000200,
    SpvMemorySemanticsAtomicCounterMemoryMask = 0x00000400,
    SpvMemorySemanticsImageMemoryMask = 0x00000800,
} SpvMemorySemanticsMask;

typedef enum SpvMemoryAccessShift_ {
    SpvMemoryAccessVolatileShift = 0,
    SpvMemoryAccessAlignedShift = 1,
    SpvMemoryAccessNontemporalShift = 2,
    SpvMemoryAccessMax = 0x7fffffff,
} SpvMemoryAccessShift;

typedef enum SpvMemoryAccessMask_ {
    SpvMemoryAccessMaskNone = 0,
    SpvMemoryAccessVolatileMask = 0x00000001,
    SpvMemoryAccessAlignedMask = 0x00000002,
    SpvMemoryAccessNontemporalMask = 0x00000004,
} SpvMemoryAccessMask;

typedef enum SpvScope_ {
    SpvScopeCrossDevice = 0,
    SpvScopeDevice = 1,
    SpvScopeWorkgroup = 2,
    SpvScopeSubgroup = 3,
    SpvScopeInvocation = 4,
    SpvScopeMax = 0x7fffffff,
} SpvScope;

typedef enum SpvGroupOperation_ {
    SpvGroupOperationReduce = 0,
    SpvGroupOperationInclusiveScan = 1,
    SpvGroupOperationExclusiveScan = 2,
    SpvGroupOperationMax = 0x7fffffff,
} SpvGroupOperation;

typedef enum SpvKernelEnqueueFlags_ {
    SpvKernelEnqueueFlagsNoWait = 0,
    SpvKernelEnqueueFlagsWaitKernel = 1,
    SpvKernelEnqueueFlagsWaitWorkGroup = 2,
    SpvKernelEnqueueFlagsMax = 0x7fffffff,
} SpvKernelEnqueueFlags;

typedef enum SpvKernelProfilingInfoShift_ {
    SpvKernelProfilingInfoCmdExecTimeShift = 0,
    SpvKernelProfilingInfoMax = 0x7fffffff,
} SpvKernelProfilingInfoShift;

typedef enum SpvKernelProfilingInfoMask_ {
    SpvKernelProfilingInfoMaskNone = 0,
    SpvKernelProfilingInfoCmdExecTimeMask = 0x00000001,
} SpvKernelProfilingInfoMask;

typedef enum SpvCapability_ {
    SpvCapabilityMatrix = 0,
    SpvCapabilityShader = 1,
    SpvCapabilityGeometry = 2,
    SpvCapabilityTessellation = 3,
    SpvCapabilityAddresses = 4,
    SpvCapabilityLinkage = 5,
    SpvCapabilityKernel = 6,
    SpvCapabilityVector16 = 7,
    SpvCapabilityFloat16Buffer = 8,
    SpvCapabilityFloat16 = 9,
    SpvCapabilityFloat64 = 10,
    SpvCapabilityInt64 = 11,
    SpvCapabilityInt64Atomics = 12,
    SpvCapabilityImageBasic = 13,
    SpvCapabilityImageReadWrite = 14,
    SpvCapabilityImageMipmap = 15,
    SpvCapabilityPipes = 17,
    SpvCapabilityGroups = 18,
    SpvCapabilityDeviceEnqueue = 19,
    SpvCapabilityLiteralSampler = 20,
    SpvCapabilityAtomicStorage = 21,
    SpvCapabilityInt16 = 22,
    SpvCapabilityTessellationPointSize = 23,
    SpvCapabilityGeometryPointSize = 24,
    SpvCapabilityImageGatherExtended = 25,
    SpvCapabilityStorageImageMultisample = 27,
    SpvCapabilityUniformBufferArrayDynamicIndexing = 28,
    SpvCapabilitySampledImageArrayDynamicIndexing = 29,
    SpvCapabilityStorageBufferArrayDynamicIndexing = 30,
    SpvCapabilityStorageImageArrayDynamicIndexing = 31,
    SpvCapabilityClipDistance = 32,
    SpvCapabilityCullDistance = 33,
    SpvCapabilityImageCubeArray = 34,
    SpvCapabilitySampleRateShading = 35,
    SpvCapabilityImageRect = 36,
    SpvCapabilitySampledRect = 37,
    SpvCapabilityGenericPointer = 38,
    SpvCapabilityInt8 = 39,
    SpvCapabilityInputAttachment = 40,
    SpvCapabilitySparseResidency = 41,
    SpvCapabilityMinLod = 42,
    SpvCapabilitySampled1D = 43,
    SpvCapabilityImage1D = 44,
    SpvCapabilitySampledCubeArray = 45,
    SpvCapabilitySampledBuffer = 46,
    SpvCapabilityImageBuffer = 47,
    SpvCapabilityImageMSArray = 48,
    SpvCapabilityStorageImageExtendedFormats = 49,
    SpvCapabilityImageQuery = 50,
    SpvCapabilityDerivativeControl = 51,
    SpvCapabilityInterpolationFunction = 52,
    SpvCapabilityTransformFeedback = 53,
    SpvCapabilityGeometryStreams = 54,
    SpvCapabilityStorageImageReadWithoutFormat = 55,
    SpvCapabilityStorageImageWriteWithoutFormat = 56,
    SpvCapabilityMultiViewport = 57,
    SpvCapabilitySubgroupBallotKHR = 4423,
    SpvCapabilityDrawParameters = 4427,
    SpvCapabilitySubgroupVoteKHR = 4431,
    SpvCapabilityStorageBuffer16BitAccess = 4433,
    SpvCapabilityStorageUniformBufferBlock16 = 4433,
    SpvCapabilityStorageUniform16 = 4434,
    SpvCapabilityUniformAndStorageBuffer16BitAccess = 4434,
    SpvCapabilityStoragePushConstant16 = 4435,
    SpvCapabilityStorageInputOutput16 = 4436,
    SpvCapabilityDeviceGroup = 4437,
    SpvCapabilityMultiView = 4439,
    SpvCapabilityVariablePointersStorageBuffer = 4441,
    SpvCapabilityVariablePointers = 4442,
    SpvCapabilityAtomicStorageOps = 4445,
    SpvCapabilitySampleMaskPostDepthCoverage = 4447,
    SpvCapabilityImageGatherBiasLodAMD = 5009,
    SpvCapabilityFragmentMaskAMD = 5010,
    SpvCapabilityStencilExportEXT = 5013,
    SpvCapabilityImageReadWriteLodAMD = 5015,
    SpvCapabilitySampleMaskOverrideCoverageNV = 5249,
    SpvCapabilityGeometryShaderPassthroughNV = 5251,
    SpvCapabilityShaderViewportIndexLayerEXT = 5254,
    SpvCapabilityShaderViewportIndexLayerNV = 5254,
    SpvCapabilityShaderViewportMaskNV = 5255,
    SpvCapabilityShaderStereoViewNV = 5259,
    SpvCapabilityPerViewAttributesNV = 5260,
    SpvCapabilitySubgroupShuffleINTEL = 5568,
    SpvCapabilitySubgroupBufferBlockIOINTEL = 5569,
    SpvCapabilitySubgroupImageBlockIOINTEL = 5570,
    SpvCapabilityMax = 0x7fffffff,
} SpvCapability;

typedef enum SpvOp_ {
    SpvOpNop = 0,
    SpvOpUndef = 1,
    SpvOpSourceContinued = 2,
    SpvOpSource = 3,
    SpvOpSourceExtension = 4,
    SpvOpName = 5,
    SpvOpMemberName = 6,
    SpvOpString = 7,
    SpvOpLine = 8,
    SpvOpExtension = 10,
    SpvOpExtInstImport = 11,
    SpvOpExtInst = 12,
    SpvOpMemoryModel = 14,
    SpvOpEntryPoint = 15,
    SpvOpExecutionMode = 16,
    SpvOpCapability = 17,
    SpvOpTypeVoid = 19,
    SpvOpTypeBool = 20,
    SpvOpTypeInt = 21,
    SpvOpTypeFloat = 22,
    SpvOpTypeVector = 23,
    SpvOpTypeMatrix = 24,
    SpvOpTypeImage = 25,
    SpvOpTypeSampler = 26,
    SpvOpTypeSampledImage = 27,
    SpvOpTypeArray = 28,
    SpvOpTypeRuntimeArray = 29,
    SpvOpTypeStruct = 30,
    SpvOpTypeOpaque = 31,
    SpvOpTypePointer = 32,
    SpvOpTypeFunction = 33,
    SpvOpTypeEvent = 34,
    SpvOpTypeDeviceEvent = 35,
    SpvOpTypeReserveId = 36,
    SpvOpTypeQueue = 37,
    SpvOpTypePipe = 38,
    SpvOpTypeForwardPointer = 39,
    SpvOpConstantTrue = 41,
    SpvOpConstantFalse = 42,
    SpvOpConstant = 43,
    SpvOpConstantComposite = 44,
    SpvOpConstantSampler = 45,
    SpvOpConstantNull = 46,
    SpvOpSpecConstantTrue = 48,
    SpvOpSpecConstantFalse = 49,
    SpvOpSpecConstant = 50,
    SpvOpSpecConstantComposite = 51,
    SpvOpSpecConstantOp = 52,
    SpvOpFunction = 54,
    SpvOpFunctionParameter = 55,
    SpvOpFunctionEnd = 56,
    SpvOpFunctionCall = 57,
    SpvOpVariable = 59,
    SpvOpImageTexelPointer = 60,
    SpvOpLoad = 61,
    SpvOpStore = 62,
    SpvOpCopyMemory = 63,
    SpvOpCopyMemorySized = 64,
    SpvOpAccessChain = 65,
    SpvOpInBoundsAccessChain = 66,
    SpvOpPtrAccessChain = 67,
    SpvOpArrayLength = 68,
    SpvOpGenericPtrMemSemantics = 69,
    SpvOpInBoundsPtrAccessChain = 70,
    SpvOpDecorate = 71,
    SpvOpMemberDecorate = 72,
    SpvOpDecorationGroup = 73,
    SpvOpGroupDecorate = 74,
    SpvOpGroupMemberDecorate = 75,
    SpvOpVectorExtractDynamic = 77,
    SpvOpVectorInsertDynamic = 78,
    SpvOpVectorShuffle = 79,
    SpvOpCompositeConstruct = 80,
    SpvOpCompositeExtract = 81,
    SpvOpCompositeInsert = 82,
    SpvOpCopyObject = 83,
    SpvOpTranspose = 84,
    SpvOpSampledImage = 86,
    SpvOpImageSampleImplicitLod = 87,
    SpvOpImageSampleExplicitLod = 88,
    SpvOpImageSampleDrefImplicitLod = 89,
    SpvOpImageSampleDrefExplicitLod = 90,
    SpvOpImageSampleProjImplicitLod = 91,
    SpvOpImageSampleProjExplicitLod = 92,
    SpvOpImageSampleProjDrefImplicitLod = 93,
    SpvOpImageSampleProjDrefExplicitLod = 94,
    SpvOpImageFetch = 95,
    SpvOpImageGather = 96,
    SpvOpImageDrefGather = 97,
    SpvOpImageRead = 98,
    SpvOpImageWrite = 99,
    SpvOpImage = 100,
    SpvOpImageQueryFormat = 101,
    SpvOpImageQueryOrder = 102,
    SpvOpImageQuerySizeLod = 103,
    SpvOpImageQuerySize = 104,
    SpvOpImageQueryLod = 105,
    SpvOpImageQueryLevels = 106,
    SpvOpImageQuerySamples = 107,
    SpvOpConvertFToU = 109,
    SpvOpConvertFToS = 110,
    SpvOpConvertSToF = 111,
    SpvOpConvertUToF = 112,
    SpvOpUConvert = 113,
    SpvOpSConvert = 114,
    SpvOpFConvert = 115,
    SpvOpQuantizeToF16 = 116,
    SpvOpConvertPtrToU = 117,
    SpvOpSatConvertSToU = 118,
    SpvOpSatConvertUToS = 119,
    SpvOpConvertUToPtr = 120,
    SpvOpPtrCastToGeneric = 121,
    SpvOpGenericCastToPtr = 122,
    SpvOpGenericCastToPtrExplicit = 123,
    SpvOpBitcast = 124,
    SpvOpSNegate = 126,
    SpvOpFNegate = 127,
    SpvOpIAdd = 128,
    SpvOpFAdd = 129,
    SpvOpISub = 130,
    SpvOpFSub = 131,
    SpvOpIMul = 132,
    SpvOpFMul = 133,
    SpvOpUDiv = 134,
    SpvOpSDiv = 135,
    SpvOpFDiv = 136,
    SpvOpUMod = 137,
    SpvOpSRem = 138,
    SpvOpSMod = 139,
    SpvOpFRem = 140,
    SpvOpFMod = 141,
    SpvOpVectorTimesScalar = 142,
    SpvOpMatrixTimesScalar = 143,
    SpvOpVectorTimesMatrix = 144,
    SpvOpMatrixTimesVector = 145,
    SpvOpMatrixTimesMatrix = 146,
    SpvOpOuterProduct = 147,
    SpvOpDot = 148,
    SpvOpIAddCarry = 149,
    SpvOpISubBorrow = 150,
    SpvOpUMulExtended = 151,
    SpvOpSMulExtended = 152,
    SpvOpAny = 154,
    SpvOpAll = 155,
    SpvOpIsNan = 156,
    SpvOpIsInf = 157,
    SpvOpIsFinite = 158,
    SpvOpIsNormal = 159,
    SpvOpSignBitSet = 160,
    SpvOpLessOrGreater = 161,
    SpvOpOrdered = 162,
    SpvOpUnordered = 163,
    SpvOpLogicalEqual = 164,
    SpvOpLogicalNotEqual = 165,
    SpvOpLogicalOr = 166,
    SpvOpLogicalAnd = 167,
    SpvOpLogicalNot = 168,
    SpvOpSelect = 169,
    SpvOpIEqual = 170,
    SpvOpINotEqual = 171,
    SpvOpUGreaterThan = 172,
    SpvOpSGreaterThan = 173,
    SpvOpUGreaterThanEqual = 174,
    SpvOpSGreaterThanEqual = 175,
    SpvOpULessThan = 176,
    SpvOpSLessThan = 177,
    SpvOpULessThanEqual = 178,
    SpvOpSLessThanEqual = 179,
    SpvOpFOrdEqual = 180,
    SpvOpFUnordEqual = 181,
    SpvOpFOrdNotEqual = 182,
    SpvOpFUnordNotEqual = 183,
    SpvOpFOrdLessThan = 184,
    SpvOpFUnordLessThan = 185,
    SpvOpFOrdGreaterThan = 186,
    SpvOpFUnordGreaterThan = 187,
    SpvOpFOrdLessThanEqual = 188,
    SpvOpFUnordLessThanEqual = 189,
    SpvOpFOrdGreaterThanEqual = 190,
    SpvOpFUnordGreaterThanEqual = 191,
    SpvOpShiftRightLogical = 194,
    SpvOpShiftRightArithmetic = 195,
    SpvOpShiftLeftLogical = 196,
    SpvOpBitwiseOr = 197,
    SpvOpBitwiseXor = 198,
    SpvOpBitwiseAnd = 199,
    SpvOpNot = 200,
    SpvOpBitFieldInsert = 201,
    SpvOpBitFieldSExtract = 202,
    SpvOpBitFieldUExtract = 203,
    SpvOpBitReverse = 204,
    SpvOpBitCount = 205,
    SpvOpDPdx = 207,
    SpvOpDPdy = 208,
    SpvOpFwidth = 209,
    SpvOpDPdxFine = 210,
    SpvOpDPdyFine = 211,
    SpvOpFwidthFine = 212,
    SpvOpDPdxCoarse = 213,
    SpvOpDPdyCoarse = 214,
    SpvOpFwidthCoarse = 215,
    SpvOpEmitVertex = 218,
    SpvOpEndPrimitive = 219,
    SpvOpEmitStreamVertex = 220,
    SpvOpEndStreamPrimitive = 221,
    SpvOpControlBarrier = 224,
    SpvOpMemoryBarrier = 225,
    SpvOpAtomicLoad = 227,
    SpvOpAtomicStore = 228,
    SpvOpAtomicExchange = 229,
    SpvOpAtomicCompareExchange = 230,
    SpvOpAtomicCompareExchangeWeak = 231,
    SpvOpAtomicIIncrement = 232,
    SpvOpAtomicIDecrement = 233,
    SpvOpAtomicIAdd = 234,
    SpvOpAtomicISub = 235,
    SpvOpAtomicSMin = 236,
    SpvOpAtomicUMin = 237,
    SpvOpAtomicSMax = 238,
    SpvOpAtomicUMax = 239,
    SpvOpAtomicAnd = 240,
    SpvOpAtomicOr = 241,
    SpvOpAtomicXor = 242,
    SpvOpPhi = 245,
    SpvOpLoopMerge = 246,
    SpvOpSelectionMerge = 247,
    SpvOpLabel = 248,
    SpvOpBranch = 249,
    SpvOpBranchConditional = 250,
    SpvOpSwitch = 251,
    SpvOpKill = 252,
    SpvOpReturn = 253,
    SpvOpReturnValue = 254,
    SpvOpUnreachable = 255,
    SpvOpLifetimeStart = 256,
    SpvOpLifetimeStop = 257,
    SpvOpGroupAsyncCopy = 259,
    SpvOpGroupWaitEvents = 260,
    SpvOpGroupAll = 261,
    SpvOpGroupAny = 262,
    SpvOpGroupBroadcast = 263,
    SpvOpGroupIAdd = 264,
    SpvOpGroupFAdd = 265,
    SpvOpGroupFMin = 266,
    SpvOpGroupUMin = 267,
    SpvOpGroupSMin = 268,
    SpvOpGroupFMax = 269,
    SpvOpGroupUMax = 270,
    SpvOpGroupSMax = 271,
    SpvOpReadPipe = 274,
    SpvOpWritePipe = 275,
    SpvOpReservedReadPipe = 276,
    SpvOpReservedWritePipe = 277,
    SpvOpReserveReadPipePackets = 278,
    SpvOpReserveWritePipePackets = 279,
    SpvOpCommitReadPipe = 280,
    SpvOpCommitWritePipe = 281,
    SpvOpIsValidReserveId = 282,
    SpvOpGetNumPipePackets = 283,
    SpvOpGetMaxPipePackets = 284,
    SpvOpGroupReserveReadPipePackets = 285,
    SpvOpGroupReserveWritePipePackets = 286,
    SpvOpGroupCommitReadPipe = 287,
    SpvOpGroupCommitWritePipe = 288,
    SpvOpEnqueueMarker = 291,
    SpvOpEnqueueKernel = 292,
    SpvOpGetKernelNDrangeSubGroupCount = 293,
    SpvOpGetKernelNDrangeMaxSubGroupSize = 294,
    SpvOpGetKernelWorkGroupSize = 295,
    SpvOpGetKernelPreferredWorkGroupSizeMultiple = 296,
    SpvOpRetainEvent = 297,
    SpvOpReleaseEvent = 298,
    SpvOpCreateUserEvent = 299,
    SpvOpIsValidEvent = 300,
    SpvOpSetUserEventStatus = 301,
    SpvOpCaptureEventProfilingInfo = 302,
    SpvOpGetDefaultQueue = 303,
    SpvOpBuildNDRange = 304,
    SpvOpImageSparseSampleImplicitLod = 305,
    SpvOpImageSparseSampleExplicitLod = 306,
    SpvOpImageSparseSampleDrefImplicitLod = 307,
    SpvOpImageSparseSampleDrefExplicitLod = 308,
    SpvOpImageSparseSampleProjImplicitLod = 309,
    SpvOpImageSparseSampleProjExplicitLod = 310,
    SpvOpImageSparseSampleProjDrefImplicitLod = 311,
    SpvOpImageSparseSampleProjDrefExplicitLod = 312,
    SpvOpImageSparseFetch = 313,
    SpvOpImageSparseGather = 314,
    SpvOpImageSparseDrefGather = 315,
    SpvOpImageSparseTexelsResident = 316,
    SpvOpNoLine = 317,
    SpvOpAtomicFlagTestAndSet = 318,
    SpvOpAtomicFlagClear = 319,
    SpvOpImageSparseRead = 320,
    SpvOpDecorateId = 332,
    SpvOpSubgroupBallotKHR = 4421,
    SpvOpSubgroupFirstInvocationKHR = 4422,
    SpvOpSubgroupAllKHR = 4428,
    SpvOpSubgroupAnyKHR = 4429,
    SpvOpSubgroupAllEqualKHR = 4430,
    SpvOpSubgroupReadInvocationKHR = 4432,
    SpvOpGroupIAddNonUniformAMD = 5000,
    SpvOpGroupFAddNonUniformAMD = 5001,
    SpvOpGroupFMinNonUniformAMD = 5002,
    SpvOpGroupUMinNonUniformAMD = 5003,
    SpvOpGroupSMinNonUniformAMD = 5004,
    SpvOpGroupFMaxNonUniformAMD = 5005,
    SpvOpGroupUMaxNonUniformAMD = 5006,
    SpvOpGroupSMaxNonUniformAMD = 5007,
    SpvOpFragmentMaskFetchAMD = 5011,
    SpvOpFragmentFetchAMD = 5012,
    SpvOpSubgroupShuffleINTEL = 5571,
    SpvOpSubgroupShuffleDownINTEL = 5572,
    SpvOpSubgroupShuffleUpINTEL = 5573,
    SpvOpSubgroupShuffleXorINTEL = 5574,
    SpvOpSubgroupBlockReadINTEL = 5575,
    SpvOpSubgroupBlockWriteINTEL = 5576,
    SpvOpSubgroupImageBlockReadINTEL = 5577,
    SpvOpSubgroupImageBlockWriteINTEL = 5578,
    SpvOpDecorateStringGOOGLE = 5632,
    SpvOpMemberDecorateStringGOOGLE = 5633,
    SpvOpMax = 0x7fffffff,
} SpvOp;


/* --SPIRV parsing */

typedef struct {

  VkDescriptorSetLayoutBinding bindings[LIDA_GFX_SHADER_MAX_BINDINGS_PER_SET];
  uint32_t binding_count;

} Binding_Set_Desc;

typedef struct {

  VkShaderStageFlags stages;
  uint32_t localX, localY, localZ;
  Binding_Set_Desc sets[LIDA_GFX_SHADER_MAX_SETS];
  size_t set_count;
  VkPushConstantRange ranges[LIDA_GFX_SHADER_MAX_RANGES];
  size_t range_count;

} Shader_Reflect;

typedef struct {

  uint32_t opcode;
  union {
    struct {
      uint32_t typeId;
      uint32_t storageClass;
      uint32_t binding;
      uint32_t set;
      uint32_t inputAttachmentIndex;
    } binding;
    struct {
      uint32_t integerWidth;
      int integerSigned;
    } val_int;
    struct {
      uint32_t floatWidth;
    } val_float;
    struct {
      uint32_t componentTypeId;
      uint32_t numComponents;
    } val_vec;
    struct {
      const uint32_t* memberTypes;
      uint32_t numMemberTypes;
      SpvDecoration structType;
    } val_struct;
    struct {
      uint32_t elementTypeId;
      uint32_t sizeConstantId;
    } val_array;
    struct {
      uint32_t constantType;
      uint32_t constantValue;
    } val_const;
  } data;

} SPIRV_ID;

static uint32_t
SPIRV_ComputeTypeSize(SPIRV_ID* ids, uint32_t id, uint32_t current_size/*for alignment*/)
{
#define ALIGN_MASK(number, mask) (((number)+(mask))&~(mask))
/** Align a number to alignment.
    NOTE: alignment must be a power of 2. */
#define ALIGN_TO(number, alignment) ALIGN_MASK(number, (alignment)-1)
  // NOTE about alignment: https://stackoverflow.com/a/45641579
  uint32_t offset = 0, alignment = 0;
  switch (ids[id].opcode) {
  case SpvOpTypeStruct:
    // A structure has a base alignment equal to the largest base alignment of
    // any of its members, rounded up to a multiple of 16.
    for (uint32_t typeId = 0; typeId < ids[id].data.val_struct.numMemberTypes; typeId++) {
      uint32_t member_size = SPIRV_ComputeTypeSize(ids, ids[id].data.val_struct.memberTypes[typeId], offset);
      offset += member_size;
      if (member_size > alignment)
        alignment = member_size;
    }
    break;
  case SpvOpTypeArray:
    // An array has a base alignment equal to the base alignment of its element type,
    // rounded up to a multiple of 16.
    {
      uint32_t arr_size = ids[ids[id].data.val_array.sizeConstantId].data.val_const.constantValue;
      // FIXME: I feel like we calculating alignment in wrong way
      uint32_t elem_alignment = SPIRV_ComputeTypeSize(ids, ids[id].data.val_array.elementTypeId, 0);
      alignment = ALIGN_TO(arr_size, 16 * elem_alignment);
      offset = arr_size * elem_alignment;
    }
    break;
  case SpvOpTypeFloat:
    return ids[id].data.val_float.floatWidth >> 3;
  case SpvOpTypeInt:
    return ids[id].data.val_int.integerWidth >> 3;
  case SpvOpTypeMatrix:
    // A column-major matrix has a base alignment equal to the base alignment of the matrix column type.
    // FIXME: should we check that matrix is row-major?
    {
      uint32_t vec_id = ids[id].data.val_vec.componentTypeId;
      uint32_t vec_size = SPIRV_ComputeTypeSize(ids, vec_id, 0);
      offset = ids[id].data.val_vec.numComponents * vec_size;
      uint32_t elem_size = SPIRV_ComputeTypeSize(ids, ids[vec_id].data.val_vec.componentTypeId, 0);
      alignment = ALIGN_TO(ids[vec_id].data.val_vec.numComponents, 2) * elem_size;
    }
    break;
  case SpvOpTypeVector:
    // A two-component vector, with components of size N, has a base alignment of 2 N.
    // A three- or four-component vector, with components of size N, has a base alignment of 4 N.
    {
      uint32_t component_size = SPIRV_ComputeTypeSize(ids, ids[id].data.val_vec.componentTypeId, 0);
      offset = ids[id].data.val_vec.numComponents * component_size;
      uint32_t num_components = ALIGN_TO(ids[id].data.val_vec.numComponents, 2);
      alignment = num_components * component_size;
    }
    break;
  default:
    assert(0 && "unrecognized type");
  }
  return ALIGN_TO(current_size, alignment) - current_size + offset;
}

static int
ReflectSPIRV(const uint32_t* code, uint32_t size, Shader_Reflect* shader)
{
  // based on https://github.com/zeux/niagara/blob/98f5d5ae2b48e15e145e3ad13ae7f4f9f1e0e297/src/shaders.cpp#L45
  // https://www.khronos.org/registry/SPIR-V/specs/unified1/SPIRV.html#_physical_layout_of_a_spir_v_module_and_instruction
  // this tool also helped me a lot: https://www.khronos.org/spir/visualizer/
  if (code[0] != SpvMagicNumber) {
    LOG_WARN("code is not valid SPIR-V");
    return -1;
  }
  uint32_t id_bound = code[3];
  SPIRV_ID* ids = push_mem(sizeof(SPIRV_ID) * id_bound);
  memset(ids, 0, sizeof(SPIRV_ID) * id_bound);
  for (uint32_t i = 0; i < id_bound; i++) {
    ids->data.binding.inputAttachmentIndex = UINT32_MAX;
  }
  const uint32_t* ins = code + 5;
  const uint32_t* const end = code + size;

  // parse all opcodes
  while (ins != end) {
    SpvOp opcode = ins[0] & 0xffff;
    uint32_t word_count = ins[0] >> 16;
    switch (opcode) {
    case SpvOpEntryPoint:
      assert(word_count >= 2);
      switch (ins[1]) {
      case SpvExecutionModelVertex: shader->stages = VK_SHADER_STAGE_VERTEX_BIT; break;
      case SpvExecutionModelFragment: shader->stages = VK_SHADER_STAGE_FRAGMENT_BIT; break;
      case SpvExecutionModelGLCompute: shader->stages = VK_SHADER_STAGE_COMPUTE_BIT; break;
      default: assert(0 && "SPIR-V: invalid shader stage");
      }
      break;
    case SpvOpExecutionMode:
      assert(word_count >= 3);
      switch (ins[2]) {
      case SpvExecutionModeLocalSize:
        assert(word_count == 6);
        shader->localX = ins[3];
        shader->localY = ins[4];
        shader->localZ = ins[5];
        break;
      }
      break;
    case SpvOpDecorate:
      assert(word_count >= 3);
      // ins[1] is id of entity that describes current instruction
      assert(ins[1] < id_bound);
      switch (ins[2]) {
      case SpvDecorationDescriptorSet:
        assert(word_count == 4);
        ids[ins[1]].data.binding.set = ins[3];
        break;
      case SpvDecorationBinding:
        assert(word_count == 4);
        ids[ins[1]].data.binding.binding = ins[3];
        break;
      case SpvDecorationBlock:
      case SpvDecorationBufferBlock:
        ids[ins[1]].data.val_struct.structType = ins[2];
        break;
      case SpvDecorationInputAttachmentIndex:
        ids[ins[1]].data.binding.inputAttachmentIndex = ins[3];
        break;
      }
      break;
    case SpvOpTypeStruct:
      ids[ins[1]].opcode = opcode;
      ids[ins[1]].data.val_struct.memberTypes = ins + 2;
      ids[ins[1]].data.val_struct.numMemberTypes = word_count - 2;
      break;
    case SpvOpTypeImage:
    case SpvOpTypeSampler:
    case SpvOpTypeSampledImage:
      assert(word_count >= 2);
      assert(ins[1] < id_bound);
      assert(ids[ins[1]].opcode == 0);
      ids[ins[1]].opcode = opcode;
      break;
    case SpvOpTypeInt:
      assert(word_count == 4);
      assert(ids[ins[1]].opcode == 0);
      ids[ins[1]].opcode = opcode;
      ids[ins[1]].data.val_int.integerWidth = ins[2];
      ids[ins[1]].data.val_int.integerSigned = ins[3];
      break;
    case SpvOpTypeFloat:
      assert(word_count == 3);
      assert(ids[ins[1]].opcode == 0);
      ids[ins[1]].opcode = opcode;
      ids[ins[1]].data.val_float.floatWidth = ins[2];
      break;
    case SpvOpTypeVector:
    case SpvOpTypeMatrix:
      assert(word_count == 4);
      assert(ids[ins[1]].opcode == 0);
      ids[ins[1]].opcode = opcode;
      ids[ins[1]].data.val_vec.componentTypeId = ins[2];
      ids[ins[1]].data.val_vec.numComponents = ins[3];
      break;
    case SpvOpTypeArray:
      assert(ids[ins[1]].opcode == 0);
      ids[ins[1]].opcode = opcode;
      ids[ins[1]].data.val_array.elementTypeId = ins[2];
      ids[ins[1]].data.val_array.sizeConstantId = ins[3];
      break;
    case SpvOpTypePointer:
      assert(word_count == 4);
      assert(ins[1] < id_bound);
      assert(ids[ins[1]].opcode == 0);
      ids[ins[1]].opcode = opcode;
      ids[ins[1]].data.binding.storageClass = ins[2];
      ids[ins[1]].data.binding.typeId = ins[3];
      break;
    case SpvOpVariable:
      assert(word_count >= 4);
      // ins[2] is id
      assert(ins[2] < id_bound);
      assert(ids[ins[2]].opcode == 0);
      ids[ins[2]].opcode = opcode;
      ids[ins[2]].data.binding.typeId = ins[1];
      ids[ins[2]].data.binding.storageClass = ins[3];
      break;
    case SpvOpConstant:
      assert(ids[ins[2]].opcode == 0);
      ids[ins[2]].opcode = opcode;
      ids[ins[2]].data.val_const.constantType = ins[1];
      ids[ins[2]].data.val_const.constantValue = ins[3];
      break;
      // avoid warnings from GCC
    default:
      break;
    }
    ins += word_count;
  }

  shader->set_count = 0;
  shader->range_count = 0;
  memset(shader->sets, 0, sizeof(Binding_Set_Desc) * LIDA_GFX_SHADER_MAX_SETS);

  // use ids that we parsed to collect reflection data
  for (uint32_t i = 0; i < id_bound; i++) {
    SPIRV_ID* id = &ids[i];

    if (id->opcode == SpvOpVariable &&
        (id->data.binding.storageClass == SpvStorageClassUniform ||
         id->data.binding.storageClass == SpvStorageClassUniformConstant ||
         id->data.binding.storageClass == SpvStorageClassStorageBuffer)) {
      // process uniform
      assert(id->data.binding.set < LIDA_GFX_SHADER_MAX_SETS &&
             "descriptor set number is bigger than max value");
      if (id->data.binding.set+1 > shader->set_count)
        shader->set_count = id->data.binding.set+1;
      assert(id->data.binding.binding < LIDA_GFX_SHADER_MAX_BINDINGS_PER_SET &&
             "descriptor binding number is bigger than max value");
      assert(ids[id->data.binding.typeId].opcode == SpvOpTypePointer);
      Binding_Set_Desc* set = &shader->sets[id->data.binding.set];
      VkDescriptorType* ds_type = &set->bindings[set->binding_count].descriptorType;
      switch (ids[ids[id->data.binding.typeId].data.binding.typeId].opcode) {
      case SpvOpTypeStruct:
        switch (ids[ids[id->data.binding.typeId].data.binding.typeId].data.val_struct.structType) {
        case SpvDecorationBlock:
          *ds_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          break;
        case SpvDecorationBufferBlock:
          *ds_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
          break;
        default: break;
        }
        break;
      case SpvOpTypeImage:
        // TODO: support input attachments
        *ds_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        break;
      case SpvOpTypeSampler:
        *ds_type = VK_DESCRIPTOR_TYPE_SAMPLER;
        break;
      case SpvOpTypeSampledImage:
        *ds_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        break;
      default:
        assert(0 && "Unknown resource type");
        break;
      }

      set->bindings[set->binding_count].binding = id->data.binding.binding;
      set->bindings[set->binding_count].descriptorCount = 1;
      set->bindings[set->binding_count].stageFlags = shader->stages;
      set->binding_count++;
    } else if (id->opcode == SpvOpVariable &&
               id->data.binding.storageClass == SpvStorageClassPushConstant) {
      // process push constant
      assert(ids[id->data.binding.typeId].data.binding.storageClass == SpvStorageClassPushConstant);
      shader->ranges[shader->range_count] = (VkPushConstantRange) {
        .stageFlags = shader->stages,
        .offset = 0,
        .size = SPIRV_ComputeTypeSize(ids, ids[id->data.binding.typeId].data.binding.typeId, 0),
      };
      shader->range_count++;
    }

  }

  pop_mem(ids);
  return 0;
}


/* --Vulkan specific code */

static VkBool32
vk_debug_log_callback(VkDebugReportFlagsEXT flags,
                      VkDebugReportObjectTypeEXT obj_type,
                      uint64_t obj,
                      size_t location,
                      int32_t code,
                      const char* layer_prefix,
                      const char* msg,
                      void* user_data)
{
  (void)flags;
  (void)obj_type;
  (void)obj;
  (void)location;
  (void)user_data;
  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    LOG_ERROR("[Vulkan:%d: %s]: %s", code, layer_prefix, msg);
    return 0;
  } else if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) ||
             (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
    LOG_WARN("[Vulkan:%d: %s]: %s", code, layer_prefix, msg);
    return 1;
  } else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    LOG_INFO("[Vulkan:%d: %s]: %s", code, layer_prefix, msg);
    return 1;
  } else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    LOG_DEBUG("[Vulkan:%d: %s]: %s", code, layer_prefix, msg);
    return 1;
  }
  return 1;
}

static void
get_instance_extensions(const GFX_Init_Info* info)
{
  uint32_t num_available_instance_extensions;
  VkExtensionProperties* available_instance_extensions;

  struct {
    const char* name; int enabled;
  } required_extensions[] = {
    // NOTE: here we declare all instance extensions we use
    { VK_EXT_DEBUG_REPORT_EXTENSION_NAME, info->enable_debug_layers },
    { VK_KHR_SURFACE_EXTENSION_NAME, 1 },
    { "VK_KHR_win32_surface", 1 },
    { "VK_KHR_android_surface", 1 },
    { "VK_KHR_xlib_surface", 1 },
    { "VK_KHR_xcb_surface", 1 },
    { "VK_KHR_wayland_surface", 1 },
  };

  vkEnumerateInstanceExtensionProperties(NULL, &num_available_instance_extensions, NULL);
  available_instance_extensions = alloca(sizeof(VkExtensionProperties) * num_available_instance_extensions);
  vkEnumerateInstanceExtensionProperties(NULL, &num_available_instance_extensions, available_instance_extensions);
  g.num_enabled_instance_extensions = 0;
  g.enabled_instance_extensions = push_mem(0);
  for (uint32_t i = 0; i < ARR_SIZE(required_extensions); i++) {
    if (required_extensions[i].enabled == 0)
      continue;

    for (uint32_t j = 0; j < num_available_instance_extensions; j++) {
      if (strcmp(required_extensions[i].name, available_instance_extensions[j].extensionName) == 0) {
        push_mem(sizeof(const char*));
        g.enabled_instance_extensions[g.num_enabled_instance_extensions++] = required_extensions[i].name;
        break;
      }
    }
  }
}

static void
get_device_extensions(const GFX_Init_Info* info)
{
  VkExtensionProperties* available_device_extensions;
  uint32_t num_available_device_extensions;

  VkResult err = vkEnumerateDeviceExtensionProperties(g.physical_device, NULL,
                                             &num_available_device_extensions, NULL);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to enumerate device extensions with error %d", err);
  }
  available_device_extensions = alloca(num_available_device_extensions * sizeof(VkExtensionProperties));
  err = vkEnumerateDeviceExtensionProperties(g.physical_device, NULL,
                                             &num_available_device_extensions,
                                             available_device_extensions);

  // for (uint32_t i = 0; i < num_available_device_extensions; i++) {
  //   LOG_INFO("%s---", available_device_extensions[i].extensionName);
  // }

  struct {
    const char* name; int enabled;
  } required_extensions[] = {
    // NOTE: here we declare all device extensions we use
    { VK_KHR_SWAPCHAIN_EXTENSION_NAME, 1 },
    { VK_EXT_DEBUG_MARKER_EXTENSION_NAME, info->enable_debug_layers },
    { VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME, 0 },
  };
  g.enabled_device_extensions = push_mem(0);
  g.num_enabled_device_extensions = 0;
  for (uint32_t i = 0; i < ARR_SIZE(required_extensions); i++) {
    if (required_extensions[i].enabled == 0)
      continue;
    int found = 0;
    for (uint32_t j = 0; j < num_available_device_extensions; j++) {
      if (strcmp(required_extensions[i].name, available_device_extensions[j].extensionName) == 0) {
        push_mem(sizeof(const char*));
        g.enabled_device_extensions[g.num_enabled_device_extensions++] = required_extensions[i].name;
        found = 1;
        break;
      }
    }
    if (found == 0) {
      LOG_WARN("extension '%s' is not supported", required_extensions[i]);
    }
  }
}

static const char*
to_string_VkResult(VkResult err)
{
  switch (err) {
#define CASE(e) case e: return #e
    CASE(VK_SUCCESS);
    CASE(VK_NOT_READY);
    CASE(VK_TIMEOUT);
    CASE(VK_EVENT_SET);
    CASE(VK_EVENT_RESET);
    CASE(VK_INCOMPLETE);
    CASE(VK_ERROR_OUT_OF_HOST_MEMORY);
    CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    CASE(VK_ERROR_INITIALIZATION_FAILED);
    CASE(VK_ERROR_DEVICE_LOST);
    CASE(VK_ERROR_MEMORY_MAP_FAILED);
    CASE(VK_ERROR_LAYER_NOT_PRESENT);
    CASE(VK_ERROR_EXTENSION_NOT_PRESENT);
    CASE(VK_ERROR_FEATURE_NOT_PRESENT);
    CASE(VK_ERROR_INCOMPATIBLE_DRIVER);
    CASE(VK_ERROR_TOO_MANY_OBJECTS);
    CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
    CASE(VK_ERROR_FRAGMENTED_POOL);
    CASE(VK_ERROR_UNKNOWN);
    CASE(VK_ERROR_OUT_OF_POOL_MEMORY);
    CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE);
    CASE(VK_ERROR_FRAGMENTATION);
    CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
    CASE(VK_ERROR_SURFACE_LOST_KHR);
    CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
    CASE(VK_SUBOPTIMAL_KHR);
    CASE(VK_ERROR_OUT_OF_DATE_KHR);
    CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
    CASE(VK_ERROR_VALIDATION_FAILED_EXT);
  default: return "VkResult(nil)";
  }
}

static VkResult
allocate_command_buffers(VkCommandBuffer* cmds, uint32_t count, VkCommandBufferLevel level)
{
  VkCommandBufferAllocateInfo alloc_info = {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool        = g.command_pool,
    .level              = level,
    .commandBufferCount = count,
  };
  return vkAllocateCommandBuffers(g.logical_device, &alloc_info, cmds);
}

typedef struct {

  GFX_Attachment_Info attachments[4];
  int count;
  VkRenderPass render_pass;

} Render_Pass;

static uint32_t
hash_render_pass(const void* obj)
{
  const Render_Pass* r = obj;
  return hash_memory(r->attachments, r->count * sizeof(GFX_Attachment_Info));
}

static int
eq_render_pass(const void* l, const void* r)
{
  const Render_Pass *left = l, *right = r;
  if (left->count != right->count)
    return 0;
  return memcmp(left->attachments, right->attachments, sizeof(GFX_Attachment_Info) * left->count) == 0;
}

static void
destroy_render_pass(void* obj)
{
  Render_Pass* r = obj;
  vkDestroyRenderPass(g.logical_device, r->render_pass, NULL);
}

static Render_Pass*
create_render_pass(const GFX_Attachment_Info* attachments, uint32_t count)
{
  if (count > 4) {
    LOG_ERROR("number of attachments is too big, 4 is max");
    return NULL;
  }
  Render_Pass temp = { .count = count };
  memcpy(temp.attachments, attachments, count * sizeof(GFX_Attachment_Info));
  int flag;
  Render_Pass* ret = lru_cache_get(&g.render_pass_cache, &temp, &flag);
  if (flag == 0)
    return ret;

  VkAttachmentDescription descriptions[4];
  VkAttachmentReference color_references[3];
  VkAttachmentReference depth_reference;
  int has_depth = 0;
  uint32_t color_attachment_count = 0;

  for (uint32_t i = 0; i < count; i++) {
    VkAttachmentStoreOp store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    if (attachments[i].store_op == GFX_ATTACHMENT_OP_STORE) {
      // FIXME: maybe raise a warning if got invalid store_op?
      store_op = VK_ATTACHMENT_STORE_OP_STORE;
    }
    VkAttachmentLoadOp load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    if (attachments[i].load_op == GFX_ATTACHMENT_OP_CLEAR) {
      load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    } else if (attachments[i].store_op == GFX_ATTACHMENT_OP_LOAD) {
      load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    }
    descriptions[i] = (VkAttachmentDescription) {
      .format = (VkFormat)attachments[i].format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = load_op,
      .storeOp = store_op,
      .initialLayout = (VkImageLayout)attachments[i].initial_layout,
      .finalLayout = (VkImageLayout)attachments[i].final_layout,
    };
    VkImageLayout work_layout = (VkImageLayout)attachments[i].work_layout;
    VkAttachmentReference temp = (VkAttachmentReference) {
        .attachment = i,
        .layout = work_layout,
    };
    if (work_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL ||
        work_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
      has_depth = 1;
      depth_reference = temp;
    } else {
      color_references[color_attachment_count++] = temp;
    }
  }

  // TODO: support for multiple subpasses
  VkSubpassDescription subpass = {
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = color_attachment_count,
    .pColorAttachments = color_references,
    .pDepthStencilAttachment = (has_depth) ? &depth_reference : NULL,
  };

  VkAccessFlags access_mask = 0;
  if (color_attachment_count > 0) access_mask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  if (has_depth) access_mask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT|VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

  VkPipelineStageFlagBits src_stage_mask = 0;
  VkPipelineStageFlagBits dst_stage_mask = 0;
  if (has_depth) {
    src_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dst_stage_mask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  }
  if (color_attachment_count > 0) {
    src_stage_mask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dst_stage_mask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }

  VkSubpassDependency dependencies[2];
  dependencies[0] = (VkSubpassDependency) {
    .srcSubpass      = VK_SUBPASS_EXTERNAL,
    .dstSubpass      = 0,
    .srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    .dstStageMask    = src_stage_mask,
    .srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
    .dstAccessMask   = access_mask,
    .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  };
  dependencies[1] = (VkSubpassDependency) {
    .srcSubpass      = 0,
    .dstSubpass      = VK_SUBPASS_EXTERNAL,
    .srcStageMask    = dst_stage_mask,
    .dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    .srcAccessMask   = access_mask,
    .dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT,
    .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
  };

  VkRenderPassCreateInfo render_pass_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = count,
    .pAttachments = descriptions,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 2,
    .pDependencies = dependencies,
  };
  VkResult err = vkCreateRenderPass(g.logical_device, &render_pass_info, NULL, &ret->render_pass);
  if (err != VK_SUCCESS) {
    LOG_WARN("failed to create render pass with error %s", to_string_VkResult(err));
    // TODO: delete from cache
    return NULL;
  }

  return ret;
}

typedef struct {

  const char* tag;
  VkShaderModule module;
  Shader_Reflect reflect;

} Shader_Info;

static uint32_t
hash_shader_info(const void* obj)
{
  const Shader_Info* s = obj;
  return hash_string(s->tag);
}

static int
eq_shader_info(const void* l, const void* r)
{
  const Shader_Info* left = l, *right = r;
  return strcmp(left->tag, right->tag) == 0;
}

static void
destroy_shader_info(void* obj)
{
  Shader_Info* s = obj;
  vkDestroyShaderModule(g.logical_device, s->module, NULL);
}

static Shader_Info*
create_shader(const char* tag)
{
  int flag;
  // NOTE: we can pass &tag because tag is first field in the
  // Shader_Info structure. This looks kind of dangerous but it works.
  Shader_Info* ret = lru_cache_get(&g.shader_cache, &(Shader_Info) { .tag = tag }, &flag);
  if (flag == 0)
    return ret;
  size_t buffer_size = 0;
  uint32_t* buffer = g.load_shader_fn(tag, &buffer_size);
  if (!buffer) {
    LOG_ERROR("failed to load shader '%s'", tag);
    return NULL;
  }
  VkShaderModuleCreateInfo module_info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = buffer_size,
    .pCode = buffer,
  };
  VkResult err = vkCreateShaderModule(g.logical_device, &module_info, NULL, &ret->module);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create shader module with error %s", to_string_VkResult(err));
  } else {
    ReflectSPIRV(buffer, buffer_size / sizeof(uint32_t), &ret->reflect);
  }
  g.free_shader_fn(buffer);
  return ret;
}

typedef struct {

  VkDescriptorSetLayoutBinding bindings[LIDA_GFX_SHADER_MAX_BINDINGS_PER_SET];
  uint32_t num_bindings;
  VkDescriptorSetLayout layout;

} DS_Layout;

static uint32_t
hash_ds_layout(const void* obj)
{
  const DS_Layout* l = obj;
  return hash_memory(l->bindings, l->num_bindings * sizeof(VkDescriptorSetLayoutBinding));
}

static int
eq_ds_layout(const void* l, const void* r)
{
  const DS_Layout* left = l, *right = r;
  if (left->num_bindings != right->num_bindings)
    return 0;
  return memcmp(left->bindings, right->bindings,
                left->num_bindings * sizeof(VkDescriptorSetLayoutBinding)) == 0;
}

static void
destroy_ds_layout(void* obj)
{
  DS_Layout* s = obj;
  vkDestroyDescriptorSetLayout(g.logical_device, s->layout, NULL);
}

static DS_Layout*
create_ds_layout(const VkDescriptorSetLayoutBinding* bindings, uint32_t num_bindings)
{
  int flag;
  if (num_bindings > LIDA_GFX_SHADER_MAX_BINDINGS_PER_SET) {
    LOG_ERROR("max number of bindings per descriptor set is configured to be %d",
              LIDA_GFX_SHADER_MAX_BINDINGS_PER_SET);
    return NULL;
  }
  // TODO: sort bindings
  DS_Layout temp;
  memcpy(temp.bindings, bindings, num_bindings * sizeof(VkDescriptorSetLayoutBinding));
  temp.num_bindings = num_bindings;
  DS_Layout* ret = lru_cache_get(&g.ds_layout_cache, &temp, &flag);
  if (flag == 0)
    return ret;
  VkDescriptorSetLayoutCreateInfo layout_info = {
    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    .bindingCount = num_bindings,
    .pBindings = bindings,
  };
  VkResult err = vkCreateDescriptorSetLayout(g.logical_device, &layout_info, NULL, &ret->layout);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create descriptor layout with error %s", to_string_VkResult(err));
  }
  return ret;
}

typedef struct {

  // NOTE: I think using pointer here is a bit dangerous as some
  // descriptor set layouts can be destroyed. I should find a better
  // way to represent reference to a ds_layout.
  VkDescriptorSetLayout set_layouts[LIDA_GFX_SHADER_MAX_SETS];
  VkPushConstantRange   ranges[LIDA_GFX_SHADER_MAX_RANGES];
  uint32_t              num_sets;
  uint32_t              num_ranges;
  VkPipelineLayout      handle;

} Pipeline_Layout;

static uint32_t
hash_pipeline_layout(const void* obj)
{
  const Pipeline_Layout* p = obj;
  uint32_t hash1 = hash_memory(p->set_layouts, p->num_sets * sizeof(VkDescriptorSetLayout));
  uint32_t hash2 = hash_memory(p->ranges, p->num_ranges * sizeof(VkPushConstantRange));
  return (hash1 ^ hash2) + (hash1<<3) + 0x696969*(hash2<<6);
}

static int
eq_pipeline_layout(const void* l, const void* r)
{
  const Pipeline_Layout* left = l, *right = r;
  if (left->num_sets != right->num_sets)
    return 0;
  if (left->num_ranges != right->num_ranges)
    return 0;
  return
    memcmp(left->set_layouts, right->set_layouts, left->num_sets * sizeof(VkDescriptorSetLayout)) == 0 &&
    memcmp(left->ranges, right->ranges, left->num_ranges * sizeof(VkPushConstantRange)) == 0;
}

static void
destroy_pipeline_layout(void* obj)
{
  Pipeline_Layout* s = obj;
  vkDestroyPipelineLayout(g.logical_device, s->handle, NULL);
}

static Pipeline_Layout*
create_pipeline_layout(Shader_Info** shader_templates, uint32_t count)
{
  Pipeline_Layout layout = { 0 };
#if 1
  (void)shader_templates;
  (void)count;
#else
  if (count > 0) {
    Shader_Info shader;
    memcpy(&shader, shader_templates[0], sizeof(Shader_Info));
    if (count > 1) {
      for (uint32_t j = 1; j < count; j++) {
        // TODO: merge shader_info's
      }
    }
  }
#endif
  int flag;
  Pipeline_Layout* ret = lru_cache_get(&g.pipeline_layout_cache, &layout, &flag);
  if (flag == 0)
    return ret;
  // create a new pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = layout.num_sets,
    .pSetLayouts = layout.set_layouts,
    .pushConstantRangeCount = layout.num_ranges,
    .pPushConstantRanges = layout.ranges,
  };
  VkResult err = vkCreatePipelineLayout(g.logical_device, &pipeline_layout, NULL, &ret->handle);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create pipeline layout with error %s", to_string_VkResult(err));
  }
  return ret;
}

typedef struct {
  VkPipeline handle;
  VkPipelineLayout layout;
} Pipeline;
_Static_assert(sizeof(Pipeline) <= sizeof(GFX_Pipeline), "internal error: need to adjust sizeof for GFX_Pipeline");
// NOTE: we don't cache pipelines as it won't be so good

typedef struct {
  VkImage       image;
  VkImageView   image_view;
  VkFramebuffer framebuffer;
} Window_Image;

typedef struct {
  VkCommandBuffer cmd;
  VkSemaphore     image_available;
} Window_Frame;

typedef struct {

  VkSurfaceKHR                surface;
  VkSwapchainKHR              swapchain;
  Render_Pass*                render_pass;
  uint32_t                    num_images;
  Window_Image                images[8]; // NOTE: we hope that no vulkan driver creates more than 8 images for a swapchain
  Window_Frame                frames[2];
  VkSemaphore                 render_finished_semaphore;
  VkFence                     resources_available_fence;
  uint64_t                    frame_counter;
  uint32_t                    current_image;
  VkExtent2D                  swapchain_extent;
  VkSurfaceFormatKHR          format;
  VkPresentModeKHR            present_mode;
  VkCompositeAlphaFlagBitsKHR composite_alpha;

} Window;
_Static_assert(sizeof(Window) <= sizeof(GFX_Window), "Internal error: adjust sizeof for GFX_Window");

static VkResult
create_swapchain(Window* window)
{
  VkResult err;

  // get device capabilities of surface
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.physical_device, window->surface, &capabilities);

  // choose surface format
  VkSurfaceFormatKHR old_format = window->format;
  uint32_t count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(g.physical_device, window->surface, &count, NULL);
  VkSurfaceFormatKHR* formats = push_mem(sizeof(VkSurfaceFormatKHR) * count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(g.physical_device, window->surface, &count, formats);
  window->format = formats[0];
  for (uint32_t i = 1; i < count; i++) {
    // try to pick R8G8B8A8_SRGB with nonlinear color space because it looks good
    // TODO: choose in more smart way
    if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB &&
        formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      window->format = formats[i];
      break;
    }
  }
  pop_mem(formats);

  // choose present mode
  vkGetPhysicalDeviceSurfacePresentModesKHR(g.physical_device, window->surface, &count, NULL);
  VkPresentModeKHR* present_modes = push_mem(sizeof(VkPresentModeKHR) * count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(g.physical_device, window->surface, &count, present_modes);
  VkPresentModeKHR required_present_mode = window->present_mode;
  window->present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (uint32_t i = 1; i < count; i++) {
    if (present_modes[i] == required_present_mode) {
      window->present_mode = present_modes[i];
      break;
    }
  }
  pop_mem(present_modes);
  if (required_present_mode != window->present_mode) {
    LOG_WARN("Present mode '%d' isn't supported, '%d' was chosen instead",
             required_present_mode, window->present_mode);
  }

  // choose swapchain extent
  if (capabilities.currentExtent.width == UINT32_MAX) {
#define CLAMP(value, low, high) (((value)<(low))?(low):(((value)>(high))?(high):(value)))
    window->swapchain_extent.width = CLAMP(window->swapchain_extent.width,
                                             capabilities.minImageExtent.width,
                                             capabilities.maxImageExtent.height);
    window->swapchain_extent.height = CLAMP(window->swapchain_extent.height,
                                              capabilities.minImageExtent.height,
                                              capabilities.maxImageExtent.height);
#undef CLAMP
  } else {
    window->swapchain_extent = capabilities.currentExtent;
  }

  // choose image count
  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    image_count = capabilities.maxImageCount;

  // choose composite alpha flag
  if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
    window->composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  } else {
    uint32_t i = 0;
    VkCompositeAlphaFlagsKHR flags = capabilities.supportedCompositeAlpha;
    // composite_alpha always will be initialized as spec says:
    // supportedCompositeAlpha is a bitmask of VkCompositeAlphaFlagBitsKHR, representing the alpha compositing modes supported by the presentation engine for the surface on the specified device, and at least one bit will be set
    while (flags > 0) {
      if (flags & 1) {
        window->composite_alpha = 1 << i;
        break;
      }
      flags >>= 1;
      i++;
    }
  }

  // TODO: present queue?
  uint32_t queue_family_indices[] = { g.graphics_queue_family };

  // create swapchain
  VkSwapchainCreateInfoKHR swapchain_info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = window->surface,
    .minImageCount = image_count,
    .imageFormat = window->format.format,
    .imageColorSpace = window->format.colorSpace,
    .imageExtent = window->swapchain_extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 1,
    .pQueueFamilyIndices = queue_family_indices,
    .preTransform = capabilities.currentTransform,
    .compositeAlpha = window->composite_alpha,
    .presentMode = window->present_mode,
    .clipped = 1,
    .oldSwapchain = window->swapchain,
  };
  // note: we specified swapchainInfo.oldSwapchain so
  // we don't need to destroy oldSwapchain when resizing
  // From vulkan spec:
  // Upon calling vkCreateSwapchainKHR with an oldSwapchain that is not
  // VK_NULL_HANDLE, oldSwapchain is retired — even if creation of the new
  // swapchain fails. The new swapchain is created in the non-retired state
  // whether or not oldSwapchain is VK_NULL_HANDLE.
  err = vkCreateSwapchainKHR(g.logical_device, &swapchain_info, NULL, &window->swapchain);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create swapchain with error %s", to_string_VkResult(err));
    return err;
  }

  // recreate render pass if needed
  if (old_format.format != window->format.format) {
    GFX_Attachment_Info attachment = {
      .format = (GFX_Format)window->format.format,
      .load_op = GFX_ATTACHMENT_OP_NONE,
      .store_op = GFX_ATTACHMENT_OP_STORE,
      .initial_layout = GFX_IMAGE_LAYOUT_UNDEFINED,
      .final_layout = GFX_IMAGE_LAYOUT_PRESENT_SRC_KHR,
      .work_layout = GFX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    window->render_pass = create_render_pass(&attachment, 1);
  }

  // get images of swapchain (Vulkan kindly manages their memory and lifetime for us)
  vkGetSwapchainImagesKHR(g.logical_device, window->swapchain, &window->num_images, NULL);
  // NOTE: we hope that no device creates more than 8 swapchain images
  VkImage swapchain_images[8];
  vkGetSwapchainImagesKHR(g.logical_device, window->swapchain, &window->num_images, swapchain_images);

  // create attachments
  VkImageViewCreateInfo image_view_info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .format = window->format.format,
    .components = { .r = VK_COMPONENT_SWIZZLE_R,
                    .g = VK_COMPONENT_SWIZZLE_G,
                    .b = VK_COMPONENT_SWIZZLE_B,
                    .a = VK_COMPONENT_SWIZZLE_A,},
    .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                          .baseMipLevel = 0,
                          .levelCount = 1,
                          .baseArrayLayer = 0,
                          .layerCount = 1 },
  };
  VkFramebufferCreateInfo framebuffer_info = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .renderPass = window->render_pass->render_pass,
    .attachmentCount = 1,
    .width = window->swapchain_extent.width,
    .height = window->swapchain_extent.height,
    .layers = 1,
  };
  for (uint32_t i = 0; i < window->num_images; i++) {
    window->images[i].image = swapchain_images[i];
    image_view_info.image = swapchain_images[i];
    err = vkCreateImageView(g.logical_device, &image_view_info, NULL, &window->images[i].image_view);
    if (err != VK_SUCCESS) {
      LOG_ERROR("failed to create image view no. %u with error %s", i, to_string_VkResult(err));
    }
    framebuffer_info.pAttachments = &window->images[i].image_view;
    err = vkCreateFramebuffer(g.logical_device, &framebuffer_info, NULL, &window->images[i].framebuffer);
    if (err != VK_SUCCESS) {
      LOG_ERROR("failed to create framebuffer no. %u with error %s", i, to_string_VkResult(err));
    }
  }

  // TODO: manage preTransform
  // This is crucial for devices you can flip

  return err;
}

static VkResult
create_window_frames(Window* window)
{
  VkResult err;
  VkCommandBuffer command_buffers[2];
  err = allocate_command_buffers(command_buffers, 2, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to allocate command buffers with error %s", to_string_VkResult(err));
    return err;
  }
  VkSemaphoreCreateInfo semaphore_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
  VkFenceCreateInfo fence_info = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                   .flags = VK_FENCE_CREATE_SIGNALED_BIT };
  for (int i = 0; i < 2; i++) {
    window->frames[i].cmd = command_buffers[i];
    err = vkCreateSemaphore(g.logical_device, &semaphore_info, NULL, &window->frames[i].image_available);
    if (err != VK_SUCCESS) {
      LOG_ERROR("failed to create semaphore with error %s", to_string_VkResult(err));
      return err;
    }
  }
  err = vkCreateSemaphore(g.logical_device, &semaphore_info, NULL, &window->render_finished_semaphore);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create semaphore with error %s", to_string_VkResult(err));
    return err;
  }
  err = vkCreateFence(g.logical_device, &fence_info, NULL, &window->resources_available_fence);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create fence with error %s", to_string_VkResult(err));
    return err;
  }
  window->frame_counter = 0;
  window->current_image = UINT32_MAX;
  return err;
}

/* --Implementation */

int
gfx_init(const GFX_Init_Info* info)
{
  g.log_fn = info->log_fn;
  g.load_shader_fn = info->load_shader_fn;
  g.free_shader_fn = info->free_shader_fn;

  g.memright = ARR_SIZE(g.membuf);

  // stage 0: load the vulkan library
#if defined(_WIN32)
  HMODULE module = LoadLibraryA("vulkan-1.dll");
  if (!module)
    return VK_ERROR_INITIALIZATION_FAILED;
  vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(void(*)(void))GetProcAddress(module, "vkGetInstanceProcAddr");
#elif defined(__APPLE__)
  void* module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
  if (!module)
    module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
  if (!module)
    module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
  if (!module)
    return VK_ERROR_INITIALIZATION_FAILED;

  vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#else
  void* module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
  if (!module)
    module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
  if (!module)
    return VK_ERROR_INITIALIZATION_FAILED;

  vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#endif

  // stage 1: load instance functions
  vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(NULL, "vkCreateInstance");
  vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
  vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");

  // stage 2: create instance
  const char* validation_layers[1];
  uint32_t layer_count = 0;
  if (info->enable_debug_layers) {
    validation_layers[0] = "VK_LAYER_KHRONOS_validation";
    layer_count++;
    // @TODO: check if the layer is present
  }
  get_instance_extensions(info);
  VkApplicationInfo app_info = {
    .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName   = info->app_name,
    .applicationVersion = info->app_version,
    .pEngineName        = "lida",
    .engineVersion      = LIDA_GFX_VERSION,
    .apiVersion         = VK_API_VERSION_1_0
  };
  VkInstanceCreateInfo instance_info = {
    .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo        = &app_info,
    .enabledLayerCount       = layer_count,
    .ppEnabledLayerNames     = validation_layers,
    .enabledExtensionCount   = g.num_enabled_instance_extensions,
    .ppEnabledExtensionNames = g.enabled_instance_extensions,
  };
  VkDebugReportCallbackCreateInfoEXT callback_info = {
    .sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
    // .flags       = VK_DEBUG_REPORT_INFORMATION_BIT_EXT|VK_DEBUG_REPORT_DEBUG_BIT_EXT|VK_DEBUG_REPORT_ERROR_BIT_EXT|VK_DEBUG_REPORT_WARNING_BIT_EXT|VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
    .flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT|VK_DEBUG_REPORT_WARNING_BIT_EXT|VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
    .pfnCallback = &vk_debug_log_callback,
    .pUserData   = NULL,
  };
  if (info->enable_debug_layers) {
    instance_info.pNext = &callback_info;
  }
  VkResult err = vkCreateInstance(&instance_info, NULL, &g.instance);
  if (err != VK_SUCCESS) {
    goto end;
  }

  // stage 3: load device functions
  // FIXME: should we use 'vkGetDeviceProcAddr' instead for some functions?
#define X(name) name = (PFN_##name)vkGetInstanceProcAddr(g.instance, #name)
  FOR_ALL_FUNCTIONS();
#undef X

  // create debug callback
  if (info->enable_debug_layers) {
    err = vkCreateDebugReportCallbackEXT(g.instance, &callback_info, NULL, &g.debug_report_callback);
    if (err != VK_SUCCESS) {
      LOG_WARN("failed to create debug report callback with error %s", to_string_VkResult(err));
    }
  } else {
    g.debug_report_callback = VK_NULL_HANDLE;
  }

  // stage 4: pick physical device
  uint32_t count = 0;
  err = vkEnumeratePhysicalDevices(g.instance, &count, NULL);
  if (err != VK_SUCCESS) {
    LOG_WARN("failed to enumerate physical devices with error %s", to_string_VkResult(err));
    goto end;
  }
  VkPhysicalDevice* devices = push_mem(count * sizeof(VkPhysicalDevice));
  err = vkEnumeratePhysicalDevices(g.instance, &count, devices);
  if (info->gpu_id <= count) {
    g.physical_device = devices[info->gpu_id];
  } else {
    LOG_WARN("info->gpu_id is out of bounds, picking GPU0");
    g.physical_device = devices[0];
  }
  pop_mem(devices);

  memset(&g.device_properties, 0, sizeof(VkPhysicalDeviceProperties));
  vkGetPhysicalDeviceProperties(g.physical_device, &g.device_properties);
  memset(&g.device_features, 0, sizeof(VkPhysicalDeviceFeatures));
  vkGetPhysicalDeviceFeatures(g.physical_device, &g.device_features);
  memset(&g.memory_properties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
  vkGetPhysicalDeviceMemoryProperties(g.physical_device, &g.memory_properties);

  vkGetPhysicalDeviceQueueFamilyProperties(g.physical_device, &g.num_queue_families, NULL);
  g.queue_families = push_mem(g.num_queue_families * sizeof(VkQueueFamilyProperties));
  vkGetPhysicalDeviceQueueFamilyProperties(g.physical_device,
                                           &g.num_queue_families,
                                           g.queue_families);
  for (uint32_t i = 0; i < count; i++) {
    if (g.queue_families[0].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      g.graphics_queue_family = i;
      break;
    }
  }

  // stage 5: create logical device
  float queue_priorities[] = { 1.0f };
  VkDeviceQueueCreateInfo queueInfo = {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = g.graphics_queue_family,
    .queueCount       = 1,
    .pQueuePriorities = queue_priorities,
  };

  get_device_extensions(info);

  VkDeviceCreateInfo device_info = {
    .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount    = 1,
    .pQueueCreateInfos       = &queueInfo,
    .ppEnabledExtensionNames = g.enabled_device_extensions,
    .enabledExtensionCount   = g.num_enabled_device_extensions,
    .pEnabledFeatures        = &g.device_features,
  };
  err = vkCreateDevice(g.physical_device, &device_info, NULL, &g.logical_device);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create Vulkan device with error %s", to_string_VkResult(err));
    goto end;
  }
  // get graphics queue
  vkGetDeviceQueue(g.logical_device, g.graphics_queue_family, 0, &g.graphics_queue);

  // stage 6: create command pool
  VkCommandPoolCreateInfo command_pool_info = {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = g.graphics_queue_family,
  };
  err = vkCreateCommandPool(g.logical_device, &command_pool_info, NULL, &g.command_pool);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create command pool with error %s", to_string_VkResult(err));
  }

#define CACHE_CREATE(bytes, t, T) lru_cache_init(bytes, push_mem_right(bytes), hash_##t, eq_##t, destroy_##t, sizeof(T), #T);
  g.render_pass_cache = CACHE_CREATE(1536, render_pass, Render_Pass);
  g.shader_cache = CACHE_CREATE(4096, shader_info, Shader_Info);
  g.ds_layout_cache = CACHE_CREATE(2048, ds_layout, DS_Layout);
  g.pipeline_layout_cache = CACHE_CREATE(1536, pipeline_layout, Pipeline_Layout);
#undef CACHE_CREATE

  return 0;

 end:
  g.memptr = 0;
  g.memright = ARR_SIZE(g.membuf);
  return err;
}

void
gfx_free()
{
  lru_cache_destroy(&g.pipeline_layout_cache);
  lru_cache_destroy(&g.ds_layout_cache);
  lru_cache_destroy(&g.shader_cache);
  lru_cache_destroy(&g.render_pass_cache);

  vkDestroyCommandPool(g.logical_device, g.command_pool, NULL);

  vkDestroyDevice(g.logical_device, NULL);

  if (g.debug_report_callback)
    vkDestroyDebugReportCallbackEXT(g.instance, g.debug_report_callback, NULL);
  vkDestroyInstance(g.instance, NULL);

  g.memptr = 0;
  g.memright = ARR_SIZE(g.membuf);
}

int
gfx_is_initialised()
{
  return g.memptr != 0;
}

void
gfx_wait_idle_gpu()
{
  vkDeviceWaitIdle(g.logical_device);
}

int
gfx_create_graphics_pipelines(GFX_Pipeline* pipelines, uint32_t count, const GFX_Pipeline_Desc* descs)
{
  VkPipeline* handles                                           = alloca(count * sizeof(VkPipeline));
  VkGraphicsPipelineCreateInfo* create_infos                    = alloca(count * sizeof(VkGraphicsPipelineCreateInfo));
  VkPipelineShaderStageCreateInfo* stages                       = alloca(2 * count * sizeof(VkPipelineShaderStageCreateInfo));
  VkShaderModule* modules                                       = alloca(2 * count * sizeof(VkShaderModule));
  VkPipelineVertexInputStateCreateInfo* vertex_input_states     = alloca(count * sizeof(VkPipelineVertexInputStateCreateInfo));
  VkPipelineInputAssemblyStateCreateInfo* input_assembly_states = alloca(count * sizeof(VkPipelineInputAssemblyStateCreateInfo));
  VkPipelineViewportStateCreateInfo* viewport_states            = alloca(count * sizeof(VkPipelineViewportStateCreateInfo));
  VkPipelineRasterizationStateCreateInfo* rasterization_states  = alloca(count * sizeof(VkPipelineRasterizationStateCreateInfo));
  VkPipelineMultisampleStateCreateInfo* multisample_states      = alloca(count * sizeof(VkPipelineMultisampleStateCreateInfo));
  VkPipelineDepthStencilStateCreateInfo* depth_stencil_states   = alloca(count * sizeof(VkPipelineDepthStencilStateCreateInfo));
  VkPipelineColorBlendStateCreateInfo* color_blend_states       = alloca(count * sizeof(VkPipelineColorBlendStateCreateInfo));
  VkPipelineDynamicStateCreateInfo* dynamic_states              = alloca(count * sizeof(VkPipelineDynamicStateCreateInfo));
  VkPipelineColorBlendAttachmentState* color_blend = alloca(count * sizeof(VkPipelineColorBlendAttachmentState));
  VkPipelineColorBlendStateCreateInfo* blend_states = alloca(count * sizeof(VkPipelineColorBlendStateCreateInfo));
  for (uint32_t i = 0; i < count; i++) {
    Shader_Info* shaders[2];
    // load vertex shader
    shaders[0] = create_shader(descs[i].vertex_shader);
    if (!shaders[0]) return -1;
    modules[2*i] = shaders[0]->module;
    stages[2*i] = (VkPipelineShaderStageCreateInfo) {
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_VERTEX_BIT,
        .module = modules[2*i],
        .pName  = "main"
    };
    // load fragment shader
    if (descs[i].fragment_shader) {
      shaders[1] = create_shader(descs[i].fragment_shader);
      if (!shaders[1]) return -1;
      modules[2*i+1] = shaders[1]->module;
      stages[2*i+1] = (VkPipelineShaderStageCreateInfo) {
        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = modules[2*i+1],
        .pName  = "main"
      };
    }
    // create pipeline layout
    Pipeline_Layout* layout = create_pipeline_layout(shaders, (descs[i].fragment_shader) ? 2 : 1);
    ((Pipeline*)&pipelines[i])->layout = layout->handle;
    // pipeline setup
    vertex_input_states[i] = (VkPipelineVertexInputStateCreateInfo) {
      .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount   = 0,
      .vertexAttributeDescriptionCount = 0,
    };
    input_assembly_states[i] = (VkPipelineInputAssemblyStateCreateInfo) {
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      // NOTE: we don't support primitiveRestartEnable
      .primitiveRestartEnable = VK_FALSE
    };
    viewport_states[i] = (VkPipelineViewportStateCreateInfo) {
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      // currently we always 1 viewport and 1 scissor, maybe I should
      // add an option to use multiple scissors. I saw that ImGui uses
      // multiple scissors for rendering.
      .viewportCount = 1,
      .pViewports    = 0,
      .scissorCount  = 1,
      .pScissors     = 0,
    };
    rasterization_states[i] = (VkPipelineRasterizationStateCreateInfo) {
      .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable        = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode             = VK_POLYGON_MODE_FILL,
      .cullMode                = VK_CULL_MODE_NONE,
      // we always use CCW
      .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable         = 0,
      .lineWidth               = 1.0,
    };

    multisample_states[i] = (VkPipelineMultisampleStateCreateInfo) {
      .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      // NOTE: we don't use MSAA in this engine
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable  = VK_FALSE,
    };

    depth_stencil_states[i] = (VkPipelineDepthStencilStateCreateInfo) {
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable       = VK_FALSE,
      .depthWriteEnable      = VK_FALSE,
      .depthCompareOp        = VK_COMPARE_OP_GREATER,
      // we're not using depth bounds
      .depthBoundsTestEnable = VK_FALSE,
    };
    color_blend_states[i] = (VkPipelineColorBlendStateCreateInfo) {
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable   = VK_FALSE,
      // .logicOp         = descs[i].blend_logic_op,
      // .attachmentCount = descs[i].attachment_count,
      // .pAttachments    = descs[i].attachments,
    };

    color_blend[i] = (VkPipelineColorBlendAttachmentState) {
      .blendEnable = VK_FALSE,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT
    };
    blend_states[i] = (VkPipelineColorBlendStateCreateInfo) {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &color_blend[i],
    };

    static const VkDynamicState todo_remove_this_dynamic_states[] = {
      VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
    };

    dynamic_states[i] = (VkPipelineDynamicStateCreateInfo) {
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = ARR_SIZE(todo_remove_this_dynamic_states),
      .pDynamicStates    = todo_remove_this_dynamic_states,
    };

    Render_Pass* render_pass = (Render_Pass*)descs[i].render_pass;

    create_infos[i] = (VkGraphicsPipelineCreateInfo) {
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount          = (descs[i].fragment_shader) ? 2 : 1,
      .pStages             = &stages[i*2],
      .pVertexInputState   = &vertex_input_states[i],
      .pInputAssemblyState = &input_assembly_states[i],
      .pViewportState      = &viewport_states[i],
      .pRasterizationState = &rasterization_states[i],
      .pMultisampleState   = &multisample_states[i],
      // I think it's pretty convenient to specify depth_write = 0 and
      // depth_test = 0 to say that pipeline doesn't use depth buffer.
      // .pDepthStencilState  = (descs[i].depth_write || descs[i].depth_test) ? &depth_stencil_states[i] : NULL,
      .pDepthStencilState  = NULL,
      .pColorBlendState    = &blend_states[i],
      .pDynamicState       = &dynamic_states[i],
      .layout              = layout->handle,
      .renderPass          = render_pass->render_pass,
      .subpass             = 0
    };
  }
  VkResult err = vkCreateGraphicsPipelines(g.logical_device, VK_NULL_HANDLE, // TODO: pipeline caches
                                           count, create_infos, VK_NULL_HANDLE, handles);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to create some of pipelines with error %s", to_string_VkResult(err));
    return -1;
  }
  for (uint32_t i = 0; i < count; i++) {
    ((Pipeline*)&pipelines[i])->handle = handles[i];
  }
  return 0;
}

void
gfx_destroy_pipeline(GFX_Pipeline* pip)
{
  Pipeline* pipeline = (Pipeline*)pip;
  vkDestroyPipeline(g.logical_device, pipeline->handle, NULL);
}

#ifdef LIDA_GFX_USE_SDL
int
gfx_create_window_sdl(GFX_Window* win, SDL_Window* handle, int vsync)
{
  if (!gfx_is_initialised())
    return -1;
  Window* window = (Window*)win;

  // step 1: create Vulkan surface
  if (!SDL_Vulkan_CreateSurface(handle, g.instance, &window->surface)) {
    LOG_ERROR("failed to create Vulkan surface. Use 'SDL_GetError()' to retrieve error message.");
    return -1;
  }

  // step 2: create swapchain
  window->render_pass = VK_NULL_HANDLE;
  window->format.format = VK_FORMAT_UNDEFINED;
  window->swapchain = VK_NULL_HANDLE;
  window->present_mode = (vsync == 0) ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_FIFO_KHR;
  VkResult err = create_swapchain(window);
  if (err != VK_SUCCESS) {
    return err;
  }

  // step 3: create in-flight frames
  err = create_window_frames(window);
  if (err != VK_SUCCESS) {
    return err;
  }

  return 0;
}
#endif

void
gfx_destroy_window(GFX_Window* win)
{
  Window* window = (Window*)win;

  for (int i = 0; i < 2; i++) {
    vkDestroySemaphore(g.logical_device, window->frames[i].image_available, NULL);
  }
  vkDestroyFence(g.logical_device, window->resources_available_fence, NULL);
  vkDestroySemaphore(g.logical_device, window->render_finished_semaphore, NULL);

  for (uint32_t i = 0; i < window->num_images; i++) {
    vkDestroyFramebuffer(g.logical_device, window->images[i].framebuffer, NULL);
    vkDestroyImageView(g.logical_device, window->images[i].image_view, NULL);
  }
  vkDestroySwapchainKHR(g.logical_device, window->swapchain, NULL);
  vkDestroySurfaceKHR(g.instance, window->surface, NULL);

  window->swapchain = VK_NULL_HANDLE;
}

void
gfx_begin_commands(GFX_Window* win)
{
  Window* window = (Window*)win;
  Window_Frame* frame = &window->frames[window->frame_counter & 1];
  VkCommandBufferBeginInfo begin_info = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  vkBeginCommandBuffer(frame->cmd, &begin_info);
  g.current_cmd = frame->cmd;
}

int
gfx_swap_buffers(GFX_Window* win)
{
  Window* window = (Window*)win;
  Window_Frame* frame = &window->frames[window->frame_counter & 1];
  VkResult err = vkAcquireNextImageKHR(g.logical_device,
                                       window->swapchain,
                                       UINT64_MAX,
                                       frame->image_available,
                                       VK_NULL_HANDLE,
                                       &window->current_image);
  switch (err)
    {
    case VK_SUCCESS:
      break;
    case VK_SUBOPTIMAL_KHR:
      LOG_WARN("acquire next image: got VK_SUBOPTIMAL_KHR");
      break;
    default:
      LOG_ERROR("failed to acquire next swapchain image with error %s", to_string_VkResult(err));
      return err;
    }
  return 0;
}

int
gfx_submit_and_present(GFX_Window* win)
{
  Window* window = (Window*)win;
  Window_Frame* frame = &window->frames[window->frame_counter % 2];
  vkEndCommandBuffer(frame->cmd);
  VkResult err;
  // wait till commands from previous frame are done, so we can safely use GPU resources
  err = vkWaitForFences(g.logical_device, 1, &window->resources_available_fence,
                        VK_TRUE, UINT64_MAX);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to wait for fence with error %s", to_string_VkResult(err));
    return err;
  }
  err = vkResetFences(g.logical_device, 1, &window->resources_available_fence);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to reset fence before presenting image with error %s", to_string_VkResult(err));
    return err;
  }
  // submit commands
  VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  VkSubmitInfo submit_info = {
    .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount   = 1,
    .pWaitSemaphores      = &frame->image_available,
    .pWaitDstStageMask    = wait_stages,
    .commandBufferCount   = 1,
    .pCommandBuffers      = &frame->cmd,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores    = &window->render_finished_semaphore,
  };
  err = vkQueueSubmit(g.graphics_queue, 1, &submit_info, window->resources_available_fence);
  if (err != VK_SUCCESS) {
    LOG_ERROR("failed to submit commands to graphics queue with error %s", to_string_VkResult(err));
    return err;
  }
  // present image to screen
  VkResult present_results[1];
  VkPresentInfoKHR present_info = {
    .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores    = &window->render_finished_semaphore,
    .swapchainCount     = 1,
    .pSwapchains        = &window->swapchain,
    .pImageIndices      = &window->current_image,
    .pResults           = present_results,
  };
  err = vkQueuePresentKHR(g.graphics_queue, &present_info);
  if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
    LOG_ERROR("queue failed to present with error %s", to_string_VkResult(err));
  }
  window->frame_counter++;
  window->current_image = UINT32_MAX;
  return 0;
}

GFX_Render_Pass*
gfx_get_main_pass(GFX_Window* win)
{
  Window* window = (Window*)win;
  return (GFX_Render_Pass*)window->render_pass;
}

void
gfx_begin_main_pass(GFX_Window* win)
{
  Window* window = (Window*)win;
  Window_Frame* frame = &window->frames[window->frame_counter % 2];
  VkRect2D render_area = { .offset = {0, 0},
                           .extent = window->swapchain_extent };
  VkRenderPassBeginInfo begin_info = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass      = window->render_pass->render_pass,
    .framebuffer     = window->images[window->current_image].framebuffer,
    .renderArea      = render_area,
    .clearValueCount = 0,
  };
  vkCmdBeginRenderPass(frame->cmd, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
  VkViewport viewport = {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = (float)render_area.extent.width,
    .height   = (float)render_area.extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(frame->cmd, 0, 1, &viewport);
  vkCmdSetScissor(frame->cmd, 0, 1, &render_area);
}

GFX_Render_Pass*
gfx_render_pass(const GFX_Attachment_Info* attachments, uint32_t count)
{
  return (GFX_Render_Pass*)create_render_pass(attachments, count);
}

void
gfx_end_render_pass()
{
  vkCmdEndRenderPass(g.current_cmd);
}

void
gfx_bind_pipeline(GFX_Pipeline* pip)
{
  Pipeline* pipeline = (Pipeline*)pip;
  vkCmdBindPipeline(g.current_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
}

void
gfx_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
  vkCmdDraw(g.current_cmd, vertex_count, instance_count, first_vertex, first_instance);
}
