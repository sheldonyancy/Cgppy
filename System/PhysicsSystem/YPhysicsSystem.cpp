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

#include "YPhysicsSystem.hpp"
#include "YAABBComponent.hpp"
#include "YBVHNodeComponent.hpp"
#include "YMeshComponent.hpp"
#include "YMaterialComponent.hpp"
#include "YSceneManager.hpp"
#include "YMaterialSystem.hpp"
#include "YRendererBackendManager.hpp"
#include "YVulkanBackend.hpp"
#include "YMath.h"


YPhysicsSystem* YPhysicsSystem::instance() {
    static YPhysicsSystem bvh_manager;
    return &bvh_manager;
}

YPhysicsSystem::YPhysicsSystem() {

}

YPhysicsSystem::~YPhysicsSystem() {

}

void YPhysicsSystem::buildBVH() {
    std::vector<YsMeshComponent*> mesh_components = YSceneManager::instance()->getComponents<YsMeshComponent>();
    this->m_root_bvh_node = std::make_unique<YsBVHNodeComponent>();
    this->recursiveCreateBVH(mesh_components, this->m_root_bvh_node.get(), 16);
    this->recursiveFillingBVHStructureBuffer(this->m_root_bvh_node.get());

    int x = 0;
}

void YPhysicsSystem::recursiveCreateBVH(const std::vector<YsMeshComponent*>& meshes, YsBVHNodeComponent* node, int max_depth, int current_depth) {
    node->aabb = this->computeAABB(meshes);

    if(current_depth > max_depth) {
        this->fillingVertexData(meshes, node);
        return;
    }

    if(1 == meshes.size()) {
        this->fillingVertexData(meshes, node);
        return;
    }

    std::vector<YsMeshComponent*> left_meshes;
    std::vector<YsMeshComponent*> right_meshes;

    switch(node->aabb.longestAxis()) {
        case YsAABBComponent::YeAxis::X_AXIS: {
            for(auto mesh : meshes) {
                if(mesh->aabb.center().x > node->aabb.center().x) {
                    right_meshes.push_back(mesh);
                }
                else {
                    left_meshes.push_back(mesh);
                }
            }
            break;
        }
        case YsAABBComponent::YeAxis::Y_AXIS: {
            for(auto mesh : meshes) {
                if(mesh->aabb.center().y > node->aabb.center().y) {
                    right_meshes.push_back(mesh);
                }
                else {
                    left_meshes.push_back(mesh);
                }
            }
            break;
        }
        case YsAABBComponent::YeAxis::Z_AXIS: {
            for(auto mesh : meshes) {
                if(mesh->aabb.center().z > node->aabb.center().z) {
                    right_meshes.push_back(mesh);
                }
                else {
                    left_meshes.push_back(mesh);
                }
            }
            break;
        }
    }

    if(!left_meshes.empty()) {
        node->left = std::make_unique<YsBVHNodeComponent>();
        this->recursiveCreateBVH(left_meshes, node->left.get(), ++current_depth, max_depth);
    }

    if(!right_meshes.empty()) {
        node->right = std::make_unique<YsBVHNodeComponent>();
        this->recursiveCreateBVH(right_meshes, node->right.get(), ++current_depth, max_depth);
    }

    if(node->isLeaf()) {
        this->fillingVertexData(meshes, node);
        return;
    }
}

void YPhysicsSystem::fillingVertexData(const std::vector<YsMeshComponent*>& meshes, YsBVHNodeComponent* node) {
    node->meshes = meshes;
    for(auto mesh : meshes) {
        this->m_mesh_vertex_index_map.insert(std::make_pair(mesh, YRendererBackendManager::instance()->backend()->vertexCount()));
        YRendererBackendManager::instance()->backend()->pushBackVertexPositions(mesh->positions.begin(), mesh->positions.end());
        YRendererBackendManager::instance()->backend()->pushBackVertexNormals(mesh->normals.begin(), mesh->normals.end());

        YsEntity* entity = YSceneManager::instance()->getEntity(mesh);
        YsMaterialComponent* material = entity->getComponent<YsMaterialComponent>();
        int material_id = YMaterialSystem::instance()->materialId(material);
        std::vector<i32> vertex_material_id(mesh->positions.size(), material_id);
        std::vector<i32> vertex_entity_id(mesh->positions.size(), entity->id);
        YRendererBackendManager::instance()->backend()->pushBackVertexMaterialId(vertex_material_id.begin(), vertex_material_id.end());
        YRendererBackendManager::instance()->backend()->pushBackVertexEntitylId(vertex_entity_id.begin(), vertex_entity_id.end());
    }
}

YsAABBComponent YPhysicsSystem::computeAABB(const std::vector<YsMeshComponent*>& meshes) {
    YsAABBComponent aabb;
    for(auto mesh : meshes) {
        aabb.max.x = fmax(aabb.max.x, mesh->aabb.max.x);
        aabb.max.y = fmax(aabb.max.y, mesh->aabb.max.y);
        aabb.max.z = fmax(aabb.max.z, mesh->aabb.max.z);

        aabb.min.x = fmin(aabb.min.x, mesh->aabb.min.x);
        aabb.min.y = fmin(aabb.min.y, mesh->aabb.min.y);
        aabb.min.z = fmin(aabb.min.z, mesh->aabb.min.z);
    }

    if(aabb.min.x == aabb.max.x) {
        aabb.min.x -= 1.0f;
        aabb.max.x += 1.0f;
    }
    if(aabb.min.y == aabb.max.y) {
        aabb.min.y -= 1.0f;
        aabb.max.y += 1.0f;
    }
    if(aabb.min.z == aabb.max.z) {
        aabb.min.z -= 1.0f;
        aabb.max.z += 1.0f;
    }

    return aabb;
}

void YPhysicsSystem::recursiveFillingBVHStructureBuffer(YsBVHNodeComponent* node) {
    GLSL_BVHNode glsl_bvh_node = {};
    glsl_bvh_node.right_node_index = -1;
    glsl_bvh_node.left_node_index = -1;
    glsl_bvh_node.vertex_index = -1;
    this->m_bvh_structure_buffer.emplace_back(glsl_bvh_node);
    u32 buffer_index = this->m_bvh_structure_buffer.size() - 1;

    yVec3ToC(node->aabb.min, this->m_bvh_structure_buffer[buffer_index].aabb.min);
    yVec3ToC(node->aabb.max, this->m_bvh_structure_buffer[buffer_index].aabb.max);

    if(nullptr != node->left) {
        this->m_bvh_structure_buffer[buffer_index].left_node_index = this->m_bvh_structure_buffer.size();
        this->recursiveFillingBVHStructureBuffer(node->left.get());
    } else {
        this->m_bvh_structure_buffer[buffer_index].left_node_index = -1;
    }

    if(nullptr != node->right) {
        this->m_bvh_structure_buffer[buffer_index].right_node_index = this->m_bvh_structure_buffer.size();
        this->recursiveFillingBVHStructureBuffer(node->right.get());
    } else {
        this->m_bvh_structure_buffer[buffer_index].right_node_index = -1;
    }

    if(node->isLeaf()) {
        this->m_bvh_structure_buffer[buffer_index].vertex_index = this->m_mesh_vertex_index_map.find(node->meshes.front())->second;
        for(auto mesh : node->meshes) {
            this->m_bvh_structure_buffer[buffer_index].vertex_count += mesh->positions.size();
        }
    } else {
        this->m_bvh_structure_buffer[buffer_index].vertex_count = 0;
    }
}