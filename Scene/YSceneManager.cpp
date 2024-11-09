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

#include "YSceneManager.hpp"
#include "YMeshComponent.hpp"
#include "YRendererFrontendManager.hpp"
#include "YRendererBackendManager.hpp"
#include "YCamera.hpp"
#include "YBVHNodeComponent.hpp"
#include "YLightComponent.hpp"
#include "YLogger.h"


YSceneManager* YSceneManager::instance() {
    static YSceneManager scene;

    return &scene;
}

YSceneManager::YSceneManager() {
    YRendererFrontendManager::instance()->camera()->setProjectionType(YeProjectionType::PERSPECTIVE);
    YRendererFrontendManager::instance()->camera()->updateProjectionMatrix();
    YRendererFrontendManager::instance()->camera()->setPosition(glm::fvec3(0.0f, 0.0f, -2.0f));
    YRendererFrontendManager::instance()->camera()->setUpDirection(glm::fvec3(0.0f, 1.0f, 0.0f));

    this->m_model_matrix = glm::fmat4x4(1.0f);
}

YSceneManager::~YSceneManager() {

}

void YSceneManager::updateSceneInfo() {
    //
    this->m_scene_bound.reset();

    std::vector<YsMeshComponent*> mesh_components = this->getComponents<YsMeshComponent>();

    for(auto mesh : mesh_components) {
        //
        this->m_scene_bound.min.x = mesh->aabb.min.x < this->m_scene_bound.min.x ? mesh->aabb.min.x : this->m_scene_bound.min.x;
        this->m_scene_bound.max.x = mesh->aabb.max.x > this->m_scene_bound.max.x ? mesh->aabb.max.x : this->m_scene_bound.max.x;
        this->m_scene_bound.min.y = mesh->aabb.min.y < this->m_scene_bound.min.y ? mesh->aabb.min.y : this->m_scene_bound.min.y;
        this->m_scene_bound.max.y = mesh->aabb.max.y > this->m_scene_bound.max.y ? mesh->aabb.max.y : this->m_scene_bound.max.y;
        this->m_scene_bound.min.z = mesh->aabb.min.z < this->m_scene_bound.min.z ? mesh->aabb.min.z : this->m_scene_bound.min.z;
        this->m_scene_bound.max.z = mesh->aabb.max.z > this->m_scene_bound.max.z ? mesh->aabb.max.z : this->m_scene_bound.max.z;
    }

    this->m_scene_center = glm::fvec3(this->m_scene_bound.min.x + this->m_scene_bound.max.x,
                                      this->m_scene_bound.min.y + this->m_scene_bound.max.y,
                                      this->m_scene_bound.min.z + this->m_scene_bound.max.z) * glm::fvec3(0.5);

    float sphere_radius = this->m_scene_bound.boundingSphereRadius();

    //
    glm::fmat4x4 scale_matrix = glm::scale(glm::fmat4x4 (1.0f), glm::fvec3(1.0f / sphere_radius) * glm::fvec3(1.0f, -1.0f, 1.0f));
    glm::fmat4x4 translate_matrix = glm::translate(glm::fmat4x4 (1.0f), -this->m_scene_center);
    this->m_model_matrix = scale_matrix * translate_matrix;
}

YsEntity* YSceneManager::createEntity() {
    auto entity = std::make_unique<YsEntity>(this->m_entities.size());
    YsEntity* ptr = entity.get();
    this->m_entities.push_back(std::move(entity));
    return ptr;
}

YsEntity* YSceneManager::getEntity(unsigned int id) {
    if (id >= this->m_entities.size()){
        return nullptr;
    }

    return this->m_entities[id].get();
}

void YSceneManager::applyRotation(const glm::mat4& rotation_matrix) {
    this->m_model_matrix = rotation_matrix * this->m_model_matrix;
}