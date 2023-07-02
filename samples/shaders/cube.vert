#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

layout (set = 0, binding = 0) uniform Camera {
  mat4 projview;
};

void main() {
  // mat4 b;
  // for (int i = 0; i < 4; i++)
  //   for (int j = 0; j < 4; j++)
  //     b[i][j] = projview[j][i];
  gl_Position = projview * vec4(in_position, 1.0);
  out_color = in_color;
}
