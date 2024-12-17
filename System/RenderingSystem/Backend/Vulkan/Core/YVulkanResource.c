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

#include "YVulkanResource.h"
#include "YVulkanBuffer.h"
#include "YVulkanImage.h"
#include "YVulkanContext.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanShadowMappingSystem.h"
#include "YVulkanPathTracingSystem.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YGlobalFunction.h"

#include "stb_image_write.h"


// Vertex
static void createVertexInputBuffer(YsVkContext* context,
                                    YsVkResources* resource,
                                    u32 vertex_count) {
    resource->current_draw_vertex_count = vertex_count;

    VkMemoryPropertyFlagBits memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | 
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    resource->vertex_input_position_buffer = yVkAllocateBufferObject();
    if (!resource->vertex_input_position_buffer->create(context,
                                                        sizeof(vec4) * vertex_count,
                                                        usage,
                                                        memory_property_flags,
                                                        resource->vertex_input_position_buffer)) {
        YERROR("Error Create VkBuffer");
    }

    resource->vertex_input_normal_buffer = yVkAllocateBufferObject();
    if (!resource->vertex_input_normal_buffer->create(context,
                                                      sizeof(vec4) * vertex_count,
                                                      usage,
                                                      memory_property_flags,
                                                      resource->vertex_input_normal_buffer)) {
        YERROR("Error Create VkBuffer");
    }

    resource->vertex_input_material_id_buffer = yVkAllocateBufferObject();
    if (!resource->vertex_input_material_id_buffer->create(context,
                                                           sizeof(i32) * vertex_count,
                                                           usage,
                                                           memory_property_flags,
                                                           resource->vertex_input_material_id_buffer)) {
        YERROR("Error Create VkBuffer");
    }
}

static void createVertexInputDescription(YsVkContext* context, YsVkResources* resources) {
    resources->vertex_position_binding_description.binding = 0;
    resources->vertex_position_binding_description.stride = sizeof(vec4);
    resources->vertex_position_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    resources->vertex_position_attribute_description.binding = 0;
    resources->vertex_position_attribute_description.location = 0;
    resources->vertex_position_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    resources->vertex_position_attribute_description.offset = 0;

    resources->vertex_normal_binding_description.binding = 1;
    resources->vertex_normal_binding_description.stride = sizeof(vec4);
    resources->vertex_normal_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    resources->vertex_normal_attribute_description.binding = 1;
    resources->vertex_normal_attribute_description.location = 1;
    resources->vertex_normal_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    resources->vertex_normal_attribute_description.offset = 0;

    resources->vertex_material_id_binding_description.binding = 2;
    resources->vertex_material_id_binding_description.stride = sizeof(i32);
    resources->vertex_material_id_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    resources->vertex_material_id_attribute_description.binding = 2;
    resources->vertex_material_id_attribute_description.location = 2;
    resources->vertex_material_id_attribute_description.format = VK_FORMAT_R32_SINT;
    resources->vertex_material_id_attribute_description.offset = 0;
}

static void updateVertexInputBuffer(YsVkContext* context,
                                    YsVkResources* resource,
                                    YsVkCommandUnit* command_unit,
                                    void* vertex_position_data,
                                    void* vertex_normal_data,
                                    void* vertex_material_id_data) {
    resource->vertex_input_position_buffer->indirectUpdate(context,
                                                           command_unit,
                                                           0,
                                                           0,
                                                           vertex_position_data,
                                                           resource->vertex_input_position_buffer);
    resource->vertex_input_normal_buffer->indirectUpdate(context,
                                                         command_unit,
                                                         0,
                                                         0,
                                                         vertex_normal_data,
                                                         resource->vertex_input_normal_buffer);
    resource->vertex_input_material_id_buffer->indirectUpdate(context,
                                                              command_unit,
                                                              0,
                                                              0,
                                                              vertex_material_id_data,
                                                              resource->vertex_input_material_id_buffer);
}

// SSBO
static void createSsboBuffer(YsVkContext* context,
                             YsVkResources* resource,
                             u32 data_size) {
    resource->ssbo_buffer = yVkAllocateBufferObject();
    if (!resource->ssbo_buffer->create(context,
                                       data_size,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       resource->ssbo_buffer)) {
        YERROR("Error creating ssbo buffer.");
    }                        
}

