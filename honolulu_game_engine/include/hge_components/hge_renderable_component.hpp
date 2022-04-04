#pragma once

#include <hge_component.hpp>

// lib
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

// 3d transformation
struct Transform
{
  glm::vec3 translation_m{}; // position offset
  glm::vec3 scale_m{1.f, 1.f, 1.f};
  // y-z-x tait-brian rotation
  glm::vec3 rotation_m{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();
  glm::mat3 normalMatrix();
};

class HgeRenderableComponent : public HgeComponent
{
  public:

    using id_t = unsigned int;

    HgeRenderableComponent(id_t id) : id_m(id) {}
    ~HgeRenderableComponent() {}

    HgeRenderableComponent(const HgeRenderableComponent &) = delete;
    HgeRenderableComponent& operator=(const HgeRenderableComponent &) = delete;
    HgeRenderableComponent(HgeRenderableComponent &&) = default;
    HgeRenderableComponent& operator=(HgeRenderableComponent &&) = default;

    inline Transform& getTransform() { return *upTransform_m; }
    template<class V> void setTranslation(V&& vec) 
    { upTransform_m->translation_m = std::forward<V>(vec); }    
    template<class V> void setScale(V&& vec) 
    { upTransform_m->scale_m = std::forward<V>(vec); }
    template<class V> void setRotation(V&& vec) 
    { upTransform_m->rotation_m = std::forward<V>(vec); }    

    virtual void update(float dt) override {}

    id_t getId() const { return id_m; }

  protected:
    // same as ownwer's
    id_t id_m;
    // update this member
    u_ptr<Transform> upTransform_m = std::make_unique<Transform>();
};

}