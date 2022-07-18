// hnll
#include <game/actor.hpp>

namespace hnll {
namespace game {

actor::actor()
{ 
  static id id = 0;
  // add automatically
  id_ = id++;
}

void actor::update(float dt)
{
  update_actor(dt);
  update_components(dt);
}

void actor::update_components(float dt)
{
for (const auto& comp : unique_components_)
  comp->update(dt);
for (const auto& comp : shared_components_)
  comp->update(dt);
}

#ifndef IMGUI_DISABLED
 void actor::update_gui()
 {
   update_actor_imgui();
   update_component_imgui();
 }

 void actor::update_component_imgui()
 {
  for (const auto& comp : unique_components_)
    comp->update_gui();
  for (const auto& comp : shared_components_)
    comp->update_gui();
 }
#endif

void actor::set_renderable_component(s_ptr<renderable_component> &&comp)
{
  // replace own transform with comp's
  if (comp->get_transform_sp() != nullptr) transform_sp_ = comp->get_transform_sp();
  else if (transform_sp_ == nullptr) {
    transform_sp_ = std::make_shared<hnll::utils::transform>();
    // use same transform
    comp->set_transform_sp(transform_sp_);
  }
  renderable_component_ = std::move(comp);
}

void actor::set_renderable_component(const s_ptr<renderable_component> &comp)
{
  if (comp->get_transform_sp() != nullptr) transform_sp_ = comp->get_transform_sp();
  else if (transform_sp_ == nullptr) {
    transform_sp_ = std::make_shared<hnll::utils::transform>();
    comp->set_transform_sp(transform_sp_);
  }
  renderable_component_ = comp;
}

s_ptr<hnll::utils::transform> actor::get_transform_sp()
{
  if (transform_sp_ == nullptr) transform_sp_ = std::make_shared<hnll::utils::transform>();
  return transform_sp_;
}

} // namespace game
} // namespace hnll