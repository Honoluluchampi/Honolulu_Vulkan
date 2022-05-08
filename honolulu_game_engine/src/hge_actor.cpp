#include <hge_actor.hpp>

namespace hnll {

 void HgeActor::update(float dt)
 {
   updateActor(dt);
   updateComponents(dt);
 }

 void HgeActor::updateComponents(float dt)
 {
  for (const auto& comp : uniqueComponents_m)
    comp->update(dt);
  for (const auto& comp : sharedComponents_m)
    comp->update(dt);
 }

#ifndef __IMGUI_DISABLED
 void HgeActor::updateImgui()
 {
   updateActorImgui();
   updateComponentsImgui();
 }

 void HgeActor::updateComponentsImgui()
 {
  for (const auto& comp : uniqueComponents_m)
    comp->updateImgui();
  for (const auto& comp : sharedComponents_m)
    comp->updateImgui();
 }
#endif

} // namespace hnll