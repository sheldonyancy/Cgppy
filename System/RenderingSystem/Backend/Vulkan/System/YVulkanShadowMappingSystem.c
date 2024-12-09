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

#include "YVulkanShadowMappingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanContext.h"
#include "YVulkanDevice.h"
#include "YVulkanResource.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YAssets.h"


static b8 initialize(YsVkContext* context,
                                          YsVkResources* resources,
                                          YsVkShadowMappingSystem* shadow_mapping_system) {
    //
    YsVkRenderStageCreateInfo* render_stage_create_info = yCMemoryAllocate(sizeof(YsVkRenderStageCreateInfo));
    render_stage_create_info->attachment_count = 1;
    render_stage_create_info->attachment_descriptions = yCMemoryAllocate(sizeof(VkAttachmentDescription) * render_stage_create_info->attachment_count);
    render_stage_create_info->attachment_descriptions[0].format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    render_stage_create_info->attachment_descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
    render_stage_create_info->attachment_descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_stage_create_info->attachment_descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_stage_create_info->attachment_descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_stage_create_info->attachment_descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    render_stage_create_info->attachment_descriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    render_stage_create_info->subpass_count = 1;
    render_stage_create_info->subpass_configs = yCMemoryAllocate(sizeof(YsSubpassConfig) * render_stage_create_info->subpass_count);
    render_stage_create_info->subpass_configs[0].reference_count = 1;
    render_stage_create_info->subpass_configs[0].attachment_references = yCMemoryAllocate(sizeof(VkAttachmentReference) * render_stage_create_info->subpass_configs[0].reference_count);
    render_stage_create_info->subpass_configs[0].attachment_references[0].attachment = 0;
    render_stage_create_info->subpass_configs[0].attachment_references[0].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    render_stage_create_info->subpass_configs[0].subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    render_stage_create_info->subpass_configs[0].subpass_description.pDepthStencilAttachment = &render_stage_create_info->subpass_configs[0].attachment_references[0];
    render_stage_create_info->framebuffer_count = 1;
    render_stage_create_info->framebuffer_create_info = yCMemoryAllocate(sizeof(VkFramebufferCreateInfo) * render_stage_create_info->framebuffer_count);
    render_stage_create_info->framebuffer_create_info[0].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    render_stage_create_info->framebuffer_create_info[0].attachmentCount = render_stage_create_info->attachment_count;
    render_stage_create_info->framebuffer_create_info[0].pAttachments = &resources->shadow_map_image.individual_views[0];
    render_stage_create_info->framebuffer_create_info[0].width = resources->shadow_map_image.create_info->extent.width;
    render_stage_create_info->framebuffer_create_info[0].height = resources->shadow_map_image.create_info->extent.height;
    render_stage_create_info->framebuffer_create_info[0].layers = 1;
    shadow_mapping_system->render_stage = yVkAllocateRenderStageObject();
    shadow_mapping_system->render_stage->create(context,
                                                render_stage_create_info,
                                                shadow_mapping_system->render_stage);
    //
    VkPipelineVertexInputStateCreateInfo pipeline_vertex_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    pipeline_vertex_info.vertexBindingDescriptionCount = 1;
    pipeline_vertex_info.pVertexBindingDescriptions = &resources->vertex_position_binding_description;
    pipeline_vertex_info.vertexAttributeDescriptionCount = 1;
    pipeline_vertex_info.pVertexAttributeDescriptions = &resources->vertex_position_attribute_description;
    pipeline_vertex_info.pNext = NULL;
    pipeline_vertex_info.flags = 0;

    YsVkPipelineConfig* shadow_map_pipeline_config = yCMemoryAllocate(sizeof(YsVkPipelineConfig));
    shadow_map_pipeline_config->shader_config.shader_stage_config_count = 2;
    shadow_map_pipeline_config->shader_config.shader_stage_config[0].stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
    shadow_map_pipeline_config->shader_config.shader_stage_config[0].source_length = getSpvCodeSize(Shadow_Map_Vert);
    shadow_map_pipeline_config->shader_config.shader_stage_config[0].source = getSpvCode(Shadow_Map_Vert);
    shadow_map_pipeline_config->shader_config.shader_stage_config[1].stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadow_map_pipeline_config->shader_config.shader_stage_config[1].source_length = getSpvCodeSize(Shadow_Map_Frag);
    shadow_map_pipeline_config->shader_config.shader_stage_config[1].source = getSpvCode(Shadow_Map_Frag);
    shadow_map_pipeline_config->render_stage = shadow_mapping_system->render_stage;
    shadow_map_pipeline_config->vertex_input_info = &pipeline_vertex_info;
    shadow_map_pipeline_config->descriptor_set_layout_count = 1;
    shadow_map_pipeline_config->descriptor_set_layouts = (VkDescriptorSetLayout*)yCMemoryAllocate(sizeof(VkDescriptorSetLayout) * shadow_map_pipeline_config->descriptor_set_layout_count);
    shadow_map_pipeline_config->descriptor_set_layouts[0] = resources->ubo_descriptor->descriptor_set_layout;
    shadow_map_pipeline_config->viewport.x = 0.0f;
    shadow_map_pipeline_config->viewport.y = 0.0f;
    shadow_map_pipeline_config->viewport.width = resources->shadow_map_image.create_info->extent.width;
    shadow_map_pipeline_config->viewport.height = resources->shadow_map_image.create_info->extent.height;
    shadow_map_pipeline_config->viewport.minDepth = 0.0f;
    shadow_map_pipeline_config->viewport.maxDepth = 1.0f;
    shadow_map_pipeline_config->scissor.offset.x = 0;
    shadow_map_pipeline_config->scissor.offset.y = 0;
    shadow_map_pipeline_config->scissor.extent.width = resources->shadow_map_image.create_info->extent.width;
    shadow_map_pipeline_config->scissor.extent.height = resources->shadow_map_image.create_info->extent.height;
    shadow_mapping_system->pipeline = yVkAllocatePipelineObject();
    if (!shadow_mapping_system->pipeline->create(context,
                                                 shadow_map_pipeline_config,
                                                 shadow_mapping_system->pipeline)) {
        YERROR("Failed to create shadow mapping pipeline.");
        return false;
    }

    //
    shadow_mapping_system->complete_semaphores = (VkSemaphore*)yCMemoryAllocate(sizeof(VkSemaphore) * context->swapchain->max_frames_in_flight);
    for (u8 i = 0; i < context->swapchain->max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context->device->logical_device,
                          &semaphore_create_info,
                          context->allocator,
                          &shadow_mapping_system->complete_semaphores[i]);
    }

    return true;
}

