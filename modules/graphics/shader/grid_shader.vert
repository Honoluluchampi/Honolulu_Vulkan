#version 450

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projection;
  mat4 view;
  vec4 ambientLightColor;
  PointLight pointLights[20];
  int numLights;
} ubo;

layout(push_constant) uniform Push {
  float height;
} push;

vec3 grid_plane[6] = vec3 [](
  vec3(1, 1, 0),
  vec3(-1, -1, 0),
  vec3(-1, 1, 0),
  vec3(-1, -1, 0),
  vec3(1, 1, 0),
  vec3(1, -1, 0)
);

void main() {
  gl_Position = ubo.projection * ubo.view * vec4(grid_plane[gl_VertexIndex].xyz, 1);
}