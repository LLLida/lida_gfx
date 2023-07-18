#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec3 in_normal;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform Camera {
  mat4 projview;
  mat4 view;
  vec3 camera_dir;
};

const vec3 sun_dir = normalize(-vec3(0.1, -1.0, 0.03));

void main() {
  float diffuse = max(dot(in_normal, sun_dir), 0.0);
  vec3 refl = reflect(-sun_dir, in_normal);
  float spec = pow(max(dot(camera_dir, refl), 0.0), 32.0);

  vec3 light = (0.01 + diffuse + spec) * vec3(1.0, 0.7, 0.7);

  // out_color = vec4(in_normal, 1.0);
  out_color = vec4(light, 1.0);
}
