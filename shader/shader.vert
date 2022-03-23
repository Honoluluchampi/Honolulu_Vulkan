#version 450 

// build in type
// corners of triangles
vec2 positions[3] = vec2[] (
  vec2(0.0, -0.5),
  vec2(0.5, 0.5),
  vec2(0.5, -0.5)
);

// executed for each vertex
void main()
{
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}