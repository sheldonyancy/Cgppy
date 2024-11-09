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

#ifndef CGPPY_YVULKANSWAPCHAIN_H
#define CGPPY_YVULKANSWAPCHAIN_H


#include "YVulkanTypes.h"
#include "YVulkanRenderStage.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsVkSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} YsVkSwapchainSupportInfo;

typedef struct YsVkSwapchain {
    void (*create)(struct YsVkContext* context,
                   u32 width,
                   u32 height,
                   VkPresentModeKHR present_mode,
                   struct YsVkSwapchain* swapchain);

    void (*destroy)(struct YsVkContext* context,
                   struct YsVkSwapchain* swapchain);

    VkSurfaceFormatKHR image_format;

    u8 max_frames_in_flight;

    VkSwapchainKHR handle;

    u32 image_count;

    struct YsVkImage* present_src_images;
} YsVkSwapchain;

YsVkSwapchain* yVkAllocateSwapchainObject();




#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANSWAPCHAIN_H