static void cmdDrawCall(YsVkContext* context,
                        YsVkCommandUnit* command_unit,
                        u32 command_buffer_index,
                        YsVkResources* resources,
                        u32 image_index,
                        YsVkShadowMappingSystem* shadow_mapping_system,
                        void* push_constant_data) {
    //
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0;
    viewport.width = (f32)resources->shadow_map_image.create_info->extent.width;
    viewport.height = (f32)resources->shadow_map_image.create_info->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = resources->shadow_map_image.create_info->extent.width;
    scissor.extent.height = resources->shadow_map_image.create_info->extent.height;

    VkClearValue clear_value;
    clear_value.depthStencil.depth = 1.0f;
    clear_value.depthStencil.stencil = 0;

    vkCmdSetViewport(command_unit->command_buffers[command_buffer_index], 0, 1, &viewport);
    vkCmdSetScissor(command_unit->command_buffers[command_buffer_index], 0, 1, &scissor);

    VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    render_pass_begin_info.renderPass = shadow_mapping_system->render_stage->render_pass_handle;
    render_pass_begin_info.framebuffer = *shadow_mapping_system->render_stage->framebuffers;
    render_pass_begin_info.renderArea = scissor;
    render_pass_begin_info.clearValueCount = shadow_mapping_system->render_stage->create_info->attachment_count;
    render_pass_begin_info.pClearValues = &clear_value;
    vkCmdBeginRenderPass(command_unit->command_buffers[command_buffer_index], 
                         &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);                                                     

    vkCmdBindPipeline(command_unit->command_buffers[command_buffer_index],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      shadow_mapping_system->pipeline->handle);

    VkDeviceSize vertex_offsets = 0;
    vkCmdBindVertexBuffers(command_unit->command_buffers[command_buffer_index],
                           0,
                           1,
                           &resources->vertex_input_position_buffer->handle,
                           &vertex_offsets);

    vkCmdBindDescriptorSets(command_unit->command_buffers[command_buffer_index],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            shadow_mapping_system->pipeline->pipeline_layout,
                            resources->ubo_descriptor->first_set,
                            1,
                            &resources->ubo_descriptor->descriptor_sets[image_index],
                            0,
                            NULL);                                                    

    vkCmdDraw(command_unit->command_buffers[command_buffer_index],
              resources->current_draw_vertex_count,
              1,
              0,
              0);

    vkCmdEndRenderPass(command_unit->command_buffers[command_buffer_index]);
}

YsVkShadowMappingSystem* yVkMainShadowMappingCreate() {
    YsVkShadowMappingSystem* shadow_mapping_system = yCMemoryAllocate(sizeof(YsVkShadowMappingSystem));
    if(shadow_mapping_system) {
        shadow_mapping_system->initialize = initialize;
        shadow_mapping_system->cmdDrawCall = cmdDrawCall;
    }

    return shadow_mapping_system;
}