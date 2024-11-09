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

#include "YMaterialSystem.hpp"
#include "YSceneManager.hpp"
#include "YMaterialComponent.hpp"


YMaterialSystem* YMaterialSystem::instance() {
    static YMaterialSystem material_sysytem;
    return &material_sysytem;
}

YMaterialSystem::YMaterialSystem() {


}

YMaterialSystem::~YMaterialSystem() {

}

void YMaterialSystem::updagteMaterial() {
    std::vector<YsMaterialComponent*> material_components = YSceneManager::instance()->getComponents<YsMaterialComponent>();
    for(auto mc : material_components) {
        GLSL_Material glsl_material;
        glsl_material.albedo[0] = mc->albedo.x;
        glsl_material.albedo[1] = mc->albedo.y;
        glsl_material.albedo[2] = mc->albedo.z;
        glsl_material.albedo[3] = mc->albedo.w;
        glsl_material.brdf_type = mc->brdf_type;
        glsl_material.kd[0] = mc->kd.x;
        glsl_material.kd[1] = mc->kd.y;
        glsl_material.kd[2] = mc->kd.z;
        glsl_material.le[0] = mc->le.x;
        glsl_material.le[1] = mc->le.y;
        glsl_material.le[2] = mc->le.z;

        this->m_material_index_map.insert(std::make_pair(mc, this->m_material_struct_buffer.size()));
        this->m_material_struct_buffer.push_back(glsl_material);
    }
}
