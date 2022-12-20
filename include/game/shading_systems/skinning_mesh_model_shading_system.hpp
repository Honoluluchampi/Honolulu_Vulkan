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
    void setup_node_desc_set();

  private:
    u_ptr<graphics::descriptor_pool>       node_desc_pool_;
    std::vector<u_ptr<graphics::buffer>>   node_desc_buffers_;
    u_ptr<graphics::descriptor_set_layout> node_desc_layout_;
    std::vector<VkDescriptorSet>           node_desc_sets_;
};

}} // namespace hnll::game