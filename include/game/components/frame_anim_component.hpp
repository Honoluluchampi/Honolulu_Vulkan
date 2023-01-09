#pragma once

// hnll
#include <game/engine.hpp>
#include <game/components/renderable_component.hpp>
#include <graphics/frame_anim_mesh_model.hpp>
#include <graphics/frame_anim_meshlet_model.hpp>

namespace hnll::game {

template <class FrameAnimModel>
class frame_anim_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<frame_anim_component> create(s_ptr<A>& owner, const std::string& model_name);

    // no default implementation
    template <Actor A>
    frame_anim_component(s_ptr<A>& owner, FrameAnimModel& model);

    void update_component(float dt) override
    {
      animation_timer_ += dt;
      while (animation_timer_ >= end_time_) {
        animation_timer_ -= (end_time_ - start_time_);
      }
      frame_index_ = animation_timer_ / end_time_ * static_cast<float>(frame_count_);
    }

    template <class... Args>
    void bind_and_draw(VkCommandBuffer command_buffer, Args... args)
    {
      model_.bind(animation_index_, frame_index_, command_buffer, args...);
      model_.draw(command_buffer);
    }

    // getter
    uint32_t get_animation_index() { return animation_index_; }
    uint32_t get_frame_index()     { return frame_index_; }

    void activate_animation(uint32_t index)
    {
      if (index >= animation_count_) {
        std::cout << "animation index " << index << " doesn't exist." << std::endl;
        return;
      }
      animation_index_ = index;
      animation_timer_ = 0.f;
      start_time_  = model_.get_start_time(index);
      end_time_    = model_.get_end_time(index);
      frame_count_ = model_.get_frame_count(index);
      animating_ = true;
    }

    void stop_animation() { animating_ = false; }

  private:
    FrameAnimModel& model_;
    uint32_t animation_index_ = 0;
    uint32_t frame_index_ = 0;
    uint32_t animation_count_;
    uint32_t frame_count_;

    float animation_timer_ = 0.f;
    float start_time_ = 0.f;
    float end_time_;

    bool animating_ = true;
};

template <> template <Actor A>
frame_anim_component<graphics::frame_anim_mesh_model>::frame_anim_component(s_ptr<A>& owner, graphics::frame_anim_mesh_model& model)
  : renderable_component(owner, utils::shading_type::FRAME_ANIM_MESH), model_(model)
{
  animation_count_ = model_.get_animation_count();
  activate_animation(0);
}

template <> template <Actor A>
frame_anim_component<graphics::frame_anim_meshlet_model>::frame_anim_component(s_ptr<A>& owner, graphics::frame_anim_meshlet_model& model)
  : renderable_component(owner, utils::shading_type::FRAME_ANIM_MESHLET), model_(model)
{
  animation_count_ = model_.get_animation_count();
  activate_animation(0);
}

template<> template <Actor A>
s_ptr<frame_anim_component<graphics::frame_anim_mesh_model>> frame_anim_component<graphics::frame_anim_mesh_model>::create(s_ptr<A>& owner, const std::string& model_name)
{
  auto& skinning_mesh = engine::get_skinning_mesh_model(model_name);
  auto& frame_mesh = engine::get_frame_anim_mesh_model(model_name);
  auto ret = std::make_shared<frame_anim_component>(owner, frame_mesh);
  owner->set_renderable_component(ret);
  return ret;
}

template<> template <Actor A>
s_ptr<frame_anim_component<graphics::frame_anim_meshlet_model>> frame_anim_component<graphics::frame_anim_meshlet_model>::create(s_ptr<A>& owner, const std::string& model_name)
{
  auto& skinning_mesh = engine::get_skinning_mesh_model(model_name);
  auto& frame_meshlet = engine::get_frame_anim_meshlet_model(model_name);
  auto ret = std::make_shared<frame_anim_component>(owner, frame_meshlet);
  owner->set_renderable_component(ret);
  return ret;
}

}// namespace hnll::game