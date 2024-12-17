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

#ifndef CGPPY_YRENDERERBACKENDMANAGER_HPP
#define CGPPY_YRENDERERBACKENDMANAGER_HPP


#include "YRendererBackend.hpp"

struct GLFWwindow;

#include <memory>
#include <map>


enum class YeRendererBackendApi : unsigned char {
    VULKAN,
    METAL,
    DIRECTX12,
    OPENGL,
    CPU
};

enum class YeRenderingModelType : unsigned char {
    PathTracing,
    Rasterization
};

enum class YeRendererResolution : unsigned char {
    Original,
    Half,
    Double
};

class YRendererBackendManager {
public:
    static YRendererBackendManager* instance();

    void initBackend();

    inline YRendererBackend* backend() {return this->m_renderer_backend.find(this->m_current_renderer_backend_api)->second.get();}

    inline YeRendererResolution rendererResolution() {return this->m_renderer_resolution;}

    inline YeRenderingModelType getRenderingModel() {return this->m_rendering_model;}
    inline void setRenderingModel(YeRenderingModelType value) {this->m_rendering_model = value;}
    inline u32 getPathTracingSpp() {return this->m_path_tracing_spp;}
    inline void setPathTracingSpp(const u32& value) {this->m_path_tracing_spp = value;}
    inline u32 getPathTracingMaxDepth() {return this->m_path_tracing_max_depth;}    
    inline void setPathTracingMaxDepth(const u32& value) {this->m_path_tracing_max_depth = value;}                                 
    inline u8 getPathTracingEnableBvhAcceleration() {return this->m_path_tracing_enable_bvh_acceleration;}
    inline void setPathTracingEnableBvhAcceleration(b8 value) {this->m_path_tracing_enable_bvh_acceleration = value;}
    inline u8 getPathTracingEnableDenoiser() {return this->m_path_tracing_enable_denoiser;}
    inline void setPathTracingEnableDenoiser(b8 value) {this->m_path_tracing_enable_denoiser = value;}

private:
    YRendererBackendManager();
    ~YRendererBackendManager();

private:
    YeRendererBackendApi m_current_renderer_backend_api;
    std::map<YeRendererBackendApi, std::unique_ptr<YRendererBackend>> m_renderer_backend;

    YeRendererResolution m_renderer_resolution;

    //
    YeRenderingModelType m_rendering_model = YeRenderingModelType::PathTracing;

    u32 m_path_tracing_spp = 1;
    u32 m_path_tracing_max_depth = 100;                                 
    u8 m_path_tracing_enable_bvh_acceleration = false;
    u8 m_path_tracing_enable_denoiser = false;
};


#endif //CGPPY_YRENDERERBACKENDMANAGER_HPP
