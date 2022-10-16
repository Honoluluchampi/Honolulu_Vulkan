#version 450

layout(location = 0) out vec3 near_point;
layout(location = 1) out vec3 far_point;


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
  vec3(1, 1, 0),
  vec3(-1, -1, 0),
  vec3(-1, 1, 0),
  vec3(-1, -1, 0),
  vec3(1, 1, 0),
  vec3(1, -1, 0)
);

vec3 unproject_point(float x, float y, float z, mat4 view, mat4 projection) {
  mat4 view_inv = inverse(view);
  mat4 proj_inv = inverse(projection);
  vec4 unprojected_point = view_inv * proj_inv * vec4(x, y, z, 1.0);
  return unprojected_point.xyz / unprojected_point.w;
}

void main() {
  vec3 p = grid_plane[gl_VertexIndex].xyz;
  near_point = unproject_point(p.x, p.y, 0.0, ubo.view, ubo.projection).xyz;
  far_point  = unproject_point(p.x, p.y, 1.0, ubo.view, ubo.projection).xyz;
  gl_Position = vec4(p, 1.0);
}