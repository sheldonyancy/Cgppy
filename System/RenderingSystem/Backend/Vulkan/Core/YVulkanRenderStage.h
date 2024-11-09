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

#ifndef CGPPY_YVULKANRENDERSTAGE_H
#define CGPPY_YVULKANRENDERSTAGE_H



#include "YVulkanTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsSubpassConfig {
    u8 reference_count;
    VkAttachmentReference* attachment_references;

    VkSubpassDescription subpass_description;
} YsSubpassConfig;

typedef struct YsVkRenderStageCreateInfo {
    u8 attachment_count;
    VkAttachmentDescription* attachment_descriptions;

    u8 subpass_count;
    YsSubpassConfig* subpass_configs;

    u8 subpass_dependency_count;
    VkSubpassDependency* subpass_dependencies;

    u8 framebuffer_count;
    VkFramebufferCreateInfo* framebuffer_create_info;
} YsVkRenderStageCreateInfo;

typedef struct YsVkRenderStage {
    b8 (*create)(struct YsVkContext* context,
                 YsVkRenderStageCreateInfo* create_info,
                 struct YsVkRenderStage* render_stage);

    void (*destroy)(struct YsVkContext* context, struct YsVkRenderStage* render_stage);

    void (*renderPassBegin)(VkCommandBuffer command_buffer,
                            VkFramebuffer* framebuffer,
                            VkRect2D render_area,
                            u32 clear_value_count,
                            VkClearValue* clear_values,
                            struct YsVkRenderStage* render_stage);

    void (*renderPassEnd)(VkCommandBuffer command_buffer);

    YsVkRenderStageCreateInfo* create_info;

    VkRenderPass render_pass_handle;

    VkFramebuffer* framebuffers;
} YsVkRenderStage;

YsVkRenderStage* yVkAllocateRenderStageObject();


#ifdef __cplusplus
}
#endif



#endif //CGPPY_YVULKANRENDERSTAGE_H
