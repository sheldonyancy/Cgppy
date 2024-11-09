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

#ifndef CGPPY_YVULKANRASTERIZATIONSYSTEM_H
#define CGPPY_YVULKANRASTERIZATIONSYSTEM_H


#include "YVulkanTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsVkRasterizationSystem {
    b8 (*initialize)(struct YsVkContext* context,
                     struct YsVkResources* resources,
                     struct YsVkRasterizationSystem* rasterization_system);

    void (*rendering)(struct YsVkContext* context,
                      struct YsVkCommandUnit* command_unit,
                      struct YsVkResources* resources,
                      u32 image_index,
                      u32 current_frame,
                      void* push_constant_data,
                      struct YsVkRasterizationSystem* rasterization_system);

    struct YsVkRenderStage* render_stage;
    struct YsVkPipeline* pipeline;
    VkSemaphore* complete_semaphores;
} YsVkRasterizationSystem;

YsVkRasterizationSystem* yVkRasterizationSystemCreate();


#ifdef __cplusplus
}
#endif


#endif


