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

#ifndef CGPPY_YVULKANCONTEXT_H
#define CGPPY_YVULKANCONTEXT_H


#include "YVulkanTypes.h"
#include "YVulkanDevice.h"
#include "YVulkanSwapchain.h"
#include "YVulkanRenderStage.h"
#include "YVulkanResource.h"
#include "YVulkanPipeline.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GLFWwindow;

typedef struct YsVkContext {
    b8 (*initialize)(u32 swapchain_width,
                     u32 swapchain_height,
                     struct GLFWwindow* glfw_window,
                     const i8* const*,
                     i32,
                     const i8* const*,
                     i32,
                     struct YsVkContext*);

    i32 (*find_memory_index)(u32 type_filter,
                             u32 property_flags,
                             struct YsVkContext* context);

    const i8* application_name;

    const i8* engine_name;

    uint32_t api_version;

    u32 api_major;

    u32 api_minor;

    u32 api_patch;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    VkInstance instance;

    VkAllocationCallbacks* allocator;

    VkSurfaceKHR surface;

    YsVkDevice* device;

    YsVkSwapchain* swapchain;
} YsVkContext;

YsVkContext* yVkContextCreate();



#ifdef __cplusplus
}
#endif

#endif //CGPPY_YVULKANCONTEXT_H
