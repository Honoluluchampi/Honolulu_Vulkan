#pragma once

// hnll
#include <graphics/model.hpp>
#include <game/component.hpp>
#include <game/components/renderable_component.hpp>

// std
#include <unordered_map>

namespace hnll {
namespace game {

template<class U> using u_ptr = std::unique_ptr<U>;
template<class S> using s_ptr = std::shared_ptr<S>;

class mesh_component : public renderable_component
{
  public:
    using map = std::unordered_map<component::id, s_ptr<mesh_component>>;
    // copy a passed shared_ptr
    mesh_component(const s_ptr<hnll::graphics::mesh_model>& spModel) : renderable_component(render_type::SIMPLE), model_sp_(spModel) {}
    mesh_component(s_ptr<hnll::graphics::mesh_model>&& spModel) : renderable_component(render_type::SIMPLE), model_sp_(std::move(spModel)) {}
    ~mesh_component(){}

    s_ptr<hnll::graphics::mesh_model>& get_model_sp() { return model_sp_; }
    template<class S>
    void set_mesh_model(S&& model) { model_sp_ = std::forward<S>(model); }
        
  private:
    // hnll::graphics::mesh_model can be shared all over a game
    s_ptr<hnll::graphics::mesh_model> model_sp_ = nullptr;
};

} // namespace game
} // namespace hnll