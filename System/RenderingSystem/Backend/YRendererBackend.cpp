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

#include "YRendererBackend.hpp"
#include "YPhysicsSystem.hpp"
#include "YMaterialSystem.hpp"
#include "YSceneManager.hpp"
#include "YEntity.hpp"
#include "YLightComponent.hpp"
#include "YRendererFrontendManager.hpp"
#include "YGlfwWindow.hpp"
#include "YCamera.hpp"
#include "YMath.h"
#include "YCMemoryManager.h"
#include "YLogger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


YRendererBackend::YRendererBackend()
    : m_init_finished(false),
      m_paint_result(false),
      m_need_draw_shadow_mapping(true),
      m_need_draw_rasterization(true),
      m_need_draw_path_tracing(true){
    this->m_shadow_map_image_resolution = glm::ivec2(1024);
    this->m_global_ubo_data.samples = 0;
}

YRendererBackend::~YRendererBackend() {

}

void YRendererBackend::pushBackVertexPositions(std::vector<glm::fvec4>::iterator input_first, std::vector<glm::fvec4>::iterator input_last) {
    this->m_vertex_positions.insert(this->m_vertex_positions.end(), input_first, input_last);
}

void YRendererBackend::pushBackVertexNormals(std::vector<glm::fvec4>::iterator input_first, std::vector<glm::fvec4>::iterator input_last) {
    this->m_vertex_normals.insert(this->m_vertex_normals.end(), input_first, input_last);
}

void YRendererBackend::pushBackVertexMaterialId(std::vector<i32>::iterator input_first, std::vector<i32>::iterator input_last) {
    this->m_vertex_material_id.insert(this->m_vertex_material_id.end(), input_first, input_last);
}

void YRendererBackend::pushBackVertexEntitylId(std::vector<i32>::iterator input_first, std::vector<i32>::iterator input_last) {
    this->m_vertex_entity_id.insert(this->m_vertex_entity_id.end(), input_first, input_last);
}

void YRendererBackend::updateVertexInputResource() {
    this->gapiUpdateVertexInputResource(this->vertexCount(),
                                        this->vertexPositionsData(),
                                        this->vertexNormalsData(),
                                        this->vertexMaterialIdData());
}

void YRendererBackend::updateGlobalUboResource() {
    YsAreaLightComponent* light_component = YSceneManager::instance()->getComponents<YsAreaLightComponent>().front();
    glm::fvec3 light_center = YSceneManager::instance()->modelMatrix() * glm::fvec4(light_component->center, 1.0);
    glm::fvec3 light_target = YSceneManager::instance()->modelMatrix() * glm::fvec4(light_component->light_space_target, 1.0);
    glm::fvec3 light_up = glm::normalize((transpose(inverse(YSceneManager::instance()->modelMatrix())) * glm::fvec4(light_component->light_space_up, 1.0)));
    glm::fmat4x4 light_view_matrix = glm::lookAt(light_center, light_target, light_up);
    float fovy = glm::radians(100.0f);
    float aspect = float(this->m_shadow_map_image_resolution.x) / float(this->m_shadow_map_image_resolution.y);
    glm::fmat4x4 light_projection_matrix = glm::perspective(fovy, aspect, CAMERA_Z_NEAR, 1.3f);
    light_component->light_space_matrix = light_projection_matrix * light_view_matrix;

    //
    yVec3ToC(YRendererFrontendManager::instance()->camera()->getPosition(), this->m_global_ubo_data.rasterization_camera.position);
    yMat4ToC(YRendererFrontendManager::instance()->camera()->getViewMatrix(), this->m_global_ubo_data.rasterization_camera.view_matrix);
    yMat4ToC(YRendererFrontendManager::instance()->camera()->getProjectionMatrix(YeRendererBackendApi::VULKAN), this->m_global_ubo_data.rasterization_camera.projection_matrix);

    YsEntity* light_entity = YSceneManager::instance()->getEntity(light_component);
    this->m_global_ubo_data.light.entity_id = light_entity->id;
    yVec3ToC(light_component->center, this->m_global_ubo_data.light.center);
    yVec3ToC(light_component->p0, this->m_global_ubo_data.light.p0);
    yVec3ToC(light_component->p1, this->m_global_ubo_data.light.p1);
    yVec3ToC(light_component->p2, this->m_global_ubo_data.light.p2);
    yVec3ToC(light_component->p3, this->m_global_ubo_data.light.p3);
    yVec3ToC(light_component->le, this->m_global_ubo_data.light.le);
    yMat4ToC(light_component->light_space_matrix, this->m_global_ubo_data.light.space_matrix);

    glm::mat4x4 cpp_model_matrix = YSceneManager::instance()->modelMatrix();
    yMat4ToC(cpp_model_matrix, this->push_constants_data.model_matrix);

    //
    this->gapiUpdateGlobalUboResource(&this->m_global_ubo_data);
}

