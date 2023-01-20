#version 450

layout(location = 0) in vec3 vertex_position;

layout(location = 0) out vec4 out_color;

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projection;
  mat4 view;
  mat4 inv_view;
  vec4 ambientLightColor;
  PointLight pointLights[20];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  float height;
} push;

vec4 grid(vec3 frag_pos_3d, float scale, bool draw_axis) {
  vec2 coord = frag_pos_3d.xz * scale;
  vec2 derivative = fwidth(coord);
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
  float line = min(grid.x, grid.y);
  float minz = min(derivative.y, 1);
  float minx = min(derivative.x, 1);
  vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
  // z axis
  if (frag_pos_3d.x > -0.1 * minx && frag_pos_3d.x < 0.1 * minx)
    color.z = 1.0;
  if (frag_pos_3d.z > -0.1 * minz && frag_pos_3d.z < 0.1 * minz)
    color.x = 1.0;
  return color;
}

float compute_depth(vec3 pos) {
  vec4 clip_space_pos = ubo.projection * ubo.view * vec4(pos.xyz, 1.0);
  return (clip_space_pos.z / clip_space_pos.w);
}

const float near = 0.01;
const float far  = 100;

float compute_linear_depth(vec3 pos) {
  vec4 clip_space_pos = ubo.projection * ubo.view * vec4(pos.xyz, 1.0);
  float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
  float linear_depth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near));
  return linear_depth / far;
}

const float scale = 500.0;

void main() {
  out_color = vec4(1, 0, 0, 1);

  vec3 frag_pos_3d = vertex_position;
  
  gl_FragDepth = compute_depth(frag_pos_3d);
  
  float linear_depth = compute_linear_depth(frag_pos_3d);
  float fading = max(0, (0.5 - linear_depth));

  out_color = grid(frag_pos_3d, 1, true);
  out_color.a *= fading;
}