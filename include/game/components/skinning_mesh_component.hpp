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
    : renderable_component(owner_sp, utils::shading_type::SKINNING_MESH), model_(_model)
    {
      auto& animations = model_.get_animations();
      max_animation_index_ = animations.size() - 1;
      activate_animation(0);
    }

    void update_component(float dt) override
    {
      animation_timer_ += dt;
      while (animation_timer_ > end_time_) {
        animation_timer_ -= (end_time_ - start_time_);
      }
      model_.update_animation(target_animation_, animation_timer_);
    }

    // getter
    graphics::skinning_mesh_model& get_model() { return model_; }

    // setter
    void activate_animation(uint32_t target_animation = 0)
    {
      if (target_animation > max_animation_index_) {
        std::cout << "animation index " << target_animation << " doesn't exist." << std::endl;
        return;
      }
      target_animation_ = target_animation;
      animation_timer_ = 0.f;
      auto& target_anim = model_.get_animations()[target_animation_];
      start_time_ = target_anim.start;
      end_time_   = target_anim.end;
      animating_ = true;
    }
    void stop_animation() { animating_ = false; }

  private:
    graphics::skinning_mesh_model& model_;
    float    animation_timer_ = 0.f;
    float    start_time_ = 0.f;
    float    end_time_;
    uint32_t target_animation_ = 0;
    uint32_t max_animation_index_;
    bool     animating_ = true;
};

} // namespace hnll::game