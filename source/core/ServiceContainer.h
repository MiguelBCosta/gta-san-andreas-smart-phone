#pragma once
#include <unordered_map>
#include <typeindex>

class ServiceContainer {
public:
    template<typename TInterface>
    static void registerService(TInterface* instance) {
        getRegistry()[std::type_index(typeid(TInterface))] = static_cast<void*>(instance);
    }

    template<typename TInterface>
    static TInterface* resolve() {
        auto& registry = getRegistry();
        auto it = registry.find(std::type_index(typeid(TInterface)));
        if (it != registry.end()) {
            return static_cast<TInterface*>(it->second);
        }
        return nullptr;
    }

    static void clear() {
        getRegistry().clear();
    }

private:
    static std::unordered_map<std::type_index, void*>& getRegistry() {
        static std::unordered_map<std::type_index, void*> registry;
        return registry;
    }
};

template<typename T>
class Inject {
private:
    mutable T* m_instance = nullptr;
public:
    Inject() = default;
    Inject(T* instance) : m_instance(instance) {}
    Inject& operator=(T* instance) {
        m_instance = instance;
        return *this;
    }
    T* operator->() const {
        if (!m_instance) {
            m_instance = ServiceContainer::resolve<T>();
        }
        return m_instance;
    }
    operator T*() const {
        if (!m_instance) {
            m_instance = ServiceContainer::resolve<T>();
        }
        return m_instance;
    }
    bool isValid() const {
        return operator T*() != nullptr;
    }
};
