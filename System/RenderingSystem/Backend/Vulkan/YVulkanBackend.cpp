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

#include "YVulkanBackend.hpp"
#include "YRendererFrontendManager.hpp"
#include "YRendererBackendManager.hpp"
#include "YProfiler.hpp"
#include "YVulkanContext.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanImage.h"
#include "YVulkanBuffer.h"
#include "YVulkanRasterizationSystem.h"
#include "YVulkanShadowMappingSystem.h"
#include "YVulkanPathTracingSystem.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "YDeveloperConsole.hpp"
#include "YMath.h"
#include "GLFW/glfw3.h"
#include "imgui_impl_vulkan.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <unistd.h>
#include <cstdio>
#include <cstdint>
#include <cinttypes>


YVulkanBackend::YVulkanBackend() {

    yCMemorySystemInitialize();

    this->initializeSupportInfo();

    //
    const i8* vk_enable_extensions[this->m_enable_extensions.size()];
    for(i32 i = 0; i < this->m_enable_extensions.size(); ++i) {
        vk_enable_extensions[i] = this->m_enable_extensions[i].extensionName;
    }

    const i8* vk_enable_layers[this->m_enable_layers.size()];
    for(i32 i = 0; i < this->m_enable_layers.size(); ++i) {
        vk_enable_layers[i] = this->m_enable_layers[i].layerName;
    }

    glm::fvec2 main_window_size = YRendererFrontendManager::instance()->mainWindowSize();
    this->m_vk_context = yVkContextCreate();
    this->m_vk_context->initialize(main_window_size.x,
                                   main_window_size.y,
                                   YRendererFrontendManager::instance()->glfwWindow(),
                                   vk_enable_extensions,
                                   this->m_enable_extensions.size(),
                                   vk_enable_layers,
                                   this->m_enable_layers.size(),
                                   this->m_vk_context);

    //
    this->m_image_available_semaphores = (VkSemaphore*)yCMemoryAllocate(sizeof(VkSemaphore) * this->m_vk_context->swapchain->max_frames_in_flight);
    this->m_in_flight_fences = (VkFence*)yCMemoryAllocate(sizeof(VkFence) * this->m_vk_context->swapchain->max_frames_in_flight);
    for (u8 i = 0; i < this->m_vk_context->swapchain->max_frames_in_flight; ++i) {
        VkSemaphoreCreateInfo semaphore_create_info = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        vkCreateSemaphore(this->m_vk_context->device->logical_device,
                          &semaphore_create_info,
                          this->m_vk_context->allocator,
                          &this->m_image_available_semaphores[i]);

        VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(this->m_vk_context->device->logical_device,
                               &fence_create_info,
                               this->m_vk_context->allocator,
                               &this->m_in_flight_fences[i]));
    }

    this->m_current_frame = 0;
    this->m_current_present_image_index = 0;

    //
    YeRendererResolution renderer_resolution = YRendererBackendManager::instance()->rendererResolution();
    glm::fvec2 renderer_image_size;
    switch (renderer_resolution){
        case YeRendererResolution::Original: {
            renderer_image_size = main_window_size;
            break;
        }
        case YeRendererResolution::Half: {
            renderer_image_size = main_window_size * 0.5f;
            break;
        }
        case YeRendererResolution::Double: {
            renderer_image_size = main_window_size * 2.0f;
            break;
        }
        default: {
            break;
        }
    }
    YsVkResourcesImageSize image_size;
    image_size.rasterization_image_width = renderer_image_size.x;
    image_size.rasterization_image_height = renderer_image_size.y;
    image_size.shadow_map_image_width = renderer_image_size.x;
    image_size.shadow_map_image_height = renderer_image_size.y;
    image_size.random_image_width = renderer_image_size.x;
    image_size.random_image_height = renderer_image_size.y;
    image_size.path_tracing_image_width = renderer_image_size.x;
    image_size.path_tracing_image_height = renderer_image_size.y;

    this->m_vk_resource = yVkAllocateResourcesObject();
    this->m_vk_resource->initialize(this->m_vk_context,
                                    this->m_vk_resource,
                                    image_size);

    this->m_ubo.physically_based_camera.position[0] = 278;
    this->m_ubo.physically_based_camera.position[1] = 273;
    this->m_ubo.physically_based_camera.position[2] = -800;
    this->m_ubo.physically_based_camera.target[0] = 278;
    this->m_ubo.physically_based_camera.target[1] = 273;
    this->m_ubo.physically_based_camera.target[2] = 279.6;
    this->m_ubo.physically_based_camera.forward[0] = 0.0f;
    this->m_ubo.physically_based_camera.forward[1] = 0.0f;
    this->m_ubo.physically_based_camera.forward[2] = 1.0f;
    this->m_ubo.physically_based_camera.up[0] = 0.0f;
    this->m_ubo.physically_based_camera.up[1] = 1.0f;
    this->m_ubo.physically_based_camera.up[2] = 0.0f;
    this->m_ubo.physically_based_camera.right[0] = 1.0f;
    this->m_ubo.physically_based_camera.right[1] = 0.0f;
    this->m_ubo.physically_based_camera.right[2] = 0.0f;
    this->m_ubo.physically_based_camera.focal_length = 35;
    this->m_ubo.physically_based_camera.image_sensor_width = 25.0f * (float(image_size.path_tracing_image_width) / float(image_size.path_tracing_image_height));
    this->m_ubo.physically_based_camera.image_sensor_height = 25;
    this->m_ubo.physically_based_camera.resolution[0] = image_size.path_tracing_image_width;
    this->m_ubo.physically_based_camera.resolution[1] = image_size.path_tracing_image_height;                                

    //
    this->m_rendering_system = (YsVkRenderingSystem*)yCMemoryAllocate(sizeof(YsVkRenderingSystem));

    this->m_rendering_system->output = yVkOutputSystemCreate();
    this->m_rendering_system->output->initialize(this->m_vk_context,
                                                 this->m_vk_resource,
                                                 this->m_rendering_system->output);

    this->m_rendering_system->rasterization = yVkRasterizationSystemCreate();
    this->m_rendering_system->rasterization->initialize(this->m_vk_context,
                                                        this->m_vk_resource,
                                                        this->m_rendering_system->rasterization);

    this->m_rendering_system->shadow_mapping = yVkMainShadowMappingCreate();
    this->m_rendering_system->shadow_mapping->initialize(this->m_vk_context,
                                                         this->m_vk_resource,
                                                         this->m_rendering_system->shadow_mapping);

    this->m_rendering_system->path_tracing = yVkPathTracingSystemCreate();
    this->m_rendering_system->path_tracing->initialize(this->m_vk_context,
                                                       this->m_vk_resource,
                                                       this->m_rendering_system->path_tracing);

    //
    YDeveloperConsole::instance()->init(this->m_vk_context,
                                        this->m_rendering_system,
                                        this->m_vk_resource);

    //
    this->m_init_finished = true;
}

