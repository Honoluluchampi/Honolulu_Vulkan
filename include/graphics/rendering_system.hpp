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
  using map = std::unordered_map<HgeComponent::compId, std::shared_ptr<HgeRenderableComponent>>;

public:
  HveRenderingSystem(HveDevice& device, RenderType type) 
   : hveDevice_m(device), renderType_m(type) {}
  virtual ~HveRenderingSystem()
  { vkDestroyPipelineLayout(hveDevice_m.device(), pipelineLayout_m, nullptr); };
  
  HveRenderingSystem(const HveRenderingSystem &) = delete;
  HveRenderingSystem &operator=(const HveRenderingSystem &) = delete;
  HveRenderingSystem(HveRenderingSystem &&) = default;
  HveRenderingSystem &operator=(HveRenderingSystem &&) = default;

  virtual void render(FrameInfo frameInfo) {}

  // takes s_ptr<RenderableComponent>
  template<class S>
  void addRenderTarget(HgeComponent::compId id, S&& target)
  { renderTargetMap_m.emplace(id, std::forward<S>(target)); }

  template<class S>
  void replaceRenderTarget(HgeComponent::compId id, S&& target)
  { renderTargetMap_m[id] = std::forward<S>(target); }

  void removeRenderTarget(HgeComponent::compId id)
  { renderTargetMap_m.erase(id); }

  RenderType getRenderType() const { return renderType_m; }

private:
  virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {}
  virtual void createPipeline(VkRenderPass renderPass) {}

protected:
  HveDevice& hveDevice_m;
  u_ptr<HvePipeline> hvePipeline_m = nullptr;
  VkPipelineLayout pipelineLayout_m;
  RenderType renderType_m;

  // derived classes must use renderTarget by down-cast values of this map
  // ex) auto modelComp = dynamic_cast<ModelComponent*>(renderTargetMap_m[1].get());
  map renderTargetMap_m;
};

} // namespace hnll