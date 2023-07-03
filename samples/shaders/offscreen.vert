#version 450
#extension GL_GOOGLE_include_directive : enable

const vec2 positions[6] = vec2[](vec2(-1.0, -1.0),
                                 vec2(-1.0, 1.0),
                                 vec2(1.0, 1.0),
                                 vec2(1.0, -1.0),
                                 vec2(-1.0, -1.0),
                                 vec2(1.0, 1.0)
                                 );

layout (location = 0) out vec2 uv;

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  uv = (positions[gl_VertexIndex] + 1.0) * 0.5;
}