YVulkanBackend::~YVulkanBackend() {

}

void YVulkanBackend::initializeSupportInfo()
{
    if (!glfwVulkanSupported()) {
        YFATAL("Vulkan not supported on this device.");
        return;
    }

    uint32_t support_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &support_extension_count, nullptr);
    this->m_support_extensions.resize(support_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &support_extension_count, this->m_support_extensions.data());

    uint32_t support_layer_count = 0;
    vkEnumerateInstanceLayerProperties(&support_layer_count, nullptr);
    this->m_support_layers.resize(support_layer_count);
    vkEnumerateInstanceLayerProperties(&support_layer_count, this->m_support_layers.data());

    this->m_enable_extensions.clear();
    for(i32 i = 0; i < this->m_support_extensions.size(); ++i) {
        this->m_enable_extensions.push_back(this->m_support_extensions[i]);
        YINFO("Enable Vulkan Instance Extension name: %s", this->m_support_extensions[i].extensionName);
    }

    this->m_enable_layers.clear();
    for(i32 i = 0; i < this->m_support_layers.size(); ++i) {
#ifdef NDEBUG
        if("VK_LAYER_KHRONOS_validation" == std::string(this->m_support_layers[i].layerName)) {
            continue;
        }
#endif
        if("VK_LAYER_LUNARG_api_dump" == std::string(this->m_support_layers[i].layerName)) {
            continue;
        }

        this->m_enable_layers.push_back(this->m_support_layers[i]);
        YINFO("Enable Vulkan layer name: %s", this->m_support_layers[i].layerName);
    }
}

void YVulkanBackend::shutdown() {

}

