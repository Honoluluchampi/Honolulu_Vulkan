#pragma once

#include <hve_model.hpp>
#include <hge_component.hpp>
#include <hve_game_object.hpp>

namespace hnll {

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
    
    TransformComponent& getTransform() { return *upTransform_m; }
    template<class V> void setTranslation(V&& vec) 
    { (*upTransform_m).translation_m = std::forward<V>(vec); }    
    template<class V> void setScale(V&& vec) 
    { (*upTransform_m).scale_m = std::forward<V>(vec); }
    template<class V> void setRotation(V&& vec) 
    { (*upTransform_m).rotation_m = std::forward<V>(vec); }    

    glm::vec3 color_m{};
    std::unique_ptr<PointLightComponent> pointLight_m = nullptr;

  private:
    // HveModel can be shared all over a game
    std::shared_ptr<HveModel> spModel_m;
    // update this member
    std::unique_ptr<TransformComponent> upTransform_m = std::make_unique<TransformComponent>();
};

} // namespace hnll