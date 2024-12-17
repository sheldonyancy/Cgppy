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

#include "YVulkanRasterizationSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanShadowMappingSystem.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanContext.h"
#include "YVulkanDevice.h"
#include "YVulkanResource.h"
#include "YVulkanImage.h"
#include "YVulkanBuffer.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YAssets.h"
#include "YGlobalFunction.h"


static b8 initialize(YsVkContext* context,
                     YsVkResources* resources,
                     YsVkRasterizationSystem* rasterization_system) {
    // Render Stage
    YsVkRenderStageCreateInfo* render_stage_create_info = yCMemoryAllocate(sizeof(YsVkRenderStageCreateInfo));
    render_stage_create_info->attachment_count = 2;
    render_stage_create_info->attachment_descriptions = yCMemoryAllocate(sizeof(VkAttachmentDescription) * render_stage_create_info->attachment_count);
    render_stage_create_info->attachment_descriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    render_stage_create_info->attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    render_stage_create_info->attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_stage_create_info->attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_stage_create_info->attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    render_stage_create_info->attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    render_stage_create_info->attachment_descriptions[1].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    render_stage_create_info->attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    render_stage_create_info->attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_stage_create_info->attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_stage_create_info->attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    render_stage_create_info->attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    render_stage_create_info->subpass_count = 1;
    render_stage_create_info->subpass_configs = yCMemoryAllocate(sizeof(YsSubpassConfig) * render_stage_create_info->subpass_count);
    render_stage_create_info->subpass_configs[0].reference_count = 2;
    render_stage_create_info->subpass_configs[0].attachment_references = yCMemoryAllocate(sizeof(VkAttachmentReference) * render_stage_create_info->subpass_configs[0].reference_count);
    render_stage_create_info->subpass_configs[0].attachment_references[0].attachment = 0;
    render_stage_create_info->subpass_configs[0].attachment_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    render_stage_create_info->subpass_configs[0].attachment_references[1].attachment = 1;
    render_stage_create_info->subpass_configs[0].attachment_references[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    render_stage_create_info->subpass_configs[0].subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    render_stage_create_info->subpass_configs[0].subpass_description.colorAttachmentCount = 1;
    render_stage_create_info->subpass_configs[0].subpass_description.pColorAttachments = &render_stage_create_info->subpass_configs[0].attachment_references[0];
    render_stage_create_info->subpass_configs[0].subpass_description.pDepthStencilAttachment = &render_stage_create_info->subpass_configs[0].attachment_references[1];
    render_stage_create_info->framebuffer_count = context->swapchain->image_count;
    render_stage_create_info->framebuffer_create_info = yCMemoryAllocate(sizeof(VkFramebufferCreateInfo) * render_stage_create_info->framebuffer_count);
    for(int i = 0; i < render_stage_create_info->framebuffer_count; ++i) {
        VkImageView* image_views = yCMemoryAllocate(sizeof(VkImageView) * render_stage_create_info->attachment_count);
        image_views[0] = resources->rasterization_color_image->layer_views[i];
        image_views[1] = resources->rasterization_depth_image->layer_views[i];
        render_stage_create_info->framebuffer_create_info[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        render_stage_create_info->framebuffer_create_info[i].attachmentCount = render_stage_create_info->attachment_count;
        render_stage_create_info->framebuffer_create_info[i].pAttachments = image_views;
        render_stage_create_info->framebuffer_create_info[i].width = resources->rasterization_color_image->create_info->extent.width;
        render_stage_create_info->framebuffer_create_info[i].height = resources->rasterization_color_image->create_info->extent.height;
        render_stage_create_info->framebuffer_create_info[i].layers = 1;
    }
    rasterization_system->render_stage = yVkAllocateRenderStageObject();
    rasterization_system->render_stage->create(context,
                                               render_stage_create_info,
                                               rasterization_system->render_stage);

    //
    const u32 vertex_input_description_count = 3;
    VkVertexInputBindingDescription binding_descriptions[vertex_input_description_count] = {resources->vertex_position_binding_description,
                                                                                            resources->vertex_normal_binding_description,
                                                                                            resources->vertex_material_id_binding_description};
    VkVertexInputAttributeDescription attribute_descriptions[vertex_input_description_count] = {resources->vertex_position_attribute_description,
                                                                                                resources->vertex_normal_attribute_description,
                                                                                                resources->vertex_material_id_attribute_description};
    VkPipelineVertexInputStateCreateInfo pipeline_vertex_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    pipeline_vertex_info.vertexBindingDescriptionCount = vertex_input_description_count;
    pipeline_vertex_info.pVertexBindingDescriptions = binding_descriptions;
    pipeline_vertex_info.vertexAttributeDescriptionCount = vertex_input_description_count;
    pipeline_vertex_info.pVertexAttributeDescriptions = attribute_descriptions;

    //
    YsVkPipelineConfig* rasterization_pipeline_config = yCMemoryAllocate(sizeof(YsVkPipelineConfig));
    rasterization_pipeline_config->pipeline_type = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    rasterization_pipeline_config->shader_config.shader_stage_config_count = 2;
    rasterization_pipeline_config->shader_config.shader_stage_config[0].stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
    rasterization_pipeline_config->shader_config.shader_stage_config[0].source_length = getSpvCodeSize(Rasterization_Vert);
    rasterization_pipeline_config->shader_config.shader_stage_config[0].source = getSpvCode(Rasterization_Vert);
    rasterization_pipeline_config->shader_config.shader_stage_config[1].stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
    rasterization_pipeline_config->shader_config.shader_stage_config[1].source_length = getSpvCodeSize(Rasterization_Frag);
    rasterization_pipeline_config->shader_config.shader_stage_config[1].source = getSpvCode(Rasterization_Frag);
    rasterization_pipeline_config->render_stage = rasterization_system->render_stage;
    rasterization_pipeline_config->vertex_input_info = &pipeline_vertex_info;
    rasterization_pipeline_config->descriptor_count = 3;
    rasterization_pipeline_config->descriptors = (YsVkDescriptor*)yCMemoryAllocate(sizeof(YsVkDescriptor) * rasterization_pipeline_config->descriptor_count);
    rasterization_pipeline_config->descriptors[0] = resources->ubo_descriptor;
    rasterization_pipeline_config->descriptors[1] = resources->ssbo_descriptor;
    rasterization_pipeline_config->descriptors[2] = resources->shadow_map_image_descriptor;
    rasterization_pipeline_config->push_constant_range_count = resources->push_constant_range_count;
    rasterization_pipeline_config->push_constant_range = resources->push_constant_range;
    rasterization_pipeline_config->viewport.x = 0.0f;
    rasterization_pipeline_config->viewport.y = 0.0f;
    rasterization_pipeline_config->viewport.width = resources->rasterization_color_image->create_info->extent.width;
    rasterization_pipeline_config->viewport.height = resources->rasterization_color_image->create_info->extent.height;
    rasterization_pipeline_config->viewport.minDepth = 0.0f;
    rasterization_pipeline_config->viewport.maxDepth = 1.0f;
    rasterization_pipeline_config->scissor.offset.x = 0;
    rasterization_pipeline_config->scissor.offset.y = 0;
    rasterization_pipeline_config->scissor.extent.width = resources->rasterization_color_image->create_info->extent.width;
    rasterization_pipeline_config->scissor.extent.height = resources->rasterization_color_image->create_info->extent.height;
    rasterization_system->pipeline = yVkAllocatePipelineObject();
    if (!rasterization_system->pipeline->create(context,
                                                rasterization_pipeline_config,
                                                rasterization_system->pipeline)) {
        YERROR("Failed to create rasterization pipeline.");
        return false;
    }

    // Sync objects.
    rasterization_system->complete_semaphores = yCMemoryAllocate(sizeof(VkSemaphore) * context->swapchain->max_frames_in_flight);
    for (u8 i = 0; i < context->swapchain->max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context->device->logical_device,
                          &semaphore_create_info,
                          context->allocator,
                          &rasterization_system->complete_semaphores[i]);
    }

    return true;
}

static void cmdDrawCall(YsVkContext* context,
                        YsVkCommandUnit* command_unit,
                        u32 command_buffer_index,
                        YsVkResources* resources,
                        u32 current_present_image_index,
                        u32 current_frame,
                        void* push_constant_data,
                        YsVkRasterizationSystem* rasterization_system) {
    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.0f;
    clear_values[0].color.float32[1] = 0.0f;
    clear_values[0].color.float32[2] = 0.0f;
    clear_values[0].color.float32[3] = 1.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    vkCmdSetViewport(command_unit->command_buffers[command_buffer_index], 0, 1, &rasterization_system->pipeline->config->viewport);
    vkCmdSetScissor(command_unit->command_buffers[command_buffer_index], 0, 1, &rasterization_system->pipeline->config->scissor);

    VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    render_pass_begin_info.renderPass = rasterization_system->render_stage->render_pass_handle;
    render_pass_begin_info.framebuffer = rasterization_system->render_stage->framebuffers[current_frame];
    render_pass_begin_info.renderArea = rasterization_system->pipeline->config->scissor;
    render_pass_begin_info.clearValueCount = rasterization_system->render_stage->create_info->attachment_count;
    render_pass_begin_info.pClearValues = clear_values;
    vkCmdBeginRenderPass(command_unit->command_buffers[command_buffer_index], 
                         &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);                                                    

    vkCmdBindPipeline(command_unit->command_buffers[command_buffer_index],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      rasterization_system->pipeline->handle);

    VkBuffer vertex_buffers[3] = {resources->vertex_input_position_buffer->handle,
                                  resources->vertex_input_normal_buffer->handle,
                                  resources->vertex_input_material_id_buffer->handle};
    VkDeviceSize vertex_offsets[3] = {0};
    vkCmdBindVertexBuffers(command_unit->command_buffers[command_buffer_index],
                           0,
                           3,
                           vertex_buffers,
                           vertex_offsets);

    for(int i = 0; i < rasterization_system->pipeline->config->descriptor_count; ++i) {
        const VkDescriptorSet* p_descriptor_set = rasterization_system->pipeline->config->descriptors[i].is_single_descriptor_set ?
                                                  &rasterization_system->pipeline->config->descriptors[i].descriptor_sets[0] :
                                                  &rasterization_system->pipeline->config->descriptors[i].descriptor_sets[current_frame];                     

        vkCmdBindDescriptorSets(command_unit->command_buffers[command_buffer_index],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                rasterization_system->pipeline->pipeline_layout,
                                rasterization_system->pipeline->config->descriptors[i].set,
                                1,
                                p_descriptor_set,
                                0,
                                NULL); 
    }
                                                      
    vkCmdPushConstants(command_unit->command_buffers[command_buffer_index],
                       rasterization_system->pipeline->pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
                       0,
                       yPushConstantSize(),
                       push_constant_data);

    vkCmdDraw(command_unit->command_buffers[command_buffer_index],
              resources->current_draw_vertex_count,
              1,
              0,
              0);

    vkCmdEndRenderPass(command_unit->command_buffers[command_buffer_index]);
}

YsVkRasterizationSystem* yVkRasterizationSystemCreate() {
    YsVkRasterizationSystem* rasterization_system = yCMemoryAllocate(sizeof(YsVkRasterizationSystem));
    if(rasterization_system) {
        rasterization_system->initialize = initialize;
        rasterization_system->cmdDrawCall = cmdDrawCall;
    }

    return rasterization_system;
}