static void createSsboDescriptor(YsVkContext* context, YsVkResources* resources) {
    resources->ssbo_descriptor.set = 0;
    resources->ssbo_descriptor.is_single_descriptor_set = true;

    VkDescriptorSetLayoutBinding ssbo_layout_binding = {};
    ssbo_layout_binding.binding = 0;
    ssbo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssbo_layout_binding.descriptorCount = 1;
    ssbo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | 
                                     VK_SHADER_STAGE_FRAGMENT_BIT |
                                     VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo ssbo_layout_info = {};
    ssbo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ssbo_layout_info.bindingCount = 1;
    ssbo_layout_info.pBindings = &ssbo_layout_binding;
    vkCreateDescriptorSetLayout(context->device->logical_device,
                                &ssbo_layout_info,
                                NULL,
                                &resources->ssbo_descriptor.descriptor_set_layout);

    VkDescriptorPoolSize ssbo_pool_size;
    ssbo_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssbo_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo ssbo_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    ssbo_pool_info.poolSizeCount = 1;
    ssbo_pool_info.pPoolSizes = &ssbo_pool_size;
    ssbo_pool_info.maxSets = 1;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &ssbo_pool_info,
                                    context->allocator,
                                    &resources->ssbo_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo ssbo_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ssbo_alloc_info.descriptorPool = resources->ssbo_descriptor.descriptor_pool;
    ssbo_alloc_info.descriptorSetCount = 1;
    ssbo_alloc_info.pSetLayouts = &resources->ssbo_descriptor.descriptor_set_layout;
    resources->ssbo_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * ssbo_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device, 
                                      &ssbo_alloc_info, 
                                      resources->ssbo_descriptor.descriptor_sets));
}

static void updateSsboDescriptorSets(YsVkContext* context, YsVkResources* resource) {
    VkDescriptorBufferInfo ssbo_buffer_info;
    ssbo_buffer_info.buffer = resource->ssbo_buffer->handle;
    ssbo_buffer_info.offset = 0;
    ssbo_buffer_info.range = resource->ssbo_buffer->total_size; 

    VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write_descriptor_set.dstSet = resource->ssbo_descriptor.descriptor_sets[0];
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.pBufferInfo = &ssbo_buffer_info;
    vkUpdateDescriptorSets(context->device->logical_device,
                           1,
                           &write_descriptor_set,
                           0,
                           0);
}

static void createSsbo(YsVkContext* context,
                       YsVkResources* resource,
                       u32 data_size) {
    createSsboBuffer(context,
                     resource,
                     data_size);                                 
    
    updateSsboDescriptorSets(context, resource);             
}

static void updateSsboBuffer(YsVkContext* context,
                             YsVkResources* resource,
                             YsVkCommandUnit* command_unit,
                             void* data) {
    resource->ssbo_buffer->indirectUpdate(context,
                                          command_unit,
                                          0,
                                          0,
                                          data,
                                          resource->ssbo_buffer);
}

// UBO
static void createUboBuffer(YsVkContext* context, YsVkResources* resource) {
    resource->ubo_buffer = yVkAllocateBufferObject();
    if (!resource->ubo_buffer->create(context,
                                      yUboSize(),
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      resource->ubo_buffer)) {
        YERROR("Error creating ubo buffer.");
    }
}

