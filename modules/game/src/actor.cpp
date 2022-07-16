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

} // namespace game
} // namespace hnll