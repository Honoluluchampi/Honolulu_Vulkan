#version 460

layout(location = 0) out vec4 FragColor;

layout (location = 0) in PerVertexData {
    vec4 color;
} frag_in;

// bindings
// ------------------------------------------------------------------------
// scene info

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
  vec4 ambient_light_color;
  PointLight point_lights[20];
  int num_lights;
} ubo;

void main() {
    FragColor = frag_in.color;
}