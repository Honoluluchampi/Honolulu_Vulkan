#pragma once

#include <hve_model.hpp>

// lib
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace hve {

// 3d transformation
struct TransformComponent
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

// pointLight can be treated as gameObject
struct PointLightComponent
{
  float lightIntensity_m = 1.0f;
};

class HveGameObject
{
  public:
    using id_t = unsigned int;
    // why dont we use std::vector which can be randomly accessed
    using Map = std::unordered_map<id_t, HveGameObject>;
    
    static HveGameObject createGameObject() 
    {
      static id_t currentId = 0;
      return HveGameObject{currentId++};
    }

    static HveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

    // uncopyable, movable
    HveGameObject(const HveGameObject &) = delete;
    HveGameObject& operator=(const HveGameObject &) = delete;
    HveGameObject(HveGameObject &&) = default;
    HveGameObject& operator=(HveGameObject &&) = default;

    id_t getId() { return id_m; }

    glm::vec3 color_m{};
    TransformComponent transform_m{};

    // wanna make it boost::intrusive_ptr
    // optional opinter components
    std::shared_ptr<HveModel> model_m{};
    std::unique_ptr<PointLightComponent> pointLight_m = nullptr;

  private:
    HveGameObject(id_t objId) : id_m(objId) {}
    // each game object has unique id;
    id_t id_m;
};
} // namespace std