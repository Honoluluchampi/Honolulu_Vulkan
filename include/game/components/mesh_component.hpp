#pragma once

#include <hve_model.hpp>
#include <game/component.hpp>
#include <game/components/renderable_component.hpp>

#include <unordered_map>

namespace hnll {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class MeshComponent : public HgeRenderableComponent
{
  public:
    using map = std::unordered_map<compId, s_ptr<MeshComponent>>;
    // copy a passed shared_ptr
    MeshComponent(actorId id, const s_ptr<HveModel>& spModel)
     : HgeRenderableComponent(id, RenderType::SIMPLE), spModel_m(spModel) {}
    MeshComponent(actorId id, s_ptr<HveModel>&& spModel)
     : HgeRenderableComponent(id, RenderType::SIMPLE), spModel_m(std::move(spModel)) {}
    ~MeshComponent(){}

    s_ptr<HveModel>& getSpModel() { return spModel_m; }
    template<class S>
    void replaceHveModel(S&& model)
    { spModel_m = std::forward<S>(model); }
        
  private:
    // HveModel can be shared all over a game
    s_ptr<HveModel> spModel_m = nullptr;
};

} // namespace hnll