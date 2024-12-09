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

#include "YOpenGLBackend.hpp"
#include "YGlobalInterface.hpp"
#include "YSceneManager.hpp"
#include "YEntity.hpp"
#include "YRendererFrontendManager.hpp"
#include "YCamera.hpp"
#include "YMeshComponent.hpp"
#include "YAssets.h"
#include "YLogger.h"

#include <iostream>


// ^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^^^^^^

static void GLClearAllErrors() {
#ifndef NDEBUG
    while(GL_NO_ERROR != glGetError()){}
#endif
}

static b8 GLCheckErrorStatus(const i8* function, i32 line) {
#ifndef NDEBUG
    while(GLenum error = glGetError()) {
        YERROR("OpenGL Error: %i, Line: %i, Function: %s", error, line, function);
        return true;
    }

    return false;
#else
    return true;
#endif
}

#define GLCheck(x) GLClearAllErrors();x;GLCheckErrorStatus(#x, __LINE__);

// ^^^^^^^^^^^^^^^^^ Error Handling Routines ^^^^^^^^^^^^^^^^^^^^


#define BUFFER_OFFSET(a) ((void*)(a))


enum AttribIDs { vin_position = 0, vin_fnormal = 1, vin_vnormal = 2, vin_color = 3, vin_texture_coordinate = 4 };


YOpenGLBackend::YOpenGLBackend()
    : m_framebuffer_object(0),
      m_program_result_object(0),
      m_program_polyhedron_object(0),
      m_program_image_object(0),
      m_gl_object_result(new YsGLObject),
      m_mvp_mat(glm::mat4(1.0f)) {

    this->m_framebuffer_texture_size = YGlobalInterface::instance()->getMainWindowSize();

    this->initFramebuffer();

    this->initShaderProgram();

    this->initResultGLObject();
}

YOpenGLBackend::~YOpenGLBackend() {

}

void YOpenGLBackend::initFramebuffer() {
    glGenFramebuffers(1, &this->m_framebuffer_object); // 3.0-4.5
    glBindFramebuffer(GL_FRAMEBUFFER, this->m_framebuffer_object); // 3.0-4.5

    glGenTextures(1, &this->m_gl_object_result->texture_object); // 2.0-4.5
    glBindTexture(GL_TEXTURE_2D, this->m_gl_object_result->texture_object); // 2.0-4.5
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // 2.0-4.5
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA32F,
                 this->m_framebuffer_texture_size.x,
                 this->m_framebuffer_texture_size.y,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 nullptr); // 2.0-4.5

    glBindTexture(GL_TEXTURE_2D, 0); // 2.0-4.5

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           this->m_gl_object_result->texture_object,
                           0); // 3.0-4.5

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        YERROR("glCheckFramebufferStatus error");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 3.0-4.5
}

void YOpenGLBackend::initShaderProgram()
{


}

void YOpenGLBackend::initResultGLObject() {
    static const GLfloat quad_position_data[] = {
                    1.0f, -1.0f,
                    -1.0f, -1.0f,
                    -1.0f, 1.0f,
                    1.0f, 1.0f,
            };
    static const GLfloat quad_texture_coordinate[] = {
                    0.0f, 0.0f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f
            };

    GLCheck(glGenVertexArrays(1, &this->m_gl_object_result->vao_object));
    GLCheck(glBindVertexArray(this->m_gl_object_result->vao_object));

    GLCheck(glGenBuffers(1, &this->m_gl_object_result->vbo_positions_object));
    GLCheck(glBindBuffer(GL_ARRAY_BUFFER, this->m_gl_object_result->vbo_positions_object));
    GLCheck(glBufferData(GL_ARRAY_BUFFER,
                         sizeof(quad_position_data),
                         quad_position_data,
                         GL_STATIC_DRAW));
    GLCheck(glEnableVertexAttribArray(AttribIDs::vin_position));
    GLCheck(glVertexAttribPointer(AttribIDs::vin_position,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  0,
                                  BUFFER_OFFSET(0)));

    GLCheck(glGenBuffers(1, &this->m_gl_object_result->vbo_texture_coordinate_object));
    GLCheck(glBindBuffer(GL_ARRAY_BUFFER, this->m_gl_object_result->vbo_texture_coordinate_object));
    GLCheck(glBufferData(GL_ARRAY_BUFFER,
                         sizeof(quad_texture_coordinate),
                         quad_texture_coordinate,
                         GL_STATIC_DRAW));
    GLCheck(glEnableVertexAttribArray(AttribIDs::vin_texture_coordinate));
    GLCheck(glVertexAttribPointer(AttribIDs::vin_texture_coordinate,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  0,
                                  BUFFER_OFFSET(0)));
}

