#pragma once

#include <hve_model.hpp>

// std
#include <memory>

namespace hve {

struct Transform2dComponent
{
  glm::vec2 translation{}; // position offset
  glm::vec2 scale{1.f, 1.f};
  float rotation = 0.0f; // clockwise radian

  glm::mat2 mat2() 
  { 
    const float s = glm::sin(rotation), c = glm::cos(rotation);
    glm::mat2 rotMat{{c, s}, {-s, c}};
    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMat * scaleMat; 
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
    Transform2dComponent transform2d_m;

  private:
    HveGameObject(id_t objId) : id_m(objId) {}
    // each game object has unique id;
    id_t id_m;
};
} // namespace std