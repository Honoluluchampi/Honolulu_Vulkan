// hnll
#include <game/actors/point_light_manager.hpp>
#include <graphics/frame_info.hpp>

namespace hnll {

point_light_manager::point_light_manager(global_ubo& ubo) : 
  actor(), ubo_(ubo)
{
}

void point_light_manager::update_actor(float dt)
{
  int lightIndex = 0;
  auto lightRotation = glm::rotate(glm::mat4(1), dt, {0.f, -1.0f, 0.f});
  for (auto& kv : light_comp_map_) {
    auto lightComp = kv.second;
    // update light position
    // lightComp->get_transform().translation = glm::vec3(lightRotation * glm::vec4(lightComp->get_transform().translation, 1.f));
    // copy light data to ubo 
    ubo_.pointLights[lightIndex].position = glm::vec4(lightComp->get_transform().translation, 1.f);
    ubo_.pointLights[lightIndex].color = glm::vec4(lightComp->get_color(), lightComp->get_light_info().light_intensity);
    lightIndex++;
  }
  ubo_.numLights = lightIndex;
}

} // namespace hnll