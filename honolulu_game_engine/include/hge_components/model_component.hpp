#pragma once

#include <hve_model.hpp>
#include <hge_component.hpp>
#include <hve_game_object.hpp>
#include <hge_components/hge_renderable_component.hpp>

#include <unordered_map>

namespace hnll {

class ModelComponent : public HgeRenderableComponent
{
  public:
    using id_t = unsigned int;
    using map = std::unordered_map<id_t, s_ptr<ModelComponent>>;
    // copy a passed shared_ptr
    ModelComponent(id_t id, const s_ptr<HveModel>& spModel) : HgeRenderableComponent(id), spModel_m(spModel) {}
    ModelComponent(id_t id, s_ptr<HveModel>&& spModel) : HgeRenderableComponent(id), spModel_m(std::move(spModel)) {}
    ~ModelComponent(){}

    void update(float dt) override
    {}

    s_ptr<HveModel>& getSpModel() { return spModel_m; }
        
  private:
    // HveModel can be shared all over a game
    s_ptr<HveModel> spModel_m = nullptr;
};

} // namespace hnll