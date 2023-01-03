#pragma once

// hnll
#include <game/components/renderable_component.hpp>

namespace hnll::game {

// pointLight can be treated as gameObject
struct point_light_info { float light_intensity = 1.0f; };

class point_light_component : public renderable_component
{
  public:
    template <Actor A>
    static s_ptr<point_light_component> create(s_ptr<A>& owner, float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f))
    {
      auto light = std::make_shared<point_light_component>(owner);
      light->color_ = color;
      light->set_scale(glm::vec3(radius, radius, radius));
      light->light_info_.light_intensity = intensity;
      return light;
    }

    template <Actor A>
    point_light_component(s_ptr<A>& owner) : renderable_component(owner, utils::shading_type::POINT_LIGHT) {}
    ~point_light_component() override = default;
    point_light_component(const point_light_component&) = delete;
    point_light_component& operator=(const point_light_component&) = delete;

    // getter
    point_light_info get_light_info() { return light_info_; }
    glm::vec3 get_color() { return color_; }

    // setter
    void set_color(const glm::vec3& color) { color_ = color; }
    void set_light_info(const point_light_info& info) { light_info_ = info; }
    void set_radius(float radius) { this->set_scale(glm::vec3(radius, radius, radius)); }
  
  private:
    point_light_info light_info_{};
    glm::vec3 color_{};
};

} // namespace hnll::game