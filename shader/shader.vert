#version 450 

// build in type
// corners of triangles

// in keyword indicates that 'position' gets the data from a vertex buffer
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
  // more efficient than cpu's projection
  // mat4 projectionMatrix;
  mat4 transform;
  vec3 color;
} push;

// executed for each vertex
void main()
{
  // gl_Position = vec4(push.transform * positions + push.offset, 0.0, 1.0);
  gl_Position = push.transform * vec4(position, 1.0);
  fragColor = color;
}