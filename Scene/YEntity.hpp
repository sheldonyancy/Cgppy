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

#ifndef CGPPY_YENTITY_HPP
#define CGPPY_YENTITY_HPP


#include <vector>
#include <unordered_map>
#include <typeindex>

#include "YMeshComponent.hpp"


struct YsEntity {
    unsigned int id;
    YsEntity* parent = nullptr;
    std::vector<YsEntity*> children;
    std::unordered_map<std::type_index, YsComponent*> components;

    YsEntity(unsigned int id) : id(id) {}
    ~YsEntity(){}

    void addChild(YsEntity* child) {
        this->children.push_back(child);
        child->parent = this;
    }

    template<typename T>
    void addComponent(T* component) {
        this->components[std::type_index(typeid(T))] = component;
    }

    template<typename T>
    T* getComponent() {
        auto it = this->components.find(std::type_index(typeid(T)));
        if (it != this->components.end()) {
            return static_cast<T*>(it->second);
        }
        return nullptr;
    }
};


#endif //CGPPY_YENTITY_HPP
