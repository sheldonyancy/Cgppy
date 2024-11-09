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

#ifndef CGPPY_YMETALBACKEND_HPP
#define CGPPY_YMETALBACKEND_HPP


#include "YRendererBackend.hpp"


class YMetalBackend : public YRendererBackend {
public:
    YMetalBackend();
    ~YMetalBackend();

private:
    void gapiUpdateVertexInputResource(u32 vertex_count,
                                       void* vertex_position_data,
                                       void* vertex_normal_data,
                                       void* vertex_material_id_data) override;

    void gapiUpdateGlobalUboResource(void* global_ubo_data) override;

    void gapiUpdateGlobalSceneBlockResource(u32 global_scene_block_data_size, void* global_scene_block_data) override;

    b8 frameRun() override;
};



#endif //CGPPY_YMETALBACKEND_HPP
