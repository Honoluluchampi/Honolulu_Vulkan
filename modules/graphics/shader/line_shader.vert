#version 450

layout (location = 0) out vec2 fragOffset;

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

layout(push_constant) uniform Push
{
  vec4 head_n_tail[2];
  vec4 color;
  float radius;
} push;

void line(vec3 s, vec3 e) {
  vec3 p = gl_FragCoord.xyz / resolution;
}

void main() 
{
  vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  // float push.radius = distance(push.head_n_tail[0], push.head_n_tail[1]);

  vec2 offSet[6] = {
    vec2(push.radius, push.radius), vec2(push.radius, -push.radius), vec2(-push.radius, push.radius),
    vec2(-push.radius, push.radius), vec2(push.radius, -push.radius), vec2(-push.radius, -push.radius)
  };

  fragOffset = offSet[gl_VertexIndex];
  
  vec3 positionWorld = push.head_n_tail[gl_VertexIndex % 2].xyz
                      + fragOffset.x * cameraRightWorld;
                      // + fragOffset.y * cameraUpWorld;

  gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}