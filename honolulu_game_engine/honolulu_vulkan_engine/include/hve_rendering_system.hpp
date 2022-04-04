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

class HveRenderingSystem
{
public:
  template<class T> using u_ptr = std::unique_ptr<T>;
  template<class T> using s_ptr = std::shared_ptr<T>;
  // share Renderable Component with its owner actor
  using map = std::unordered_map<id_t, s_ptr<HgeRenderableComponent>>;

  HveRenderingSystem(HveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
  virtual ~HveRenderingSystem();

  HveRenderingSystem(const HveRenderingSystem &) = delete;
  HveRenderingSystem &operator=(const HveRenderingSystem &) = delete;
  HveRenderingSystem(HveRenderingSystem &&) = default;
  HveRenderingSystem &operator=(HveRenderingSystem &&) = default;

  virtual void render(FrameInfo frameInfo) = 0;
  // to do : complete transfer
  void addRenderTarget(id_t id, s_ptr<HgeRenderableComponent> target)
  { renderTargetMap_m.emplace(id, std::move(target)); }
  void removeRenderTarget(id_t id)
  { renderTargetMap_m.erase(id); }

private:
  virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout){}
  virtual void createPipeline(VkRenderPass renderPass){}

protected:
  HveDevice& hveDevice_m;
  u_ptr<HvePipeline> hvePipeline_m = nullptr;
  VkPipelineLayout pipelineLayout_m;
  // hge
  map renderTargetMap_m;
};

} // namespace hnll