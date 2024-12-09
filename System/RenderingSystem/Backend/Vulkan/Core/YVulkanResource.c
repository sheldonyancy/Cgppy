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
#include "YVulkanContext.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanShadowMappingSystem.h"
#include "YVulkanPathTracingSystem.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YGlobalFunction.h"

#include "stb_image_write.h"


static void bufferBind(YsVkContext* context,
                       YsVkBuffer* buffer,
                       u64 offset) {
    VK_CHECK(vkBindBufferMemory(context->device->logical_device,
                                buffer->handle,
                                buffer->memory,
                                offset));
}

static b8 bufferCreate(YsVkContext* context,
                       u64 size,
                       VkBufferUsageFlagBits usage,
                       u32 memory_property_flags,
                       b8 bind_on_create,
                       YsVkBuffer* out_buffer) {
    out_buffer->total_size = size;
    out_buffer->usage = usage;
    out_buffer->memory_property_flags = memory_property_flags;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(context->device->logical_device, &buffer_info, context->allocator, &out_buffer->handle));

    // Gather memory requirements.
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(context->device->logical_device, out_buffer->handle, &requirements);
    out_buffer->memory_index = context->find_memory_index(requirements.memoryTypeBits, out_buffer->memory_property_flags, context);
    if (out_buffer->memory_index == -1) {
        YERROR("Unable to create vulkan buffer because the required memory type index was not found.");
        return false;
    }

    // Allocate memory info
    VkMemoryAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex = (u32)out_buffer->memory_index;

    // Allocate the memory.
    VkResult result = vkAllocateMemory(context->device->logical_device,
                                       &allocate_info,
                                       context->allocator,
                                       &out_buffer->memory);

    if (result != VK_SUCCESS) {
        YERROR("Unable to create vulkan buffer because the required memory allocation failed. Error: %i", result);
        return false;
    }

    if (bind_on_create) {
        bufferBind(context, out_buffer, 0);
    }

    return true;
}

static void bufferDestroy(YsVkContext* context, YsVkBuffer* buffer) {
    if (buffer->memory) {
        vkFreeMemory(context->device->logical_device, buffer->memory, context->allocator);
        buffer->memory = 0;
    }
    if (buffer->handle) {
        vkDestroyBuffer(context->device->logical_device, buffer->handle, context->allocator);
        buffer->handle = 0;
    }
    buffer->total_size = 0;
    buffer->usage = 0;
    buffer->is_locked = false;
}

static void bufferLoadData(YsVkContext* context,
                           YsVkBuffer* buffer,
                           u64 offset,
                           u32 flags,
                           const void* data) {
    void* data_ptr;
    VK_CHECK(vkMapMemory(context->device->logical_device, 
                         buffer->memory, 
                         offset, 
                         buffer->total_size, 
                         flags, 
                         &data_ptr));
    yCMemoryCopy(data_ptr, data, buffer->total_size);
    vkUnmapMemory(context->device->logical_device, buffer->memory);
}

static void bufferCopyToBuffer(YsVkContext* context,
                               YsVkCommandUnit* command_unit,
                               VkFence fence,
                               VkBuffer source,
                               u64 source_offset,
                               VkBuffer dest,
                               u64 dest_offset,
                               u64 size) {
    vkQueueWaitIdle(command_unit->queue);
    VkCommandBuffer temp_command_buffer;
    context->device->commandBufferAllocateAndBeginSingleUse(context,
                                                            command_unit,
                                                            &temp_command_buffer);

    VkBufferCopy copy_region;
    copy_region.srcOffset = source_offset;
    copy_region.dstOffset = dest_offset;
    copy_region.size = size;

    vkCmdCopyBuffer(temp_command_buffer,
                    source,
                    dest,
                    1,
                    &copy_region);

    context->device->commandBufferEndSingleUse(context,
                                               command_unit,
                                               &temp_command_buffer);
}

static void bufferLoadDataRange(YsVkContext* context,
                                YsVkCommandUnit* command_unit,
                                VkFence fence,
                                YsVkBuffer* buffer,
                                u64 offset,
                                void* data) {
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    YsVkBuffer staging;
    bufferCreate(context,
                 buffer->total_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 flags,
                 true,
                 &staging);

    bufferLoadData(context,
                   &staging,
                   0,
                   0,
                   data);

    bufferCopyToBuffer(context,
                          command_unit,
                          fence,
                          staging.handle,
                          0,
                          buffer->handle,
                          offset,
                          buffer->total_size);

    bufferDestroy(context, &staging);
}