static void createUboDescription(YsVkContext* context, YsVkResources* resources) {
    resources->ubo_descriptor.set = 1;
    resources->ubo_descriptor.is_single_descriptor_set = true;
    
    VkDescriptorSetLayoutBinding ubo_layout_binding;
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.pImmutableSamplers = NULL;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | 
                                    VK_SHADER_STAGE_FRAGMENT_BIT |
                                    VK_SHADER_STAGE_COMPUTE_BIT;
    VkDescriptorSetLayoutCreateInfo ubo_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    ubo_layout_info.bindingCount = 1;
    ubo_layout_info.pBindings = &ubo_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &ubo_layout_info,
                                         context->allocator,
                                         &resources->ubo_descriptor.descriptor_set_layout));

    VkDescriptorPoolSize ubo_pool_size;
    ubo_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo ubo_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    ubo_pool_info.poolSizeCount = 1;
    ubo_pool_info.pPoolSizes = &ubo_pool_size;
    ubo_pool_info.maxSets = 1;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &ubo_pool_info,
                                    context->allocator,
                                    &resources->ubo_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo ubo_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ubo_alloc_info.descriptorPool = resources->ubo_descriptor.descriptor_pool;
    ubo_alloc_info.descriptorSetCount = 1;
    ubo_alloc_info.pSetLayouts = &resources->ubo_descriptor.descriptor_set_layout;
    resources->ubo_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * ubo_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device, 
                                      &ubo_alloc_info, 
                                      resources->ubo_descriptor.descriptor_sets));
}

static void updateUboDescriptorSets(YsVkContext* context, YsVkResources* resource) {
    VkDescriptorBufferInfo ubo_buffer_info;
    ubo_buffer_info.buffer = resource->ubo_buffer->handle;
    ubo_buffer_info.offset = 0;
    ubo_buffer_info.range = resource->ubo_buffer->total_size;   

    VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write_descriptor_set.dstSet = resource->ubo_descriptor.descriptor_sets[0];
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.pBufferInfo = &ubo_buffer_info;

    vkUpdateDescriptorSets(context->device->logical_device,
                           1,
                           &write_descriptor_set,
                           0,
                           0);
}

static void createUbo(YsVkContext* context,
                      YsVkResources* resource) {
    createUboBuffer(context, resource);                    

    createUboDescription(context, resource);

    updateUboDescriptorSets(context, resource);                                     
}

static void updateUboBuffer(struct YsVkContext* context,
                            struct YsVkResources* resource,
                            void* data) {
    resource->ubo_buffer->directUpdate(context,
                                       0,
                                       0,
                                       data,
                                       resource->ubo_buffer);
}

