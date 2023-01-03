#pragma once

#include <game/actor.hpp>

namespace hnll {

// forward declaration
namespace utils    { class frustum_info; }
namespace geometry { class perspective_frustum; }

namespace game {

// forward declaration
class graphics_engine;
class viewer_component;
class wire_frame_frustum_component;
class keyboard_movement_component;

class virtual_camera : public actor {
  public:
    static s_ptr<virtual_camera> create(graphics_engine& _engine);

    virtual_camera()  = default;
    ~virtual_camera() = default;

    void update_frustum_planes();

    // getter
    const geometry::perspective_frustum& get_perspective_frustum() const;
    utils::frustum_info get_frustum_info();

    // setter
    void set_movement_updating_on();
    void set_movement_updating_off();

  private:
    s_ptr<viewer_component> viewer_comp_;
    s_ptr<wire_frame_frustum_component> wire_frustum_comp_;
    s_ptr<keyboard_movement_component> key_comp_;
};

}} // hnll::game