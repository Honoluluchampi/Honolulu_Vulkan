// hnll
#include <game/actor.hpp>
#include <game/engine.hpp>

namespace hnll::game {

s_ptr<actor> actor::create()
{
  auto _actor = std::make_shared<actor>();
  // register it to the actor map
  hnll::game::engine::add_actor(_actor);
  return _actor;
}

actor::actor()
{ 
  static actor_id id = 0;
  // add automatically
  id_ = id++;
  transform_sp_ = std::make_shared<hnll::utils::transform>();
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

} // namespace hnll::game