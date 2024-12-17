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


#include "YVulkanTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


struct YsVkResourcesImageSize {
    u32 rasterization_image_width;
    u32 rasterization_image_height;

    u32 shadow_map_image_width;
    u32 shadow_map_image_height;

    u32 random_image_width;
    u32 random_image_height;

    u32 path_tracing_image_width;
    u32 path_tracing_image_height;
};

typedef struct YsVkResources {
    //
    void (*initialize)(struct YsVkContext* context, 
                       struct YsVkResources* resource,
                       struct YsVkResourcesImageSize image_size);

    // Vertex
    void (*createVertexInputBuffer)(struct YsVkContext* context,
                                   struct YsVkResources* resource,
                                   u32 vertex_count);

    void (*updateVertexInputBuffer)(struct YsVkContext* context,
                                    struct YsVkResources* resource,
                                    struct YsVkCommandUnit* command_unit,
                                    void* vertex_position_data,
                                    void* vertex_normal_data,
                                    void* vertex_material_id_data);
    // SSBO
    void (*createSsbo)(struct YsVkContext* context,
                       struct YsVkResources* resource,
                       u32 data_size);

    void (*updateSsboBuffer)(struct YsVkContext* context,
                             struct YsVkResources* resource,
                             struct YsVkCommandUnit* command_unit,
                             void* data);

    // UBO
    void (*updateUboBuffer)(struct YsVkContext* context,
                            struct YsVkResources* resource,
                            void* data);

    // Vertex
    u32 current_draw_vertex_count;

    struct YsVkBuffer* vertex_input_position_buffer;
    VkVertexInputBindingDescription vertex_position_binding_description;
    VkVertexInputAttributeDescription vertex_position_attribute_description;

    struct YsVkBuffer* vertex_input_normal_buffer;
    VkVertexInputBindingDescription vertex_normal_binding_description;
    VkVertexInputAttributeDescription vertex_normal_attribute_description;

    struct YsVkBuffer* vertex_input_material_id_buffer;
    VkVertexInputBindingDescription vertex_material_id_binding_description;
    VkVertexInputAttributeDescription vertex_material_id_attribute_description;

    // SSBO
    struct YsVkBuffer* ssbo_buffer;
    YsVkDescriptor ssbo_descriptor;

    // UBO
    struct YsVkBuffer* ubo_buffer;
    YsVkDescriptor ubo_descriptor;
    
    // Push Constant
    u32 push_constant_range_count;
    VkPushConstantRange* push_constant_range;

    // Image
    struct YsVkImage* rasterization_color_image;
    YsVkDescriptor rasterization_color_image_descriptor;

    struct YsVkImage* rasterization_depth_image;

    struct YsVkImage* shadow_map_image;
    YsVkDescriptor shadow_map_image_descriptor;

    struct YsVkImage* random_image;
    YsVkDescriptor random_image_descriptor;

    struct YsVkImage* path_tracing_image;
    YsVkDescriptor path_tracing_image_compute_storage_descriptor;
    YsVkDescriptor path_tracing_image_fragment_sampled_descriptor;

    // Sampler
    VkSampler sampler_linear;
    VkSampler sampler_nearest;
} YsVkResources;
YsVkResources* yVkAllocateResourcesObject();


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANRESOURCE_H
