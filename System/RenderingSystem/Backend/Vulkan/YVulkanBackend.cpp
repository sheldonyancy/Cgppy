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
#include "YGlfwWindow.hpp"
#include "YVulkanContext.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanOutputSystem.h"
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


VkBool32 myDebugCallback(VkDebugReportFlagsEXT flags,
                         VkDebugReportObjectTypeEXT objectType,
                         uint64_t object,
                         size_t location,
                         int32_t messageCode,
                         const char* pLayerPrefix,
                         const char* pMessage,
                         void* pUserData) {
    if(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        printf("debugPrintfEXT: %s", pMessage);
    }

    return false;
}

PFN_vkCreateDebugReportCallbackEXT pfnVkCreateDebugReportCallbackEXT = nullptr;

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

    this->m_vk_context = yVkContextCreate();
    this->m_vk_context->initialize(YRendererFrontendManager::instance()->glfwWindow()->glfwWindow(),
                                   vk_enable_extensions,
                                   this->m_enable_extensions.size(),
                                   vk_enable_layers,
                                   0, //this->m_enable_layers.size(),
                                   this->m_vk_context);
    //
    this->m_global_ubo_data.physically_based_camera.position[0] = 278;
    this->m_global_ubo_data.physically_based_camera.position[1] = 273;
    this->m_global_ubo_data.physically_based_camera.position[2] = -800;
    this->m_global_ubo_data.physically_based_camera.target[0] = 278;
    this->m_global_ubo_data.physically_based_camera.target[1] = 273;
    this->m_global_ubo_data.physically_based_camera.target[2] = 279.6;
    this->m_global_ubo_data.physically_based_camera.forward[0] = 0.0f;
    this->m_global_ubo_data.physically_based_camera.forward[1] = 0.0f;
    this->m_global_ubo_data.physically_based_camera.forward[2] = 1.0f;
    this->m_global_ubo_data.physically_based_camera.up[0] = 0.0f;
    this->m_global_ubo_data.physically_based_camera.up[1] = 1.0f;
    this->m_global_ubo_data.physically_based_camera.up[2] = 0.0f;
    this->m_global_ubo_data.physically_based_camera.right[0] = 1.0f;
    this->m_global_ubo_data.physically_based_camera.right[1] = 0.0f;
    this->m_global_ubo_data.physically_based_camera.right[2] = 0.0f;
    this->m_global_ubo_data.physically_based_camera.focal_length = 35;
    this->m_global_ubo_data.physically_based_camera.image_sensor_width = 25.0f * (float(this->m_vk_context->framebuffer_width) / float(this->m_vk_context->framebuffer_height));
    this->m_global_ubo_data.physically_based_camera.image_sensor_height = 25;
    this->m_global_ubo_data.physically_based_camera.resolution[0] = 1920;
    this->m_global_ubo_data.physically_based_camera.resolution[1] = (this->m_global_ubo_data.physically_based_camera.image_sensor_height
                                                                     / this->m_global_ubo_data.physically_based_camera.image_sensor_width
                                                                    ) * this->m_global_ubo_data.physically_based_camera.resolution[0];

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
    this->m_image_index = 0;

    //
    vec2 shadow_map_image_resolution;
    yVec2ToC(this->m_shadow_map_image_resolution, shadow_map_image_resolution);

    this->m_vk_resource = yVkAllocateResourcesObject();
    this->m_vk_resource->initialize(this->m_vk_context,
                                    this->m_vk_resource,
                                    shadow_map_image_resolution,
                                    this->m_global_ubo_data.physically_based_camera.resolution);

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
    /*
    VkDebugReportCallbackEXT debug_callback_handle;

    VkDebugReportCallbackCreateInfoEXT ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ci.pfnCallback = myDebugCallback;
    ci.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    ci.pUserData = this->m_vk_context;

    //vkCreateDebugReportCallbackEXT(this->m_vk_context->instance,
    //                               &ci,
    //                               nullptr,
    //                               &debug_callback_handle);

    pfnVkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(this->m_vk_context->instance, "vkCreateDebugReportCallbackEXT"));

    pfnVkCreateDebugReportCallbackEXT(this->m_vk_context->instance, &ci, nullptr, &debug_callback_handle);
    */
    //
    this->m_vk_resource->updateRandomImage(this->m_vk_context,
                                           this->m_vk_resource,
                                           this->m_vk_context->device->commandUnitsFront(this->m_vk_context->device),
                                           this->m_global_ubo_data.physically_based_camera.resolution);

    //
    this->m_async_task = std::make_unique<YAsyncTask<void>>();

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
        this->m_enable_layers.push_back(this->m_support_layers[i]);
        YINFO("Enable Vulkan layer name: %s", this->m_support_layers[i].layerName);
    }
}

void YVulkanBackend::shutdown() {

}

