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
#include "YAABBComponent.hpp"
#include "YBVHNodeComponent.hpp"
#include "YMeshComponent.hpp"
#include "YMaterialComponent.hpp"
#include "YRendererFrontendManager.hpp"
#include "YRendererBackendManager.hpp"
#include "YCamera.hpp"
#include "YMath.h"
#include "YCMemoryManager.h"
#include "GLFW/glfw3.h"
#include "YLogger.h"
#include "YProfiler.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


YRendererBackend::YRendererBackend()
    : m_init_finished(false),
      m_need_draw(false),
      m_need_update_device_vertex_input(false),
      m_need_update_device_ssbo(false),
      m_need_update_device_ubo(false){

}

YRendererBackend::~YRendererBackend() {

}

void YRendererBackend::updateHostVertexInput() {
    std::vector<YsMeshComponent*> meshes = YSceneManager::instance()->getComponents<YsMeshComponent>();
    for(auto mesh : meshes) {
        YsEntity* entity = YSceneManager::instance()->getEntity(mesh);
        YsMaterialComponent* material = entity->getComponent<YsMaterialComponent>();
        int material_id = YMaterialSystem::instance()->materialId(material);
        std::vector<i32> vertex_material_id(mesh->positions.size(), material_id);
        std::vector<i32> vertex_entity_id(mesh->positions.size(), entity->id);

        this->m_vertex_positions.insert(this->m_vertex_positions.end(), mesh->positions.begin(), mesh->positions.end());
        this->m_vertex_normals.insert(this->m_vertex_normals.end(), mesh->normals.begin(), mesh->normals.end());
        this->m_vertex_material_id.insert(this->m_vertex_material_id.end(), vertex_material_id.begin(), vertex_material_id.end());
        this->m_vertex_entity_id.insert(this->m_vertex_entity_id.end(), vertex_entity_id.begin(), vertex_entity_id.end());
    }

    this->m_need_update_device_vertex_input = true;
}

void YRendererBackend::updateHostSsbo() {
    std::vector<GLSL_BVHNode> bvh_buffers;
    this->recursiveFillingBVHBuffer(&bvh_buffers, YPhysicsSystem::instance()->rootBVHNode());

    this->m_ssbo.bvh_node_count = bvh_buffers.size();
    this->m_ssbo.material_count = YMaterialSystem::instance()->materialCount();
    this->m_ssbo.vertex_count = this->m_vertex_positions.size();
    yCMemoryCopy(this->m_ssbo.bvh_node,
                 bvh_buffers.data(),
                 sizeof(GLSL_BVHNode) * this->m_ssbo.bvh_node_count);
    yCMemoryCopy(this->m_ssbo.materials,
                 YMaterialSystem::instance()->materialData(),
                 sizeof(GLSL_Material) * this->m_ssbo.material_count);
    yCMemoryCopy(this->m_ssbo.vertex_position,
                 this->m_vertex_positions.data(),
                 sizeof(vec4) * this->m_ssbo.vertex_count);
    yCMemoryCopy(this->m_ssbo.vertex_normal,
                 this->m_vertex_normals.data(),
                 sizeof(vec4) * this->m_ssbo.vertex_count);
    yCMemoryCopy(this->m_ssbo.vertex_material_id,
                 this->m_vertex_material_id.data(),
                 sizeof(i32) * this->m_ssbo.vertex_count);
    yCMemoryCopy(this->m_ssbo.vertex_entity_id,
                 this->m_vertex_entity_id.data(),
                 sizeof(i32) * this->m_ssbo.vertex_count);

    this->m_need_update_device_ssbo = true;
}

void YRendererBackend::updateHostUbo() {
    //
    this->m_ubo.rendering_model = static_cast<int>(YRendererBackendManager::instance()->getRenderingModel());
    this->m_ubo.path_tracing_spp = YRendererBackendManager::instance()->getPathTracingSpp();
    this->m_ubo.path_tracing_max_depth = YRendererBackendManager::instance()->getPathTracingMaxDepth();
    
    //
    YsAreaLightComponent* light_component = YSceneManager::instance()->getComponents<YsAreaLightComponent>().front();
    glm::fvec3 light_center = YSceneManager::instance()->modelMatrix() * glm::fvec4(light_component->center, 1.0);
    glm::fvec3 light_target = YSceneManager::instance()->modelMatrix() * glm::fvec4(light_component->light_space_target, 1.0);
    glm::fvec3 light_up = glm::normalize((transpose(inverse(YSceneManager::instance()->modelMatrix())) * glm::fvec4(light_component->light_space_up, 1.0)));
    glm::fmat4x4 light_view_matrix = glm::lookAt(light_center, light_target, light_up);
    float fovy = glm::radians(100.0f);
    glm::fvec2 window_size = YRendererFrontendManager::instance()->mainWindowSize();
    float aspect = window_size.x / window_size.y;
    glm::fmat4x4 light_projection_matrix = glm::perspective(fovy, aspect, CAMERA_Z_NEAR, 1.3f);
    light_component->light_space_matrix = light_projection_matrix * light_view_matrix;

    //
    this->m_ubo.rasterization_camera.position = YRendererFrontendManager::instance()->camera()->getPosition();
    this->m_ubo.rasterization_camera.view_matrix = YRendererFrontendManager::instance()->camera()->getViewMatrix();
    this->m_ubo.rasterization_camera.projection_matrix = YRendererFrontendManager::instance()->camera()->getProjectionMatrix(YeRendererBackendApi::VULKAN);

    YsEntity* light_entity = YSceneManager::instance()->getEntity(light_component);
    this->m_ubo.light.entity_id = light_entity->id;
    this->m_ubo.light.center = light_component->center;
    this->m_ubo.light.p0 = light_component->p0;
    this->m_ubo.light.p1 = light_component->p1;
    this->m_ubo.light.p2 = light_component->p2;
    this->m_ubo.light.p3 = light_component->p3;
    this->m_ubo.light.le = light_component->le;
    this->m_ubo.light.space_matrix = light_component->light_space_matrix;

    this->m_ubo.model_matrix = YSceneManager::instance()->modelMatrix();

    this->m_need_update_device_ubo = true;
}

