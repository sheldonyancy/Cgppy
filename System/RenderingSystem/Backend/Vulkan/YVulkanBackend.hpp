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

#ifndef CGPPY_YVULKANBACKEND_HPP
#define CGPPY_YVULKANBACKEND_HPP


#include "YRendererBackend.hpp"
#include "YVulkanTypes.h"
#include "YVulkanResource.h"

#include <vector>
#include <string>

struct GLFWwindow;
struct YsEntity;



class YVulkanBackend : public YRendererBackend {
public:
    YVulkanBackend();
    ~YVulkanBackend();

    void changingRenderingModel(int model) override;

private:
    void initializeSupportInfo();

    void shutdown();

    b8 framePrepare(i32* accumulate_image_index) override;
    b8 frameRun() override;
    b8 framePresent() override;

    void gapiUpdateVertexInputResource(u32 vertex_count,
                                       void* vertex_position_data,
                                       void* vertex_normal_data,
                                       void* vertex_material_id_data) override;

    void gapiUpdateGlobalUboResource(void* global_ubo_data) override;

    void gapiUpdateGlobalSceneBlockResource(u32 global_scene_block_data_size, void* global_scene_block_data) override;

    void gapiDrawProcess();
    void gapiDrawResult();
    void gapiRotatePhysicallyBasedCamera() override;

private:
    std::vector<VkExtensionProperties> m_support_extensions;
    std::vector<VkLayerProperties> m_support_layers;

    std::vector<VkExtensionProperties> m_enable_extensions;
    std::vector<VkLayerProperties> m_enable_layers;

    //
    YsVkContext* m_vk_context;
    YsVkResources* m_vk_resource;
    YsVkRenderingSystem* m_rendering_system;

    //
    const u32 m_max_semaphores = 10;
    VkSemaphore* m_image_available_semaphores;
    VkFence* m_in_flight_fences;
};


#endif //CGPPY_YVULKANBACKEND_HPP

