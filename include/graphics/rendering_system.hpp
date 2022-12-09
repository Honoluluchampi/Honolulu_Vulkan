#pragma once

// hnll
#include <graphics/device.hpp>
#include <graphics/pipeline.hpp>
#include <game/components/renderable_component.hpp>
#include <utils/rendering_utils.hpp>

// lib
#include <vulkan/vulkan.h>

// std
#include <memory>
#include <unordered_map>

namespace hnll {

template<class T> using u_ptr = std::unique_ptr<T>;
template<class T> using s_ptr = std::shared_ptr<T>;

namespace graphics {

class rendering_system
{
  using map = std::unordered_map<hnll::game::component_id, std::shared_ptr<hnll::game::renderable_component>>;

public:
  rendering_system(device& device, hnll::utils::rendering_type type) : device_(device), render_type_(type) {}
  virtual ~rendering_system() { vkDestroyPipelineLayout(device_.get_device(), pipeline_layout_, nullptr); };
  
  rendering_system(const rendering_system &) = delete;
  rendering_system &operator=(const rendering_system &) = delete;
  rendering_system(rendering_system &&) = default;
  rendering_system &operator=(rendering_system &&) = default;

protected:
};

} // namespace graphics
} // namespace hnll