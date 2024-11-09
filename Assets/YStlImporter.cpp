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

#include "YStlImporter.hpp"
#include "YSceneManager.hpp"
#include "YEntity.hpp"
#include "YMeshComponent.hpp"
#include "YMaterialComponent.hpp"
#include "YLightComponent.hpp"
#include "YGlobalInterface.hpp"
#include "YEvent.hpp"
#include "YEventHandlerManager.hpp"
#include "YRendererFrontendManager.hpp"
#include "YCamera.hpp"
#include "YLogger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



void YStlImporter::import(const std::string& file_path) {

    std::string thread_id = YGlobalInterface::instance()->getCurrentThreadId();
    YINFO("Parse stl file begin, thread id: %s", thread_id.c_str());

    YsEntity* entity = YSceneManager::instance()->createEntity();
    YsMaterialComponent* material_component = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity->id);
    material_component->albedo = glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f);

    YsMeshComponent* mesh_component = YSceneManager::instance()->createComponent<YsMeshComponent>(entity->id);

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(file_path,
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error: " << importer.GetErrorString() << std::endl;
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[i];

        std::cout << "Mesh " << i << ": " << std::endl;

        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            const aiFace& face = mesh->mFaces[j];

            if (face.mNumIndices == 3) {

                for (unsigned int k = 0; k < 3; k++) {
                    const aiVector3D& vertex = mesh->mVertices[face.mIndices[k]];
                    mesh_component->positions.push_back(glm::fvec4(vertex.x, vertex.y, vertex.z, 1.0f));
                    const aiVector3D& normal = mesh->mNormals[face.mIndices[k]];
                    mesh_component->normals.push_back(glm::fvec4(normal.x, normal.y, normal.z, 1.0f));

                    mesh_component->aabb.min.x = vertex.x < mesh_component->aabb.min.x ? vertex.x : mesh_component->aabb.min.x;
                    mesh_component->aabb.max.x = vertex.x > mesh_component->aabb.max.x ? vertex.x : mesh_component->aabb.max.x;
                    mesh_component->aabb.min.y = vertex.y < mesh_component->aabb.min.y ? vertex.y : mesh_component->aabb.min.y;
                    mesh_component->aabb.max.y = vertex.y > mesh_component->aabb.max.y ? vertex.y : mesh_component->aabb.max.y;
                    mesh_component->aabb.min.z = vertex.z < mesh_component->aabb.min.z ? vertex.z : mesh_component->aabb.min.z;
                    mesh_component->aabb.max.z = vertex.z > mesh_component->aabb.max.z ? vertex.z : mesh_component->aabb.max.z;
                }
            }
        }
    }

    YSceneManager::instance()->updateSceneInfo();

    YsUpdateSceneEvent update_scene_event;

    YEventHandlerManager::instance()->pushEvent(update_scene_event);

    YINFO("Parse stl file end, thread id: %s", thread_id.c_str());
}
