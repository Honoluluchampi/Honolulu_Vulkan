#pragma once

#include <hve_window.hpp>
#include <hve_pipeline.hpp>
#include <hve_device.hpp>
#include <hve_game_object.hpp>
#include <hve_renderer.hpp>

// lib
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace hve {

struct SimplePushConstantData
{
  glm::mat2 transform_m{1.0f};
  glm::vec2 offset_m;
  // to align data offsets with shader
  alignas(16) glm::vec3 color_m;
};

class HveApp
{
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    HveApp();
    ~HveApp();

    HveApp(const HveApp &) = delete;
    HveApp &operator= (const HveApp &) = delete;

    void run();
    
  private:
    void loadGameObjects();
    void createPipelineLayout();
    void createPipeline();
    void renderGameObjects(VkCommandBuffer commandBuffer);

    HveWindow hveWindow_m {WIDTH, HEIGHT, "Honolulu Vulkan"};
    HveDevice hveDevice_m {hveWindow_m};
    HveRenderer hveRenderer_m {hveWindow_m, hveDevice_m};
    std::unique_ptr<HvePipeline> hvePipeline_m;
    VkPipelineLayout pipelineLayout_m;

    // game
    std::vector<HveGameObject> gameObjects_m;
};

} // namespace hv