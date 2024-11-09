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

#ifndef CGPPY_YSCENEMANAGER_HPP
#define CGPPY_YSCENEMANAGER_HPP


#include <memory>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>

#include "YAsyncTask.hpp"
#include "YEntity.hpp"
#include "YAABBComponent.hpp"

struct YsBVHNodeComponent;


class YSceneManager {
public:
    static YSceneManager* instance();

    void updateSceneInfo();

    inline const glm::fmat4x4& modelMatrix() {return this->m_model_matrix;}

    inline void applyTranslation(const glm::mat4& translation_matrix) {this->m_model_matrix = translation_matrix * this->m_model_matrix;}

    inline void applyScale(const glm::mat4& scale_matrix) {this->m_model_matrix = scale_matrix * this->m_model_matrix;}

    void applyRotation(const glm::mat4& rotation_matrix);

    inline YsAABBComponent& sceneBounds() {return this->m_scene_bound;}

    // Entity
    YsEntity* createEntity();

    YsEntity* getEntity(unsigned int id);

    inline YsEntity* getEntity(YsComponent* component) {return this->m_component_object_correspond_entity_object_map.find(component)->second;};

    template<typename T, typename... Args>
    T* addComponentToEntity(YsEntity* entity, Args&&... args) {
        T* component = entity->addComponent<T>(std::forward<Args>(args)...);
        this->m_entities_component_type[typeid(T)].insert(entity);
        return component;
    }

    template<typename T>
    const std::unordered_set<YsEntity*>& getEntitiesWithComponent() {
        return this->m_entities_component_type[typeid(T)];
    }

    // Component
    template<typename T>
    T* createComponent(unsigned int id) {
        std::unique_ptr<T> component = std::make_unique<T>();
        T* ptr = component.get();
        this->m_components[typeid(T)].push_back(std::make_pair(id, std::move(component)));
        YsEntity* entity = this->getEntity(id);
        entity->addComponent<T>(ptr);
        this->m_component_object_correspond_entity_object_map.insert(std::make_pair(ptr, entity));
        return ptr;
    }

    template<typename T>
    void destroyComponent(unsigned int id) {
        auto& vec = this->m_components[typeid(T)];
        vec.erase(std::remove_if(vec.begin(), vec.end(), [id](const auto& pair) { return pair.first == id; }), vec.end());
    }

    template<typename T>
    std::vector<T*>& getComponents() {
        static std::vector<T*> filtered_components;
        filtered_components.clear();
        auto& vec = this->m_components[typeid(T)];
        for (auto& pair : vec) {
            filtered_components.push_back(static_cast<T*>(pair.second.get()));
        }
        return filtered_components;
    }

private:
    YSceneManager();
    ~YSceneManager();

private:

private:
    std::vector<std::unique_ptr<YsEntity>> m_entities;
    std::unordered_map<std::type_index, std::unordered_set<YsEntity*>> m_entities_component_type;
    std::unordered_map<std::type_index, std::vector<std::pair<unsigned int, std::unique_ptr<YsComponent>>>> m_components;
    std::unordered_map<YsComponent*, YsEntity*> m_component_object_correspond_entity_object_map;
    //
    YsAABBComponent m_scene_bound;

    glm::fvec3 m_scene_center;

    glm::fmat4x4 m_model_matrix;
};


#endif //CGPPY_YSCENEMANAGER_HPP