// Push Constant
static void createPushConstants(YsVkResources* resource) {
    resource->push_constant_range_count = 1;
    resource->push_constant_range = yCMemoryAllocate(sizeof(VkPushConstantRange));
    resource->push_constant_range[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | 
                                                  VK_SHADER_STAGE_FRAGMENT_BIT |
                                                  VK_SHADER_STAGE_COMPUTE_BIT;
    resource->push_constant_range[0].offset = 0;
    resource->push_constant_range[0].size = yPushConstantSize();
}

// Sampler
static void createSampler(YsVkContext* context, YsVkResources* resource) {
    //
    VkSamplerCreateInfo linear_sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    linear_sampler_info.magFilter = VK_FILTER_LINEAR;
    linear_sampler_info.minFilter = VK_FILTER_LINEAR;
    linear_sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    linear_sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    linear_sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    linear_sampler_info.maxAnisotropy = 16;
    linear_sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    linear_sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    linear_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    linear_sampler_info.mipLodBias = 0.0f;
    linear_sampler_info.minLod = 0.0f;
    linear_sampler_info.maxLod = 1.0f;
    VK_CHECK(vkCreateSampler(context->device->logical_device,
                             &linear_sampler_info,
                             context->allocator,
                             &resource->sampler_linear));

    VkSamplerCreateInfo nearest_sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    nearest_sampler_info.magFilter = VK_FILTER_NEAREST;
    nearest_sampler_info.minFilter = VK_FILTER_NEAREST;
    nearest_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    nearest_sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    nearest_sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    nearest_sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    nearest_sampler_info.maxAnisotropy = 16;
    nearest_sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    nearest_sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    nearest_sampler_info.mipLodBias = 0.0f;
    nearest_sampler_info.minLod = 0.0f;
    nearest_sampler_info.maxLod = 0.0f;
    VK_CHECK(vkCreateSampler(context->device->logical_device,
                             &nearest_sampler_info,
                             context->allocator,
                             &resource->sampler_nearest));
}

// Image Rasterization
static void createRasterizationImage(YsVkContext* context, 
                                     YsVkResources* resource,
                                     u32 width,
                                     u32 height) {
    //                                    
    VkImageCreateInfo* color_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    color_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    color_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    color_image_create_info->extent.width = width;
    color_image_create_info->extent.height = height;
    color_image_create_info->extent.depth = 1;
    color_image_create_info->mipLevels = 1;
    color_image_create_info->arrayLayers = context->swapchain->image_count;
    color_image_create_info->format = VK_FORMAT_R32G32B32A32_SFLOAT;
    color_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    color_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_image_create_info->usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    color_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    color_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    resource->rasterization_color_image = yVkAllocateImageObject();
    resource->rasterization_color_image->create(context,
                                                color_image_create_info,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                VK_IMAGE_ASPECT_COLOR_BIT,
                                                resource->rasterization_color_image);

    //
    VkImageCreateInfo* depth_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    depth_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    depth_image_create_info->extent.width = width;
    depth_image_create_info->extent.height = height;
    depth_image_create_info->extent.depth = 1;
    depth_image_create_info->mipLevels = 1;
    depth_image_create_info->arrayLayers = context->swapchain->image_count;
    depth_image_create_info->format = context->device->depth_format;
    depth_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    depth_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_image_create_info->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    depth_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    depth_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    resource->rasterization_depth_image = yVkAllocateImageObject();
    resource->rasterization_depth_image->create(context,
                                                depth_image_create_info,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                VK_IMAGE_ASPECT_DEPTH_BIT,
                                                resource->rasterization_depth_image);
}

static void createRasterizationImageDescriptor(YsVkContext* context, YsVkResources* resources) {
    resources->rasterization_color_image_descriptor.set = 2;
    resources->rasterization_color_image_descriptor.is_single_descriptor_set = false;

    VkDescriptorSetLayoutBinding image_layout_binding;
    image_layout_binding.binding = 0;
    image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding.descriptorCount = 1;
    image_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo image_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    image_layout_info.bindingCount = 1;
    image_layout_info.pBindings = &image_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &image_layout_info,
                                         context->allocator,
                                         &resources->rasterization_color_image_descriptor.descriptor_set_layout));

    VkDescriptorPoolSize image_pool_size;
    image_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo image_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    image_pool_info.poolSizeCount = 1;
    image_pool_info.pPoolSizes = &image_pool_size;
    image_pool_info.maxSets = resources->rasterization_color_image->create_info->arrayLayers;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &image_pool_info,
                                    context->allocator,
                                    &resources->rasterization_color_image_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo image_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    image_alloc_info.descriptorPool = resources->rasterization_color_image_descriptor.descriptor_pool;
    image_alloc_info.descriptorSetCount = resources->rasterization_color_image->create_info->arrayLayers;
    VkDescriptorSetLayout set_layouts[image_alloc_info.descriptorSetCount] = {};
    for(int i = 0; i < image_alloc_info.descriptorSetCount; ++i) {
        set_layouts[i] = resources->rasterization_color_image_descriptor.descriptor_set_layout;
    }
    image_alloc_info.pSetLayouts = set_layouts;
    resources->rasterization_color_image_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * image_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device,
                                      &image_alloc_info,
                                      resources->rasterization_color_image_descriptor.descriptor_sets));
}

static void updateRasterizationImageDescriptor(YsVkContext* context, YsVkResources* resource) {
    for(int i = 0; i < resource->rasterization_color_image->create_info->arrayLayers; ++i) {
        VkDescriptorImageInfo rasterization_image_info;
        rasterization_image_info.sampler = resource->sampler_linear;
        rasterization_image_info.imageView = resource->rasterization_color_image->layer_views[i];
        rasterization_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write_descriptor_set.dstSet = resource->rasterization_color_image_descriptor.descriptor_sets[i];
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pImageInfo = &rasterization_image_info;
        vkUpdateDescriptorSets(context->device->logical_device,
                               1,
                               &write_descriptor_set,
                               0,
                               NULL);
    }
}

