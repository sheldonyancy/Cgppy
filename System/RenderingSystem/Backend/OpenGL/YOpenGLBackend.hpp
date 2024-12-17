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

#ifndef CGPPY_YOPENGLBACKEND_HPP
#define CGPPY_YOPENGLBACKEND_HPP


#include <GL/glew.h>


#include "YRendererBackend.hpp"
#include "YDefines.h"


#include <list>
#include <memory>

struct YsEntity;


typedef struct YsShaderInfo{
    GLenum type = 0;
    const i8* file_name = nullptr;
    GLuint shader = 0;
} YsShaderInfo;

typedef struct YsGLObject{
    i32 tpye = 0;

    GLuint vao_object = 0;
    GLuint vbo_positions_object = 0;
    GLuint vbo_normals_object = 0;
    GLuint vbo_colors_object = 0;
    GLuint vbo_texture_coordinate_object = 0;
    GLuint texture_object = 0;

    u32 vertex_count = 0;
} YsGLObject;

class YOpenGLBackend : public YRendererBackend {
public:
    YOpenGLBackend();

    ~YOpenGLBackend();

private:
    void initFramebuffer();
    void initShaderProgram();
    void initResultGLObject();
    GLuint createShaderProgram(YsShaderInfo* shaders);

    void drawScene();

    void entity2GLObject(YsEntity* entity);

    void drawPolyhedron(const YsGLObject* gl_object);
    void drawImage(const YsGLObject* gl_object);

    void copyResultTextureDataToMemory();

    void updateFramebuffer();

private:
    void setUniform1i(const GLuint& program, const std::string& name, const GLint& value) const;
    void setUniform3fv(const GLuint& program, const std::string& name, const glm::fvec3& value) const;
    void setUniformMatrix4fv(const GLuint& program, const std::string& name, const glm::mat4x4& value) const;
    void setUniformTexture2D(const GLuint& program,
                             const std::string& name,
                             const GLuint& texture_object,
                             const u32 slot) const;

    void deviceUpdateVertexInput(u32 vertex_count,
                               void* vertex_position_data,
                               void* vertex_normal_data,
                               void* vertex_material_id_data) override;

    void deviceUpdateUbo(void* ubo_data) override;

    void deviceUpdateSsbo(u32 ssbo_data_size, void* ssbo_data) override;

    b8 frameRun() override;
private:
    bool m_need_upate_framebuffer;

    glm::ivec2 m_framebuffer_texture_size;
    GLuint m_framebuffer_object;
    YsGLObject* m_gl_object_result;

    GLuint m_program_result_object;
    GLuint m_program_polyhedron_object;
    GLuint m_program_image_object;

    //
    std::list<std::unique_ptr<YsGLObject>> m_gl_object_list;

    glm::mat4 m_mvp_mat;

    //
    std::string m_result_texture_data;
};


#endif //CGPPY_YOPENGLBACKEND_HPP
