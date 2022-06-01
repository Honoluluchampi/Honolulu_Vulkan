#pragma once

#include <hve_model.hpp>
#include <hge_component.hpp>
#include <hge_components/hge_renderable_component.hpp>

#include <unordered_map>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class ModelComponent : public HgeRenderableComponent
{
  public:
    using map = std::unordered_map<compId, s_ptr<ModelComponent>>;
    // copy a passed shared_ptr
    ModelComponent(actorId id, const s_ptr<HveModel>& spModel)
     : HgeRenderableComponent(id, RenderType::SIMPLE), spModel_m(spModel) {}
    ModelComponent(actorId id, s_ptr<HveModel>&& spModel)
     : HgeRenderableComponent(id, RenderType::SIMPLE), spModel_m(std::move(spModel)) {}
    ~ModelComponent(){}

    s_ptr<HveModel>& getSpModel() { return spModel_m; }
    template<class S>
    void replaceHveModel(S&& model)
    { spModel_m = std::forward<S>(model); }
        
  private:
    // HveModel can be shared all over a game
    s_ptr<HveModel> spModel_m = nullptr;
};

} // namespace hnll