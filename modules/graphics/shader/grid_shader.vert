#version 450

layout(location = 0) out vec3 vertex_position;


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

vec3 grid_plane[6] = vec3 [](
  vec3( 1, 0,  1),
  vec3(-1, 0, -1),
  vec3(-1, 0,  1),
  vec3(-1, 0, -1),
  vec3( 1, 0,  1),
  vec3( 1, 0, -1)
);

const float scale = 500.0;

void main() {
  vec3 position = scale * grid_plane[gl_VertexIndex].xyz;
  position.y = -push.height;
  vertex_position = position;
  gl_Position = ubo.projection * ubo.view * vec4(position, 1.0);
}