// this class organizes all the graphics specific features
// hnll
#include <game/graphics_engine.hpp>
#include <game/shading_system.hpp>

//std
#include <stdexcept>
#include <array>
#include <iostream>

// lib
#include <glm/gtc/constants.hpp>

namespace hnll::game {

// static members
graphics_engine::shading_system_map    graphics_engine::shading_systems_;

graphics_engine::graphics_engine(const char* window_name, utils::rendering_type rendering_type)
{
  window_ = std::make_unique<graphics::window>(WIDTH, HEIGHT, window_name);

  device_ = std::make_unique<graphics::device>(
    *window_,
    rendering_type
  );

  renderer_ = std::make_unique<graphics::renderer>(*window_, *device_);

  init();
}

graphics_engine::~graphics_engine()
{ }

// todo : separate into some functions
void graphics_engine::init()
{
  // // 2 uniform buffer descriptor
  global_pool_ = graphics::descriptor_pool::builder(*device_)
    .set_max_sets(graphics::swap_chain::MAX_FRAMES_IN_FLIGHT)
    .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, graphics::swap_chain::MAX_FRAMES_IN_FLIGHT)
    .build();

  // creating ubo for each frame version
  for (int i = 0; i < ubo_buffers_.size(); i++) {
    ubo_buffers_[i] = std::make_unique<graphics::buffer>(
      *device_,
      sizeof(utils::global_ubo),
      1,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    ubo_buffers_[i]->map();
  }

  // this is set layout of master system
  // enable ubo to be referenced by oall stages of a graphics pipeline
  global_set_layout_ = graphics::descriptor_set_layout::builder(*device_)
    .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_MESH_BIT_NV)
    .build();
  // may add additional layout of child system

  for (int i = 0; i < global_descriptor_sets_.size(); i++) {
    auto bufferInfo = ubo_buffers_[i]->descriptor_info();
    graphics::descriptor_writer(*global_set_layout_, *global_pool_)
      .write_buffer(0, &bufferInfo)
      .build(global_descriptor_sets_[i]);
  }

  configure_shading_system();
}

// each render systems automatically detect render target components
void graphics_engine::render(const utils::viewer_info& _viewer_info, const utils::frustum_info& _frustum_info)
{
  // returns nullptr if the swap chain is need to be recreated
  if (auto command_buffer = renderer_->begin_frame()) {
    int frame_index = renderer_->get_frame_index();

    utils::frame_info frame_info{
        frame_index, 
        command_buffer, 
        global_descriptor_sets_[frame_index],
    };

    // update 
    ubo_.projection   = _viewer_info.projection;
    ubo_.view         = _viewer_info.view;
    ubo_.inverse_view = _viewer_info.inverse_view;
    update_ubo(frame_index);

    // rendering
    // TODO : configure hve_render_pass_id as the 
    // member and detect it in begin_swap_chain_render_pass func
    renderer_->begin_swap_chain_render_pass(command_buffer, HVE_RENDER_PASS_ID);
    // programmable stage of rendering
    // system can now access game objects via frame_info

    for (auto& system : shading_systems_) {
      system.second->render(frame_info);
    }

    renderer_->end_swap_chain_render_pass(command_buffer);
    renderer_->end_frame();
  }
}

void graphics_engine::configure_shading_system()
{
  shading_system::set_default_render_pass(renderer_->get_swap_chain_render_pass(HVE_RENDER_PASS_ID));
  shading_system::set_global_desc_set_layout(global_set_layout_->get_descriptor_set_layout());
}

void graphics_engine::add_shading_system(u_ptr<shading_system> &&system)
{ shading_systems_[static_cast<uint32_t>(system->get_rendering_type())] = std::move(system); }

void graphics_engine::add_renderable_component(const renderable_component& comp)
{ shading_systems_[static_cast<uint32_t>(comp.get_render_type())]->add_render_target(comp.get_id(), comp); }

void graphics_engine::remove_renderable_component(const renderable_component& comp)
{ shading_systems_[static_cast<uint32_t>(comp.get_render_type())]->remove_render_target(comp.get_id()); }

void graphics_engine::remove_renderable_component(utils::rendering_type type, component_id id)
{ shading_systems_[static_cast<uint32_t>(type)]->remove_render_target(id); }

} // namespace hnll::graphics