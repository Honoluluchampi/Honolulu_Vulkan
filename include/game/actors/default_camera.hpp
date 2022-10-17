#pragma once 

// hnll
#include <game/actor.hpp>
#include <game/components/viewer_component.hpp>
#include <game/components/keyboard_movement_component.hpp>
#include <utils/utils.hpp>

namespace hnll {

// forward declaration
namespace graphics { class engine; }

namespace game {

class default_camera : public actor
{
public:
  default_camera(hnll::graphics::engine& engine);
  ~default_camera(){}

  default_camera(const default_camera &) = delete;
  default_camera& operator=(const default_camera &) = delete;
  default_camera(default_camera &&) = default;
  default_camera& operator=(default_camera &&) = default;

  // getter
  utils::viewer_info get_viewer_info() const { return {viewer_comp_sp_->get_projection_ref(), viewer_comp_sp_->get_view_ref(), viewer_comp_sp_->get_inverse_view_ref()}; }
  bool is_movement_updating() const { return key_comp_sp_->is_updating(); }
  inline hnll::utils::transform& get_transform() { return transform_; }
  inline s_ptr<viewer_component> get_viewer_component_sp() const { return viewer_comp_sp_; }  
  // setter
  void set_movement_updating_on()  { key_comp_sp_->set_updating_on(); }
  void set_movement_updating_off() { key_comp_sp_->set_updating_off(); }
  template<class V> void set_translation(V&& vec) { transform_.translation = std::forward<V>(vec); }
  template<class V> void set_scale(V&& vec) { transform_.scale = std::forward<V>(vec); }
  template<class V> void set_rotation(V&& vec) { transform_.rotation = std::forward<V>(vec); }  

private:
  hnll::utils::transform transform_ {};
  s_ptr<viewer_component> viewer_comp_sp_;
  s_ptr<keyboard_movement_component> key_comp_sp_;
};

} // namespace graphics
} // namespace hnll