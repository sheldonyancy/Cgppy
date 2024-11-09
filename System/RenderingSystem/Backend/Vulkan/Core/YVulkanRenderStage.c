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


#include "YVulkanRenderStage.h"
#include "YVulkanContext.h"
#include "YLogger.h"
#include "YCMemoryManager.h"


static b8 create(YsVkContext* context,
                 YsVkRenderStageCreateInfo* create_info,
                 YsVkRenderStage* render_stage) {
    if (!create_info) {
        YERROR("VulkanRenderStage create is required.");
        return false;
    }

    if (0 == create_info->framebuffer_count) {
        YERROR("Cannot have a framebuffer_count of 0");
        return false;
    }

    render_stage->create_info = create_info;

    VkSubpassDescription all_subpass_descriptions[create_info->subpass_count];
    for(int i = 0; i < create_info->subpass_count; ++i) {
        all_subpass_descriptions[i] = create_info->subpass_configs[i].subpass_description;
    }

    VkRenderPassCreateInfo render_pass_create_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_create_info.attachmentCount = create_info->attachment_count;
    render_pass_create_info.pAttachments = create_info->attachment_descriptions;
    render_pass_create_info.subpassCount = create_info->subpass_count;
    render_pass_create_info.pSubpasses = all_subpass_descriptions;
    render_pass_create_info.dependencyCount = create_info->subpass_dependency_count;
    render_pass_create_info.pDependencies = create_info->subpass_dependencies;
    VK_CHECK(vkCreateRenderPass(context->device->logical_device,
                                &render_pass_create_info,
                                context->allocator,
                                &render_stage->render_pass_handle));

    //
    render_stage->framebuffers = yCMemoryAllocate(sizeof(VkFramebuffer) * create_info->framebuffer_count);
    for(u32 i = 0; i < create_info->framebuffer_count; ++i) {
        create_info->framebuffer_create_info[i].renderPass = render_stage->render_pass_handle;
        VK_CHECK(vkCreateFramebuffer(context->device->logical_device,
                                     &create_info->framebuffer_create_info[i],
                                     context->allocator,
                                     &render_stage->framebuffers[i]));
    }

    return true;
}

static void destroy(YsVkContext* context, YsVkRenderStage* render_stage) {
    if (render_stage && render_stage->render_pass_handle) {
        vkDestroyRenderPass(context->device->logical_device, render_stage->render_pass_handle, context->allocator);
        render_stage->render_pass_handle = 0;
    }
}

static void renderPassBegin(VkCommandBuffer command_buffer,
                            VkFramebuffer* framebuffer,
                            VkRect2D render_area,
                            u32 clear_value_count,
                            VkClearValue* clear_values,
                            YsVkRenderStage* render_stage) {
    VkRenderPassBeginInfo begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    begin_info.renderPass = render_stage->render_pass_handle;
    begin_info.framebuffer = *framebuffer;
    begin_info.renderArea = render_area;
    begin_info.clearValueCount = clear_value_count;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

static void renderPassEnd(VkCommandBuffer command_buffer) {
    vkCmdEndRenderPass(command_buffer);
}

YsVkRenderStage* yVkAllocateRenderStageObject() {
    YsVkRenderStage* render_stage = yCMemoryAllocate(sizeof(YsVkRenderStage));
    if(render_stage) {
        render_stage->create = create;
        render_stage->destroy = destroy;
        render_stage->renderPassBegin = renderPassBegin;
        render_stage->renderPassEnd = renderPassEnd;
    }

    return render_stage;
}

