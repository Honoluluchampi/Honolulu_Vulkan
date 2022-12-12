#pragma once

// hnll
#include <game/shading_system.hpp>

namespace hnll {

namespace game {

class mesh_model_shading_system : public shading_system
{
  public:
    static u_ptr<mesh_model_shading_system> create(graphics::device& device);

    explicit mesh_model_shading_system(graphics::device& device);
    ~mesh_model_shading_system() override = default;

    void render(const utils::frame_info& frame_info) override;
};

}} // namespace hnll::game