#pragma once

// hnll
#include <graphics/meshlet_model.hpp>
#include <game/engine.hpp>
#include <game/component.hpp>
#include <game/components/renderable_component.hpp>
#include <utils/rendering_utils.hpp>

// std
#include <unordered_map>

namespace hnll::game {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class meshlet_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<meshlet_component> create(s_ptr<A>& owner_sp, const std::string& model_name)
    {
      auto& meshlet_model = engine::get_meshlet_model(model_name);
      auto mesh = std::make_shared<meshlet_component>(owner_sp, meshlet_model);
      owner_sp->set_renderable_component(mesh);
      return mesh;
    }
    template <Actor A>
    meshlet_component(s_ptr<A>& owner_sp, graphics::meshlet_model& model)
      : renderable_component(owner_sp, utils::shading_type::MESHLET), model_(model) {}
    ~meshlet_component() override = default;

    void bind_and_draw(
      VkCommandBuffer command_buffer,
      std::vector<VkDescriptorSet>&& desc_sets,
      VkPipelineLayout& pipeline_layout)
    {
      model_.bind(command_buffer, desc_sets, pipeline_layout);
      model_.draw(command_buffer);
    }

    // getter
    uint32_t get_meshlet_count() const { return model_.get_meshlets_count(); }

  private:
    // hnll::graphics::mesh_model can be shared all over a game
    graphics::meshlet_model& model_;
};

using meshlet_component_map = std::unordered_map<game::component_id, u_ptr<game::meshlet_component>>;

} // namespace hnll::game