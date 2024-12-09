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

#ifndef CGPPY_YMATERIALSYSTEM_HPP
#define CGPPY_YMATERIALSYSTEM_HPP


#include "YDefines.h"
#include "YVulkanTypes.h"
#include "YGLSLStructs.hpp"

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

struct YsMaterialComponent;


class YMaterialSystem {
public:
    static YMaterialSystem* instance();

    void updagteMaterial();

    inline i32 materialCount() {return this->m_material_struct_buffer.size();};

    inline void* materialData() {return this->m_material_struct_buffer.data();};

    inline i32 materialId(YsMaterialComponent* mc) {return this->m_material_index_map.find(mc)->second;};

private:
    YMaterialSystem();
    ~YMaterialSystem();

private:


private:
    std::map<YsMaterialComponent*, i32> m_material_index_map;

    std::vector<GLSL_Material> m_material_struct_buffer;
};


#endif //CGPPY_YMATERIALSYSTEM_HPP
