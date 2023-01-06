#pragma once

// hnll
#include <utils/common_using.hpp>

namespace hnll::graphics {

namespace frame_anim_utils {

constexpr float MAX_FPS = 60.f;

struct dynamic_attributes {
  alignas(16) vec3 position;
  alignas(16) vec3 normal;
};
struct common_attributes {
  vec2 uv0;
  vec2 uv1;
  vec4 color;
};

} // namespace frame_anim_utils
} // namespace hnll::graphics