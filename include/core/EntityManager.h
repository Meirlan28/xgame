#pragma once

#include "Components.h"
#include "Logger.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <any>
#include <typeindex>
#include <string>
#include <algorithm>

using EntityID = uint32_t;


class EntityManager {
public:
    EntityID createEntity() {
        return nextID++;
    }

    template<typename T>
    void addComponent(EntityID entity, T component) {
        components[typeid(T)][entity] = component;
    }

    template<typename T>
    T& getComponent(EntityID entity) {
        return std::any_cast<T&>(components[typeid(T)][entity]);
    }

    template<typename T>
    bool hasComponent(EntityID entity) const {
        auto it = components.find(typeid(T));
        if (it == components.end()) return false;
        return it->second.find(entity) != it->second.end();
    }

    // Получение сущностей с определенными компонентами
    std::vector<EntityID> getEntitiesWithComponents(const std::vector<std::type_index>& types) const {
        std::vector<EntityID> result;
        for (const auto& [type, entityMap] : components) {
            if (type == types[0]) {
                for (const auto& [entity, _] : entityMap) {
                    bool hasAll = true;
                    for (size_t i = 1; i < types.size(); ++i) {
                        if (!hasComponent(entity, types[i])) {
                            hasAll = false;
                            break;
                        }
                    }
                    if (hasAll) result.push_back(entity);
                }
                break;
            }
        }
        return result;
    }

    template<typename... Components>
    std::vector<EntityID> getEntitiesWith() {
        std::vector<std::type_index> types = { typeid(Components)... };
        return getEntitiesWithComponents(types);
    }

private:
    EntityID nextID = 0;
    std::unordered_map<std::type_index, std::unordered_map<EntityID, std::any>> components;

    bool hasComponent(EntityID entity, std::type_index type) const {
        auto it = components.find(type);
        if (it == components.end()) return false;
        return it->second.find(entity) != it->second.end();
    }
};