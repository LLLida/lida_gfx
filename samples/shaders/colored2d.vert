#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

void main() {
  gl_Position = vec4(in_position * 2.0 - 1.0, 0.0, 1.0);
  out_color = in_color;
}
