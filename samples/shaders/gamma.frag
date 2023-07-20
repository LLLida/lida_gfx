#version 450
#extension GL_GOOGLE_include_directive : enable

layout (set = 0, binding = 0) uniform sampler2D offscreen_image;

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 out_color;

float lerp(float a, float b, float f) {
  return a + f * (b - a);
}

vec3 lerp(vec3 a, vec3 b, vec3 f) {
  return a + f * (b - a);
}

float tonemap_inner(float x) {
  float z = pow(x, 2.35);
  return z / (pow(z, 1.0) * 1 + 0.8099);
}

vec3 tonemap(vec3 color, float exposure) {
  float max_component = max(color.x, max(color.y, color.z));
  vec3 ratio = color / max_component;
  float tonemapped_max = tonemap_inner(max_component);
  const float saturation = 1.0;
  ratio = pow(ratio, vec3(saturation));
  ratio = lerp(ratio, vec3(1.0), vec3(pow(tonemapped_max, 10.0)));
  ratio = pow(ratio, vec3(1.0));
  return clamp(ratio * tonemapped_max, vec3(0.0), vec3(1.0));
}

void main() {
  out_color = vec4(tonemap(texture(offscreen_image, uv).xyz, 1.0), 1.0);
}
