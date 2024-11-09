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
#include "YGlobalInterface.hpp"
#include "YVulkanContext.h"
#include "YVulkanResource.h"
#include "YVulkanRenderingSystem.h"
#include "YVulkanOutputSystem.h"
#include "YVulkanRasterizationSystem.h"
#include "YVulkanShadowMappingSystem.h"
#include "YVulkanPathTracingSystem.h"
#include "YCMemoryManager.h"
#include "imgui_impl_glfw.h"
#include "glfw/glfw3.h"
#include "YEventHandlerManager.hpp"
#include "YRendererBackendManager.hpp"
#include "YRendererFrontendManager.hpp"
#include "YGlfwWindow.hpp"

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
    VK_CHECK(vkCreateDescriptorPool(vk_context->device->logical_device, &pool_info, vk_context->allocator, &this->m_imgui_descriptor_pool));

    //
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(vk_context->framebuffer_width, vk_context->framebuffer_height);
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("/System/Library/Fonts/HelveticaNeue.ttc", 16.0f);

    ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)YRendererFrontendManager::instance()->glfwWindow()->glfwWindow(), true);
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

    this->m_shadow_mapping_descriptor_set = ImGui_ImplVulkan_AddTexture(vk_resource->sampler_linear,
                                                                        vk_resource->shadow_map_image.individual_views[0],
                                                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    this->m_accumulate_image_descriptor_sets = (VkDescriptorSet*)yCMemoryAllocate(sizeof(VkDescriptorSet) * vk_context->swapchain->image_count);
    this->m_rasterization_image_descriptor_sets = (VkDescriptorSet*)yCMemoryAllocate(sizeof(VkDescriptorSet) * vk_context->swapchain->image_count);
    for(int i = 0; i < vk_context->swapchain->image_count; ++i) {
        this->m_accumulate_image_descriptor_sets[i] = ImGui_ImplVulkan_AddTexture(vk_resource->sampler_linear,
                                                                                  vk_resource->path_tracing_accumulate_image.individual_views[i],
                                                                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        this->m_rasterization_image_descriptor_sets[i] = ImGui_ImplVulkan_AddTexture(vk_resource->sampler_linear,
                                                                                     vk_resource->rasterization_color_image.individual_views[i],
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

void YDeveloperConsole::cmdDraw(unsigned int current_frame, unsigned int image_index) {
    glm::fvec2 window_size = YGlobalInterface::instance()->getMainWindowSize();
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
        double current_time = glfwGetTime();
        ++this->m_render_frame_count;
        if(current_time - last_time >= 1.0) {
            this->m_render_fps = this->m_render_frame_count;
            this->m_render_frame_count = 0;
            last_time = current_time;
        }

        if(this->m_gpu_frame_time_accumulator >= 1000.0) {
            this->m_gpu_fps = this->m_gpu_frame_count;
            this->m_gpu_frame_time_accumulator = 0;
            this->m_gpu_frame_count = 0;
        }

        ImVec4 yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        ImGui::Text("Render FPS: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%i", this->m_render_fps);ImGui::PopStyleColor();
        ImGui::Text("GPU FPS: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%i", this->m_gpu_fps);ImGui::PopStyleColor();
        ImGui::Text("CPU FPS: ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("unknown");ImGui::PopStyleColor();
        ImGui::Text("Render Frame Time(ms): ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%f", 1000.0f / this->m_render_fps);ImGui::PopStyleColor();
        ImGui::Text("GPU Frame Time(ms): ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("%f", 1000.0f / this->m_gpu_fps);ImGui::PopStyleColor();
        ImGui::Text("CPU Frame Time(ms): ");ImGui::SameLine();ImGui::PushStyleColor(ImGuiCol_Text, yellow);ImGui::Text("unknown");ImGui::PopStyleColor();
        ImGui::Text("Samples: %llu", YRendererBackendManager::instance()->samples());
    }
    ImGui::End();

    ImGui::Begin("Intermediate Image", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        ImGui::Image(this->m_shadow_mapping_descriptor_set,
                     ImVec2(this->m_vk_resource->shadow_map_image.create_info->extent.width * 0.2f,
                            this->m_vk_resource->shadow_map_image.create_info->extent.height * 0.2f));
        ImGui::Text("Shadow Map Image");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Image(this->m_rasterization_image_descriptor_sets[image_index],
                     ImVec2(this->m_vk_resource->rasterization_color_image.create_info->extent.width * 0.1f,
                            this->m_vk_resource->rasterization_color_image.create_info->extent.height * 0.1f));
        ImGui::Text("Rasterization Image");
        ImGui::Dummy(ImVec2(0.0f, 20.0f));
        ImGui::Image(this->m_accumulate_image_descriptor_sets[image_index],
                     ImVec2(this->m_vk_resource->path_tracing_accumulate_image.create_info->extent.width * 0.2f,
                            this->m_vk_resource->path_tracing_accumulate_image.create_info->extent.height * 0.2f));
        ImGui::Text("Path Tracing Accumulate Image");
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
            YEventHandlerManager::instance()->pushEvent(changing_rendering_model_event);
        }

        ImGui::SetNextItemWidth(150.0f);
        static int current_rendering_api_item = 0;
        ImGui::Combo("Rendering API",
                     &current_rendering_api_item,
                     this->m_rendering_api_items,
                     IM_ARRAYSIZE(this->m_rendering_api_items));
    }
    ImGui::End();

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    YsVkCommandUnit* result_command_unit = this->m_vk_context->device->commandUnitsAt(this->m_vk_context->device, current_frame);
    VkCommandBuffer result_command_buffer = result_command_unit->command_buffers[1];
    ImGui_ImplVulkan_RenderDrawData(draw_data, result_command_buffer);
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