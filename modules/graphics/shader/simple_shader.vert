#version 450 

// build in type
// corners of triangles

// in keyword indicates that 'position' gets the data from a vertex buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

// ubo
// should be compatible with a description layout
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

// compatible with a renderer system
layout(push_constant) uniform Push {
  // more efficient than cpu's projection
  // mat4 projectionMatrix;
  mat4 modelMatrix; // projection * view * model
  mat4 normalMatrix;
} push;

// executed for each vertex
void main()
{
  vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
  gl_Position = ubo.projection * ubo.view * positionWorld;

  fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
  fragPosWorld = positionWorld.xyz;
  fragColor = color;
}