void YOpenGLBackend::updateFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, this->m_framebuffer_object); // 3.0-4.5
    {
        GLCheck(glViewport(0,
                           0,
                           this->m_framebuffer_texture_size.x,
                           this->m_framebuffer_texture_size.y));

        GLCheck(glClearColor(0, 0, 0, 1)); // 2.0-4.5
        GLCheck(glClearDepth(1.0));
        GLCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // 2.0-4.5

        this->drawScene();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 3.0-4.5

    this->copyResultTextureDataToMemory();

    this->m_need_upate_framebuffer = false;
}

void YOpenGLBackend::drawScene() {
    GLCheck(glUseProgram(this->m_program_polyhedron_object));
    for(auto& object : this->m_gl_object_list) {
        this->drawPolyhedron(object.get());
    }
    GLCheck(glUseProgram(0));
}

void YOpenGLBackend::copyResultTextureDataToMemory() {
    i32 buffer_size = this->m_framebuffer_texture_size.x * this->m_framebuffer_texture_size.y * 4;

    this->m_result_texture_data.resize(buffer_size);

    GLCheck(glBindTexture(GL_TEXTURE_2D, this->m_gl_object_result->texture_object));
    GLCheck(glGetTexImage(GL_TEXTURE_2D,
                          0,
                          GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          this->m_result_texture_data.data()));
    GLCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

GLuint YOpenGLBackend::createShaderProgram(YsShaderInfo* shaders) {
    if (nullptr == shaders) {
        return 0;
    }

    GLuint program = glCreateProgram(); //2.0-4.5

    YsShaderInfo* entry = shaders;
    while (entry->type != GL_NONE) {
        GLuint shader = glCreateShader(entry->type); //2.0-4.5

        entry->shader = shader;

        std::string text = YGlobalInterface::instance()->readTextFile(entry->file_name);
        const GLchar* source = text.c_str();
        if (nullptr == source) {
            for (entry = shaders; entry->type != GL_NONE; ++entry) {
                GLCheck(glDeleteShader(entry->shader)); // 2.0-4.5
                entry->shader = 0;
            }

            return 0;
        }

        GLCheck(glShaderSource(shader,
                               1,
                               &source,
                               nullptr)); // 2.0-4.5

        GLCheck(glCompileShader(shader)); // 2.0-4.5

        GLint compiled;
        GLCheck(glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled)); // 2.0-4.5
        if (!compiled) {
            return 0;
        }

        GLCheck(glAttachShader(program, shader)); // 2.0-4.5

        ++entry;
    }

    GLCheck(glLinkProgram(program)); // 2.0-4.5

    GLint linked;
    GLCheck(glGetProgramiv(program, GL_LINK_STATUS, &linked)); // 2.0-4.5
    if (!linked) {
        for (entry = shaders; entry->type != GL_NONE; ++entry) {
            GLCheck(glDeleteShader(entry->shader)); // 2.0-4.5
            entry->shader = 0;
        }

        return 0;
    }

    return program;
}

void YOpenGLBackend::setUniform1i(const GLuint& program, const std::string& name, const GLint& value) const {
    GLint location = glGetUniformLocation(program, name.c_str()); // 2.0-4.5
    GLCheck(glProgramUniform1i(program, location,value)); // 4.1-4.5
}

void YOpenGLBackend::setUniform3fv(const GLuint& program, const std::string& name, const glm::fvec3& value) const {
    GLint location = glGetUniformLocation(program, name.c_str());
    GLCheck(glProgramUniform3fv(program,
                                location,
                                1,
                                glm::value_ptr(value))); // 4.1-4.5
}

