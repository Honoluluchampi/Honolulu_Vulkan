#pragma once

// hnll
#include <game/components/renderable_component.hpp>
#include <graphics/skinning_mesh_model.hpp>

namespace hnll::game {

class skinning_mesh_component : public renderable_component
{
  public:
    // getter
    graphics::skinning_mesh_model& get_model() { return model_; }

  private:
    graphics::skinning_mesh_model& model_;
};

} // namespace hnll::game