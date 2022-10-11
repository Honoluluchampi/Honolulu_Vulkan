#pragma once

// hnll
#include <game/components/renderable_component.hpp>
#include <geometry/perspective_frustum.hpp>

namespace hnll {

namespace game {

class wire_frame_frustum_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<wire_frame_frustum_component> create(s_ptr<A>& owner_sp, s_ptr<geometry::perspective_frustum>& frustum)
    {
      auto wire_frustum = std::make_shared<wire_frame_frustum_component>();
      wire_frustum->set_perspective_frustum(frustum);
      owner_sp->set_renderable_component(wire_frustum);
      return wire_frustum;
    }
    wire_frame_frustum_component(s_ptr<actor>& owner_sp) : renderable_component(owner_sp, render_type::WIRE_FRUSTUM) {}
    ~wire_frame_frustum_component() = default;

    // getter
    const geometry::perspective_frustum& get_perspective_frustum() const { return *frustum_; }

    // setter
    void set_perspective_frustum(s_ptr<geometry::perspective_frustum>& p_frustum) { frustum_ = p_frustum; }
  private:
    s_ptr<geometry::perspective_frustum> frustum_ = nullptr;
};

}} // namespace hnll::game