#pragma once

// hnll
#include <utils/rendering_type.hpp>

namespace hnll {

// forward declaration
namespace graphics { class rendering_system; }

namespace game {

class shading_system
{
  public:
    virtual void render(const graphics::frame_info& frame_info) = 0;
    virtual void create_pipeline_layout() = 0;
    virtual void create_pipeline() = 0;

    // getter
    graphics::rendering_system& get_rendering_system() { return *rendering_system_; }
    utils::rendering_type       get_rendering_type() const { return rendering_type_; }

    // setter
    shading_system& set_shading_order(utils::rendering_type type) { rendering_type_ = type; return *this; }

  private:
    // shading system is called in shading_order at rendering process
    utils::rendering_type rendering_type_;
    u_ptr<graphics::rendering_system> rendering_system_;
};
}} // namespace hnll::game