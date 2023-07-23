#version 450
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in uint color;

layout (location = 0) out vec2 out_uv;
layout (location = 1) out vec4 out_color;

struct gl_PerVertex {
  vec4 gl_Position;
};

vec4 decompress_color(uint color) {
  // note: this doesn't consider endianness
  uint a = (color >> 24) & 255;
  uint b = (color >> 16) & 255;
  uint g = (color >> 8) & 255;
  uint r = color & 255;
  return vec4(r/255.0, g/255.0, b/255.0, a/255.0);
}

void main() {
  gl_Position = vec4(2.0 * pos - 1.0, 0, 1);
  out_uv = uv;
  out_color = decompress_color(color);
}
