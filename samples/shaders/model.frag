#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform Camera {
  mat4 projview;
  mat4 view;
  vec3 camera_dir;
};

const vec3 sun_dir = normalize(vec3(0.1, 1.0, 0.03));

void main() {

  const float PI = 3.14159265358979;
  const float shiny = 16.0;
  const float energ_conserv = ( 8.0 + shiny ) / ( 8.0 * PI );

  float diffuse = max(dot(in_normal, sun_dir), 0.0);
  vec3 halfway = normalize(sun_dir + camera_dir);
  float spec = energ_conserv * pow(max(dot(in_normal, halfway), 0.0), shiny);

  vec3 light = (0.01 + diffuse + spec) * in_color;

  // out_color = vec4(in_normal, 1.0);
  out_color = vec4(light, 1.0);
}