void YRendererBackend::updateGlobalSceneBlockResource() {
    u64 global_scene_block_data_size = sizeof(GLSL_GlobalSceneBlock);

    this->m_global_scene_block_data.bvh_node_count = YPhysicsSystem::instance()->bvhNodeCount();
    this->m_global_scene_block_data.material_count = YMaterialSystem::instance()->materialCount();
    this->m_global_scene_block_data.vertex_count = this->vertexCount();
    yCMemoryCopy(this->m_global_scene_block_data.bvh_node,
                 YPhysicsSystem::instance()->bvhData(),
                 sizeof(GLSL_BVHNode) * this->m_global_scene_block_data.bvh_node_count);
    yCMemoryCopy(this->m_global_scene_block_data.materials,
                 YMaterialSystem::instance()->materialData(),
                 sizeof(GLSL_Material) * this->m_global_scene_block_data.material_count);
    yCMemoryCopy(this->m_global_scene_block_data.vertex_position,
                 this->vertexPositionsData(),
                 sizeof(vec4) * this->m_global_scene_block_data.vertex_count);
    yCMemoryCopy(this->m_global_scene_block_data.vertex_normal,
                 this->vertexNormalsData(),
                 sizeof(vec4) * this->m_global_scene_block_data.vertex_count);
    yCMemoryCopy(this->m_global_scene_block_data.vertex_material_id,
                 this->vertexMaterialIdData(),
                 sizeof(i32) * this->m_global_scene_block_data.vertex_count);
    yCMemoryCopy(this->m_global_scene_block_data.vertex_entity_id,
                 this->vertexEntityIdData(),
                 sizeof(i32) * this->m_global_scene_block_data.vertex_count);

    //
    this->gapiUpdateGlobalSceneBlockResource(global_scene_block_data_size, &this->m_global_scene_block_data);
}

void YRendererBackend::draw() {
    if (!this->m_init_finished) {
        return;
    }

    if (!this->m_paint_result) {
        return;
    }

    if (!this->framePrepare(&(this->m_global_ubo_data.accumulate_image_index))) {
        YERROR("Frame Prepare Error!");
        return;
    }

    this->m_global_ubo_data.samples++;

    this->updateGlobalUboResource();

    this->frameRun();

    if(!this->framePresent()) {
        YERROR("Frame Present Error!");
        return;
    }
}

void YRendererBackend::rotatePhysicallyBasedCamera(const glm::fquat& rotation) {
    glm::fvec3 position = glm::fvec3(this->m_global_ubo_data.physically_based_camera.position[0],
                                     this->m_global_ubo_data.physically_based_camera.position[1],
                                     this->m_global_ubo_data.physically_based_camera.position[2]);
    glm::fvec3 target = glm::fvec3(this->m_global_ubo_data.physically_based_camera.target[0],
                                     this->m_global_ubo_data.physically_based_camera.target[1],
                                     this->m_global_ubo_data.physically_based_camera.target[2]);
    glm::fvec3 up = glm::fvec3(this->m_global_ubo_data.physically_based_camera.up[0],
                               this->m_global_ubo_data.physically_based_camera.up[1],
                               this->m_global_ubo_data.physically_based_camera.up[2]);
    glm::fvec3 right = glm::fvec3(this->m_global_ubo_data.physically_based_camera.right[0],
                                  this->m_global_ubo_data.physically_based_camera.right[1],
                                  this->m_global_ubo_data.physically_based_camera.right[2]);

    YRendererFrontendManager::instance()->rotateCameraOnSphere(position,
                                                               target,
                                                               up,
                                                               right,
                                                               rotation);

    glm::fvec3 forward = glm::normalize(target - position);

    yVec3ToC(position, this->m_global_ubo_data.physically_based_camera.position);
    yVec3ToC(forward, this->m_global_ubo_data.physically_based_camera.forward);
    yVec3ToC(up, this->m_global_ubo_data.physically_based_camera.up);
    yVec3ToC(right, this->m_global_ubo_data.physically_based_camera.right);

    //
    this->m_global_ubo_data.samples = 0;

    this->gapiRotatePhysicallyBasedCamera();
}