#pragma once

// hnll
#include <graphics/mesh_model.hpp>
#include <game/engine.hpp>
#include <game/component.hpp>
#include <game/components/renderable_component.hpp>

// std
#include <unordered_map>

namespace hnll::game {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class mesh_component : public renderable_component
{
  public:

    template <Actor A>
    static s_ptr<mesh_component> create(s_ptr<A>& owner_sp, const std::string& model_name)
    {
      auto& mesh_model = engine::get_mesh_model(model_name);
      auto mesh = std::make_shared<mesh_component>(owner_sp, mesh_model);
      owner_sp->set_renderable_component(mesh);
      return mesh;
    }

    template <Actor A>
    mesh_component(s_ptr<A>& owner_sp, graphics::mesh_model& _model)
    : renderable_component(owner_sp, utils::shading_type::MESH), model_(_model) {}
    ~mesh_component() override = default;

    // getter
    graphics::mesh_model& get_model() { return model_; }
    bool get_should_be_drawn() const                  { return should_be_drawn_; }
    unsigned get_face_count() const                   { return model_.get_face_count(); }
    // setter
    void set_should_be_drawn()     { should_be_drawn_ = true; }
    void set_should_not_be_drawn() { should_be_drawn_ = false;}
  private:
    // hnll::graphics::mesh_model can be shared all over a game
    graphics::mesh_model& model_;
    // represents weather its model should be drawn
    bool should_be_drawn_ = true;
};

using mesh_component_map = std::unordered_map<game::component_id, s_ptr<game::mesh_component>>;

} // namespace hnll::game