static void imageCreate(YsVkContext* context,
                        VkImageCreateInfo* create_info,
                        VkMemoryPropertyFlags memory_flags,
                        b8 create_individual_views,
                        b8 create_common_view,
                        VkImageAspectFlags view_aspect_flags,
                        YsVkImage* out_image) {
    //
    out_image->create_info = create_info;
    out_image->memory_flags = memory_flags;

    VK_CHECK(vkCreateImage(context->device->logical_device,
                           out_image->create_info,
                           context->allocator,
                           &out_image->handle));

    //
    vkGetImageMemoryRequirements(context->device->logical_device,
                                 out_image->handle,
                                 &out_image->memory_requirements);

    i32 memory_type = context->find_memory_index(out_image->memory_requirements.memoryTypeBits,
                                                 memory_flags,
                                                 context);
    if (memory_type == -1) {
        YERROR("Required memory type not found. Image not valid.");
        return;
    }

    VkMemoryAllocateInfo memory_allocate_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memory_allocate_info.allocationSize = out_image->memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_type;
    VK_CHECK(vkAllocateMemory(context->device->logical_device,
                              &memory_allocate_info,
                              context->allocator,
                              &out_image->memory));
    VK_CHECK(vkBindImageMemory(context->device->logical_device,
                               out_image->handle,
                               out_image->memory,
                               0));
    //
    if(create_individual_views) {
        VkImageViewType use_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        switch (out_image->create_info->imageType) {
            case VK_IMAGE_TYPE_1D: {
                use_view_type = VK_IMAGE_VIEW_TYPE_1D;
                break;
            }
            case VK_IMAGE_TYPE_2D: {
                use_view_type = VK_IMAGE_VIEW_TYPE_2D;
                break;
            }
            case VK_IMAGE_TYPE_3D: {
                use_view_type = VK_IMAGE_VIEW_TYPE_3D;
                break;
            }
            default: {
                use_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
                break;
            }
        }
        out_image->individual_views = yCMemoryAllocate(sizeof(VkImageView) * out_image->create_info->arrayLayers);
        for(int i = 0; i < out_image->create_info->arrayLayers; ++i) {
            VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view_create_info.image = out_image->handle;
            view_create_info.viewType = use_view_type;
            view_create_info.format = out_image->create_info->format;
            view_create_info.subresourceRange.aspectMask = view_aspect_flags;
            view_create_info.subresourceRange.baseMipLevel = 0;
            view_create_info.subresourceRange.levelCount = out_image->create_info->mipLevels;
            view_create_info.subresourceRange.baseArrayLayer = i;
            view_create_info.subresourceRange.layerCount = 1;
            VK_CHECK(vkCreateImageView(context->device->logical_device,
                                       &view_create_info,
                                       context->allocator,
                                       &out_image->individual_views[i]));
        }
    }

    //
    if(create_common_view) {
        VkImageViewType use_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        switch (out_image->create_info->imageType) {
            case VK_IMAGE_TYPE_1D: {
                use_view_type = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                break;
            }
            case VK_IMAGE_TYPE_2D: {
                use_view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                break;
            }
            case VK_IMAGE_TYPE_3D: {
                use_view_type = VK_IMAGE_VIEW_TYPE_3D;
                break;
            }
            default: {
                use_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
                break;
            }
        }

        VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_create_info.image = out_image->handle;
        view_create_info.viewType = use_view_type;
        view_create_info.format = out_image->create_info->format;
        view_create_info.subresourceRange.aspectMask = view_aspect_flags;
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.levelCount = out_image->create_info->mipLevels;
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.layerCount = out_image->create_info->arrayLayers;
        VK_CHECK(vkCreateImageView(context->device->logical_device,
                                   &view_create_info,
                                   context->allocator,
                                   &out_image->common_view));
    }
}

static void transitionImageLayout(VkCommandBuffer command_buffer,
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
                                  VkPipelineStageFlags dst_stage_mask) {
    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = src_queue_family_index;
    barrier.dstQueueFamilyIndex = dst_queue_family_index;
    barrier.image = image->handle;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = base_array_layer;
    barrier.subresourceRange.layerCount = layer_count;
    barrier.srcAccessMask = src_access_mask;
    barrier.dstAccessMask = dst_access_mask;

    vkCmdPipelineBarrier(command_buffer,
                         src_stage_mask,
                         dst_stage_mask,
                         0,
                         0,
                         NULL,
                         0,
                         NULL,
                         1,
                         &barrier);
}

static void imageClear(YsVkContext* context,
                       YsVkCommandUnit* command_unit,
                       YsVkImage* image,
                       u32 base_array_layer,
                       u32 layer_count) {
    VkClearColorValue clear_color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT};
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = base_array_layer;
    range.layerCount = layer_count;

    VkCommandBuffer temp_command_buffer;
    context->device->commandBufferAllocateAndBeginSingleUse(context,
                                                            command_unit,
                                                            &temp_command_buffer);

    vkCmdClearColorImage(temp_command_buffer,
                         image->handle,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         &clear_color,
                         1,
                         &range);

    context->device->commandBufferEndSingleUse(context,
                                               command_unit,
                                               &temp_command_buffer);
}

