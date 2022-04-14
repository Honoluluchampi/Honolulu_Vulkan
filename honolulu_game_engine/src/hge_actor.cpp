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
} // namespace hnll