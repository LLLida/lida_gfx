#version 450
#extension GL_GOOGLE_include_directive : enable

layout (set = 0, binding = 0) uniform sampler2D offscreen_image;

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 out_color;

void main() {
  out_color = texture(offscreen_image, uv);
}
