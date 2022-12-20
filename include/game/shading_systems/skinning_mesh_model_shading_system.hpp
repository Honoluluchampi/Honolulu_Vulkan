#pragma once

// hnll
#include <game/shading_system.hpp>
#include <graphics/descriptor_set_layout.hpp>
#include <graphics/buffer.hpp>

namespace hnll {
namespace game {

class skinning_mesh_model_shading_system : public shading_system
{
  public:
    static u_ptr<skinning_mesh_model_shading_system> create(graphics::device& device);

    explicit skinning_mesh_model_shading_system(graphics::device& device);
    skinning_mesh_model_shading_system& operator=(const skinning_mesh_model_shading_system&) = default;

    void render(const utils::frame_info& frame_info) override;
};

}} // namespace hnll::game