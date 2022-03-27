#version 450 

// build in type
// corners of triangles

// in keyword indicates that 'position' gets the data from a vertex buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

// ubo
// should be compatible with a description layout
layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projectionViewMatrix;
  vec4 ambientLightColor;
  vec3 lightPosition;
  vec4 lightColor;
} ubo;

layout(location = 0) out vec3 fragColor;

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
  // gl_Position = vec4(push.transform * positions + push.offset, 0.0, 1.0);
  gl_Position = ubo.projectionViewMatrix * positionWorld;

  vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

  vec3 directionToLight = ubo.lightPosition - positionWorld.xyz;
  float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
  vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0);

  fragColor = (diffuseLight + ambientLight) * color;
}