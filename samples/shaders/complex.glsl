// GLSL doesn't have builtin complex number operations. We define them
// ourselves.

#define PI 3.14159265358979

#define complex_t vec2

#define c_mul(a, b) complex_t(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define c_conj(a) complex_t(a.x,-a.y)
#define c_div(a, b) complex_t(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))

vec2 imag_exp(float j) {
  vec2 r;
  r.x = cos(j);
  r.y = sin(j);
  return r;
}
