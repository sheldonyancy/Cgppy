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
#include "YGLSLStructs.hpp"

#include <list>

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

struct YsBVHNodeComponent;

struct YsFrameStatus {
    bool need_draw_shadow_mapping = true;
    bool need_draw_rasterization = true;
    bool need_draw_path_tracing = true;
};

class YRendererBackend {
public:
    ~YRendererBackend();

    void updateHostVertexInput();
    void updateHostSsbo();
    void updateHostUbo();

    inline void setNeedDraw(bool status) {this->m_need_draw = status;}
    void draw();

    void rotatePhysicallyBasedCamera(const glm::fquat& rotation);

protected:
    YRendererBackend();

    virtual b8 framePrepare() = 0;
    virtual b8 frameRun() = 0;
    virtual b8 framePresent() = 0;

    virtual void deviceUpdateVertexInput(u32 vertex_count,
                                       void* vertex_position_data,
                                       void* vertex_normal_data,
                                       void* vertex_material_id_data) = 0;

    virtual void deviceUpdateSsbo(u32 ssbo_data_size, void* ssbo_data) = 0;

    virtual void deviceUpdateUbo(void* ubo_data) = 0;

private:
    void recursiveFillingBVHBuffer(std::vector<GLSL_BVHNode>* bvh_buffers, YsBVHNodeComponent* node);    

protected:
    bool m_init_finished;

    bool m_need_draw;

    YsFrameStatus m_frame_status[3];

    bool m_need_update_device_vertex_input;
    bool m_need_update_device_ssbo;
    bool m_need_update_device_ubo;

    // vertex_data
    std::vector<glm::fvec4> m_vertex_positions;
    std::vector<glm::fvec4> m_vertex_normals;
    std::vector<i32> m_vertex_material_id;
    std::vector<i32> m_vertex_entity_id;

    // ssbo_data
    GLSL_SSBO m_ssbo = {};
    // ubo_data
    GLSL_UBO m_ubo = {};
    // push_constant_data
    GLSL_PushConstantObject m_push_constant[3] = {};

    //
    u32 m_current_present_image_index;
    u32 m_current_frame;

    //
    
};


#endif //CGPPY_YRENDERERBACKEND_HPP