// Image Shadow Map
static void createShadowMapImage(YsVkContext* context, 
                                 YsVkResources* resource,
                                 u32 width,
                                 u32 height) {
    VkImageCreateInfo* shadow_map_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    shadow_map_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    shadow_map_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    shadow_map_image_create_info->extent.width = width;
    shadow_map_image_create_info->extent.height = height;
    shadow_map_image_create_info->extent.depth = 1;
    shadow_map_image_create_info->mipLevels = 1;
    shadow_map_image_create_info->arrayLayers = context->swapchain->image_count;
    shadow_map_image_create_info->format = context->device->depth_format;
    shadow_map_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    shadow_map_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    shadow_map_image_create_info->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                          VK_IMAGE_USAGE_SAMPLED_BIT |
                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    shadow_map_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    shadow_map_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    resource->shadow_map_image = yVkAllocateImageObject();
    resource->shadow_map_image->create(context,
                                       shadow_map_image_create_info,
                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       VK_IMAGE_ASPECT_DEPTH_BIT,
                                       resource->shadow_map_image);
}

static void createShadowMapImageDescriptor(YsVkContext* context, YsVkResources* resources) {
    resources->shadow_map_image_descriptor.set = 3;
    resources->shadow_map_image_descriptor.is_single_descriptor_set = false;
    
    VkDescriptorSetLayoutBinding image_layout_binding;
    image_layout_binding.binding = 0;
    image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding.descriptorCount = 1;
    image_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo image_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    image_layout_info.bindingCount = 1;
    image_layout_info.pBindings = &image_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &image_layout_info,
                                         context->allocator,
                                         &resources->shadow_map_image_descriptor.descriptor_set_layout));

    VkDescriptorPoolSize image_pool_size;
    image_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo image_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    image_pool_info.poolSizeCount = 1;
    image_pool_info.pPoolSizes = &image_pool_size;
    image_pool_info.maxSets = resources->shadow_map_image->create_info->arrayLayers;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &image_pool_info,
                                    context->allocator,
                                    &resources->shadow_map_image_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo image_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    image_alloc_info.descriptorPool = resources->shadow_map_image_descriptor.descriptor_pool;
    image_alloc_info.descriptorSetCount = resources->shadow_map_image->create_info->arrayLayers;
    VkDescriptorSetLayout set_layouts[image_alloc_info.descriptorSetCount] = {};
    for(int i = 0; i < image_alloc_info.descriptorSetCount; ++i) {
        set_layouts[i] = resources->shadow_map_image_descriptor.descriptor_set_layout;
    }
    image_alloc_info.pSetLayouts = set_layouts;
    resources->shadow_map_image_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * image_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device,
                                      &image_alloc_info,
                                      resources->shadow_map_image_descriptor.descriptor_sets));                                     
}

static void updateShadowMapImageDescriptor(YsVkContext* context, YsVkResources* resource) {
    for(int i = 0; i < resource->shadow_map_image->create_info->arrayLayers; ++i) {
        VkDescriptorImageInfo shadow_map_image_info;
        shadow_map_image_info.sampler = resource->sampler_linear;
        shadow_map_image_info.imageView = resource->shadow_map_image->layer_views[i];
        shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write_descriptor_set.dstSet = resource->shadow_map_image_descriptor.descriptor_sets[i];
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pImageInfo = &shadow_map_image_info;
        vkUpdateDescriptorSets(context->device->logical_device,
                               1,
                               &write_descriptor_set,
                               0,
                               0);
    }
}

// Image Random
static void createRandomImage(YsVkContext* context, 
                              YsVkResources* resource,
                              u32 width,
                              u32 height) {
    //
    VkImageCreateInfo* random_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    random_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    random_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    random_image_create_info->extent.width = width;
    random_image_create_info->extent.height = height;
    random_image_create_info->extent.depth = 1;
    random_image_create_info->mipLevels = 1;
    random_image_create_info->arrayLayers = 1;
    random_image_create_info->format = VK_FORMAT_R32_UINT;
    random_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    random_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    random_image_create_info->usage = VK_IMAGE_USAGE_SAMPLED_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    random_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    random_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    resource->random_image = yVkAllocateImageObject();
    resource->random_image->create(context,
                                   random_image_create_info,
                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT,
                                   resource->random_image);
}

