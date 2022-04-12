#pragma once

#include <hve_device.hpp>
#include <hve_pipeline.hpp>
#include <hve_frame_info.hpp>
#include <hge_components/hge_renderable_component.hpp>

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
  using map = std::unordered_map<id_t, std::shared_ptr<HgeRenderableComponent>>;

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

  // delete later
  virtual void update(FrameInfo& frameInfo, GlobalUbo& ubo) {}

  // takes s_ptr<RenderableComponent>
  template<class S>
  void addRenderTarget(id_t id, S&& target)
  { renderTargetMap_m.emplace(id, std::forward<S>(target)); }

  void removeRenderTarget(id_t id)
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