void YVulkanBackend::gapiUpdateVertexInputResource(u32 vertex_count,
                                                   void* vertex_position_data,
                                                   void* vertex_normal_data,
                                                   void* vertex_material_id_data) {
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

void YVulkanBackend::gapiUpdateGlobalUboResource(void* global_ubo_data) {
    this->m_vk_resource->updateGlobalUboBuffer(this->m_vk_context,
                                               this->m_vk_resource,
                                               global_ubo_data);
}

void YVulkanBackend::gapiUpdateGlobalSceneBlockResource(u32 global_scene_block_data_size, void* global_scene_block_data) {
    this->m_vk_resource->createGlobalSceneBlockBuffer(this->m_vk_context,
                                                      this->m_vk_resource,
                                                      global_scene_block_data_size);
    this->m_vk_resource->updateGlobalSceneBlockBuffer(this->m_vk_context,
                                                      this->m_vk_resource,
                                                      global_scene_block_data);

    this->m_paint_result = true;

    this->m_rendering_system->output->updateVertex(this->m_vk_context,
                                                   this->m_vk_resource,
                                                   this->m_rendering_system->output,
                                                   this->m_global_ubo_data.rendering_model);
}

b8 YVulkanBackend::framePrepare(i32* accumulate_image_index) {
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
                                            &this->m_image_index);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        YFATAL("Failed to acquire swapchain image!");
        return false;
    }

    this->m_rendering_system->path_tracing->accumulate_image_index = (this->m_image_index + 1) % this->m_vk_context->swapchain->image_count;
    *accumulate_image_index = this->m_rendering_system->path_tracing->accumulate_image_index;

    return true;
}

b8 YVulkanBackend::frameRun() {
    this->m_vk_resource->updateDescriptorSets(this->m_vk_context,
                                              this->m_vk_resource,
                                              this->m_rendering_system,
                                              this->m_image_index);
    this->gapiDrawProcess();

    this->gapiDrawResult();

    return true;
}