static void imageDestroy(struct YsVkContext* context, YsVkImage* image) {
    if (image->common_view) {
        vkDestroyImageView(context->device->logical_device, image->common_view, context->allocator);
        image->common_view = 0;
    }
    if (image->memory) {
        vkFreeMemory(context->device->logical_device, image->memory, context->allocator);
        image->memory = 0;
    }
    if (image->handle) {
        vkDestroyImage(context->device->logical_device, image->handle, context->allocator);
        image->handle = 0;
    }

    yCMemoryFreeReport(image->memory_requirements.size);
    yCMemoryZero(&image->memory_requirements);
}

static void bufferCopyToImage(YsVkContext* context,
                              YsVkCommandUnit* command_unit,
                              VkFence fence,
                              YsVkBuffer* buffer,
                              YsVkImage* image) {
    vkQueueWaitIdle(command_unit->queue);
    VkCommandBuffer temp_command_buffer;
    context->device->commandBufferAllocateAndBeginSingleUse(context,
                                                            command_unit,
                                                            &temp_command_buffer);

    transitionImageLayout(temp_command_buffer,
                          image,
                          0,
                          image->create_info->arrayLayers,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_ACCESS_NONE,
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          command_unit->queue_family_index,
                          command_unit->queue_family_index,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy region;
    yCMemoryZero(&region);
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = image->create_info->arrayLayers;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = image->create_info->extent.width;
    region.imageExtent.height = image->create_info->extent.height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(temp_command_buffer,
                           buffer->handle,
                           image->handle,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    transitionImageLayout(temp_command_buffer,
                          image,
                          0,
                          1,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_NONE,
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          command_unit->queue_family_index,
                          command_unit->queue_family_index,
                          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT);

    context->device->commandBufferEndSingleUse(context,
                                               command_unit,
                                               &temp_command_buffer);
}

static void saveImageToPng(YsVkContext* context,
                           YsVkResources* resource,
                           YsVkImage* image,
                           YsVkCommandUnit* command_unit,
                           const char* output_png_file) {
    u32 pixel_count = image->create_info->extent.width * image->create_info->extent.height;                       
    YsVkBuffer pixel_buffer;
    bufferCreate(context,
                 4 * pixel_count,
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 true,
                 &pixel_buffer);

    vkQueueWaitIdle(command_unit->queue);
    VkCommandBuffer temp_command_buffer;
    context->device->commandBufferAllocateAndBeginSingleUse(context,
                                                            command_unit,
                                                            &temp_command_buffer);

    resource->transitionImageLayout(temp_command_buffer,
                                    image,
                                    0,
                                    1,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    VK_ACCESS_MEMORY_READ_BIT,
                                    VK_ACCESS_TRANSFER_READ_BIT,
                                    command_unit->queue_family_index,
                                    command_unit->queue_family_index,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = image->create_info->arrayLayers;
    region.imageExtent.width = image->create_info->extent.width;
    region.imageExtent.height = image->create_info->extent.height;
    region.imageExtent.depth = 1;
    vkCmdCopyImageToBuffer(temp_command_buffer,
                           image->handle,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           pixel_buffer.handle,
                           1,
                           &region);

    resource->transitionImageLayout(temp_command_buffer,
                                    image,
                                    0,
                                    1,
                                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                    VK_ACCESS_TRANSFER_READ_BIT,
                                    VK_ACCESS_MEMORY_READ_BIT,
                                    command_unit->queue_family_index,
                                    command_unit->queue_family_index,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);                       

    context->device->commandBufferEndSingleUse(context,
                                               command_unit,
                                               &temp_command_buffer);

    void* data = NULL;
    vkMapMemory(context->device->logical_device, 
                pixel_buffer.memory, 
                0, 
                VK_WHOLE_SIZE, 
                0, 
                &data);
    stbi_write_jpg(output_png_file, 
                   image->create_info->extent.width, 
                   image->create_info->extent.height, 
                   4, 
                   data, 
                   image->create_info->extent.width * 4);
    vkUnmapMemory(context->device->logical_device, pixel_buffer.memory);
}

static void initializeVertexInputDescription(YsVkContext* context,
                                             YsVkResources* out_resources) {
    out_resources->vertex_position_binding_description.binding = 0;
    out_resources->vertex_position_binding_description.stride = sizeof(vec4);
    out_resources->vertex_position_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    out_resources->vertex_position_attribute_description.binding = 0;
    out_resources->vertex_position_attribute_description.location = 0;
    out_resources->vertex_position_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    out_resources->vertex_position_attribute_description.offset = 0;

    out_resources->vertex_normal_binding_description.binding = 1;
    out_resources->vertex_normal_binding_description.stride = sizeof(vec4);
    out_resources->vertex_normal_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    out_resources->vertex_normal_attribute_description.binding = 1;
    out_resources->vertex_normal_attribute_description.location = 1;
    out_resources->vertex_normal_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    out_resources->vertex_normal_attribute_description.offset = 0;

    out_resources->vertex_material_id_binding_description.binding = 2;
    out_resources->vertex_material_id_binding_description.stride = sizeof(i32);
    out_resources->vertex_material_id_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    out_resources->vertex_material_id_attribute_description.binding = 2;
    out_resources->vertex_material_id_attribute_description.location = 2;
    out_resources->vertex_material_id_attribute_description.format = VK_FORMAT_R32_SINT;
    out_resources->vertex_material_id_attribute_description.offset = 0;
}

static void initializeUboDescription(YsVkContext* context,
                                     YsVkResources* out_resources) {
    out_resources->ubo_descriptor = yCMemoryAllocate(sizeof(YsVkDescription));
    out_resources->ubo_descriptor->first_set = 0;

    VkDescriptorSetLayoutBinding ubo_layout_binding;
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.pImmutableSamplers = NULL;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo ubo_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    ubo_layout_info.bindingCount = 1;
    ubo_layout_info.pBindings = &ubo_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &ubo_layout_info,
                                         context->allocator,
                                         &out_resources->ubo_descriptor->descriptor_set_layout));

    VkDescriptorPoolSize ubo_pool_size;
    ubo_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_pool_size.descriptorCount = context->swapchain->image_count;
    VkDescriptorPoolCreateInfo ubo_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    ubo_pool_info.poolSizeCount = 1;
    ubo_pool_info.pPoolSizes = &ubo_pool_size;
    ubo_pool_info.maxSets = context->swapchain->image_count;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &ubo_pool_info,
                                    context->allocator,
                                    &out_resources->ubo_descriptor->descriptor_pool));

    VkDescriptorSetLayout* ubo_layouts = yCMemoryAllocate(sizeof(VkDescriptorSetLayout) * context->swapchain->image_count);
    for (int i = 0; i < context->swapchain->image_count; ++i) {
        ubo_layouts[i] = out_resources->ubo_descriptor->descriptor_set_layout;
    }
    VkDescriptorSetAllocateInfo ubo_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ubo_alloc_info.descriptorPool = out_resources->ubo_descriptor->descriptor_pool;
    ubo_alloc_info.descriptorSetCount = context->swapchain->image_count;
    ubo_alloc_info.pSetLayouts = ubo_layouts;
    out_resources->ubo_descriptor->descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * context->swapchain->image_count);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device, &ubo_alloc_info, out_resources->ubo_descriptor->descriptor_sets));
}