void YVulkanBackend::deviceUpdateVertexInput(u32 vertex_count,
                                             void* vertex_position_data,
                                             void* vertex_normal_data,
                                             void* vertex_material_id_data) {
    for(int i = 0; i < this->m_vk_context->swapchain->max_frames_in_flight; ++i) {
        vkWaitForFences(this->m_vk_context->device->logical_device,
                        1,
                        &this->m_in_flight_fences[i],
                        true,
                        UINT64_MAX);                    
    }                                             

    this->m_vk_resource->createVertexInputBuffer(this->m_vk_context,
                                                 this->m_vk_resource,
                                                 vertex_count);
    this->m_vk_resource->updateVertexInputBuffer(this->m_vk_context,
                                                 this->m_vk_resource,
                                                 this->m_vk_context->device->commandUnitsFront(this->m_vk_context->device),
                                                 vertex_position_data,
                                                 vertex_normal_data,
                                                 vertex_material_id_data);
}

void YVulkanBackend::deviceUpdateSsbo(u32 ssbo_data_size, void* ssbo_data) {
    for(int i = 0; i < this->m_vk_context->swapchain->max_frames_in_flight; ++i) {
        vkWaitForFences(this->m_vk_context->device->logical_device,
                        1,
                        &this->m_in_flight_fences[i],
                        true,
                        UINT64_MAX);                    
    } 

    this->m_vk_resource->createSsbo(this->m_vk_context,
                                    this->m_vk_resource,
                                    ssbo_data_size);
    this->m_vk_resource->updateSsboBuffer(this->m_vk_context,
                                          this->m_vk_resource,
                                          this->m_vk_context->device->commandUnitsBack(this->m_vk_context->device),
                                          ssbo_data);                                  
}

void YVulkanBackend::deviceUpdateUbo(void* ubo_data) {
    for(int i = 0; i < this->m_vk_context->swapchain->max_frames_in_flight; ++i) {
        vkWaitForFences(this->m_vk_context->device->logical_device,
                        1,
                        &this->m_in_flight_fences[i],
                        true,
                        UINT64_MAX);                    
    } 

    this->m_vk_resource->updateUboBuffer(this->m_vk_context,
                                         this->m_vk_resource,
                                         ubo_data);   

    for(int i = 0; i < this->m_vk_context->swapchain->max_frames_in_flight; ++i) {
        this->m_frame_status[i].need_draw_path_tracing = true;
        this->m_frame_status[i].need_draw_rasterization = true;     
    }                                                                                                      
}

b8 YVulkanBackend::framePrepare() {
    vkWaitForFences(this->m_vk_context->device->logical_device,
                    1,
                    &this->m_in_flight_fences[this->m_current_frame],
                    true,
                    UINT64_MAX);
                
    VkResult result = vkAcquireNextImageKHR(this->m_vk_context->device->logical_device,
                                            this->m_vk_context->swapchain->handle,
                                            UINT64_MAX,
                                            this->m_image_available_semaphores[this->m_current_frame],
                                            VK_NULL_HANDLE,
                                            &this->m_current_present_image_index);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        YFATAL("Failed to acquire swapchain image!");
        return false;
    }
    
    return true;
}

