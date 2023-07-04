#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;

layout (location = 0) out vec3 out_normal;

layout (set = 0, binding = 0) uniform Camera {
  mat4 projview;
};

layout (push_constant) uniform Transform {
  mat4 model_matrix;
};

void main() {
  // gl_Position = projview * model_matrix * vec4(in_position, 1.0);
  gl_Position = projview * vec4(in_position, 1.0);
  // TODO: properly rotate normal.
  out_normal = in_normal;
}
