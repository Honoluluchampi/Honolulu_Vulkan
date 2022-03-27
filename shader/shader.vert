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
  vec3 directionToLight;
} ubo;

layout(location = 0) out vec3 fragColor;

// compatible with a renderer system
layout(push_constant) uniform Push {
  // more efficient than cpu's projection
  // mat4 projectionMatrix;
  mat4 modelMatrix; // projection * view * model
  mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

// executed for each vertex
void main()
{
  // gl_Position = vec4(push.transform * positions + push.offset, 0.0, 1.0);
  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

  vec3 noramlWorldSpace = normalize(mat3(push.normalMatrix) * normal);

  float lightIntensity = AMBIENT + max(dot(noramlWorldSpace, ubo.directionToLight), 0);
  fragColor = lightIntensity * color;
}