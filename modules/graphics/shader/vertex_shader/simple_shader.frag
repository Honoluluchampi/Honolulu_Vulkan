#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 0) out vec4 outColor;

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
  mat4 modelMatrix;
  vec3 normalMatrix;
} push;

void main() 
{
  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 specular_light = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 camera_pos_world = ubo.inv_view[3].xyz;
  vec3 view_direction = normalize(camera_pos_world - fragPosWorld);

  for (int i = 0; i < ubo.numLights; i++) {
    PointLight light = ubo.pointLights[i];
    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

    directionToLight = normalize(directionToLight);
    float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;

    diffuseLight += intensity * cosAngIncidence;
    
    // specular light
    vec3 half_angle = normalize(directionToLight + view_direction);
    float blinn_term = dot(surfaceNormal, half_angle);
    blinn_term = clamp(blinn_term, 0 , 1);
    blinn_term = pow(blinn_term, 500);
    specular_light += light.color.xyz * attenuation * blinn_term;
  }

  // rgba
  outColor = vec4(diffuseLight * fragColor + specular_light * fragColor, 1.0);
}