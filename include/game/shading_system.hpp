#pragma once

// hnll
#include <graphics/device.hpp>
#include <graphics/pipeline.hpp>
#include <utils/rendering_utils.hpp>

// std
#include <concepts>

namespace hnll {

// forward declaration
namespace graphics { class device; }

namespace game {

template <class T>
concept ShadingSystem = requires (T& x) {
  x.get_rendering_type();
  x.get_global_desc_set_layout();
};

class shading_system
{
    using render_target_map = std::unordered_map<component_id, renderable_component&>;

  public:
    shading_system(graphics::device& device, utils::rendering_type type)
    : device_(device), rendering_type_(type) {}

    virtual void render(const utils::frame_info& frame_info) = 0;

    void add_render_target(component_id id, renderable_component& target)
    { render_target_map_.emplace(id, target); }
    void remove_render_target(component_id id)
    { render_target_map_.erase(id); }

    // getter
    utils::rendering_type        get_rendering_type()   const { return rendering_type_; }
    static VkDescriptorSetLayout get_global_desc_set_layout() { return global_desc_set_layout_; }

    // setter
    shading_system& set_rendering_type(utils::rendering_type type)           { rendering_type_ = type; return *this; }
    static void     set_global_desc_set_layout(VkDescriptorSetLayout layout) { global_desc_set_layout_ = layout; }
  protected:
    // vulkan objects
    graphics::device&  device_;
    u_ptr<graphics::pipeline> pipeline_;
    VkPipelineLayout   pipeline_layout_;

    static VkDescriptorSetLayout global_desc_set_layout_;

    // shading system is called in enum class rendering_type-order at rendering process
    utils::rendering_type rendering_type_;
    render_target_map     render_target_map_;
};

namespace shading_system_helper {
// takes push_constant struct as type parameter
template <typename PushConstant>
VkPipelineLayout create_pipeline_layout(
  VkDevice device,
  VkShaderStageFlagBits shader_stage_flags,
  std::vector<VkDescriptorSetLayout> descriptor_set_layouts)
{
  // configure push constant
  VkPushConstantRange push_constant_range{};
  push_constant_range.stageFlags = shader_stage_flags;
  push_constant_range.offset     = 0;

  // configure desc sets layout
  VkPipelineLayoutCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
  create_info.pSetLayouts = descriptor_set_layouts.data();
  create_info.pushConstantRangeCount = 1;
  create_info.pPushConstantRanges = &push_constant_range;

  // create
  VkPipelineLayout ret;
  if (vkCreatePipelineLayout(device, &create_info, nullptr, &ret) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout.");

  return ret;
}

} // namespace shading_system_helper
}} // namespace hnll::game