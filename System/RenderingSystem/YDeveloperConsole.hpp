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

#ifndef CGPPY_YDEVELOPERCONSOLE_HPP
#define CGPPY_YDEVELOPERCONSOLE_HPP


#include "imgui.h"
#include "imgui_impl_vulkan.h"

#include "YAsyncTask.hpp"
#include "YDefines.h"

#include <string>
#include <vector>
#include <iostream>

struct YsVkContext;
struct YsVkResources;
struct YsVkRenderingSystem;


struct YsLogLineMessage {
    std::string message;
    ImVec4 font_color;
    int font_size;
};

class YDeveloperConsole {
public:
    static YDeveloperConsole* instance();

    void init(YsVkContext* vk_context,
              YsVkRenderingSystem* rendering_system,
              YsVkResources* vk_resource);

    void addLogMessage(int log_level, const std::string& message);

    void cmdDraw(unsigned int current_frame, unsigned int image_index);

    inline void accumulateGpuFrameTime(double time) {this->m_gpu_frame_time_accumulator += time; this->m_gpu_frame_count++;};

private:
    YDeveloperConsole();
    ~YDeveloperConsole();

    void drawLog() const;

    std::string currentTime();

private:
    YsVkContext* m_vk_context;
    YsVkRenderingSystem* m_rendering_system;
    YsVkResources* m_vk_resource;

    std::vector<YsLogLineMessage> m_log_message;

    VkDescriptorPool m_imgui_descriptor_pool;

    VkDescriptorSet m_shadow_mapping_descriptor_set;
    VkDescriptorSet* m_accumulate_image_descriptor_sets;
    VkDescriptorSet* m_rasterization_image_descriptor_sets;

    //
    const char* m_rendering_model_items[2] = {"Path Tracing", "Rasterization"};
    const char* m_rendering_api_items[4] = {"Vulkan", "Metal", "DirectX", "CPU"};
    const char* m_scene_items[10] = {"Cornell Box",
                                     "Utah Teapot",
                                     "Armadillo",
                                     "Stanford Bunny",
                                     "Stanford Dragon",
                                     "San Miguel",
                                     "Crytek Sponza",
                                     "Dubrovnik Sponza",
                                     "Conference Room",
                                     "LPS Head"};

    //
    int m_render_frame_count = 0;
    u32 m_render_fps = 0;

    double m_gpu_frame_time_accumulator = 0;
    u32 m_gpu_frame_count = 0;
    u32 m_gpu_fps = 0;
};

#endif //CGPPY_YDEVELOPERCONSOLE_HPP