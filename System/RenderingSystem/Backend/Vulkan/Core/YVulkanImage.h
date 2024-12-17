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

#ifndef CGPPY_YVULKANIMAGE_H
#define CGPPY_YVULKANIMAGE_H


#include "YVulkanTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsVkImage {
    void (*create)(struct YsVkContext* context,
                   VkImageCreateInfo* create_info,
                   VkMemoryPropertyFlags memory_flags,
                   VkImageAspectFlags view_aspect_flags,
                   struct YsVkImage* out_image);

    void (*clear)(struct YsVkContext* context,
                  struct YsVkCommandUnit* command_unit,
                  u32 base_array_layer,
                  u32 layer_count,
                  struct YsVkImage* image);

    void (*destroy)(struct YsVkContext* context, struct YsVkImage* image);

    void (*transitionLayout)(VkCommandBuffer command_buffer,
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
                             struct YsVkImage* image);

    void (*updatImageData)(struct YsVkContext* context,
                           struct YsVkCommandUnit* command_unit,
                           u32 data_size,
                           void* data,
                           struct YsVkImage* image);                         

    VkImageCreateInfo* create_info;

    VkImage handle;

    VkDeviceMemory memory;

    VkImageView* layer_views;
    VkImageView image_view;

    VkMemoryRequirements memory_requirements;
    VkMemoryPropertyFlags memory_flags;
} YsVkImage;

YsVkImage* yVkAllocateImageObject();


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANIMAGE_H