static void createRandomImageDescriptor(YsVkContext* context, YsVkResources* resources) {
    resources->random_image_descriptor.set = 4;
    resources->random_image_descriptor.is_single_descriptor_set = true;

    VkDescriptorSetLayoutBinding image_layout_binding;
    image_layout_binding.binding = 0;
    image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding.descriptorCount = 1;
    image_layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    image_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo image_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    image_layout_info.bindingCount = 1;
    image_layout_info.pBindings = &image_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &image_layout_info,
                                         context->allocator,
                                         &resources->random_image_descriptor.descriptor_set_layout));

    VkDescriptorPoolSize image_pool_size;
    image_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo image_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    image_pool_info.poolSizeCount = 1;
    image_pool_info.pPoolSizes = &image_pool_size;
    image_pool_info.maxSets = resources->random_image->create_info->arrayLayers;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &image_pool_info,
                                    context->allocator,
                                    &resources->random_image_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo image_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    image_alloc_info.descriptorPool = resources->random_image_descriptor.descriptor_pool;
    image_alloc_info.descriptorSetCount = resources->random_image->create_info->arrayLayers;
    VkDescriptorSetLayout set_layouts[image_alloc_info.descriptorSetCount] = {};
    for(int i = 0; i < image_alloc_info.descriptorSetCount; ++i) {
        set_layouts[i] = resources->random_image_descriptor.descriptor_set_layout;
    }
    image_alloc_info.pSetLayouts = set_layouts;
    resources->random_image_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * image_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device,
                                      &image_alloc_info,
                                      resources->random_image_descriptor.descriptor_sets));     
}

static void updateRandomImageDescriptor(YsVkContext* context, YsVkResources* resource) {
    VkDescriptorImageInfo random_image_info;
    random_image_info.sampler = resource->sampler_nearest;
    random_image_info.imageView = resource->random_image->image_view;
    random_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write_descriptor_set.dstSet = resource->random_image_descriptor.descriptor_sets[0];
    write_descriptor_set.dstBinding = 0;
    write_descriptor_set.dstArrayElement = 0;
    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.pImageInfo = &random_image_info;
    vkUpdateDescriptorSets(context->device->logical_device,
                           1,
                           &write_descriptor_set,
                           0,
                           0);
}

// Image Path Tracing
static void createPathTracingImage(YsVkContext* context, 
                                   YsVkResources* resource,
                                   u32 width,
                                   u32 height) {
    //
    VkImageCreateInfo* path_tracing_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    path_tracing_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    path_tracing_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    path_tracing_image_create_info->extent.width = width;
    path_tracing_image_create_info->extent.height = height;
    path_tracing_image_create_info->extent.depth = 1;
    path_tracing_image_create_info->mipLevels = 1;
    path_tracing_image_create_info->arrayLayers = context->swapchain->image_count;
    path_tracing_image_create_info->format = VK_FORMAT_R32G32B32A32_SFLOAT;
    path_tracing_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    path_tracing_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    path_tracing_image_create_info->usage = VK_IMAGE_USAGE_SAMPLED_BIT |
                                            VK_IMAGE_USAGE_STORAGE_BIT;
    path_tracing_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    path_tracing_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    resource->path_tracing_image = yVkAllocateImageObject();
    resource->path_tracing_image->create(context,
                                         path_tracing_image_create_info,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                         VK_IMAGE_ASPECT_COLOR_BIT,
                                         resource->path_tracing_image);
}

