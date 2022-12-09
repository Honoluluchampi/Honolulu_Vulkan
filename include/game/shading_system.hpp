#pragma once

// hnll
#include <utils/rendering_utils.hpp>

namespace hnll {

// forward declaration
namespace graphics { class device; }

namespace game {

class shading_system
{
  public:
    shading_system(graphics::device& device, utils::rendering_type type)
    : device_(device), rendering_type_(type) {}

    virtual void render(const utils::frame_info& frame_info) = 0;

    // getter
    utils::rendering_type get_rendering_type() const { return rendering_type_; }

    // setter
    shading_system& set_rendering_type(utils::rendering_type type) { rendering_type_ = type; return *this; }

  private:
    graphics::device& device_;
    // shading system is called in shading_order at rendering process
    utils::rendering_type rendering_type_;
};
}} // namespace hnll::game