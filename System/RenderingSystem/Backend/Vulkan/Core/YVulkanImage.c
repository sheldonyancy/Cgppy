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


#include "YVulkanImage.h"
#include "YVulkanContext.h"
#include "YVulkanDevice.h"
#include "YVulkanResource.h"
#include "YVulkanBuffer.h"
#include "YCMemoryManager.h"
#include "YLogger.h"

//
static void imageCreate(YsVkContext* context,
                        VkImageCreateInfo* create_info,
                        VkMemoryPropertyFlags memory_flags,
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
    VkImageViewType layer_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    VkImageViewType image_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    switch (out_image->create_info->imageType) {
        case VK_IMAGE_TYPE_1D: {
            layer_view_type = VK_IMAGE_VIEW_TYPE_1D;
            image_view_type = out_image->create_info->arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
            break;
        }
        case VK_IMAGE_TYPE_2D: {
            layer_view_type = VK_IMAGE_VIEW_TYPE_2D;
            image_view_type = out_image->create_info->arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
            break;
        }
        case VK_IMAGE_TYPE_3D: {
            layer_view_type = VK_IMAGE_VIEW_TYPE_3D;
            image_view_type = VK_IMAGE_VIEW_TYPE_3D;
            break;
        }
        default: {
            layer_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            image_view_type = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
            break;
        }
    }

    if(out_image->create_info->arrayLayers > 1) {
        out_image->layer_views = yCMemoryAllocate(sizeof(VkImageView) * out_image->create_info->arrayLayers);
        for(int i = 0; i < out_image->create_info->arrayLayers; ++i) {
            VkImageViewCreateInfo layer_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            layer_create_info.image = out_image->handle;
            layer_create_info.viewType = layer_view_type;
            layer_create_info.format = out_image->create_info->format;
            layer_create_info.subresourceRange.aspectMask = view_aspect_flags;
            layer_create_info.subresourceRange.baseMipLevel = 0;
            layer_create_info.subresourceRange.levelCount = out_image->create_info->mipLevels;
            layer_create_info.subresourceRange.baseArrayLayer = i;
            layer_create_info.subresourceRange.layerCount = 1;
            VK_CHECK(vkCreateImageView(context->device->logical_device,
                                       &layer_create_info,
                                       context->allocator,
                                       &out_image->layer_views[i]));
        }        
    }

    VkImageViewCreateInfo view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_create_info.image = out_image->handle;
    view_create_info.viewType = image_view_type;
    view_create_info.format = out_image->create_info->format;
    view_create_info.subresourceRange.aspectMask = view_aspect_flags;
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = out_image->create_info->mipLevels;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = out_image->create_info->arrayLayers;
    VK_CHECK(vkCreateImageView(context->device->logical_device,
                               &view_create_info,
                               context->allocator,
                               &out_image->image_view));                           
}

static void imageClear(YsVkContext* context,
                       YsVkCommandUnit* command_unit,
                       u32 base_array_layer,
                       u32 layer_count,
                       YsVkImage* image) {
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

static void imageDestroy(YsVkContext* context, YsVkImage* image) {
    if (image->image_view) {
        vkDestroyImageView(context->device->logical_device, 
                           image->image_view, 
                           context->allocator);
        image->image_view = 0;
    }

    for(int i = 0; i < image->create_info->arrayLayers; ++i) {
        if(image->layer_views[i]) {
             vkDestroyImageView(context->device->logical_device, 
                                image->layer_views[i],
                                context->allocator);
        }
        image->layer_views = NULL;
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

static void transitionImageLayout(VkCommandBuffer command_buffer,
                                  u32 base_array_layer,
                                  u32 layer_count,
                                  VkImageLayout old_layout,
                                  VkImageLayout new_layout,
                                  VkAccessFlags src_access_mask,
                                  VkAccessFlags dst_access_mask,
                                  uint32_t src_queue_family_index,
                                  uint32_t dst_queue_family_index,
                                  VkPipelineStageFlags src_stage_mask,
                                  VkPipelineStageFlags dst_stage_mask,
                                  YsVkImage* image) {
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

static void updatData(YsVkContext* context,
                      YsVkCommandUnit* command_unit,
                      u32 data_size,
                      void* data,
                      YsVkImage* image) {
    YsVkBuffer* buffer = yVkAllocateBufferObject();
    if (buffer->create(context,
                       data_size,
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       buffer)){
        buffer->indirectUpdate(context,
                               command_unit,
                               VK_NULL_HANDLE,
                               0,
                               data,
                               buffer);
    } else {
        YERROR("Error creating buffer.");
    }

    buffer->copyToImage(context,
                        command_unit,
                        VK_NULL_HANDLE,
                        image,
                        buffer);

    yCMemoryFree(buffer);                  
}

YsVkImage* yVkAllocateImageObject() {
    YsVkImage* image = yCMemoryAllocate(sizeof(YsVkImage));
    if(image) {
        image->create = imageCreate;
        image->clear = imageClear;
        image->destroy = imageDestroy;
        image->transitionLayout = transitionImageLayout;
        image->updatImageData = updatData;
    }    
    return image;
}