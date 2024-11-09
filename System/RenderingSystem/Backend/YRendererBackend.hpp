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

#ifndef CGPPY_YRENDERERBACKEND_HPP
#define CGPPY_YRENDERERBACKEND_HPP

#include "YDefines.h"
#include "YAsyncTask.hpp"

#include <list>

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <vector>

enum class YeRendererBackendApi : unsigned char {
    VULKAN,
    METAL,
    DIRECTX12,
    OPENGL,
    CPU
};


class YRendererBackend {
public:
    ~YRendererBackend();

    void draw();

    void pushBackVertexPositions(std::vector<glm::fvec4>::iterator input_first, std::vector<glm::fvec4>::iterator input_last);
    void pushBackVertexNormals(std::vector<glm::fvec4>::iterator input_first, std::vector<glm::fvec4>::iterator input_last);
    void pushBackVertexMaterialId(std::vector<i32>::iterator input_first, std::vector<i32>::iterator input_last);
    void pushBackVertexEntitylId(std::vector<i32>::iterator input_first, std::vector<i32>::iterator input_last);

    inline i32 vertexCount() {return this->m_vertex_positions.size();}
    inline void* vertexPositionsData() {return &(this->m_vertex_positions.front());}
    inline void* vertexNormalsData() {return &(this->m_vertex_normals.front());}
    inline void* vertexMaterialIdData() {return &(this->m_vertex_material_id.front());}
    inline void* vertexEntityIdData() {return &(this->m_vertex_entity_id.front());}

    void updateVertexInputResource();
    void updateGlobalSceneBlockResource();

    virtual void changingRenderingModel(int model) = 0;

    inline i32 samples() {return this->m_global_ubo_data.samples;}

    void rotatePhysicallyBasedCamera(const glm::fquat& rotation);

protected:
    YRendererBackend();

    void updateGlobalUboResource();

    virtual b8 framePrepare(i32* accumulate_image_index) = 0;
    virtual b8 frameRun() = 0;
    virtual b8 framePresent() = 0;

    virtual void gapiUpdateVertexInputResource(u32 vertex_count,
                                               void* vertex_position_data,
                                               void* vertex_normal_data,
                                               void* vertex_material_id_data) = 0;

    virtual void gapiUpdateGlobalUboResource(void* global_ubo_data) = 0;

    virtual void gapiUpdateGlobalSceneBlockResource(u32 global_scene_block_data_size, void* global_scene_block_data) = 0;

    virtual void gapiRotatePhysicallyBasedCamera() = 0;

protected:
    bool m_init_finished;

    bool m_paint_result;

    bool m_need_draw_shadow_mapping;
    bool m_need_draw_rasterization;
    bool m_need_draw_path_tracing;
    //
    glm::ivec2 m_shadow_map_image_resolution;

    // vertex_data;
    std::vector<glm::fvec4> m_vertex_positions;
    std::vector<glm::fvec4> m_vertex_normals;
    std::vector<i32> m_vertex_material_id;
    std::vector<i32> m_vertex_entity_id;

    GLSL_GlobalUniformObject m_global_ubo_data;
    GLSL_GlobalSceneBlock m_global_scene_block_data;
    GLSL_PushConstantsObject push_constants_data;

    //
    u32 m_image_index;
    u32 m_current_frame;

    //
    std::unique_ptr<YAsyncTask<void>> m_async_task;
};


#endif //CGPPY_YRENDERERBACKEND_HPP