b8 YVulkanBackend::frameRun() {
    YsVkCommandUnit* command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, this->m_current_frame);
   
    uint64_t time_stamps[2] = {0};
    vkGetQueryPoolResults(this->m_vk_context->device->logical_device,
                          command_unit->query_pool_timestamps,
                          0,
                          2,
                          sizeof(time_stamps),
                          time_stamps,
                          sizeof(uint64_t),
                          VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    double execution_time_in_milliseconds = (time_stamps[1] - time_stamps[0]) * this->m_vk_context->device->properties.limits.timestampPeriod / 1000000.0;
    execution_time_in_milliseconds = round(execution_time_in_milliseconds * 10.0) / 10.0;
    YProfiler::instance()->accumulateGpuFrameTime(execution_time_in_milliseconds);

    u32 command_buffer_index = 0;
    VkCommandBuffer command_buffer = command_unit->command_buffers[command_buffer_index];
    this->m_vk_context->device->commandBufferBegin(command_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    {
        vkCmdResetQueryPool(command_buffer,
                            command_unit->query_pool_timestamps,
                            0,
                            2);
        vkCmdWriteTimestamp(command_buffer,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            command_unit->query_pool_timestamps,
                            0);                   

        if(this->m_frame_status[this->m_current_frame].need_draw_shadow_mapping) {
            this->m_rendering_system->shadow_mapping->cmdDrawCall(this->m_vk_context,
                                                                  command_unit,
                                                                  command_buffer_index,
                                                                  this->m_vk_resource,
                                                                  this->m_current_present_image_index,
                                                                  this->m_current_frame,
                                                                  &this->m_push_constant[this->m_current_frame],
                                                                  this->m_rendering_system->shadow_mapping);

            this->m_frame_status[this->m_current_frame].need_draw_shadow_mapping = false;                                                         
        }

        bool need_reset_path_tracing_image_layout = false;
        if(this->m_frame_status[this->m_current_frame].need_draw_path_tracing) {
            this->m_vk_resource->path_tracing_image->transitionLayout(command_buffer,
                                                                      this->m_current_frame,
                                                                      1,
                                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                                      VK_ACCESS_SHADER_READ_BIT,
                                                                      VK_ACCESS_SHADER_WRITE_BIT,
                                                                      command_unit->queue_family_index,
                                                                      command_unit->queue_family_index,
                                                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                                      this->m_vk_resource->path_tracing_image);

            this->m_rendering_system->path_tracing->cmdDispatchCall(this->m_vk_context,
                                                                    command_unit,
                                                                    command_buffer_index,
                                                                    this->m_vk_resource,
                                                                    this->m_current_present_image_index,
                                                                    this->m_current_frame,
                                                                    &this->m_push_constant[this->m_current_frame],
                                                                    this->m_rendering_system->path_tracing);

            this->m_vk_resource->path_tracing_image->transitionLayout(command_buffer,
                                                                      this->m_current_frame,
                                                                      1,
                                                                      VK_IMAGE_LAYOUT_GENERAL,
                                                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                                      VK_ACCESS_SHADER_WRITE_BIT,
                                                                      VK_ACCESS_SHADER_READ_BIT,
                                                                      command_unit->queue_family_index,
                                                                      command_unit->queue_family_index,
                                                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                                                      this->m_vk_resource->path_tracing_image);                                                        
            need_reset_path_tracing_image_layout = true;
            this->m_frame_status[this->m_current_frame].need_draw_path_tracing = false;                                                            
        }

        if(this->m_frame_status[this->m_current_frame].need_draw_rasterization) {
            this->m_rendering_system->rasterization->cmdDrawCall(this->m_vk_context,
                                                                 command_unit,
                                                                 command_buffer_index,
                                                                 this->m_vk_resource,
                                                                 this->m_current_present_image_index,
                                                                 this->m_current_frame,
                                                                 &this->m_push_constant[this->m_current_frame],
                                                                 this->m_rendering_system->rasterization);

            this->m_frame_status[this->m_current_frame].need_draw_rasterization = false;
        }

        this->m_rendering_system->output->cmdDrawCall(this->m_vk_context,
                                                      command_unit,
                                                      command_buffer_index,
                                                      this->m_vk_resource,
                                                      this->m_current_present_image_index,
                                                      this->m_current_frame,
                                                      &this->m_push_constant[this->m_current_frame],
                                                      this->m_rendering_system->output);

        vkCmdWriteTimestamp(command_buffer,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            command_unit->query_pool_timestamps,
                            1);
    }
    this->m_vk_context->device->commandBufferEnd(command_buffer);

    VK_CHECK(vkResetFences(this->m_vk_context->device->logical_device,
                           1,
                           &this->m_in_flight_fences[this->m_current_frame]));
    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &this->m_image_available_semaphores[this->m_current_frame];
    submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &this->m_rendering_system->output->complete_semaphores[this->m_current_frame];
    VkResult result = vkQueueSubmit(command_unit->queue,
                                    1,
                                    &submit_info,
                                    this->m_in_flight_fences[this->m_current_frame]);
    if (result != VK_SUCCESS) {
        YERROR("vkQueueSubmit failed with result: %s", string_VkResult(result));
        return false;
    }

    return true;
}

b8 YVulkanBackend::framePresent() {
    YsVkCommandUnit* result_command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, this->m_current_frame);
    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &this->m_rendering_system->output->complete_semaphores[this->m_current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &this->m_vk_context->swapchain->handle;
    present_info.pImageIndices = &this->m_current_present_image_index;
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(result_command_unit->queue, &present_info);
    if (result != VK_SUCCESS) {
        YFATAL("Failed to present swap chain image: %i", this->m_current_present_image_index);
    }

    this->m_current_frame = (this->m_current_frame + 1) % this->m_vk_context->swapchain->max_frames_in_flight;

    return true;
}


