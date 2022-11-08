#pragma once

// hnll
#include <graphics/meshlet_model.hpp>
#include <game/component.hpp>
#include <game/components/renderable_component.hpp>

// std
#include <unordered_map>

namespace hnll::game {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class meshlet_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<meshlet_component> create(s_ptr<A>& owner_sp, s_ptr<hnll::graphics::meshlet_model>&& model_sp)
    {
      auto mesh = std::make_shared<meshlet_component>(owner_sp);
      mesh->set_mesh_model(std::move(model_sp));
      owner_sp->set_renderable_component(mesh);
      return mesh;
    }
    template <Actor A>
    meshlet_component(s_ptr<A>& owner_sp) : renderable_component(owner_sp, render_type::MESHLET) {}
    ~meshlet_component() override = default;

    // getter
    s_ptr<hnll::graphics::meshlet_model>& get_model_sp() { return model_sp_; }
    // setter
    void set_meshlet_model(s_ptr<hnll::graphics::meshlet_model>&& model) { model_sp_ = std::move(model); }
  private:
    // hnll::graphics::mesh_model can be shared all over a game
    s_ptr<hnll::graphics::meshlet_model> model_sp_ = nullptr;
};

using mesh_component_map = std::unordered_map<game::component_id, s_ptr<game::meshlet_component>>;

} // namespace hnll::game