void YVulkanBackend::gapiDrawProcess() {
    // process
    YsVkCommandUnit* process_command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, this->m_current_frame);
    VkCommandBuffer process_command_buffer = process_command_unit->command_buffers[0];

    uint64_t time_stamps[2] = {0};
    vkGetQueryPoolResults(this->m_vk_context->device->logical_device,
                          process_command_unit->query_pool_timestamps,
                          0,
                          2,
                          sizeof(time_stamps),
                          time_stamps,
                          sizeof(uint64_t),
                          VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    double execution_time_in_milliseconds = (time_stamps[1] - time_stamps[0]) * this->m_vk_context->device->properties.limits.timestampPeriod / 1000000.0;
    execution_time_in_milliseconds = round(execution_time_in_milliseconds * 10.0) / 10.0;
    YDeveloperConsole::instance()->accumulateGpuFrameTime(execution_time_in_milliseconds);

    this->m_vk_context->device->commandBufferBegin(process_command_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    {
        vkCmdResetQueryPool(process_command_buffer,
                            process_command_unit->query_pool_timestamps,
                            0,
                            2);
        vkCmdWriteTimestamp(process_command_buffer,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            process_command_unit->query_pool_timestamps,
                            0);

        if(this->m_need_draw_shadow_mapping) {
            this->m_rendering_system->shadow_mapping->rendering(this->m_vk_context,
                                                                process_command_unit,
                                                                this->m_vk_resource,
                                                                this->m_image_index,
                                                                this->m_rendering_system->shadow_mapping,
                                                                &this->push_constants_data);
        }
        if(this->m_need_draw_path_tracing) {
            this->m_rendering_system->path_tracing->rendering(this->m_vk_context,
                                                              process_command_unit,
                                                              this->m_vk_resource,
                                                              this->m_image_index,
                                                              this->m_current_frame,
                                                              &this->push_constants_data,
                                                              this->m_rendering_system->path_tracing);
        }
        if(this->m_need_draw_rasterization) {
            this->m_rendering_system->rasterization->rendering(this->m_vk_context,
                                                               process_command_unit,
                                                               this->m_vk_resource,
                                                               this->m_image_index,
                                                               this->m_current_frame,
                                                               &this->push_constants_data,
                                                               this->m_rendering_system->rasterization);
        }

        vkCmdWriteTimestamp(process_command_buffer,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            process_command_unit->query_pool_timestamps,
                            1);
    }
    this->m_vk_context->device->commandBufferEnd(process_command_buffer);

    VkSemaphore process_signal_semaphores[this->m_max_semaphores];
    u32 process_signal_semaphores_count = 0;
    if(this->m_need_draw_shadow_mapping) {
        process_signal_semaphores[process_signal_semaphores_count++] = this->m_rendering_system->shadow_mapping->complete_semaphores[this->m_current_frame];
    }
    if(this->m_need_draw_rasterization) {
        process_signal_semaphores[process_signal_semaphores_count++] = this->m_rendering_system->rasterization->complete_semaphores[this->m_current_frame];
    }
    if(this->m_need_draw_path_tracing) {
        process_signal_semaphores[process_signal_semaphores_count++] = this->m_rendering_system->path_tracing->complete_semaphores[this->m_current_frame];
    }
    VkSubmitInfo process_submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    process_submit_info.commandBufferCount = 1;
    process_submit_info.pCommandBuffers = &process_command_buffer;
    process_submit_info.signalSemaphoreCount = process_signal_semaphores_count;
    process_submit_info.pSignalSemaphores = process_signal_semaphores;
    VkResult result = vkQueueSubmit(process_command_unit->queue,
                                    1,
                                    &process_submit_info,
                                    VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        YERROR("vkQueueSubmit failed with result: %s", string_VkResult(result));
        return;
    }
}

void YVulkanBackend::gapiDrawResult() {
    // result
    YsVkCommandUnit* result_command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, this->m_current_frame);
    VkCommandBuffer result_command_buffer = result_command_unit->command_buffers[1];
    this->m_vk_context->device->commandBufferBegin(result_command_buffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    {
        this->m_rendering_system->output->rendering(this->m_vk_context,
                                                    result_command_unit,
                                                    this->m_vk_resource,
                                                    this->m_image_index,
                                                    this->m_current_frame,
                                                    this->m_rendering_system->output);
    }
    this->m_vk_context->device->commandBufferEnd(result_command_buffer);

    VkSemaphore result_wait_semaphores[this->m_max_semaphores];
    result_wait_semaphores[0] = this->m_image_available_semaphores[this->m_current_frame];
    u32 result_wait_semaphores_count = 1;
    if(this->m_need_draw_shadow_mapping) {
        result_wait_semaphores[result_wait_semaphores_count++] = this->m_rendering_system->shadow_mapping->complete_semaphores[this->m_current_frame];
    }
    if(this->m_need_draw_rasterization) {
        result_wait_semaphores[result_wait_semaphores_count++] = this->m_rendering_system->rasterization->complete_semaphores[this->m_current_frame];
    }
    if(this->m_need_draw_path_tracing) {
        result_wait_semaphores[result_wait_semaphores_count++] = this->m_rendering_system->path_tracing->complete_semaphores[this->m_current_frame];
    }
    VkPipelineStageFlags result_wait_dst_stage_mask[result_wait_semaphores_count];
    for(int i = 0; i < result_wait_semaphores_count; ++i) {
        result_wait_dst_stage_mask[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    VK_CHECK(vkResetFences(this->m_vk_context->device->logical_device,
                           1,
                           &this->m_in_flight_fences[this->m_current_frame]));
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.waitSemaphoreCount = result_wait_semaphores_count;
    submit_info.pWaitSemaphores = result_wait_semaphores;
    submit_info.pWaitDstStageMask = result_wait_dst_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &result_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &this->m_rendering_system->output->complete_semaphores[this->m_current_frame];
    VkResult result = vkQueueSubmit(result_command_unit->queue,
                                    1,
                                    &submit_info,
                                    this->m_in_flight_fences[this->m_current_frame]);
    if (result != VK_SUCCESS) {
        YERROR("vkQueueSubmit failed with result: %s", string_VkResult(result));
    }

    //this->m_need_draw_shadow_mapping = false;
    //if(this->m_global_ubo_data.samples > 3) {
    //    this->m_need_draw_path_tracing = false;
    //}
}

b8 YVulkanBackend::framePresent() {
    YsVkCommandUnit* result_command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, this->m_current_frame);
    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &this->m_rendering_system->output->complete_semaphores[this->m_current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &this->m_vk_context->swapchain->handle;
    present_info.pImageIndices = &this->m_image_index;
    present_info.pResults = nullptr;
    VkResult result = vkQueuePresentKHR(result_command_unit->queue, &present_info);
    if (result != VK_SUCCESS) {
        YFATAL("Failed to present swap chain image: %i", this->m_image_index);
    }

    this->m_current_frame = (this->m_current_frame + 1) % this->m_vk_context->swapchain->max_frames_in_flight;

    return true;
}

void YVulkanBackend::changingRenderingModel(int model) {
    this->m_global_ubo_data.rendering_model = model;

    this->m_rendering_system->output->updateVertex(this->m_vk_context,
                                                   this->m_vk_resource,
                                                   this->m_rendering_system->output,
                                                   this->m_global_ubo_data.rendering_model);
}

void YVulkanBackend::gapiRotatePhysicallyBasedCamera() {
    YsVkCommandUnit* result_command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, this->m_current_frame);
    this->m_vk_resource->imageClear(this->m_vk_context,
                                    result_command_unit,
                                    &this->m_vk_resource->path_tracing_accumulate_image,
                                    0,
                                    this->m_vk_context->swapchain->image_count);
}

