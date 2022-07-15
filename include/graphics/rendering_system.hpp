#pragma once

// hnll
#include <graphics/device.hpp>
#include <graphics/pipeline.hpp>
#include <graphics/frame_info.hpp>
#include <game/components/renderable_component.hpp>

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
  using map = std::unordered_map<hnll::game::component::id, std::shared_ptr<hnll::game::renderable_component>>;

public:
  rendering_system(device& device, hnll::game::render_type type) : device_(device), render_type_(type) {}
  virtual ~rendering_system() { vkDestroyPipelineLayout(device_.get_device(), pipeline_layout_, nullptr); };
  
  rendering_system(const rendering_system &) = delete;
  rendering_system &operator=(const rendering_system &) = delete;
  rendering_system(rendering_system &&) = default;
  rendering_system &operator=(rendering_system &&) = default;

  virtual void render(frame_info frame_info) {}

  // takes s_ptr<RenderableComponent>
  template<class S>
  void add_render_target(hnll::game::component::id id, S&& target)
  { render_target_map_.emplace(id, std::forward<S>(target)); }

  template<class S>
  void replace_render_target(hnll::game::component::id id, S&& target)
  { render_target_map_[id] = std::forward<S>(target); }

  void remove_render_target(hnll::game::component::id id)
  { render_target_map_.erase(id); }

  hnll::game::render_type get_render_type() const { return render_type_; }

private:
  virtual void create_pipeline_layout(VkDescriptorSetLayout global_set_layout) {}
  virtual void create_pipeline(VkRenderPass render_pass) {}

protected:
  device& device_;
  u_ptr<pipeline> pipeline_ = nullptr;
  VkPipelineLayout pipeline_layout_;
  hnll::game::render_type render_type_;

  // derived classes must use renderTarget by down-cast values of this map
  // ex) auto modelComp = dynamic_cast<ModelComponent*>(render_target_map_[1].get());
  map render_target_map_;
};

} // namespace graphics
} // namespace hnll