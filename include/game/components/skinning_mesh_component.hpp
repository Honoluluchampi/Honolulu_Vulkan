#pragma once

// hnll
#include <game/engine.hpp>
#include <game/components/renderable_component.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::game {

class skinning_mesh_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<skinning_mesh_component> create(s_ptr<A>& owner_sp, const std::string& model_name)
    {
      auto& mesh_model = engine::get_skinning_mesh_model(model_name);
      auto mesh = std::make_shared<skinning_mesh_component>(owner_sp, mesh_model);
      owner_sp->set_renderable_component(mesh);
      return mesh;
    }

    template <Actor A>
    skinning_mesh_component(s_ptr<A>& owner_sp, graphics::skinning_mesh_model& _model)
    : renderable_component(owner_sp, utils::shading_type::SKINNING_MESH), model_(_model) {}

    // getter
    graphics::skinning_mesh_model& get_model() { return model_; }

  private:
    graphics::skinning_mesh_model& model_;
};

} // namespace hnll::game