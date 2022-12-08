#pragma once

namespace hnll::utils {

// rendering order matters for alpha blending
// solid object should be drawn first, then transparent object should be drawn after that
enum class rendering_type : uint32_t
{
  MESH         = 10,
  MESHLET      = 20,
  POINT_LIGHT  = 40,
  LINE         = 0,
  WIRE_FRUSTUM = 30,
  GRID         = 50
};

} // namespace hnll::utils