void YRendererBackend::draw() {
    if(!this->m_init_finished) {
        return;
    }

    if(!this->m_need_draw) {
        return;
    }

    if(!this->framePrepare()){
        YERROR("Frame Prepare Error!");
        return;
    }

    auto start_cpu = std::chrono::high_resolution_clock::now();

    if(this->m_need_update_device_vertex_input) {
        this->deviceUpdateVertexInput(this->m_vertex_positions.size(),
                                      this->m_vertex_positions.data(),
                                      this->m_vertex_normals.data(),
                                      this->m_vertex_material_id.data());
        this->m_need_update_device_vertex_input = false;            
    }

    if(this->m_need_update_device_ssbo) {
        this->deviceUpdateSsbo(sizeof(GLSL_SSBO), &this->m_ssbo);
        this->m_need_update_device_ssbo = false;            
    }

    if(this->m_need_update_device_ubo) {
        this->deviceUpdateUbo(&this->m_ubo);
        this->m_need_update_device_ubo = false;            
    }

    this->m_push_constant[this->m_current_frame].current_present_image_index = this->m_current_present_image_index;
    this->m_push_constant[this->m_current_frame].current_frame = this->m_current_frame;
    
    this->frameRun();

    if(!this->framePresent()) {
        YERROR("Frame Present Error!");
        return;
    }

    auto end_cpu = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> duration = end_cpu - start_cpu;
    double cpu_time = duration.count() / 1000.0;
    YProfiler::instance()->accumulateCpuFrameTime(cpu_time);
}

void YRendererBackend::rotatePhysicallyBasedCamera(const glm::fquat& rotation) {
    YRendererFrontendManager::instance()->rotateCameraOnSphere(this->m_ubo.physically_based_camera.position,
                                                               this->m_ubo.physically_based_camera.target,
                                                               this->m_ubo.physically_based_camera.up,
                                                               this->m_ubo.physically_based_camera.right,
                                                               rotation);

    this->m_ubo.physically_based_camera.forward = glm::normalize(this->m_ubo.physically_based_camera.target - this->m_ubo.physically_based_camera.position);
}

void YRendererBackend::recursiveFillingBVHBuffer(std::vector<GLSL_BVHNode>* bvh_buffers, YsBVHNodeComponent* node) {
    GLSL_BVHNode glsl_bvh_node = {};
    glsl_bvh_node.right_node_index = -1;
    glsl_bvh_node.left_node_index = -1;
    glsl_bvh_node.vertex_index = -1;
    bvh_buffers->emplace_back(glsl_bvh_node);
    u32 buffer_index = bvh_buffers->size() - 1;

    bvh_buffers->at(buffer_index).aabb.min = node->aabb.min;
    bvh_buffers->at(buffer_index).aabb.max = node->aabb.max;

    if(nullptr != node->left) {
        bvh_buffers->at(buffer_index).left_node_index = bvh_buffers->size();
        this->recursiveFillingBVHBuffer(bvh_buffers, node->left.get());
    } else {
        bvh_buffers->at(buffer_index).left_node_index = -1;
    }

    if(nullptr != node->right) {
        bvh_buffers->at(buffer_index).right_node_index = bvh_buffers->size();
        this->recursiveFillingBVHBuffer(bvh_buffers, node->right.get());
    } else {
        bvh_buffers->at(buffer_index).right_node_index = -1;
    }

    if(node->isLeaf()) {
        bvh_buffers->at(buffer_index).vertex_index = YPhysicsSystem::instance()->meshVertexIndex(node->meshes.front());
        for(auto mesh : node->meshes) {
            bvh_buffers->at(buffer_index).vertex_count += mesh->positions.size();
        }
    } else {
        bvh_buffers->at(buffer_index).vertex_count = 0;
    }
}