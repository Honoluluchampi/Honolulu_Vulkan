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

class HveRenderingSystem
{
  using map = std::unordered_map<component::id, std::shared_ptr<renderable_component>>;

public:
  HveRenderingSystem(device& device, render_type type) 
   : hveDevice_m(device), render_type_(type) {}
  virtual ~HveRenderingSystem()
  { vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr); };
  
  HveRenderingSystem(const HveRenderingSystem &) = delete;
  HveRenderingSystem &operator=(const HveRenderingSystem &) = delete;
  HveRenderingSystem(HveRenderingSystem &&) = default;
  HveRenderingSystem &operator=(HveRenderingSystem &&) = default;

  virtual void render(FrameInfo frameInfo) {}

  // takes s_ptr<RenderableComponent>
  template<class S>
  void addRenderTarget(component::id id, S&& target)
  { renderTargetMap_m.emplace(id, std::forward<S>(target)); }

  template<class S>
  void replaceRenderTarget(component::id id, S&& target)
  { renderTargetMap_m[id] = std::forward<S>(target); }

  void removeRenderTarget(component::id id)
  { renderTargetMap_m.erase(id); }

  render_type get_render_type() const { return render_type_; }

private:
  virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {}
  virtual void createPipeline(VkRenderPass renderPass) {}

protected:
  device& hveDevice_m;
  u_ptr<HvePipeline> hvePipeline_m = nullptr;
  VkPipelineLayout pipelineLayout_m;
  render_type render_type_;

  // derived classes must use renderTarget by down-cast values of this map
  // ex) auto modelComp = dynamic_cast<ModelComponent*>(renderTargetMap_m[1].get());
  map renderTargetMap_m;
};

} // namespace hnll