static void createPathTracingImageComputeStorageDescriptor(YsVkContext* context, YsVkResources* resources) {
    resources->path_tracing_image_compute_storage_descriptor.set = 5;
    resources->path_tracing_image_compute_storage_descriptor.is_single_descriptor_set = false;
    
    VkDescriptorSetLayoutBinding image_layout_binding;
    image_layout_binding.binding = 0;
    image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    image_layout_binding.descriptorCount = 1;
    image_layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    image_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo image_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    image_layout_info.bindingCount = 1;
    image_layout_info.pBindings = &image_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &image_layout_info,
                                         context->allocator,
                                         &resources->path_tracing_image_compute_storage_descriptor.descriptor_set_layout));

    VkDescriptorPoolSize image_pool_size;
    image_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    image_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo image_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    image_pool_info.poolSizeCount = 1;
    image_pool_info.pPoolSizes = &image_pool_size;
    image_pool_info.maxSets = resources->path_tracing_image->create_info->arrayLayers;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &image_pool_info,
                                    context->allocator,
                                    &resources->path_tracing_image_compute_storage_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo image_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    image_alloc_info.descriptorPool = resources->path_tracing_image_compute_storage_descriptor.descriptor_pool;
    image_alloc_info.descriptorSetCount = resources->path_tracing_image->create_info->arrayLayers;
    VkDescriptorSetLayout set_layouts[image_alloc_info.descriptorSetCount] = {};
    for(int i = 0; i < image_alloc_info.descriptorSetCount; ++i) {
        set_layouts[i] = resources->path_tracing_image_compute_storage_descriptor.descriptor_set_layout;
    }
    image_alloc_info.pSetLayouts = set_layouts;
    resources->path_tracing_image_compute_storage_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * image_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device,
                                      &image_alloc_info,
                                      resources->path_tracing_image_compute_storage_descriptor.descriptor_sets));        
}

static void updatePathTracingImageComputeStorageDescriptor(YsVkContext* context, YsVkResources* resource) {
    for(int i = 0; i <resource->path_tracing_image->create_info->arrayLayers; ++i) {
        VkDescriptorImageInfo path_tracing_image_info;
        path_tracing_image_info.sampler = resource->sampler_linear;
        path_tracing_image_info.imageView = resource->path_tracing_image->layer_views[i];
        path_tracing_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
        VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write_descriptor_set.dstSet = resource->path_tracing_image_compute_storage_descriptor.descriptor_sets[i];
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pImageInfo = &path_tracing_image_info;
        vkUpdateDescriptorSets(context->device->logical_device,
                               1,
                               &write_descriptor_set,
                               0,
                               0);
    }
}

static void createPathTracingImageFragmentSampledDescriptor(YsVkContext* context, YsVkResources* resources) {
    resources->path_tracing_image_fragment_sampled_descriptor.set = 6;
    resources->path_tracing_image_fragment_sampled_descriptor.is_single_descriptor_set = false;
    
    VkDescriptorSetLayoutBinding image_layout_binding;
    image_layout_binding.binding = 0;
    image_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding.descriptorCount = 1;
    image_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding.pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo image_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    image_layout_info.bindingCount = 1;
    image_layout_info.pBindings = &image_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &image_layout_info,
                                         context->allocator,
                                         &resources->path_tracing_image_fragment_sampled_descriptor.descriptor_set_layout));

    VkDescriptorPoolSize image_pool_size;
    image_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size.descriptorCount = 1;
    VkDescriptorPoolCreateInfo image_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    image_pool_info.poolSizeCount = 1;
    image_pool_info.pPoolSizes = &image_pool_size;
    image_pool_info.maxSets = resources->path_tracing_image->create_info->arrayLayers;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &image_pool_info,
                                    context->allocator,
                                    &resources->path_tracing_image_fragment_sampled_descriptor.descriptor_pool));

    VkDescriptorSetAllocateInfo image_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    image_alloc_info.descriptorPool = resources->path_tracing_image_fragment_sampled_descriptor.descriptor_pool;
    image_alloc_info.descriptorSetCount = resources->path_tracing_image->create_info->arrayLayers;
    VkDescriptorSetLayout set_layouts[image_alloc_info.descriptorSetCount] = {};
    for(int i = 0; i < image_alloc_info.descriptorSetCount; ++i) {
        set_layouts[i] = resources->path_tracing_image_fragment_sampled_descriptor.descriptor_set_layout;
    }
    image_alloc_info.pSetLayouts = set_layouts;
    resources->path_tracing_image_fragment_sampled_descriptor.descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * image_alloc_info.descriptorSetCount);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device,
                                      &image_alloc_info,
                                      resources->path_tracing_image_fragment_sampled_descriptor.descriptor_sets)); 
}

