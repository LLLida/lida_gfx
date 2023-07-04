#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 in_normal;

layout (location = 0) out vec4 out_color;

void main() {
  out_color = vec4(in_normal, 1.0);
}
