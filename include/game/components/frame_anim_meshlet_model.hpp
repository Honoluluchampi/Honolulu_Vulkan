#pragma once

// hnll
#include <graphics/frame_anim_meshlet_model.hpp>
#include <game/engine.hpp>
#include <game/components/renderable_component.hpp>
#include <utils/common_using.hpp>

namespace hnll::game {

class frame_anim_meshlet_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<frame_anim_meshlet_component> create(s_ptr<A>& owner, const std::string& model_name)
    {
      auto& model = engine::
    }

    template <Actor A>
    frame_anim_meshlet_component(s_ptr<A>& owner, const graphics::frame_anim_meshlet_model& model)
      : renderable_component(owner, utils::shading_type::FRAME_ANIM_MESHLET), model_(model) {}
      ~frame_anim_meshlet_component() override = default;
  private:
    const graphics::frame_anim_meshlet_model& model_;
};

} // namespace hnll::game