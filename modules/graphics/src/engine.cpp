// this class organizes all the graphics specific features
// hnll
#include <graphics/engine.hpp>
#include <graphics/systems/point_light.hpp>
#include <graphics/systems/mesh_rendering_system.hpp>
#include <graphics/systems/line_rendering_system.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>

// lib
#include <glm/gtc/constants.hpp>

namespace hnll {

engine::engine(const char* windowName) : window_{WIDTH, HEIGHT, windowName}
{
  init();
}

engine::~engine()
{ }

// todo : separate into some functions
void engine::init()
{
  // // 2 uniform buffer descriptor
  global_pool_ = descriptor_pool::builder(device_)
    .set_max_sets(swap_chain::MAX_FRAMES_IN_FLIGHT)
    .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain::MAX_FRAMES_IN_FLIGHT)
    .build();

  // creating ubo for each frames version
  for (int i = 0; i < ubo_buffers_.size(); i++) {
    ubo_buffers_[i] = std::make_unique<buffer>(
      device_,
      sizeof(global_ubo),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    ubo_buffers_[i]->map();
  }

  // this is set layout of master system
  // enable ubo to be referenced by oall stages of a graphics pipeline
  global_set_layout_ = descriptor_set_layout::builder(device_)
    .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
    .build();
  // may add additional layout of child system

  for (int i = 0; i < global_descriptor_sets_.size(); i++) {
    auto bufferInfo = ubo_buffers_[i]->descriptor_info();
    descriptor_writer(*global_set_layout_, *global_pool_)
      .write_buffer(0, &bufferInfo)
      .build(global_descriptor_sets_[i]);
  }

  // create renderer system as local variable
  auto meshRenderingSystem = std::make_unique<mesh_rendering_system>(
    device_, 
    renderer_.get_swap_chain_render_pass(HVE_RENDER_PASS_ID),
    global_set_layout_->get_descriptor_set_layout()
  );

  auto pointLightSystem = std::make_unique<point_light_rendering_system>(
    device_, 
    renderer_.get_swap_chain_render_pass(HVE_RENDER_PASS_ID),
    global_set_layout_->get_descriptor_set_layout()
  );

  auto lineRenderingSystem = std::make_unique<line_rendering_system>(
    device_,
    renderer_.get_swap_chain_render_pass(HVE_RENDER_PASS_ID),
    global_set_layout_->get_descriptor_set_layout()
  );

  rendering_systems_.emplace
    (meshRenderingSystem->get_render_type(), std::move(meshRenderingSystem));
  rendering_systems_.emplace
    (pointLightSystem->get_render_type(), std::move(pointLightSystem));
  rendering_systems_.emplace
    (lineRenderingSystem->get_render_type(), std::move(lineRenderingSystem));
}

// each render systems automatically detect render target components
void engine::render(viewer_component& viewerComp)
{
  // returns nullptr if the swap chain is need to be recreated
  if (auto commandBuffer = renderer_.begin_frame()) {
    int frameIndex = renderer_.get_frame_index();

    frame_info frameInfo{
        frameIndex, 
        commandBuffer, 
        global_descriptor_sets_[frameIndex]
    };

    // update 
    ubo_.projection = viewerComp.get_projection();
    ubo_.view = viewerComp.get_view();
    ubo_buffers_[frameIndex]->write_to_buffer(&ubo_);
    ubo_buffers_[frameIndex]->flush();

    // rendering
    // TODO : configure hve_render_pass_id as the 
    // member and detect it in begin_swap_chain_render_pass func
    renderer_.begin_swap_chain_render_pass(commandBuffer, HVE_RENDER_PASS_ID);
    // programmable stage of rendering
    // system can now access gameobjects via frameInfo
    for (auto& system : rendering_systems_)
      system.second->render(frameInfo);

    renderer_.end_swap_chain_render_pass(commandBuffer);
    renderer_.end_frame();
  }
}

void engine::remove_renderable_component_without_owner(render_type type, component::id id)
{
  rendering_systems_[type]->remove_render_target(id);
}
} // namespace hve