#pragma once

#include <hve_model.hpp>
#include <hge_component.hpp>
#include <hve_game_object.hpp>

namespace hnll {

// to do : delete hveGameObject's transformComponent
// 3d transformation
// struct TransformComponent
// {
//   glm::vec3 translation_m{}; // position offset
//   glm::vec3 scale_m{1.f, 1.f, 1.f};
//   // y-z-x tait-brian rotation
//   glm::vec3 rotation_m{};

//   // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
//   // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
//   // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
//   glm::mat4 mat4();
//   glm::mat3 normalMatrix();
// };

class ModelComponent : public HgeComponent
{
  public:
    // copy a passed shared_ptr
    ModelComponent(std::shared_ptr<HveModel>& spModel) : HgeComponent(), spModel_m(spModel) {}
    ModelComponent(std::shared_ptr<HveModel>&& spModel) : HgeComponent(), spModel_m(std::move(spModel)) {}
    ~ModelComponent(){}

    void update(float dt) override
    {}

    std::shared_ptr<HveModel>& getSpModel() { return spModel_m; }
    static HveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
    TransformComponent& getTransform() { return transform_m; }
    glm::vec3 color_m{};
    std::unique_ptr<PointLightComponent> pointLight_m = nullptr;
    // update this member
    TransformComponent transform_m;

  private:
    std::shared_ptr<HveModel> spModel_m;
};

} // namespace hnll