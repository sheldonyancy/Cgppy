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

#ifndef REALTIMERENDERING3D_YCAMERA_H
#define REALTIMERENDERING3D_YCAMERA_H


#include "YRendererBackendManager.hpp"


#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define CAMERA_Z_NEAR 0.1f
#define CAMERA_Z_FAR 5.0f

enum class YeProjectionType : unsigned char {
    PERSPECTIVE,
    ORTHOGONAL,
    FRUSTUM
};

class YCamera {
public:
    YCamera();
    ~YCamera();

    void updateProjectionMatrix();

    void setProjectionType(const YeProjectionType projection_type);

    void setPosition(const glm::fvec3& position);
    void setTarget(const glm::fvec3& target);
    void setUpDirection(const glm::fvec3& up_direction);

    inline const glm::fvec3 getPosition() const {return this->m_position;}
    inline const glm::fvec3 getTarget() const {return this->m_target;}
    inline const glm::fvec3 getUpDirection() const {return this->m_up_direction;}
    inline const glm::fvec3 getViewDirection() const {return glm::normalize(this->m_target - this->m_position);}
    inline const glm::fvec3 getRightDirection() const {return glm::normalize(glm::cross(this->getUpDirection(), this->getViewDirection()));}

    inline const glm::fmat4x4& getViewMatrix() const {return this->m_view_matrix;}
    const glm::fmat4x4 getProjectionMatrix(YeRendererBackendApi renderer_backend_type) const;

    float calculateFocalLength(float fov_degrees, float sensor_width);

private:
    void updateViewMatrix();

private:
    YeProjectionType m_projection_type;

    glm::fvec3 m_position;
    glm::fvec3 m_target;
    glm::fvec3 m_up_direction;

    glm::fmat4x4 m_view_matrix;
    glm::fmat4x4 m_vulkan_projection_matrix;
    glm::fmat4x4 m_metal_projection_matrix;
    glm::fmat4x4 m_opengl_projection_matrix;
    glm::fmat4x4 m_directx12_projection_matrix;
};


#endif //REALTIMERENDERING3D_YCAMERA_H