static void initializeSsboDescriptor(YsVkContext* context,
                                     YsVkResources* out_resources) {
    out_resources->ssbo_descriptor = yCMemoryAllocate(sizeof(YsVkDescription));
    out_resources->ssbo_descriptor->first_set = 1;

    VkDescriptorSetLayoutBinding ssbo_layout_binding = {};
    ssbo_layout_binding.binding = 0;
    ssbo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssbo_layout_binding.descriptorCount = 1;
    ssbo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo ssbo_layout_info = {};
    ssbo_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ssbo_layout_info.bindingCount = 1;
    ssbo_layout_info.pBindings = &ssbo_layout_binding;
    vkCreateDescriptorSetLayout(context->device->logical_device,
                                &ssbo_layout_info,
                                NULL,
                                &out_resources->ssbo_descriptor->descriptor_set_layout);

    VkDescriptorPoolSize ssbo_pool_size;
    ssbo_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssbo_pool_size.descriptorCount = context->swapchain->image_count;
    VkDescriptorPoolCreateInfo ssbo_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    ssbo_pool_info.poolSizeCount = 1;
    ssbo_pool_info.pPoolSizes = &ssbo_pool_size;
    ssbo_pool_info.maxSets = context->swapchain->image_count;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &ssbo_pool_info,
                                    context->allocator,
                                    &out_resources->ssbo_descriptor->descriptor_pool));

    out_resources->ssbo_descriptor->descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * context->swapchain->image_count);
    VkDescriptorSetLayout* ssbo_layouts = yCMemoryAllocate(sizeof(VkDescriptorSetLayout) * context->swapchain->image_count);
    for (int i = 0; i < context->swapchain->image_count; ++i) {
        ssbo_layouts[i] = out_resources->ssbo_descriptor->descriptor_set_layout;
    }
    VkDescriptorSetAllocateInfo ssbo_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    ssbo_alloc_info.descriptorPool = out_resources->ssbo_descriptor->descriptor_pool;
    ssbo_alloc_info.descriptorSetCount = context->swapchain->image_count;
    ssbo_alloc_info.pSetLayouts = ssbo_layouts;
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device, &ssbo_alloc_info, out_resources->ssbo_descriptor->descriptor_sets));
}

