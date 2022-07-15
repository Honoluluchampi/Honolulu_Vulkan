#pragma once

// hnll
#include <graphics/rendering_system.hpp>
#include <utils/utils.hpp>

// std
#include <vector>

namespace hnll {
namespace graphics {

class mesh_rendering_system : public rendering_system
{
  public:
    mesh_rendering_system(device& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
    ~mesh_rendering_system();

    // dont make camera object as a member variable so as to share the camera between multiple render system
    void render(frame_info frame_info) override;
    
  private:
    void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) override;
    void create_pipeline(VkRenderPass render_pass) override;
};

} // namespac graphics
} // namespace hnll