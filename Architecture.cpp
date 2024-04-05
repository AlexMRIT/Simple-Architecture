#include <iostream>
#include <unordered_map>
#include <memory>
#include <functional>
#include <cassert>

enum component_type_e {
    Transform = 0x0A
};

class GameObjectBase;

class Vector3f {
public:
    Vector3f() : m_x(0.0f), m_y(0.0f), m_z(0.0f) { }
    Vector3f(float x, float y, float z)
        : m_x(x), m_y(y), m_z(z) { }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

private:
    float m_x, m_y, m_z;
};

class Quaternion {
public:
    Quaternion() : m_x(0.0f), m_y(0.0f), m_z(0.0f) { }
    Quaternion(float x, float y, float z)
        : m_x(x), m_y(y), m_z(z) { }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

private:
    float m_x, m_y, m_z;
};

class Component {
public:
    Component(component_type_e type, const GameObjectBase& parent)
        : m_type(type), m_parent(parent), m_hash(std::hash<uint32_t>{}(static_cast<uint32_t>(type))) {}

    virtual ~Component() = default;

    size_t GetHash() const {
        return m_hash;
    }

    const GameObjectBase& GetParent() const {
        return m_parent;
    }

protected:
    const GameObjectBase& m_parent;
    component_type_e m_type;
    size_t m_hash;
};

template<component_type_e Type>
class TransformComponent : public Component {
public:
    TransformComponent(const GameObjectBase& base)
        : Component(Type, base) {}

    static size_t GetHash() {
        return std::hash<uint32_t>{}(static_cast<uint32_t>(Type));
    }

    Vector3f position() { return m_position; }
    Quaternion rotation() { return m_rotation; }
    Vector3f scale() { return m_scale; }

private:
    Vector3f m_position;
    Quaternion m_rotation;
    Vector3f m_scale;
};

class Components {
public:
    bool TryAddComponent(const std::shared_ptr<Component>& component) {
        if (!component) {
            assert(component && "Trying to add a null component");
            return false;
        }

        size_t hash = component->GetHash();
        auto result = m_components.emplace(hash, component);
        return result.second;
    }

    bool TryDeleteComponent(size_t hash) {
        return m_components.erase(hash) > 0;
    }

    void RemoveAllComponents() {
        m_components.clear();
    }

    std::shared_ptr<Component> operator[](size_t hash) const {
        auto it = m_components.find(hash);
        return it != m_components.end() ? it->second : nullptr;
    }

private:
    std::unordered_map<size_t, std::shared_ptr<Component>> m_components;
};

class GameObjectBase {
public:
    bool TryAddComponent(const std::shared_ptr<Component>& component) {
        assert(component && "Trying to add a null component to GameObjectBase");
        return m_components.TryAddComponent(component);
    }

    std::shared_ptr<Component> TryGetComponent(size_t hash) const {
        return m_components[hash];
    }

    bool TryDeleteComponent(size_t hash) {
        return m_components.TryDeleteComponent(hash);
    }

    void Destroy() {
        m_components.RemoveAllComponents();
    }

protected:
    Components m_components;
};

class GameObject : public GameObjectBase {

};

int main() {
    std::shared_ptr<GameObject> player = std::make_shared<GameObject>();
    assert(player && "Failed to create GameObject");

    std::shared_ptr<TransformComponent<Transform>> transformComponent = std::make_shared<TransformComponent<Transform>>(*player);
    assert(player->TryAddComponent(transformComponent) && "Failed to add TransformComponent to GameObject");

    std::weak_ptr<TransformComponent<Transform>> transform = std::static_pointer_cast<TransformComponent<Transform>>(player->TryGetComponent(TransformComponent<Transform>::GetHash()));
    if (!transform.expired()) {
        Vector3f position = transform.lock()->position();
        std::cout << "X: " << position.x() << " Y: " << position.y() << " Z: " << position.z() << std::endl;
    }
    else
        assert(!"Failed to get TransformComponent from GameObject");

    if (player->TryDeleteComponent(TransformComponent<Transform>::GetHash()))
        std::cout << "Component has been deleted!" << std::endl;
    else
        assert(!"Failed to delete TransformComponent from GameObject");

    player->Destroy();
    player.reset();

    return 0;
}