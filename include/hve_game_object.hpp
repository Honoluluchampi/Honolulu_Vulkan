#pragma once

#include <hve_model.hpp>

// lib
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

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
  glm::mat4 mat4() 
  {
    const float c3 = glm::cos(rotation_m.z);
    const float s3 = glm::sin(rotation_m.z);
    const float c2 = glm::cos(rotation_m.x);
    const float s2 = glm::sin(rotation_m.x);
    const float c1 = glm::cos(rotation_m.y);
    const float s1 = glm::sin(rotation_m.y);
    return glm::mat4{
        {
            scale_m.x * (c1 * c3 + s1 * s2 * s3),
            scale_m.x * (c2 * s3),
            scale_m.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale_m.y * (c3 * s1 * s2 - c1 * s3),
            scale_m.y * (c2 * c3),
            scale_m.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale_m.z * (c2 * s1),
            scale_m.z * (-s2),
            scale_m.z * (c1 * c2),
            0.0f,
        },
        {translation_m.x, translation_m.y, translation_m.z, 1.0f}};
}
};

class HveGameObject
{
  using id_t = unsigned int;
  public:
    static HveGameObject createGameObject() 
    {
      static id_t currentId = 0;
      return HveGameObject{currentId++};
    }

    // uncopyable, movable
    HveGameObject(const HveGameObject &) = delete;
    HveGameObject& operator=(const HveGameObject &) = delete;
    HveGameObject(HveGameObject &&) = default;
    HveGameObject& operator=(HveGameObject &&) = default;

    id_t getId() { return id_m; }

    // wanna make it boost::intrusive_ptr
    std::shared_ptr<HveModel> model_m{};
    glm::vec3 color_m{};
    TransformComponent transform_m{};

  private:
    HveGameObject(id_t objId) : id_m(objId) {}
    // each game object has unique id;
    id_t id_m;
};
} // namespace std