void YOpenGLBackend::setUniformMatrix4fv(const GLuint& program, const std::string& name, const glm::mat4x4& value) const {
    GLint location = glGetUniformLocation(program, name.c_str()); // 2.0-4.5
    GLCheck(glProgramUniformMatrix4fv(program,
                                      location,
                                      1,
                                      false,
                                      glm::value_ptr(value))); // 4.1-4.5
}

void YOpenGLBackend::setUniformTexture2D(const GLuint& program,
                                         const std::string& name,
                                         const GLuint& texture_object,
                                         const u32 slot) const {
    GLCheck(glActiveTexture(GL_TEXTURE0 + slot)); // 2.0-4.5
    GLCheck(glBindTexture(GL_TEXTURE_2D, texture_object));

    this->setUniform1i(program, name, slot);
}

void YOpenGLBackend::entity2GLObject(YsEntity* entity) {

}

void YOpenGLBackend::drawPolyhedron(const YsGLObject* gl_object) {
    glm::fmat4x4 model_matrix = YSceneManager::instance()->modelMatrix();
    glm::fmat4x4 view_matrix = YRendererFrontendManager::instance()->camera()->getViewMatrix();
    glm::fmat4x4 projection_matrix = YRendererFrontendManager::instance()->camera()->getProjectionMatrix(YeRendererBackendApi::OPENGL);
    glm::fvec3 camera_position = YRendererFrontendManager::instance()->camera()->getPosition();

    this->setUniformMatrix4fv(this->m_program_polyhedron_object, "uniform_model_mat", model_matrix);
    this->setUniformMatrix4fv(this->m_program_polyhedron_object, "uniform_view_mat", view_matrix);
    this->setUniformMatrix4fv(this->m_program_polyhedron_object, "uniform_projection_mat", projection_matrix);
    this->setUniform3fv(this->m_program_polyhedron_object, "uniform_camera_position", camera_position);

    GLCheck(glBindVertexArray(gl_object->vao_object));
    GLCheck(glDrawArrays(GL_TRIANGLES, 0, gl_object->vertex_count));
    GLCheck(glBindVertexArray(0));
}

void YOpenGLBackend::drawImage(const YsGLObject* gl_object) {
    glm::vec3 scale_factor(1, -1, 1);
    glm::mat4 mvp_mat(1.0);
    mvp_mat = glm::scale(mvp_mat, scale_factor);

    this->setUniformMatrix4fv(this->m_program_image_object, "uniform_mvp_mat", mvp_mat);

    this->setUniformTexture2D(this->m_program_image_object,
                              "uniform_texture_image",
                              gl_object->texture_object,
                              0);

    GLCheck(glBindVertexArray(gl_object->vao_object));
    GLCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    GLCheck(glBindVertexArray(0));
}

void YOpenGLBackend::deviceUpdateVertexInput(u32 vertex_count,
                                                   void* vertex_position_data,
                                                   void* vertex_normal_data,
                                                   void* vertex_material_id_data) {

}

void YOpenGLBackend::deviceUpdateUbo(void* ubo_data) {

}

void YOpenGLBackend::deviceUpdateSsbo(u32 ssbo_data_size, void* ssbo_data) {

}

b8 YOpenGLBackend::frameRun() {
    if (this->m_need_upate_framebuffer) {
        this->updateFramebuffer();
    }

    GLCheck(glClearColor(0, 0, 0, 1)); // 2.0-4.5
    GLCheck(glClearDepth(1.0));
    GLCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)); // 2.0-4.5

    GLCheck(glUseProgram(this->m_program_result_object));

    this->setUniformTexture2D(this->m_program_result_object,
                              "uniform_texture_result",
                              this->m_gl_object_result->texture_object,
                              0);

    GLCheck(glBindVertexArray(this->m_gl_object_result->vao_object));
    GLCheck(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
    GLCheck(glBindVertexArray(0));

    GLCheck(glUseProgram(0));
}
