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


#include "YVulkanBuffer.h"
#include "YVulkanContext.h"
#include "YVulkanDevice.h"
#include "YVulkanResource.h"
#include "YVulkanImage.h"
#include "YCMemoryManager.h"
#include "YLogger.h"

// 
static b8 bufferCreate(YsVkContext* context,
                       u64 size,
                       VkBufferUsageFlagBits usage,
                       VkMemoryPropertyFlagBits memory_property_flags,
                       YsVkBuffer* out_buffer) {
    out_buffer->total_size = size;
    out_buffer->usage = usage;
    out_buffer->memory_property_flags = memory_property_flags;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(context->device->logical_device, 
                            &buffer_info, 
                            context->allocator, 
                            &out_buffer->handle));

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

    VK_CHECK(vkBindBufferMemory(context->device->logical_device,
                                out_buffer->handle,
                                out_buffer->memory,
                                0));

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

static void bufferCopyToImage(YsVkContext* context,
                              YsVkCommandUnit* command_unit,
                              VkFence fence,
                              YsVkImage* image,
                              YsVkBuffer* buffer) {
    vkQueueWaitIdle(command_unit->queue);
    VkCommandBuffer temp_command_buffer;
    context->device->commandBufferAllocateAndBeginSingleUse(context,
                                                            command_unit,
                                                            &temp_command_buffer);

    image->transitionLayout(temp_command_buffer,
                            0,
                            image->create_info->arrayLayers,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_ACCESS_NONE,
                            VK_ACCESS_TRANSFER_WRITE_BIT,
                            command_unit->queue_family_index,
                            command_unit->queue_family_index,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            image);

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

    image->transitionLayout(temp_command_buffer,
                            0,
                            1,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            VK_ACCESS_NONE,
                            VK_ACCESS_TRANSFER_WRITE_BIT,
                            command_unit->queue_family_index,
                            command_unit->queue_family_index,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                            image);

    context->device->commandBufferEndSingleUse(context,
                                               command_unit,
                                               &temp_command_buffer);
}

static void bufferDirectUpdate(YsVkContext* context,
                               u64 offset,
                               u32 flags,
                               const void* data,
                               YsVkBuffer* buffer) {
    void* data_ptr = NULL;
    VK_CHECK(vkMapMemory(context->device->logical_device, 
                         buffer->memory, 
                         offset, 
                         buffer->total_size, 
                         flags, 
                         &data_ptr));
    yCMemoryCopy(data_ptr, data, buffer->total_size);
    vkUnmapMemory(context->device->logical_device, buffer->memory);
}

static void bufferIndirectUpdate(YsVkContext* context,
                                 YsVkCommandUnit* command_unit,
                                 VkFence fence,
                                 u64 offset,
                                 void* data,
                                 YsVkBuffer* buffer) {
    VkMemoryPropertyFlagBits staging_memory_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    YsVkBuffer staging_buffer;
    bufferCreate(context,
                 buffer->total_size,
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 staging_memory_property_flags,
                 &staging_buffer);

    bufferDirectUpdate(context,
                       0,
                       0,
                       data,
                       &staging_buffer);

    bufferCopyToBuffer(context,
                       command_unit,
                       fence,
                       staging_buffer.handle,
                       0,
                       buffer->handle,
                       offset,
                       buffer->total_size);

    bufferDestroy(context, &staging_buffer);
}

YsVkBuffer* yVkAllocateBufferObject() {
    YsVkBuffer* buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if(buffer) {
        buffer->create = bufferCreate;
        buffer->destroy = bufferDestroy;
        buffer->copyToBuffer = bufferCopyToBuffer;
        buffer->copyToImage = bufferCopyToImage;
        buffer->directUpdate = bufferDirectUpdate;
        buffer->indirectUpdate = bufferIndirectUpdate;
    }
    return buffer;
}