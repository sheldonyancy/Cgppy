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

#ifndef CGPPY_YVULKANDEVICE_H
#define CGPPY_YVULKANDEVICE_H

#include "YVulkanTypes.h"
#include "YVulkanSwapchain.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct YsVkCommandUnit {
    u32 queue_family_index;
    u32 queue_index;
    VkQueue queue;
    u32 command_pool_count;
    VkCommandPool* command_pools;
    u32 command_buffer_count;
    VkCommandBuffer* command_buffers;
    VkQueryPool query_pool_timestamps;
}YsVkCommandUnit;

typedef struct YsVkDevice {
    b8 (*create)(struct YsVkContext* context);

    void (*destroy)(struct YsVkContext* context);

    b8 (*detectDepthFormat)(struct YsVkDevice* device);

    YsVkCommandUnit* (*commandUnitsFront)(struct YsVkDevice* device);
    YsVkCommandUnit* (*commandUnitsBack)(struct YsVkDevice* device);
    YsVkCommandUnit* (*commandUnitsAt)(struct YsVkDevice* device, u32 index);


    void (*commandBufferFree)(struct YsVkContext* context,
                              YsVkCommandUnit* command_unit,
                              VkCommandBuffer* command_buffer);

    void (*commandBufferBegin)(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags flags);

    void (*commandBufferEnd)(VkCommandBuffer command_buffer);

    void (*commandBufferReset)(VkCommandBuffer command_buffer);

    void (*commandBufferAllocateAndBeginSingleUse)(struct YsVkContext* context,
                                                   YsVkCommandUnit* command_unit,
                                                   VkCommandBuffer* tmp_command_buffer);

    void (*commandBufferEndSingleUse)(struct YsVkContext* context,
                                      YsVkCommandUnit* command_unit,
                                      VkCommandBuffer* tmp_command_buffer);

    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceVulkan12Features features_12;
    VkPhysicalDeviceMemoryProperties memory;

    VkDevice logical_device;

    YsVkSwapchainSupportInfo swapchain_support;

    VkFormat depth_format;

    u32 graphics_compute_command_units_count;
    YsVkCommandUnit* graphics_compute_command_units;

} YsVkDevice;

YsVkDevice* yVkAllocateDeviceObject();


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANDEVICE_H
