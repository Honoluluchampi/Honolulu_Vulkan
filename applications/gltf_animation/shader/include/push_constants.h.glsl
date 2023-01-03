// push_constant holds variables which chage frame by frame
layout (push_constant) uniform PushConstants {
  mat4 world_matrix;
  mat4 projection_matrix;
  mat4 view_matrix;
  vec4 camera_pos;
  vec4 light_pos;
} push_constants;

// binding holds static variables (don't change frame by frame)
// each varialbe corresponds to the glTF's material variable
layout (binding = 0) uniform Material {
  vec4 base_color;
  vec4 emissive;
  float roughness;
  float metalness;
  float normal_scale;
  float occlusion_strength;
} materials;