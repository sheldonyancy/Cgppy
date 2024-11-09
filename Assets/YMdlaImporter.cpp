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

#include "YMdlaImporter.hpp"
#include "YSceneManager.hpp"
#include "YEntity.hpp"
#include "YMeshComponent.hpp"
#include "YLightComponent.hpp"
#include "YMaterialComponent.hpp"
#include "YGlobalInterface.hpp"
#include "YEvent.hpp"
#include "YEventHandlerManager.hpp"
#include "YRendererFrontendManager.hpp"
#include "YCamera.hpp"
#include "YLogger.h"
#include "YMath.h"


void YMdlaImporter::import(const std::string& file_path) {
    std::string thread_id = YGlobalInterface::instance()->getCurrentThreadId();
    YINFO("Parse stl file begin, thread id: %s", thread_id.c_str());

    glm::fvec4 kd_white = glm::fvec4(0.85f, 0.85f, 0.85f, 1.0f);
    glm::fvec4 kd_green = glm::fvec4(0.1f, 0.85f, 0.1f, 1.0f);
    glm::fvec4 kd_red = glm::fvec4(0.85f, 0.1f, 0.1f, 1.0f);

    YsEntity* entity_floor = YSceneManager::instance()->createEntity();
    {
        //
        YsMeshComponent* mesh_floor = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_floor->id);

        glm::fvec4 floor_p1(552.8f, 0.0f, 0.0f, 1.0f);
        glm::fvec4 floor_p2(0.0f, 0.0f,   0.0f, 1.0f);
        glm::fvec4 floor_p3(0.0f, 0.0f, 559.2f, 1.0f);
        glm::fvec4 floor_p4(549.6f, 0.0f, 559.2f, 1.0f);

        mesh_floor->positions.push_back(floor_p1);
        mesh_floor->positions.push_back(floor_p2);
        mesh_floor->positions.push_back(floor_p3);

        mesh_floor->positions.push_back(floor_p1);
        mesh_floor->positions.push_back(floor_p3);
        mesh_floor->positions.push_back(floor_p4);

        glm::fvec4 normal = yCalculatePlaneNormal(floor_p1, floor_p2, floor_p3);

        for(int i = 0; i < 6; ++i) {
            mesh_floor->normals.push_back(normal);
        }

        mesh_floor->aabb.min = glm::fvec3(0.0f, 0.0f, 0.0f);
        mesh_floor->aabb.max = glm::fvec3(549.6f, 0.0f, 559.2f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_floor->id);
        material->albedo = glm::fvec4(1.0f);
        material->kd = kd_white;
    }

    YsEntity* entity_light = YSceneManager::instance()->createEntity();
    {
        //
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_light->id);

        glm::fvec4 light_p1(343.0f, 547.8f, 227.0f, 1.0f);
        glm::fvec4 light_p2(343.0f, 547.8f, 332.0f, 1.0f);
        glm::fvec4 light_p3(213.0f, 547.8f, 332.0f, 1.0f);
        glm::fvec4 light_p4(213.0f, 547.8f, 227.0f, 1.0f);

        mesh->positions.push_back(light_p1);
        mesh->positions.push_back(light_p2);
        mesh->positions.push_back(light_p3);

        mesh->positions.push_back(light_p1);
        mesh->positions.push_back(light_p3);
        mesh->positions.push_back(light_p4);

        glm::fvec4 normal = yCalculatePlaneNormal(light_p1, light_p2, light_p3);

        for(int i = 0; i < 6; ++i) {
            mesh->normals.push_back(normal);
        }

        mesh->aabb.min = glm::fvec3(213.0f, 547.8f, 227.0f);
        mesh->aabb.max = glm::fvec3(343.0f, 547.8f, 332.0f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_light->id);
        material->albedo = glm::fvec4(1.0f, 1.0f, 1.0f, 0.0f);
        material->le = glm::vec3(30, 25, 15);

        //
        YsAreaLightComponent* light = YSceneManager::instance()->createComponent<YsAreaLightComponent>(entity_light->id);
        light->center = glm::fvec3((213.0f + 343.0f) / 2.0f, 547.8f, (227.0f + 332.0f) / 2.0f);
        light->p0 = light_p1;
        light->p1 = light_p2;
        light->p2 = light_p3;
        light->p3 = light_p4;
        light->le = glm::vec3(30, 25, 15);
        light->light_space_target = glm::fvec3(light->center.x, 0.0f, light->center.z);
        light->light_space_up = glm::fvec3(0.0f, 0.0f, 1.0f);
    }

    YsEntity* entity_ceiling = YSceneManager::instance()->createEntity();
    {
        //
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_ceiling->id);

        glm::fvec4 ceiling_p1(556.0f, 548.8f, 0.0f, 1.0f);
        glm::fvec4 ceiling_p2(556.0f, 548.8f, 559.2f, 1.0f);
        glm::fvec4 ceiling_p3(0.0f, 548.8f, 559.2f, 1.0f);
        glm::fvec4 ceiling_p4(0.0f, 548.8f, 0.0f, 1.0f);

        mesh->positions.push_back(ceiling_p1);
        mesh->positions.push_back(ceiling_p2);
        mesh->positions.push_back(ceiling_p3);

        mesh->positions.push_back(ceiling_p1);
        mesh->positions.push_back(ceiling_p3);
        mesh->positions.push_back(ceiling_p4);

        glm::fvec4 normal = yCalculatePlaneNormal(ceiling_p1, ceiling_p2, ceiling_p3);

        for(int i = 0; i < 6; ++i) {
            mesh->normals.push_back(normal);
        }

        mesh->aabb.min = glm::fvec3(0.0f, 548.8f, 0.0f);
        mesh->aabb.max = glm::fvec3(556.0f, 548.8f, 559.2f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_ceiling->id);
        material->albedo = glm::fvec4(1.0f);
        material->kd = kd_white;
    }

    YsEntity* entity_back_wall = YSceneManager::instance()->createEntity();
    {
        //
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_back_wall->id);

        glm::fvec4 back_wall_p1(549.6f, 0.0f, 559.2f, 1.0f);
        glm::fvec4 back_wall_p2(0.0f, 0.0f, 559.2f, 1.0f);
        glm::fvec4 back_wall_p3(0.0f, 548.8f, 559.2f, 1.0f);
        glm::fvec4 back_wall_p4(556.0f, 548.8f, 559.2f, 1.0f);

        mesh->positions.push_back(back_wall_p1);
        mesh->positions.push_back(back_wall_p2);
        mesh->positions.push_back(back_wall_p3);

        mesh->positions.push_back(back_wall_p1);
        mesh->positions.push_back(back_wall_p3);
        mesh->positions.push_back(back_wall_p4);

        glm::fvec4 normal = yCalculatePlaneNormal(back_wall_p1, back_wall_p2, back_wall_p3);

        for(int i = 0; i < 6; ++i) {
            mesh->normals.push_back(normal);
        }

        mesh->aabb.min = glm::fvec3(0.0f, 0.0f, 559.2f);
        mesh->aabb.max = glm::fvec3(556.0f, 548.8f, 559.2f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_back_wall->id);
        material->albedo = glm::fvec4(1.0f);
        material->kd = kd_white;
    }

    YsEntity* entity_right_wall = YSceneManager::instance()->createEntity();
    {
        //
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_right_wall->id);

        glm::fvec4 right_wall_p1(0.0f, 0.0f, 559.2f, 1.0f);
        glm::fvec4 right_wall_p2(0.0f, 0.0f, 0.0f, 1.0f);
        glm::fvec4 right_wall_p3(0.0f, 548.8f, 0.0f, 1.0f);
        glm::fvec4 right_wall_p4(0.0f, 548.8f, 559.2f, 1.0f);

        mesh->positions.push_back(right_wall_p1);
        mesh->positions.push_back(right_wall_p2);
        mesh->positions.push_back(right_wall_p3);

        mesh->positions.push_back(right_wall_p1);
        mesh->positions.push_back(right_wall_p3);
        mesh->positions.push_back(right_wall_p4);

        glm::fvec4 normal = yCalculatePlaneNormal(right_wall_p1, right_wall_p2, right_wall_p3);

        for(int i = 0; i < 6; ++i) {
            mesh->normals.push_back(normal);
        }

        mesh->aabb.min = glm::fvec3(0.0f, 0.0f, 0.0f);
        mesh->aabb.max = glm::fvec3(0.0f, 548.8f, 559.2f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_right_wall->id);
        material->albedo = glm::fvec4(0.0f, 1.0f, 0.0f, 1.0f);
        material->kd = kd_green;
    }

    YsEntity* entity_left_wall = YSceneManager::instance()->createEntity();
    {
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_left_wall->id);

        glm::fvec4 left_wall_p1(552.8f, 0.0f, 0.0f, 1.0f);
        glm::fvec4 left_wall_p2(549.6f, 0.0f, 559.2f, 1.0f);
        glm::fvec4 left_wall_p3(556.0f, 548.8f, 559.2f, 1.0f);
        glm::fvec4 left_wall_p4(556.0f, 548.8f, 0.0f, 1.0f);

        mesh->positions.push_back(left_wall_p1);
        mesh->positions.push_back(left_wall_p2);
        mesh->positions.push_back(left_wall_p3);

        mesh->positions.push_back(left_wall_p1);
        mesh->positions.push_back(left_wall_p3);
        mesh->positions.push_back(left_wall_p4);

        glm::fvec4 normal = yCalculatePlaneNormal(left_wall_p1, left_wall_p2, left_wall_p3);

        for(int i = 0; i < 6; ++i) {
            mesh->normals.push_back(normal);
        }

        mesh->aabb.min = glm::fvec3(549.6f, 0.0f, 0.0f);
        mesh->aabb.max = glm::fvec3(556.0f, 548.8f, 559.2f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_left_wall->id);
        material->albedo = glm::fvec4(1.0f, 0.0f, 0.0f, 1.0f);
        material->kd = kd_red;
    }

    YsEntity* entity_short_block = YSceneManager::instance()->createEntity();
    {
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_short_block->id);
        {
            glm::fvec4 short_block_f1_p1(130.0f, 165.0f, 65.0f, 1.0f);
            glm::fvec4 short_block_f1_p2(82.0f, 165.0f, 225.0f, 1.0f);
            glm::fvec4 short_block_f1_p3(240.0f, 165.0f, 272.0f, 1.0f);
            glm::fvec4 short_block_f1_p4(290.0f, 165.0f, 114.0f, 1.0f);

            mesh->positions.push_back(short_block_f1_p1);
            mesh->positions.push_back(short_block_f1_p2);
            mesh->positions.push_back(short_block_f1_p3);

            mesh->positions.push_back(short_block_f1_p1);
            mesh->positions.push_back(short_block_f1_p3);
            mesh->positions.push_back(short_block_f1_p4);

            glm::fvec4 n = yCalculatePlaneNormal(short_block_f1_p1, short_block_f1_p2, short_block_f1_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(n);
            }
        }
        {
            glm::fvec4 short_block_f2_p1(290.0f, 0.0f, 114.0f, 1.0f);
            glm::fvec4 short_block_f2_p2(290.0f, 165.0f, 114.0f, 1.0f);
            glm::fvec4 short_block_f2_p3(240.0f, 165.0f, 272.0f, 1.0f);
            glm::fvec4 short_block_f2_p4(240.0f, 0.0f, 272.0f, 1.0f);

            mesh->positions.push_back(short_block_f2_p1);
            mesh->positions.push_back(short_block_f2_p2);
            mesh->positions.push_back(short_block_f2_p3);

            mesh->positions.push_back(short_block_f2_p1);
            mesh->positions.push_back(short_block_f2_p3);
            mesh->positions.push_back(short_block_f2_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(short_block_f2_p1, short_block_f2_p2, short_block_f2_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 short_block_f3_p1(130.0f, 0.0f, 65.0f, 1.0f);
            glm::fvec4 short_block_f3_p2(130.0f, 165.0f, 65.0f, 1.0f);
            glm::fvec4 short_block_f3_p3(290.0f, 165.0f, 114.0f, 1.0f);
            glm::fvec4 short_block_f3_p4(290.0f, 0.0f, 114.0f, 1.0f);

            mesh->positions.push_back(short_block_f3_p1);
            mesh->positions.push_back(short_block_f3_p2);
            mesh->positions.push_back(short_block_f3_p3);

            mesh->positions.push_back(short_block_f3_p1);
            mesh->positions.push_back(short_block_f3_p3);
            mesh->positions.push_back(short_block_f3_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(short_block_f3_p1, short_block_f3_p2, short_block_f3_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 short_block_f4_p1(82.0f, 0.0f, 225.0f, 1.0f);
            glm::fvec4 short_block_f4_p2(82.0f, 165.0f, 225.0f, 1.0f);
            glm::fvec4 short_block_f4_p3(130.0f, 165.0f, 65.0f, 1.0f);
            glm::fvec4 short_block_f4_p4(130.0f, 0.0f, 65.0f, 1.0f);

            mesh->positions.push_back(short_block_f4_p1);
            mesh->positions.push_back(short_block_f4_p2);
            mesh->positions.push_back(short_block_f4_p3);

            mesh->positions.push_back(short_block_f4_p1);
            mesh->positions.push_back(short_block_f4_p3);
            mesh->positions.push_back(short_block_f4_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(short_block_f4_p1, short_block_f4_p2, short_block_f4_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 short_block_f5_p1(240.0f, 0.0f, 272.0f, 1.0f);
            glm::fvec4 short_block_f5_p2(240.0f, 165.0f, 272.0f, 1.0f);
            glm::fvec4 short_block_f5_p3(82.0f, 165.0f, 225.0f, 1.0f);
            glm::fvec4 short_block_f5_p4(82.0f, 0.0f, 225.0f, 1.0f);

            mesh->positions.push_back(short_block_f5_p1);
            mesh->positions.push_back(short_block_f5_p2);
            mesh->positions.push_back(short_block_f5_p3);

            mesh->positions.push_back(short_block_f5_p1);
            mesh->positions.push_back(short_block_f5_p3);
            mesh->positions.push_back(short_block_f5_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(short_block_f5_p1, short_block_f5_p2, short_block_f5_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        mesh->aabb.min = glm::fvec3(82.0f, 0.0f, 65.0f);
        mesh->aabb.max = glm::fvec3(290.0f, 165.0f, 272.0f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_short_block->id);
        material->albedo = glm::fvec4(1.0f);
        material->kd = kd_white;
    }

    YsEntity* entity_tall_block = YSceneManager::instance()->createEntity();
    {
        YsMeshComponent* mesh = YSceneManager::instance()->createComponent<YsMeshComponent>(entity_tall_block->id);
        {
            glm::fvec4 tall_block_f1_p1(423.0f, 330.0f, 247.0f, 1.0f);
            glm::fvec4 tall_block_f1_p2(265.0f, 330.0f, 296.0f, 1.0f);
            glm::fvec4 tall_block_f1_p3(314.0f, 330.0f, 456.0f, 1.0f);
            glm::fvec4 tall_block_f1_p4(472.0f, 330.0f, 406.0f, 1.0f);

            mesh->positions.push_back(tall_block_f1_p1);
            mesh->positions.push_back(tall_block_f1_p2);
            mesh->positions.push_back(tall_block_f1_p3);

            mesh->positions.push_back(tall_block_f1_p1);
            mesh->positions.push_back(tall_block_f1_p3);
            mesh->positions.push_back(tall_block_f1_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(tall_block_f1_p1, tall_block_f1_p2, tall_block_f1_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 tall_block_f2_p1(423.0f, 0.0f, 247.0f, 1.0f);
            glm::fvec4 tall_block_f2_p2(423.0f, 330.0f, 247.0f, 1.0f);
            glm::fvec4 tall_block_f2_p3(472.0f, 330.0f, 406.0f, 1.0f);
            glm::fvec4 tall_block_f2_p4(472.0f, 0.0f, 406.0f, 1.0f);

            mesh->positions.push_back(tall_block_f2_p1);
            mesh->positions.push_back(tall_block_f2_p2);
            mesh->positions.push_back(tall_block_f2_p3);

            mesh->positions.push_back(tall_block_f2_p1);
            mesh->positions.push_back(tall_block_f2_p3);
            mesh->positions.push_back(tall_block_f2_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(tall_block_f2_p1, tall_block_f2_p2, tall_block_f2_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 tall_block_f3_p1(472.0f, 0.0f, 406.0f, 1.0f);
            glm::fvec4 tall_block_f3_p2(472.0f, 330.0f, 406.0f, 1.0f);
            glm::fvec4 tall_block_f3_p3(314.0f, 330.0f, 456.0f, 1.0f);
            glm::fvec4 tall_block_f3_p4(314.0f, 0.0f, 456.0f, 1.0f);

            mesh->positions.push_back(tall_block_f3_p1);
            mesh->positions.push_back(tall_block_f3_p2);
            mesh->positions.push_back(tall_block_f3_p3);

            mesh->positions.push_back(tall_block_f3_p1);
            mesh->positions.push_back(tall_block_f3_p3);
            mesh->positions.push_back(tall_block_f3_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(tall_block_f3_p1, tall_block_f3_p2, tall_block_f3_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 tall_block_f4_p1(314.0f, 0.0f, 456.0f, 1.0f);
            glm::fvec4 tall_block_f4_p2(314.0f, 330.0f, 456.0f, 1.0f);
            glm::fvec4 tall_block_f4_p3(265.0f, 330.0f, 296.0f, 1.0f);
            glm::fvec4 tall_block_f4_p4(265.0f, 0.0f, 296.0f, 1.0f);

            mesh->positions.push_back(tall_block_f4_p1);
            mesh->positions.push_back(tall_block_f4_p2);
            mesh->positions.push_back(tall_block_f4_p3);

            mesh->positions.push_back(tall_block_f4_p1);
            mesh->positions.push_back(tall_block_f4_p3);
            mesh->positions.push_back(tall_block_f4_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(tall_block_f4_p1, tall_block_f4_p2, tall_block_f4_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        {
            glm::fvec4 tall_block_f5_p1(265.0f, 0.0f, 296.0f, 1.0f);
            glm::fvec4 tall_block_f5_p2(265.0f, 330.0f, 296.0f, 1.0f);
            glm::fvec4 tall_block_f5_p3(423.0f, 330.0f, 247.0f, 1.0f);
            glm::fvec4 tall_block_f5_p4(423.0f, 0.0f, 247.0f, 1.0f);

            mesh->positions.push_back(tall_block_f5_p1);
            mesh->positions.push_back(tall_block_f5_p2);
            mesh->positions.push_back(tall_block_f5_p3);

            mesh->positions.push_back(tall_block_f5_p1);
            mesh->positions.push_back(tall_block_f5_p3);
            mesh->positions.push_back(tall_block_f5_p4);

            glm::fvec4 normal = yCalculatePlaneNormal(tall_block_f5_p1, tall_block_f5_p2, tall_block_f5_p3);

            for(int i = 0; i < 6; ++i) {
                mesh->normals.push_back(normal);
            }
        }
        mesh->aabb.min = glm::fvec3(265.0f, 0.0f, 247.0f);
        mesh->aabb.min = glm::fvec3(472.0f, 330.0f, 456.0f);

        //
        YsMaterialComponent* material = YSceneManager::instance()->createComponent<YsMaterialComponent>(entity_tall_block->id);
        material->albedo = glm::fvec4(1.0f);
        material->kd = kd_white;
    }

    //
    YSceneManager::instance()->updateSceneInfo();

    YsUpdateSceneEvent update_scene_event;
    YEventHandlerManager::instance()->pushEvent(update_scene_event);

    YINFO("Parse stl file end, thread id: %s", thread_id.c_str());
}