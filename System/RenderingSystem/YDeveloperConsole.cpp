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

#include "YDeveloperConsole.hpp"
#include "YLogger.h"
#include "YProfiler.hpp"
#include "YGlobalInterface.hpp"
#include "YVulkanContext.h"
#include "YVulkanResource.h"
#include "YVulkanSwapchain.h"
#include "YVulkanImage.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanRasterizationSystem.h"
#include "YVulkanShadowMappingSystem.h"
#include "YVulkanPathTracingSystem.h"
#include "YCMemoryManager.h"
#include "imgui_impl_glfw.h"
#include "glfw/glfw3.h"
#include "YRendererBackendManager.hpp"
#include "YRendererFrontendManager.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>


#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR) / sizeof(*(_ARR))))


YDeveloperConsole* YDeveloperConsole::instance() {
    static YDeveloperConsole dc;
    return &dc;
}

YDeveloperConsole::YDeveloperConsole() {

}

YDeveloperConsole::~YDeveloperConsole() {

}

void YDeveloperConsole::init(YsVkContext* vk_context,
                             YsVkRenderingSystem* rendering_system,
                             YsVkResources* vk_resource) {
    this->m_vk_context = vk_context;
    this->m_rendering_system = rendering_system;
    this->m_vk_resource = vk_resource;
    VkDescriptorPoolSize pool_sizes[] = {{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 }};
    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 100;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;
    VK_CHECK(vkCreateDescriptorPool(vk_context->device->logical_device, 
                                    &pool_info, 
                                    vk_context->allocator, 
                                    &this->m_imgui_descriptor_pool));

    //
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(vk_context->swapchain->present_src_images->create_info->extent.width, 
                            vk_context->swapchain->present_src_images->create_info->extent.height);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/HelveticaNeue.ttc", 16.0f);

    ImGui_ImplGlfw_InitForVulkan(YRendererFrontendManager::instance()->glfwWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vk_context->instance;
    init_info.PhysicalDevice = vk_context->device->physical_device;
    init_info.Device = vk_context->device->logical_device;
    init_info.QueueFamily = vk_context->device->commandUnitsBack(vk_context->device)->queue_family_index;
    init_info.Queue = vk_context->device->commandUnitsBack(vk_context->device)->queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = this->m_imgui_descriptor_pool;
    init_info.RenderPass = rendering_system->output->render_stage->render_pass_handle;
    init_info.Subpass = 0;
    init_info.Allocator = vk_context->allocator;
    init_info.MinImageCount = 2;
    init_info.ImageCount = vk_context->swapchain->image_count;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);

    this->m_rasterization_image_descriptor_sets = (VkDescriptorSet*)yCMemoryAllocate(sizeof(VkDescriptorSet) * vk_context->swapchain->image_count);
    this->m_shadow_mapping_descriptor_sets = (VkDescriptorSet*)yCMemoryAllocate(sizeof(VkDescriptorSet) * vk_context->swapchain->image_count);
    for(int i = 0; i < vk_context->swapchain->image_count; ++i) {
        this->m_rasterization_image_descriptor_sets[i] = ImGui_ImplVulkan_AddTexture(vk_resource->sampler_linear,
                                                                                     vk_resource->rasterization_color_image->layer_views[i],
                                                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        this->m_shadow_mapping_descriptor_sets[i] = ImGui_ImplVulkan_AddTexture(vk_resource->sampler_linear,
                                                                                vk_resource->shadow_map_image->layer_views[i],
                                                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);                                                                                    
    }
}

void YDeveloperConsole::addLogMessage(int log_level, const std::string& message) {
    YsLogLineMessage m;
    m.message += this->currentTime();
    switch (log_level) {
        case YeLogLevel::LOG_LEVEL_FATAL:
            m.message += "  FATAL:  " + message;
            m.font_color = ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
            break;
        case YeLogLevel::LOG_LEVEL_ERROR:
            m.message += "  ERROR:  " + message;
            m.font_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            std::cout << m.message << std::endl;
            break;
        case YeLogLevel::LOG_LEVEL_WARN:
            m.message += "  WARN:  " + message;
            m.font_color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
            break;
        case YeLogLevel::LOG_LEVEL_INFO:
            m.message += "  INFO:  " + message;
            m.font_color = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
            break;
        case YeLogLevel::LOG_LEVEL_DEBUG:
            m.message += "  DEBUG:  " + message;
            m.font_color = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
            break;
        case YeLogLevel::LOG_LEVEL_TRACE:
            m.message += "  TRACE:  " + message;
            m.font_color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            break;
    }

    this->m_log_message.push_back(m);
}

void YDeveloperConsole::cmdDraw(YsVkCommandUnit* command_unit,
                                u32 command_buffer_index,
                                u32 current_frame, 
                                u32 current_present_image_index) {
    glm::fvec2 window_size = glm::fvec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
    static double last_time = glfwGetTime();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));
    ImGui::Begin("Log Debug", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        this->drawLog();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::End();

    ImGui::Begin("Performance Monitor");
    {
        ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        YeRendererResolution renderer_resolution = YRendererBackendManager::instance()->rendererResolution();
        glm::fvec2 renderer_image_size;
        switch (renderer_resolution){
            case YeRendererResolution::Original: {
                renderer_image_size = window_size;
                break;
            }
            case YeRendererResolution::Half: {
                renderer_image_size = window_size * 0.5f;
                break;
            }
            case YeRendererResolution::Double: {
                renderer_image_size = window_size * 2.0f;
                break;
            }
            default: {
                break;
            }
        }
        std::string str_render_res = std::to_string(u32(renderer_image_size.x)) + " x " + std::to_string(u32(renderer_image_size.y));
        ImGui::Text("Render Resolution: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text(str_render_res.c_str());ImGui::PopStyleColor();
        ImGui::Text("Render FPS: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%i", YProfiler::instance()->renderingFPS());ImGui::PopStyleColor();
        ImGui::Text("CPU FPS: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%i", YProfiler::instance()->cpuFPS());ImGui::PopStyleColor();
        ImGui::Text("GPU FPS: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%i", YProfiler::instance()->gpuFPS());ImGui::PopStyleColor();
        ImGui::Text("Render Frame Time(ms): ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%f", 1000.0f / YProfiler::instance()->renderingFPS());ImGui::PopStyleColor();
        ImGui::Text("CPU Frame Time(ms): ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%f", 1000.0f / YProfiler::instance()->cpuFPS());ImGui::PopStyleColor();
        ImGui::Text("GPU Frame Time(ms): ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%f", 1000.0f / YProfiler::instance()->gpuFPS());ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::Begin("Intermediate Image", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        ImGui::Image(u64(this->m_shadow_mapping_descriptor_sets[current_frame]),
                     ImVec2(this->m_vk_resource->shadow_map_image->create_info->extent.width * 0.05f,
                            this->m_vk_resource->shadow_map_image->create_info->extent.height * 0.05f));
        ImGui::Text("Shadow Map Image");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Image(u64(this->m_rasterization_image_descriptor_sets[current_frame]),
                     ImVec2(this->m_vk_resource->rasterization_color_image->create_info->extent.width * 0.05f,
                            this->m_vk_resource->rasterization_color_image->create_info->extent.height * 0.05f));
        ImGui::Text("Rasterization Image");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
    }
    ImGui::End();

    ImGui::Begin("Rendering Settings");
    {
        ImGui::SetNextItemWidth(150.0f);
        static int current_scene_item = 0;
        ImGui::Combo("Scene",
                     &current_scene_item,
                     this->m_scene_items,
                     IM_ARRAYSIZE(this->m_scene_items));

        ImGui::SetNextItemWidth(150.0f);
        static int current_rendering_model_item = 0;
        ImGui::Combo("Rendering Model",
                     &current_rendering_model_item,
                     this->m_rendering_model_items,
                     IM_ARRAYSIZE(this->m_rendering_model_items));
        {
            YsChangingRenderingModelEvent changing_rendering_model_event;
            switch(current_rendering_model_item) {
                case 0:{
                    changing_rendering_model_event.type = YeRenderingModelType::PathTracing;
                    break;
                }
                case 1:{
                    changing_rendering_model_event.type = YeRenderingModelType::Rasterization;
                    break;
                }
            }
            if(YRendererBackendManager::instance()->getRenderingModel() != changing_rendering_model_event.type) {
                YRendererBackendManager::instance()->setRenderingModel(changing_rendering_model_event.type);
                YEventHandlerManager::instance()->pushEvent(changing_rendering_model_event);
            }
        }

        ImGui::SetNextItemWidth(150.0f);
        static int current_rendering_api_item = 0;
        ImGui::Combo("Rendering API",
                     &current_rendering_api_item,
                     this->m_rendering_api_items,
                     IM_ARRAYSIZE(this->m_rendering_api_items));
    }
    ImGui::End();

    ImGui::Begin("Path Tracing Settings");
    {
        i32 spp = YRendererBackendManager::instance()->getPathTracingSpp();
        ImGui::InputInt("SPP", &spp, 1, 100, ImGuiInputTextFlags_CharsDecimal);
        spp = spp < 1 ? 1 : spp;
        if(spp != YRendererBackendManager::instance()->getPathTracingSpp()) {
            YsChangingPathTracingSppEvent e;
            e.spp = spp;
            YEventHandlerManager::instance()->pushEvent(e);
        }
        YRendererBackendManager::instance()->setPathTracingSpp(spp);

        i32 max_depth = YRendererBackendManager::instance()->getPathTracingMaxDepth();
        ImGui::InputInt("Max Depth", &max_depth, 1, 100, ImGuiInputTextFlags_CharsDecimal);
        max_depth = max_depth < 1 ? 1 : max_depth;
        if(max_depth != YRendererBackendManager::instance()->getPathTracingMaxDepth()) {
            YsChangingPathTracingMaxDepthEvent e;
            e.max_depth = max_depth;
            YEventHandlerManager::instance()->pushEvent(e);
        }
        YRendererBackendManager::instance()->setPathTracingMaxDepth(max_depth);

        bool enable_bvh_acceleration = YRendererBackendManager::instance()->getPathTracingEnableBvhAcceleration();
        ImGui::Checkbox("Enable BVH Acceleration", &enable_bvh_acceleration);
        if(enable_bvh_acceleration != YRendererBackendManager::instance()->getPathTracingEnableBvhAcceleration()) {
            YsChangingPathTracingEnableBvhAccelerationEvent e;
            e.enable_bvh_acceleration = enable_bvh_acceleration;
            YEventHandlerManager::instance()->pushEvent(e);
        }
        YRendererBackendManager::instance()->setPathTracingEnableBvhAcceleration(enable_bvh_acceleration);

        bool enable_denoiser = YRendererBackendManager::instance()->getPathTracingEnableDenoiser();
        ImGui::Checkbox("Enable Denoiser", &enable_denoiser);
        if(enable_denoiser != YRendererBackendManager::instance()->getPathTracingEnableDenoiser()) {
            YsChangingPathTracingEnableDenoiserEvent e;
            e.enable_denoiser = enable_denoiser;
            YEventHandlerManager::instance()->pushEvent(e);
        }
        YRendererBackendManager::instance()->setPathTracingEnableDenoiser(enable_denoiser);
    }  
    ImGui::End();  

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    ImGui_ImplVulkan_RenderDrawData(draw_data, command_unit->command_buffers[command_buffer_index]);
}

void YDeveloperConsole::drawLog() const {
    for (auto m : this->m_log_message) {
        ImGui::PushStyleColor(ImGuiCol_Text, m.font_color);
        ImGui::Text(m.message.c_str());
        ImGui::PopStyleColor();
    }
}

std::string YDeveloperConsole::currentTime() {
    auto now = std::chrono::system_clock::now();

    std::time_t now_t = std::chrono::system_clock::to_time_t(now);
    auto local_tm = *std::localtime(&now_t);

    auto since_epoch = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(since_epoch);
    since_epoch -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);

    std::ostringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S,") << std::setfill('0') << std::setw(3) << milliseconds.count();

    return ss.str();
}