static void initializeImageDescriptor(YsVkContext* context,
                                      YsVkResources* out_resources) {
    out_resources->image_descriptor = yCMemoryAllocate(sizeof(YsVkDescription));
    out_resources->image_descriptor->first_set = 2;

    VkDescriptorSetLayoutBinding image_layout_binding[5];
    image_layout_binding[0].binding = 0;
    image_layout_binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding[0].descriptorCount = 1;
    image_layout_binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding[0].pImmutableSamplers = NULL;
    image_layout_binding[1].binding = 1;
    image_layout_binding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding[1].descriptorCount = 1;
    image_layout_binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding[1].pImmutableSamplers = NULL;
    image_layout_binding[2].binding = 2;
    image_layout_binding[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding[2].descriptorCount = 1;
    image_layout_binding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding[2].pImmutableSamplers = NULL;
    image_layout_binding[3].binding = 3;
    image_layout_binding[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding[3].descriptorCount = 1;
    image_layout_binding[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding[3].pImmutableSamplers = NULL;
    image_layout_binding[4].binding = 4;
    image_layout_binding[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_layout_binding[4].descriptorCount = 1;
    image_layout_binding[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    image_layout_binding[4].pImmutableSamplers = NULL;
    VkDescriptorSetLayoutCreateInfo image_layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    image_layout_info.bindingCount = 5;
    image_layout_info.pBindings = image_layout_binding;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device,
                                         &image_layout_info,
                                         context->allocator,
                                         &out_resources->image_descriptor->descriptor_set_layout));

    VkDescriptorPoolSize image_pool_size[5];
    image_pool_size[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size[0].descriptorCount = context->swapchain->image_count;
    image_pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size[1].descriptorCount = context->swapchain->image_count;
    image_pool_size[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size[2].descriptorCount = context->swapchain->image_count;
    image_pool_size[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size[3].descriptorCount = context->swapchain->image_count;
    image_pool_size[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_pool_size[4].descriptorCount = context->swapchain->image_count;
    VkDescriptorPoolCreateInfo image_pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    image_pool_info.poolSizeCount = 5;
    image_pool_info.pPoolSizes = image_pool_size;
    image_pool_info.maxSets = context->swapchain->image_count;
    VK_CHECK(vkCreateDescriptorPool(context->device->logical_device,
                                    &image_pool_info,
                                    context->allocator,
                                    &out_resources->image_descriptor->descriptor_pool));

    VkDescriptorSetLayout* image_layouts = yCMemoryAllocate(sizeof(VkDescriptorSetLayout) * context->swapchain->image_count);
    for (int i = 0; i < context->swapchain->image_count; ++i) {
        image_layouts[i] = out_resources->image_descriptor->descriptor_set_layout;
    }
    VkDescriptorSetAllocateInfo image_alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    image_alloc_info.descriptorPool = out_resources->image_descriptor->descriptor_pool;
    image_alloc_info.descriptorSetCount = context->swapchain->image_count;
    image_alloc_info.pSetLayouts = image_layouts;
    out_resources->image_descriptor->descriptor_sets = yCMemoryAllocate(sizeof(VkDescriptorSet) * context->swapchain->image_count);
    VK_CHECK(vkAllocateDescriptorSets(context->device->logical_device,
                                      &image_alloc_info,
                                      out_resources->image_descriptor->descriptor_sets));
}

static void createVertexInputBuffer(YsVkContext* context,
                                    YsVkResources* resource,
                                    u32 vertex_count) {
    resource->current_draw_vertex_count = vertex_count;

    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
                                                          | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                                          | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    resource->vertex_input_position_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (!bufferCreate(context,
                      sizeof(vec4) * vertex_count,
                      usage,
                      flags,
                      true,
                      resource->vertex_input_position_buffer)) {
        YERROR("Error Create VkBuffer");
    }

    resource->vertex_input_normal_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (!bufferCreate(context,
                      sizeof(vec4) * vertex_count,
                      usage,
                      flags,
                      true,
                      resource->vertex_input_normal_buffer)) {
        YERROR("Error Create VkBuffer");
    }

    resource->vertex_input_material_id_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (!bufferCreate(context,
                      sizeof(i32) * vertex_count,
                      usage,
                      flags,
                      true,
                      resource->vertex_input_material_id_buffer)) {
        YERROR("Error Create VkBuffer");
    }
}

static void updateVertexInputBuffer(YsVkContext* context,
                             YsVkResources* resource,
                             YsVkCommandUnit* command_unit,
                             void* vertex_position_data,
                             void* vertex_normal_data,
                             void* vertex_material_id_data) {
    bufferLoadDataRange(context,
                       command_unit,
                       0,
                       resource->vertex_input_position_buffer,
                       0,
                       vertex_position_data);
    bufferLoadDataRange(context,
                       command_unit,
                       0,
                       resource->vertex_input_normal_buffer,
                       0,
                       vertex_normal_data);
    bufferLoadDataRange(context,
                       command_unit,
                       0,
                       resource->vertex_input_material_id_buffer,
                       0,
                       vertex_material_id_data);
}

static void createUboBuffer(YsVkContext* context,
                            YsVkResources* resource) {
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    resource->ubo_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (!bufferCreate(context,
                      yUboSize(),
                      usage,
                      flags,
                      true,
                      resource->ubo_buffer)) {
        YERROR("Vulkan buffer creation failed for shader.");
    }
}

static void updateUboBuffer(struct YsVkContext* context,
                            struct YsVkResources* resource,
                            void* data) {
    bufferLoadData(context,
                   resource->ubo_buffer,
                   0,
                   0,
                   data);
}

static void createSsboBuffer(YsVkContext* context,
                             YsVkResources* resource,
                             u32 data_size) {
    resource->ssbo_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkBufferUsageFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (bufferCreate(context,
                     data_size,
                     usage,
                     flags,
                     true,
                     resource->ssbo_buffer)) {

    } else {
        YERROR("Error creating ssbo buffer.");
    }
}

static void updateSsboBuffer(YsVkContext* context,
                             YsVkResources* resource,
                             void* data) {
    bufferLoadData(context,
                   resource->ssbo_buffer,
                   0,
                   0,
                   data);
}

static void createRasterizationImage(YsVkContext* context,
                                     YsVkResources* resource) {
    VkImageCreateInfo* color_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    color_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    color_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    color_image_create_info->extent.width = context->framebuffer_width;
    color_image_create_info->extent.height = context->framebuffer_height;
    color_image_create_info->extent.depth = 1;
    color_image_create_info->mipLevels = 1;
    color_image_create_info->arrayLayers = context->swapchain->image_count;
    color_image_create_info->format = VK_FORMAT_R32G32B32A32_SFLOAT;
    color_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    color_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_image_create_info->usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                     VK_IMAGE_USAGE_SAMPLED_BIT |
                                     VK_IMAGE_USAGE_STORAGE_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    color_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    color_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreate(context,
                color_image_create_info,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true,
                false,
                VK_IMAGE_ASPECT_COLOR_BIT,
                &resource->rasterization_color_image);

    VkImageCreateInfo* depth_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    depth_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    depth_image_create_info->extent.width = context->framebuffer_width;
    depth_image_create_info->extent.height = context->framebuffer_height;
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
    imageCreate(context,
                depth_image_create_info,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true,
                false,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                &resource->rasterization_depth_image);
}

static void createShadowMapImage(YsVkContext* context,
                                 YsVkResources* resource) {
    VkImageCreateInfo* shadow_map_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    shadow_map_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    shadow_map_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    shadow_map_image_create_info->extent.width = context->framebuffer_width;
    shadow_map_image_create_info->extent.height = context->framebuffer_height;
    shadow_map_image_create_info->extent.depth = 1;
    shadow_map_image_create_info->mipLevels = 1;
    shadow_map_image_create_info->arrayLayers = 1;
    shadow_map_image_create_info->format = context->device->depth_format;
    shadow_map_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    shadow_map_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    shadow_map_image_create_info->usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                          VK_IMAGE_USAGE_SAMPLED_BIT |
                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    shadow_map_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    shadow_map_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreate(context,
                shadow_map_image_create_info,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true,
                false,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                &resource->shadow_map_image);
}

static void createPathTracingImage(YsVkContext* context,
                                   YsVkResources* resource) {
    //
    VkImageCreateInfo* path_tracing_random_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    path_tracing_random_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    path_tracing_random_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    path_tracing_random_image_create_info->extent.width = context->framebuffer_width;
    path_tracing_random_image_create_info->extent.height = context->framebuffer_height;
    path_tracing_random_image_create_info->extent.depth = 1;
    path_tracing_random_image_create_info->mipLevels = 1;
    path_tracing_random_image_create_info->arrayLayers = context->swapchain->image_count;
    path_tracing_random_image_create_info->format = VK_FORMAT_R32_UINT;
    path_tracing_random_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    path_tracing_random_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    path_tracing_random_image_create_info->usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                   VK_IMAGE_USAGE_SAMPLED_BIT |
                                                   VK_IMAGE_USAGE_STORAGE_BIT |
                                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    path_tracing_random_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    path_tracing_random_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreate(context,
                path_tracing_random_image_create_info,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true,
                false,
                VK_IMAGE_ASPECT_COLOR_BIT,
                &resource->path_tracing_random_image);

    //
    VkImageCreateInfo* path_tracing_accumulate_image_create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
    path_tracing_accumulate_image_create_info->sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    path_tracing_accumulate_image_create_info->imageType = VK_IMAGE_TYPE_2D;
    path_tracing_accumulate_image_create_info->extent.width = context->framebuffer_width;
    path_tracing_accumulate_image_create_info->extent.height = context->framebuffer_height;
    path_tracing_accumulate_image_create_info->extent.depth = 1;
    path_tracing_accumulate_image_create_info->mipLevels = 1;
    path_tracing_accumulate_image_create_info->arrayLayers = context->swapchain->image_count;
    path_tracing_accumulate_image_create_info->format = VK_FORMAT_R32G32B32A32_SFLOAT;
    path_tracing_accumulate_image_create_info->tiling = VK_IMAGE_TILING_OPTIMAL;
    path_tracing_accumulate_image_create_info->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    path_tracing_accumulate_image_create_info->usage = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                       VK_IMAGE_USAGE_SAMPLED_BIT |
                                                       VK_IMAGE_USAGE_STORAGE_BIT |
                                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    path_tracing_accumulate_image_create_info->samples = VK_SAMPLE_COUNT_1_BIT;
    path_tracing_accumulate_image_create_info->sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreate(context,
                path_tracing_accumulate_image_create_info,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                true,
                true,
                VK_IMAGE_ASPECT_COLOR_BIT,
                &resource->path_tracing_accumulate_image);
}

static void createSampler(YsVkContext* context,
                          YsVkResources* resource) {
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

static void updateSsboDescriptorSets(YsVkContext* context,
                                     YsVkResources* resource,
                                     YsVkRenderingSystem* rendering_system,
                                     u32 image_index) {
    VkDescriptorBufferInfo ssbo_buffer_info;
    ssbo_buffer_info.buffer = resource->ssbo_buffer->handle;
    ssbo_buffer_info.offset = 0;
    ssbo_buffer_info.range = resource->ssbo_buffer->total_size; 

    VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write_descriptor_set.dstSet = resource->ssbo_descriptor->descriptor_sets[image_index];
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

static void updateUboDescriptorSets(YsVkContext* context,
                                    YsVkResources* resource,
                                    YsVkRenderingSystem* rendering_system,
                                    u32 image_index) {
    VkDescriptorBufferInfo ubo_buffer_info;
    ubo_buffer_info.buffer = resource->ubo_buffer->handle;
    ubo_buffer_info.offset = 0;
    ubo_buffer_info.range = resource->ubo_buffer->total_size;   

    VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write_descriptor_set.dstSet = resource->ubo_descriptor->descriptor_sets[image_index];
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

static void updateImageDescriptorSets(YsVkContext* context,
                                 YsVkResources* resource,
                                 YsVkRenderingSystem* rendering_system,
                                 u32 image_index) {
    VkDescriptorImageInfo rasterization_texture_info;
    rasterization_texture_info.sampler = resource->sampler_linear;
    rasterization_texture_info.imageView = resource->rasterization_color_image.individual_views[image_index];
    rasterization_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo shadow_map_image_info;
    shadow_map_image_info.sampler = resource->sampler_linear;
    shadow_map_image_info.imageView = resource->shadow_map_image.individual_views[0];
    shadow_map_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo path_tracing_random_texture_info;
    path_tracing_random_texture_info.sampler = resource->sampler_nearest;
    path_tracing_random_texture_info.imageView = resource->path_tracing_random_image.individual_views[rendering_system->path_tracing->accumulate_image_index];
    path_tracing_random_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo target_accumulate_texture_info;
    target_accumulate_texture_info.sampler = resource->sampler_linear;
    target_accumulate_texture_info.imageView = resource->path_tracing_accumulate_image.individual_views[rendering_system->path_tracing->accumulate_image_index];
    target_accumulate_texture_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo all_accumulate_textures_info;
    all_accumulate_textures_info.sampler = resource->sampler_linear;
    all_accumulate_textures_info.imageView = resource->path_tracing_accumulate_image.common_view;
    all_accumulate_textures_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write_descriptor_sets[5] = {{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET},
                                                     {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET},
                                                     {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET},
                                                     {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET},
                                                     {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}};
    
    write_descriptor_sets[0].dstSet = resource->image_descriptor->descriptor_sets[image_index];
    write_descriptor_sets[0].dstBinding = 0;
    write_descriptor_sets[0].dstArrayElement = 0;
    write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_sets[0].descriptorCount = 1;
    write_descriptor_sets[0].pImageInfo = &rasterization_texture_info;
    write_descriptor_sets[1].dstSet = resource->image_descriptor->descriptor_sets[image_index];
    write_descriptor_sets[1].dstBinding = 1;
    write_descriptor_sets[1].dstArrayElement = 0;
    write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_sets[1].descriptorCount = 1;
    write_descriptor_sets[1].pImageInfo = &shadow_map_image_info;
    write_descriptor_sets[2].dstSet = resource->image_descriptor->descriptor_sets[image_index];
    write_descriptor_sets[2].dstBinding = 2;
    write_descriptor_sets[2].dstArrayElement = 0;
    write_descriptor_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_sets[2].descriptorCount = 1;
    write_descriptor_sets[2].pImageInfo = &path_tracing_random_texture_info;
    write_descriptor_sets[3].dstSet = resource->image_descriptor->descriptor_sets[image_index];
    write_descriptor_sets[3].dstBinding = 3;
    write_descriptor_sets[3].dstArrayElement = 0;
    write_descriptor_sets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_sets[3].descriptorCount = 1;
    write_descriptor_sets[3].pImageInfo = &target_accumulate_texture_info;
    write_descriptor_sets[4].dstSet = resource->image_descriptor->descriptor_sets[image_index];
    write_descriptor_sets[4].dstBinding = 4;
    write_descriptor_sets[4].dstArrayElement = 0;
    write_descriptor_sets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write_descriptor_sets[4].descriptorCount = 1;
    write_descriptor_sets[4].pImageInfo = &all_accumulate_textures_info;

    vkUpdateDescriptorSets(context->device->logical_device,
                           5,
                           write_descriptor_sets,
                           0,
                           0);
}

static void createPushConstants(YsVkResources* resource) {
    resource->push_constant_range_count = 1;
    resource->push_constant_range = yCMemoryAllocate(sizeof(VkPushConstantRange));
    resource->push_constant_range[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    resource->push_constant_range[0].offset = 0;
    resource->push_constant_range[0].size = yPushConstantSize();
}

static void updateRandomImage(YsVkContext* context,
                              YsVkResources* resource,
                              YsVkCommandUnit* command_unit,
                              vec2 resolution) {
    u32 pixel_count = resolution[0] * resolution[1] * context->swapchain->image_count;

    u32* rand_data = yCMemoryAllocate(sizeof(u32) * pixel_count);
    yGenerateUintRand(pixel_count, rand_data);

    YsVkBuffer random_buffer;
    if (bufferCreate(context,
                     sizeof(u32) * pixel_count,
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                     true,
                     &random_buffer)){
        bufferLoadDataRange(context,
                            command_unit,
                            0,
                            &random_buffer,
                            0,
                            rand_data);
    } else {
        YERROR("Error creating buffer.");
    }

    bufferCopyToImage(context,
                      command_unit,
                      0,
                      &random_buffer,
                      &resource->path_tracing_random_image);

    yCMemoryFree(rand_data);
}

static void initialize(YsVkContext* context,
                       YsVkResources* resource) {
    initializeVertexInputDescription(context, resource);

    initializeUboDescription(context, resource);
    initializeSsboDescriptor(context, resource);
    initializeImageDescriptor(context, resource);

    createUboBuffer(context, resource);

    createRasterizationImage(context, resource);
    createShadowMapImage(context, resource);
    createPathTracingImage(context, resource);

    createSampler(context, resource);

    createPushConstants(resource);
}

YsVkResources* yVkAllocateResourcesObject() {
    YsVkResources* vk_resources = yCMemoryAllocate(sizeof(YsVkResources));
    if(vk_resources) {
        vk_resources->initialize = initialize;
        vk_resources->bufferCreate = bufferCreate;
        vk_resources->bufferDestroy = bufferDestroy;
        vk_resources->bufferBind = bufferBind;
        vk_resources->bufferLoadDataRange = bufferLoadDataRange;
        vk_resources->bufferCopyToBuffer = bufferCopyToBuffer;
        vk_resources->bufferCopyToImage = bufferCopyToImage;
        vk_resources->imageCreate = imageCreate;
        vk_resources->transitionImageLayout = transitionImageLayout;
        vk_resources->imageClear = imageClear;
        vk_resources->imageDestroy = imageDestroy;
        vk_resources->saveImageToPng = saveImageToPng;
        vk_resources->createVertexInputBuffer = createVertexInputBuffer;
        vk_resources->updateVertexInputBuffer = updateVertexInputBuffer;
        vk_resources->updateUboBuffer = updateUboBuffer;
        vk_resources->createSsboBuffer = createSsboBuffer;
        vk_resources->updateSsboBuffer = updateSsboBuffer;
        vk_resources->updateSsboDescriptorSets = updateSsboDescriptorSets;
        vk_resources->updateUboDescriptorSets = updateUboDescriptorSets;
        vk_resources->updateImageDescriptorSets = updateImageDescriptorSets;
        vk_resources->updateRandomImage = updateRandomImage;
    }
    return vk_resources;
}

