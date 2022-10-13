#version 450

layout (location = 0) in vec3 frustum_color;
layout (location = 1) in vec2 uv;
layout (location = 0) out vec4 out_color;

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
  mat4 modelMatrix;
} push;

void main() 
{
  const float thresh = 0.005;
  bvec2 to_discard = greaterThan(fract(uv), vec2(thresh, thresh));

  if (all (to_discard))
    discard;
  out_color = vec4(frustum_color, 1.0);
}