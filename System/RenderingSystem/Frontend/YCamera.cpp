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

#include "YCamera.hpp"
#include "YSceneManager.hpp"
#include "YGlobalInterface.hpp"
#include "YRendererFrontendManager.hpp"


YCamera::YCamera() {
    this->m_projection_type = YeProjectionType::ORTHOGONAL;
    this->m_position = glm::fvec3(0.0, 0.0, 0.0);
    this->m_target = glm::fvec3(0.0, 0.0, 0.0);
    this->m_up_direction = glm::fvec3(0.0, 0.0, 0.0);
    this->m_view_matrix = glm::fmat4x4(1.0);
    this->m_vulkan_projection_matrix = glm::fmat4x4(1.0);
    this->m_metal_projection_matrix = glm::fmat4x4(1.0);
    this->m_directx12_projection_matrix = glm::fmat4x4(1.0);
    this->m_opengl_projection_matrix = glm::fmat4x4(1.0);
    this->updateProjectionMatrix();
}

YCamera::~YCamera() {

}

void YCamera::setProjectionType(const YeProjectionType projection_type) {
    this->m_projection_type = projection_type;
    this->updateProjectionMatrix();
}

void YCamera::setPosition(const glm::fvec3& position) {
    this->m_position = position;
    this->updateViewMatrix();
}

void YCamera::setTarget(const glm::fvec3& target) {
    this->m_target = target;
    this->updateViewMatrix();
}

void YCamera::setUpDirection(const glm::fvec3& up_direction) {
    this->m_up_direction = glm::normalize(up_direction);
    this->updateViewMatrix();
}

void YCamera::updateProjectionMatrix() {
    glm::fvec2 main_window_size = YRendererFrontendManager::instance()->mainWindowSize();
    switch (this->m_projection_type) {
        case YeProjectionType::ORTHOGONAL:{
            glm::fvec2 delta = main_window_size / glm::fvec2(fmax(main_window_size.x, main_window_size.y));

            float left = -delta.x;
            float right = delta.x;
            float bottom = -delta.y;
            float top = delta.y;

            this->m_opengl_projection_matrix = glm::ortho(left, right, bottom, top, CAMERA_Z_NEAR, CAMERA_Z_FAR);

            glm::fmat4 clip = glm::fmat4(1.0f, 0.0f, 0.0f, 0.0f,
                                       0.0f, 1.0f, 0.0f, 0.0f,
                                       0.0f, 0.0f, 0.5f, 0.0f,
                                       0.0f, 0.0f, 0.5f, 1.0f);
            this->m_vulkan_projection_matrix = clip * this->m_opengl_projection_matrix;

            this->m_metal_projection_matrix = this->m_vulkan_projection_matrix;
            this->m_directx12_projection_matrix = this->m_vulkan_projection_matrix;

            break;
        }
        case YeProjectionType::PERSPECTIVE:{
            float fovy = glm::radians(45.0f);
            float aspect = main_window_size.x / main_window_size.y;
            this->m_opengl_projection_matrix = glm::perspective(fovy, aspect, CAMERA_Z_NEAR, CAMERA_Z_FAR);
            this->m_vulkan_projection_matrix = this->m_opengl_projection_matrix;

            break;
        }
        case YeProjectionType::FRUSTUM:{
            break;
        }
    }
}

void YCamera::updateViewMatrix() {
    this->m_view_matrix = glm::lookAt(this->m_position, this->m_target, this->m_up_direction);
}

const glm::fmat4x4 YCamera::getProjectionMatrix(YeRendererBackendApi renderer_backend_type) const {
    switch (renderer_backend_type) {
        case YeRendererBackendApi::VULKAN:{
            return this->m_vulkan_projection_matrix;
        }
        case YeRendererBackendApi::METAL:{
            return this->m_metal_projection_matrix;
        }
        case YeRendererBackendApi::DIRECTX12:{
            return this->m_directx12_projection_matrix;
        }
        case YeRendererBackendApi::OPENGL:{
            return this->m_opengl_projection_matrix;
        }
    }

    return this->m_vulkan_projection_matrix;
}

float YCamera::calculateFocalLength(float fov_degrees, float sensor_width) {
    float fov_radians = glm::radians(fov_degrees);
    return sensor_width / (2.0f * glm::tan(fov_radians / 2.0f));
}