static void updatePathTracingImageFragmentSampledDescriptor(YsVkContext* context, YsVkResources* resource) {
    for(int i = 0; i <resource->path_tracing_image->create_info->arrayLayers; ++i) {
         VkDescriptorImageInfo path_tracing_image_info;
        path_tracing_image_info.sampler = resource->sampler_linear;
        path_tracing_image_info.imageView = resource->path_tracing_image->layer_views[i];
        path_tracing_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        write_descriptor_set.dstSet = resource->path_tracing_image_fragment_sampled_descriptor.descriptor_sets[i];
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstArrayElement = 0;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.pImageInfo = &path_tracing_image_info;
        vkUpdateDescriptorSets(context->device->logical_device,
                               1,
                               &write_descriptor_set,
                               0,
                               0);
    }
}

//
static void initialize(YsVkContext* context, 
                       YsVkResources* resource,
                       struct YsVkResourcesImageSize image_size) {
    // Vertex
    createVertexInputDescription(context, resource);

    // SSBO
    createSsboDescriptor(context, resource); 
    
    // UBO
    createUbo(context, resource);
    
    // Push Constant
    createPushConstants(resource);

    // Image   
    createSampler(context, resource);

    createRasterizationImage(context, 
                             resource,
                             image_size.rasterization_image_width,
                             image_size.rasterization_image_height);
    createRasterizationImageDescriptor(context, resource);
    updateRasterizationImageDescriptor(context, resource);

    createShadowMapImage(context, 
                         resource,
                         image_size.shadow_map_image_width,
                         image_size.shadow_map_image_height);
    createShadowMapImageDescriptor(context, resource);
    updateShadowMapImageDescriptor(context, resource);

    createRandomImage(context, 
                      resource,
                      image_size.random_image_width,
                      image_size.random_image_height);
    createRandomImageDescriptor(context, resource);
    updateRandomImageDescriptor(context, resource);

    createPathTracingImage(context, 
                           resource,
                           image_size.path_tracing_image_width,
                           image_size.path_tracing_image_height);
    createPathTracingImageComputeStorageDescriptor(context, resource);
    updatePathTracingImageComputeStorageDescriptor(context, resource);
    createPathTracingImageFragmentSampledDescriptor(context, resource);
    updatePathTracingImageFragmentSampledDescriptor(context, resource);

    //
    u32 pixel_count = resource->random_image->create_info->extent.width * resource->random_image->create_info->extent.height;
    u32* rand_data = yCMemoryAllocate(sizeof(u32) * pixel_count);
    yGenerateUintRand(pixel_count, rand_data);
    resource->random_image->updatImageData(context,
                                           context->device->commandUnitsFront(context->device),
                                           sizeof(u32) * pixel_count,
                                           rand_data,
                                           resource->random_image);

    //
    vkQueueWaitIdle(context->device->commandUnitsFront(context->device)->queue);
    VkCommandBuffer temp_command_buffer;
    context->device->commandBufferAllocateAndBeginSingleUse(context,
                                                            context->device->commandUnitsFront(context->device),
                                                            &temp_command_buffer);

    resource->path_tracing_image->transitionLayout(temp_command_buffer,
                                                   0,
                                                   resource->path_tracing_image->create_info->arrayLayers,
                                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                   VK_ACCESS_NONE,
                                                   VK_ACCESS_NONE,
                                                   context->device->commandUnitsFront(context->device)->queue_family_index,
                                                   context->device->commandUnitsFront(context->device)->queue_family_index,
                                                   VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                   VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                                   resource->path_tracing_image);

    context->device->commandBufferEndSingleUse(context,
                                               context->device->commandUnitsFront(context->device),
                                               &temp_command_buffer);                                                                                               
}

YsVkResources* yVkAllocateResourcesObject() {
    YsVkResources* vk_resources = yCMemoryAllocate(sizeof(YsVkResources));
    if(vk_resources) {
        vk_resources->initialize = initialize;
        vk_resources->createVertexInputBuffer = createVertexInputBuffer;
        vk_resources->updateVertexInputBuffer = updateVertexInputBuffer;
        vk_resources->createSsbo = createSsbo;
        vk_resources->updateSsboBuffer = updateSsboBuffer;
        vk_resources->updateUboBuffer = updateUboBuffer;
    }
    return vk_resources;
}

