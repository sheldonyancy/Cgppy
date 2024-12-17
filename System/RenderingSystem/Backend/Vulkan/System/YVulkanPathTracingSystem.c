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
#include "YVulkanImage.h"
#include "YVulkanResource.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YAssets.h"
#include "YGlobalFunction.h"

#include <stdio.h>
#include <math.h>


static b8 initialize(YsVkContext* context,
                     YsVkResources* resources,
                     YsVkPathTracingSystem* path_tracing_system) {
    YsVkPipelineConfig* path_tracing_pipeline_config = yCMemoryAllocate(sizeof(YsVkPipelineConfig));
    path_tracing_pipeline_config->pipeline_type = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    path_tracing_pipeline_config->shader_config.shader_stage_config_count = 1;
    path_tracing_pipeline_config->shader_config.shader_stage_config[0].stage_flag = VK_SHADER_STAGE_COMPUTE_BIT;
    path_tracing_pipeline_config->shader_config.shader_stage_config[0].source_length = getSpvCodeSize(Path_Tracing_Comp);
    path_tracing_pipeline_config->shader_config.shader_stage_config[0].source = getSpvCode(Path_Tracing_Comp);
    
    path_tracing_pipeline_config->descriptor_count = 4;
    path_tracing_pipeline_config->descriptors = (YsVkDescriptor*)yCMemoryAllocate(sizeof(YsVkDescriptor) * path_tracing_pipeline_config->descriptor_count);
    path_tracing_pipeline_config->descriptors[0] = resources->ubo_descriptor;
    path_tracing_pipeline_config->descriptors[1] = resources->ssbo_descriptor;
    path_tracing_pipeline_config->descriptors[2] = resources->random_image_descriptor;
    path_tracing_pipeline_config->descriptors[3] = resources->path_tracing_image_compute_storage_descriptor;
    path_tracing_pipeline_config->push_constant_range_count = resources->push_constant_range_count;
    path_tracing_pipeline_config->push_constant_range = resources->push_constant_range;
    
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
    path_tracing_system->group_count_x = (resources->path_tracing_image->create_info->extent.width + 32 - 1) / 32;
    path_tracing_system->group_count_y = (resources->path_tracing_image->create_info->extent.height + 32 - 1) / 32;
    path_tracing_system->group_count_z = 1;

    return true;
}

static void cmdDispatchCall(YsVkContext* context,
                        YsVkCommandUnit* command_unit,
                        u32 command_buffer_index,
                        YsVkResources* resources,
                        u32 current_present_image_index,
                        u32 current_frame,
                        void* push_constant_data,
                        YsVkPathTracingSystem* path_tracing_system) {
    //
    vkCmdBindPipeline(command_unit->command_buffers[command_buffer_index],
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      path_tracing_system->pipeline->handle);

    for(int i = 0; i < path_tracing_system->pipeline->config->descriptor_count; ++i) {
        const VkDescriptorSet* p_descriptor_set = path_tracing_system->pipeline->config->descriptors[i].is_single_descriptor_set ?
                                                  &path_tracing_system->pipeline->config->descriptors[i].descriptor_sets[0] :
                                                  &path_tracing_system->pipeline->config->descriptors[i].descriptor_sets[current_frame];

        vkCmdBindDescriptorSets(command_unit->command_buffers[command_buffer_index],
                                VK_PIPELINE_BIND_POINT_COMPUTE,
                                path_tracing_system->pipeline->pipeline_layout,
                                path_tracing_system->pipeline->config->descriptors[i].set,
                                1,
                                p_descriptor_set,
                                0,
                                NULL);
    }                  
    
    vkCmdPushConstants(command_unit->command_buffers[command_buffer_index],
                       path_tracing_system->pipeline->pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
                       0,
                       yPushConstantSize(),
                       push_constant_data);

    vkCmdDispatch(command_unit->command_buffers[command_buffer_index], 
                  path_tracing_system->group_count_x, 
                  path_tracing_system->group_count_y,
                  path_tracing_system->group_count_z);
}

YsVkPathTracingSystem* yVkPathTracingSystemCreate() {
    YsVkPathTracingSystem* path_tracing_system = yCMemoryAllocate(sizeof(YsVkPathTracingSystem));
    if(path_tracing_system) {
        path_tracing_system->initialize = initialize;
        path_tracing_system->cmdDispatchCall = cmdDispatchCall;
    }

    return path_tracing_system;
}