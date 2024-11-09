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

#include "YVulkanPathTracingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanContext.h"
#include "YVulkanDevice.h"
#include "YVulkanResource.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YAssets.h"


static b8 initializeVkPathTracingSystem(YsVkContext* context,
                                        YsVkResources* resources,
                                        YsVkPathTracingSystem* path_tracing_system) {
    //
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
    render_stage_create_info->attachment_descriptions[1].format = VK_FORMAT_R32_UINT;
    render_stage_create_info->attachment_descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
    render_stage_create_info->attachment_descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_stage_create_info->attachment_descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_stage_create_info->attachment_descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    render_stage_create_info->attachment_descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    render_stage_create_info->attachment_descriptions[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    render_stage_create_info->subpass_count = 1;
    render_stage_create_info->subpass_configs = yCMemoryAllocate(sizeof(YsSubpassConfig) * render_stage_create_info->subpass_count);
    render_stage_create_info->subpass_configs[0].reference_count = 2;
    render_stage_create_info->subpass_configs[0].attachment_references = yCMemoryAllocate(sizeof(VkAttachmentReference) * render_stage_create_info->subpass_configs[0].reference_count);
    render_stage_create_info->subpass_configs[0].attachment_references[0].attachment = 0;
    render_stage_create_info->subpass_configs[0].attachment_references[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    render_stage_create_info->subpass_configs[0].attachment_references[1].attachment = 1;
    render_stage_create_info->subpass_configs[0].attachment_references[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    render_stage_create_info->subpass_configs[0].subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    render_stage_create_info->subpass_configs[0].subpass_description.colorAttachmentCount = 2;
    render_stage_create_info->subpass_configs[0].subpass_description.pColorAttachments = render_stage_create_info->subpass_configs[0].attachment_references;
    render_stage_create_info->framebuffer_count = context->swapchain->image_count;
    render_stage_create_info->framebuffer_create_info = yCMemoryAllocate(sizeof(VkFramebufferCreateInfo) * render_stage_create_info->framebuffer_count);
    for(int i = 0; i < render_stage_create_info->framebuffer_count; ++i) {
        VkImageView* image_views = yCMemoryAllocate(sizeof(VkImageView) * render_stage_create_info->attachment_count);
        image_views[0] = resources->path_tracing_accumulate_image.individual_views[i];
        image_views[1] = resources->path_tracing_random_image.individual_views[i];
        render_stage_create_info->framebuffer_create_info[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        render_stage_create_info->framebuffer_create_info[i].attachmentCount = render_stage_create_info->attachment_count;
        render_stage_create_info->framebuffer_create_info[i].pAttachments = image_views;
        render_stage_create_info->framebuffer_create_info[i].width = resources->path_tracing_accumulate_image.create_info->extent.width;
        render_stage_create_info->framebuffer_create_info[i].height = resources->path_tracing_accumulate_image.create_info->extent.height;
        render_stage_create_info->framebuffer_create_info[i].layers = 1;
    }
    path_tracing_system->render_stage = yVkAllocateRenderStageObject();
    path_tracing_system->render_stage->create(context,
                                              render_stage_create_info,
                                              path_tracing_system->render_stage);
    //
    const u8 vertex_input_description_count = 2;
    VkVertexInputBindingDescription position_binding_description = {};
    position_binding_description.binding = 0;
    position_binding_description.stride = sizeof(vec4);
    position_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputBindingDescription texcoord_binding_description = {};
    texcoord_binding_description.binding = 1;
    texcoord_binding_description.stride = sizeof(vec4);
    texcoord_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputAttributeDescription position_attribute_description = {};
    position_attribute_description.binding = 0;
    position_attribute_description.location = 0;
    position_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    position_attribute_description.offset = 0;
    VkVertexInputAttributeDescription texcoord_attribute_description = {};
    texcoord_attribute_description.binding = 1;
    texcoord_attribute_description.location = 1;
    texcoord_attribute_description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    texcoord_attribute_description.offset = 0;

    VkVertexInputBindingDescription binding_descriptions[vertex_input_description_count] = {position_binding_description,
                                                                                            texcoord_binding_description};
    VkVertexInputAttributeDescription attribute_descriptions[vertex_input_description_count] = {position_attribute_description,
                                                                                                texcoord_attribute_description};
    VkPipelineVertexInputStateCreateInfo pipeline_vertex_info = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    pipeline_vertex_info.vertexBindingDescriptionCount = vertex_input_description_count;
    pipeline_vertex_info.pVertexBindingDescriptions = binding_descriptions;
    pipeline_vertex_info.vertexAttributeDescriptionCount = vertex_input_description_count;
    pipeline_vertex_info.pVertexAttributeDescriptions = attribute_descriptions;

    YsVkPipelineConfig* path_tracing_pipeline_config = yCMemoryAllocate(sizeof(YsVkPipelineConfig));
    path_tracing_pipeline_config->shader_config.shader_stage_config_count = 2;
    path_tracing_pipeline_config->shader_config.shader_stage_config[0].stage_flag = VK_SHADER_STAGE_VERTEX_BIT;
    path_tracing_pipeline_config->shader_config.shader_stage_config[0].source_length = getSpvCodeSize(Path_Tracing_Vert);
    path_tracing_pipeline_config->shader_config.shader_stage_config[0].source = getSpvCode(Path_Tracing_Vert);
    path_tracing_pipeline_config->shader_config.shader_stage_config[1].stage_flag = VK_SHADER_STAGE_FRAGMENT_BIT;
    path_tracing_pipeline_config->shader_config.shader_stage_config[1].source_length = getSpvCodeSize(Path_Tracing_Frag);
    path_tracing_pipeline_config->shader_config.shader_stage_config[1].source = getSpvCode(Path_Tracing_Frag);
    path_tracing_pipeline_config->render_stage = path_tracing_system->render_stage;
    path_tracing_pipeline_config->vertex_input_info = &pipeline_vertex_info;
    path_tracing_pipeline_config->descriptor_set_layout_count = 3;
    path_tracing_pipeline_config->descriptor_set_layouts = (VkDescriptorSetLayout*)yCMemoryAllocate(sizeof(VkDescriptorSetLayout) * path_tracing_pipeline_config->descriptor_set_layout_count);
    path_tracing_pipeline_config->descriptor_set_layouts[0] = resources->global_ubo_descriptor->descriptor_set_layout;
    path_tracing_pipeline_config->descriptor_set_layouts[1] = resources->global_scene_block_descriptor->descriptor_set_layout;
    path_tracing_pipeline_config->descriptor_set_layouts[2] = resources->image_descriptor->descriptor_set_layout;
    path_tracing_pipeline_config->viewport.x = 0.0f;
    path_tracing_pipeline_config->viewport.y = 0.0f;
    path_tracing_pipeline_config->viewport.width = resources->path_tracing_accumulate_image.create_info->extent.width;
    path_tracing_pipeline_config->viewport.height = resources->path_tracing_accumulate_image.create_info->extent.height;
    path_tracing_pipeline_config->viewport.minDepth = 0.0f;
    path_tracing_pipeline_config->viewport.maxDepth = 1.0f;
    path_tracing_pipeline_config->scissor.offset.x = 0;
    path_tracing_pipeline_config->scissor.offset.y = 0;
    path_tracing_pipeline_config->scissor.extent.width = resources->path_tracing_accumulate_image.create_info->extent.width;
    path_tracing_pipeline_config->scissor.extent.height = resources->path_tracing_accumulate_image.create_info->extent.height;
    path_tracing_system->pipeline = yVkAllocatePipelineObject();
    if (!path_tracing_system->pipeline->create(context,
                                               path_tracing_pipeline_config,
                                               path_tracing_system->pipeline)) {
        YERROR("Create Path Tracing Pipeline Failed.");
        return false;
    }

    //
    path_tracing_system->complete_semaphores = yCMemoryAllocate(sizeof(VkSemaphore) * context->swapchain->max_frames_in_flight);
    for (u8 i = 0; i < context->swapchain->max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(context->device->logical_device,
                          &semaphore_create_info,
                          context->allocator,
                          &path_tracing_system->complete_semaphores[i]);
    }

    //
    f32 position[4*4] = {-1.0f, 1.0f, 0.0f, 1.0f,
                         1.0f,  1.0f, 0.0f, 1.0f,
                         1.0f, -1.0f, 0.0f, 1.0f,
                         -1.0f, -1.0f, 0.0f, 1.0f};
    path_tracing_system->vertex_input_position_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (resources->bufferCreate(context,
                                sizeof(vec4) * 4,
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                true,
                                path_tracing_system->vertex_input_position_buffer)){
        resources->bufferLoadDataRange(context,
                                       context->device->commandUnitsFront(context->device),
                                       0,
                                       path_tracing_system->vertex_input_position_buffer,
                                       0,
                                       position);
    } else {
        YERROR("Error creating vertex buffer.");
    }

    //
    f32 texture_coord[4*4] = {0.0f, 1.0f, 0.0f, 1.0f,
                              1.0f, 1.0f, 0.0f, 1.0f,
                              1.0f, 0.0f, 0.0f, 1.0f,
                              0.0f, 0.0f, 0.0f, 1.0f};
    path_tracing_system->vertex_input_texcoord_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (resources->bufferCreate(context,
                        sizeof(vec4) * 4,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        true,
                        path_tracing_system->vertex_input_texcoord_buffer)) {
        resources->bufferLoadDataRange(context,
                                       context->device->commandUnitsFront(context->device),
                                       0,
                                       path_tracing_system->vertex_input_texcoord_buffer,
                                       0,
                                       texture_coord);
    } else {
        YERROR("Error creating vertex buffer.");
    }
    //
    u32 indices[6] = {0, 1, 2,
                      2, 3, 0};
    path_tracing_system->vertex_input_index_buffer = yCMemoryAllocate(sizeof(YsVkBuffer));
    if (resources->bufferCreate(context,
                                sizeof(u32) * 6,
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                true,
                                path_tracing_system->vertex_input_index_buffer)) {
        resources->bufferLoadDataRange(context,
                                       context->device->commandUnitsFront(context->device),
                                       0,
                                       path_tracing_system->vertex_input_index_buffer,
                                       0,
                                       indices);
    } else {
        YERROR("Error Creating Index Buffer.");
    }

    return true;
}

static void rendering(YsVkContext* context,
                      YsVkCommandUnit* command_unit,
                      YsVkResources* resources,
                      u32 image_index,
                      u32 current_frame,
                      void* push_constant_data,
                      YsVkPathTracingSystem* path_tracing_system) {
    //
    resources->transitionImageLayout(command_unit->command_buffers[0],
                                     &resources->path_tracing_accumulate_image,
                                     path_tracing_system->accumulate_image_index,
                                     1,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     VK_ACCESS_NONE,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     command_unit->queue_family_index,
                                     command_unit->queue_family_index,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    resources->transitionImageLayout(command_unit->command_buffers[0],
                                     &resources->path_tracing_random_image,
                                     path_tracing_system->accumulate_image_index,
                                     1,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     VK_ACCESS_NONE,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     command_unit->queue_family_index,
                                     command_unit->queue_family_index,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    //
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0;
    viewport.width = resources->path_tracing_accumulate_image.create_info->extent.width;
    viewport.height = resources->path_tracing_accumulate_image.create_info->extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = resources->path_tracing_accumulate_image.create_info->extent.width;
    scissor.extent.height = resources->path_tracing_accumulate_image.create_info->extent.height;

    VkClearValue clear_value;
    clear_value.color.float32[0] = 0.0f;
    clear_value.color.float32[1] = 0.0f;
    clear_value.color.float32[2] = 0.0f;
    clear_value.color.float32[3] = 1.0f;

    vkCmdSetViewport(command_unit->command_buffers[0], 0, 1, &viewport);
    vkCmdSetScissor(command_unit->command_buffers[0], 0, 1, &scissor);

    path_tracing_system->render_stage->renderPassBegin(command_unit->command_buffers[0],
                                                       &path_tracing_system->render_stage->framebuffers[current_frame],
                                                       scissor,
                                                       path_tracing_system->render_stage->create_info->attachment_count,
                                                       &clear_value,
                                                       path_tracing_system->render_stage);

    //
    path_tracing_system->pipeline->bind(command_unit->command_buffers[0], path_tracing_system->pipeline);


    VkBuffer vertex_buffers[2] = {path_tracing_system->vertex_input_position_buffer->handle,
                                  path_tracing_system->vertex_input_texcoord_buffer->handle};
    VkDeviceSize vertex_offsets[2] = {0};
    vkCmdBindVertexBuffers(command_unit->command_buffers[0],
                           0,
                           2,
                           vertex_buffers,
                           vertex_offsets);
    vkCmdBindIndexBuffer(command_unit->command_buffers[0],
                         path_tracing_system->vertex_input_index_buffer->handle,
                         0,
                         VK_INDEX_TYPE_UINT32);



    VkDescriptorSet descriptor_sets[3] = {resources->global_ubo_descriptor->descriptor_sets[image_index],
                                          resources->global_scene_block_descriptor->descriptor_sets[image_index],
                                          resources->image_descriptor->descriptor_sets[image_index]};
    path_tracing_system->pipeline->bindDescriptorSets(command_unit->command_buffers[0],
                                                      resources->global_ubo_descriptor->first_set,
                                                      3,
                                                      descriptor_sets,
                                                      path_tracing_system->pipeline);

    resources->cmdPushConstants(command_unit->command_buffers[0],
                                path_tracing_system->pipeline,
                                push_constant_data);

    vkCmdDrawIndexed(command_unit->command_buffers[0],
                     6,
                     1,
                     0,
                     0,
                     0);

    //
    path_tracing_system->render_stage->renderPassEnd(command_unit->command_buffers[0]);
}



YsVkPathTracingSystem* yVkPathTracingSystemCreate() {
    YsVkPathTracingSystem* path_tracing_system = yCMemoryAllocate(sizeof(YsVkPathTracingSystem));
    if(path_tracing_system) {
        path_tracing_system->initialize = initializeVkPathTracingSystem;
        path_tracing_system->rendering = rendering;
    }

    return path_tracing_system;
}