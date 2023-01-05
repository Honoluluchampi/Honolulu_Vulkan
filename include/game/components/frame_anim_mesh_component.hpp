#pragma once

// hnll
#include <game/engine.hpp>
#include <game/components/renderable_component.hpp>
#include <graphics/frame_anim_mesh_model.hpp>

namespace hnll::game {

class frame_anim_mesh_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<frame_anim_mesh_component> create(s_ptr<A>& owner, graphics::device& device, const std::string& model_name)
    {
      auto& skinning_mesh = engine::get_skinning_mesh_model(model_name);
      auto frame_mesh = graphics::frame_anim_mesh_model::create_from_skinning_mesh_model(device, skinning_mesh);
      auto ret = std::make_shared<frame_anim_mesh_component>(owner, std::move(frame_mesh));
      owner->set_renderable_component(ret);
      owner->add_component(ret);
      return ret;
    }

    template <Actor A>
    frame_anim_mesh_component(s_ptr<A>& owner, u_ptr<graphics::frame_anim_mesh_model>&& model)
      : renderable_component(owner, utils::shading_type::FRAME_ANIM_MESH), model_(std::move(model))
    {

    }

    void bind_and_draw(VkCommandBuffer command_buffer)
    {
      model_->bind(animation_index_, frame_index_, command_buffer);
      model_->draw(command_buffer);
    }

    // getter
    uint32_t get_animation_index() { return animation_index_; }
    uint32_t get_frame_index()     { return frame_index_; }

  private:
    // TODO : make this reference
    u_ptr<graphics::frame_anim_mesh_model> model_;
    uint32_t animation_index_ = 0;
    uint32_t frame_index_ = 0;
};

}// namespace hnll::game