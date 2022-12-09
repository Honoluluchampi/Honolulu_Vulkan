// hnll
#include <game/shading_system.hpp>

namespace hnll {

namespace game {

class mesh_model_shading_system : public shading_system
{
  public:
    mesh_model_shading_system(graphics::device& device)
    : shading_system(device, utils::rendering_type::MESH){}

    void render(const utils::frame_info& frame_info) override;
};

}} // namespace hnll::game