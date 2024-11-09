/**
 * MIT License
 *
 * Copyright (c) 2024 Sheldon Yancy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CGPPY_YVULKANRESOURCE_H
#define CGPPY_YVULKANRESOURCE_H


#include "YDefines.h"
#include "YVulkanTypes.h"



#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsVkBuffer {
    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;
} YsVkBuffer;

typedef struct YsVkImage {
    VkImageCreateInfo* create_info;

    VkImage handle;

    VkDeviceMemory memory;

    VkImageView* individual_views;
    VkImageView common_view;

    VkMemoryRequirements memory_requirements;
    VkMemoryPropertyFlags memory_flags;
} YsVkImage;

typedef struct YsVkDescription {
    u32 first_set;
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet* descriptor_sets;
} YsVkDescription;

typedef struct YsVkResources {
    //
    void (*initialize)(struct YsVkContext* context,
                       struct YsVkResources* resource,
                       vec2 shadow_map_image_resolution,
                       vec2 path_tracing_image_resolution);

    void (*createVertexInputBuffer)(struct YsVkContext* context,
                                    struct YsVkResources* resource,
                                    u32 vertex_count);

    void (*updateVertexInputBuffer)(struct YsVkContext* context,
                                    struct YsVkResources* resource,
                                    struct YsVkCommandUnit* command_unit,
                                    void* vertex_position_data,
                                    void* vertex_normal_data,
                                    void* vertex_material_id_data);

    void (*updateGlobalUboBuffer)(struct YsVkContext* context,
                                  struct YsVkResources* resource,
                                  void* data);

    void (*createGlobalSceneBlockBuffer)(struct YsVkContext* context,
                                         struct YsVkResources* resource,
                                         u32 data_size);

    void (*updateGlobalSceneBlockBuffer)(struct YsVkContext* context,
                                         struct YsVkResources* resource,
                                         void* data);

    void (*updateDescriptorSets)(struct YsVkContext* context,
                                 struct YsVkResources* resource,
                                 struct YsVkRenderingSystem* rendering_system,
                                 u32 image_index);

    void (*cmdPushConstants)(VkCommandBuffer command_buffer,
                             struct YsVkPipeline* pipeline,
                             void* data);

    void (*updateRandomImage)(struct YsVkContext* context,
                              struct YsVkResources* resource,
                              struct YsVkCommandUnit* command_unit,
                              vec2 resolution);

    b8 (*bufferCreate)(struct YsVkContext* context,
                       u64 size,
                       VkBufferUsageFlagBits usage,
                       u32 memory_property_flags,
                       b8 bind_on_create,
                       YsVkBuffer* out_buffer);

    void (*bufferDestroy)(struct YsVkContext* context, YsVkBuffer* buffer);

    void (*bufferBind)(struct YsVkContext* context,
                       YsVkBuffer* buffer,
                       u64 offset);

    void (*bufferLoadDataRange)(struct YsVkContext* context,
                                struct YsVkCommandUnit* command_unit,
                                VkFence fence,
                                YsVkBuffer* buffer,
                                u64 offset,
                                void* data);

    void (*bufferCopyToBuffer)(struct YsVkContext* context,
                               struct YsVkCommandUnit* command_unit,
                               VkFence fence,
                               VkBuffer source,
                               u64 source_offset,
                               VkBuffer dest,
                               u64 dest_offset,
                               u64 size);

    void (*bufferCopyToImage)(struct YsVkContext* context,
                              struct YsVkCommandUnit* command_unit,
                              VkFence fence,
                              YsVkBuffer* buffer,
                              struct YsVkImage* image);

    void (*imageCreate)(struct YsVkContext* context,
                        VkImageCreateInfo* create_info,
                        VkMemoryPropertyFlags memory_flags,
                        b8 create_individual_views,
                        b8 create_common_view,
                        VkImageAspectFlags view_aspect_flags,
                        YsVkImage* out_image);

    void (*transitionImageLayout)(VkCommandBuffer command_buffer,
                                  YsVkImage* image,
                                  u32 base_array_layer,
                                  u32 layer_count,
                                  VkImageLayout old_layout,
                                  VkImageLayout new_layout,
                                  VkAccessFlags src_access_mask,
                                  VkAccessFlags dst_access_mask,
                                  uint32_t src_queue_family_index,
                                  uint32_t dst_queue_family_index,
                                  VkPipelineStageFlags src_stage_mask,
                                  VkPipelineStageFlags dst_stage_mask);

    void (*imageCopyFromBuffer)(YsVkImage* image,
                                VkBuffer buffer,
                                u64 offset,
                                VkCommandBuffer command_buffer);

    void (*imageCopyToBuffer)(YsVkImage* image,
                              VkBuffer buffer,
                              VkCommandBuffer command_buffer);

    void (*imageCopyPixelToBuffer)(YsVkImage* image,
                                   VkBuffer buffer,
                                   u32 x,
                                   u32 y,
                                   VkCommandBuffer command_buffer);

    void (*imageClear)(struct YsVkContext* context,
                       struct YsVkCommandUnit* command_unit,
                       YsVkImage* image,
                       u32 base_array_layer,
                       u32 layer_count);

    void (*imageDestroy)(struct YsVkContext* context, YsVkImage* image);

    //
    u32 current_draw_vertex_count;

    VkVertexInputBindingDescription vertex_position_binding_description;
    VkVertexInputAttributeDescription vertex_position_attribute_description;
    struct YsVkBuffer* vertex_input_position_buffer;

    VkVertexInputBindingDescription vertex_normal_binding_description;
    VkVertexInputAttributeDescription vertex_normal_attribute_description;
    struct YsVkBuffer* vertex_input_normal_buffer;

    VkVertexInputBindingDescription vertex_material_id_binding_description;
    VkVertexInputAttributeDescription vertex_material_id_attribute_description;
    struct YsVkBuffer* vertex_input_material_id_buffer;

    //
    YsVkDescription* global_ubo_descriptor;
    struct YsVkBuffer* global_ubo_buffer;

    YsVkDescription* global_scene_block_descriptor;
    struct YsVkBuffer* global_scene_block_buffer;

    YsVkDescription* image_descriptor;
    YsVkImage rasterization_color_image;
    YsVkImage rasterization_depth_image;
    YsVkImage shadow_map_image;
    YsVkImage path_tracing_random_image;
    YsVkImage path_tracing_accumulate_image;

    //
    VkSampler sampler_linear;
    VkSampler sampler_nearest;

    //
    u32 push_constant_range_count;
    VkPushConstantRange* push_constant_range;
} YsVkResources;

YsVkResources* yVkAllocateResourcesObject();


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANRESOURCE_H
