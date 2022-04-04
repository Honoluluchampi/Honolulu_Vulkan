#pragma once

#include <hve_model.hpp>
#include <hge_component.hpp>
#include <hve_game_object.hpp>
#include <hge_components/hge_renderable_component.hpp>

#include <unordered_map>
#include <memory>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class ModelComponent : public HgeRenderableComponent
{
  public:
    using id_t = unsigned int;
    using map = std::unordered_map<id_t, s_ptr<ModelComponent>>;
    // copy a passed shared_ptr
    ModelComponent(id_t id, const s_ptr<HveModel>& spModel) : HgeRenderableComponent(id, RenderType::SIMPLE), spModel_m(spModel) {}
    ModelComponent(id_t id, s_ptr<HveModel>&& spModel) : HgeRenderableComponent(id, RenderType::SIMPLE), spModel_m(std::move(spModel)) {}
    ~ModelComponent(){}

    void update(float dt) override
    {}

    s_ptr<HveModel>& getSpModel() { return spModel_m; }
    static HveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
    
    inline TransformComponent& getTransform() { return *upTransform_m; }
    template<class V> void setTranslation(V&& vec) 
    { upTransform_m->translation_m = std::forward<V>(vec); }    
    template<class V> void setScale(V&& vec) 
    { upTransform_m->scale_m = std::forward<V>(vec); }
    template<class V> void setRotation(V&& vec) 
    { upTransform_m->rotation_m = std::forward<V>(vec); }    

    inline id_t getId() const { return id_m; }
    
    glm::vec3 color_m{};
    u_ptr<PointLightComponent> pointLight_m = nullptr;

  private:
    // HveModel can be shared all over a game
    s_ptr<HveModel> spModel_m = nullptr;
    // update this member
    u_ptr<TransformComponent> upTransform_m = std::make_unique<TransformComponent>();
};

} // namespace hnll