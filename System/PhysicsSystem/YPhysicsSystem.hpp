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

#ifndef CGPPY_YPHYSICSSYSTEM_HPP
#define CGPPY_YPHYSICSSYSTEM_HPP


#include "YDefines.h"
#include "YVulkanTypes.h"

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <vector>
#include <map>

struct YsMeshComponent;
struct YsBVHNodeComponent;
struct YsAABBComponent;


class YPhysicsSystem {
public:
    enum class YePartitioningAlgorithm : unsigned char {
        SAH,
        MedianSplit
    };

    static YPhysicsSystem* instance();

    void buildBVH();

    inline i32 bvhNodeCount() {return this->m_bvh_structure_buffer.size();}
    inline void* bvhData() {return &this->m_bvh_structure_buffer.front();}

private:
    YPhysicsSystem();
    ~YPhysicsSystem();

    void recursiveCreateBVH(const std::vector<YsMeshComponent*>& meshes, YsBVHNodeComponent* node, int max_depth, int current_depth = 1);

    void fillingVertexData(const std::vector<YsMeshComponent*>& meshes, YsBVHNodeComponent* node);

    void recursiveFillingBVHStructureBuffer(YsBVHNodeComponent* node);

    YsAABBComponent computeAABB(const std::vector<YsMeshComponent*>& meshes);

private:
    //
    std::map<YsMeshComponent*, u32> m_mesh_vertex_index_map;

    std::unique_ptr<YsBVHNodeComponent> m_root_bvh_node;

    std::vector<GLSL_BVHNode> m_bvh_structure_buffer;
};


#endif //CGPPY_YPHYSICSSYSTEM_HPP
