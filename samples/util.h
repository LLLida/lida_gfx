/*
  Some common structures for 3D graphics.

  Vectors, Matrices, Camera.
 */
#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

  float x;
  float y;
  float z;

} Vec3;

typedef struct {

  float x;
  float y;
  float z;
  float w;

} Vec4;

// Column-major 4x4 matrix
typedef struct {

  float m00, m10, m20, m30;
  float m01, m11, m21, m31;
  float m02, m12, m22, m32;
  float m03, m13, m23, m33;

} Mat4;

static float
radians(float degrees)
{
  const float pi = 3.14159265358979f;
  return degrees * pi / 180.0f;
}

static Vec3
vec3_add(Vec3 l, Vec3 r)
{
  return (Vec3) { .x = l.x + r.x, .y = l.y + r.y, .z = l.z + r.z };
}

static Vec3
vec3_sub(Vec3 l, Vec3 r)
{
  return (Vec3) { .x = l.x - r.x, .y = l.y - r.y, .z = l.z - r.z };
}

static float
vec3_dot(Vec3 l, Vec3 r)
{
  return l.x*r.x + l.y*r.y + l.z*r.z;
}

static Vec3
vec3_cross(Vec3 l, Vec3 r)
{
  return (Vec3) { l.y*r.z - l.z*r.y, l.z*r.x - l.x*r.z, l.x*r.y - l.y*r.x };
}

static Vec3
vec3_normalize(Vec3 in)
{
  Vec3 out;
  float inv_len = 1.0f / sqrtf(vec3_dot(in, in));
  out.x = in.x * inv_len;
  out.y = in.y * inv_len;
  out.z = in.z * inv_len;
  return out;
}

static Vec4
vec4_add(Vec4 l, Vec4 r)
{
  return (Vec4) { .x = l.x + r.x, .y = l.y + r.y, .z = l.z + r.z, .w = l.w + r.w };
}

static Vec4
vec4_sub(Vec4 l, Vec4 r)
{
  return (Vec4) { .x = l.x - r.x, .y = l.y - r.y, .z = l.z - r.z, .w = l.w - r.w };
}

static float
vec4_dot(Vec4 l, Vec4 r)
{
  return l.x*r.x + l.y*r.y + l.z*r.z + l.w*r.w;
}

static Mat4
mat4_identity() {
  return (Mat4) {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
}

#define MAT4_XMACRO()\
  X(0, 0); X(0, 1); X(0, 2); X(0, 3);\
  X(1, 0); X(1, 1); X(1, 2); X(1, 3);\
  X(2, 0); X(2, 1); X(2, 2); X(2, 3);\
  X(3, 0); X(3, 1); X(3, 2); X(3, 3)

static Mat4
mat4_add(Mat4 lhs, Mat4 rhs)
{
  Mat4 out;
#define X(i, j) out.m##i##j = lhs.m##i##j + rhs.m##i##j
  MAT4_XMACRO();
#undef X
  return out;
}

static Mat4
mat4_sub(const Mat4 lhs, const Mat4 rhs)
{
  Mat4 out;
#define X(i, j) out.m##i##j = lhs.m##i##j - rhs.m##i##j
  MAT4_XMACRO();
#undef X
  return out;
}

static Mat4
mat4_mul(const Mat4 lhs, Mat4 rhs)
{
  Mat4 out;
  out.m00 = lhs.m00*rhs.m00 + lhs.m01*rhs.m10 + lhs.m02*rhs.m20 + lhs.m03*rhs.m30;
  out.m01 = lhs.m00*rhs.m01 + lhs.m01*rhs.m11 + lhs.m02*rhs.m21 + lhs.m03*rhs.m31;
  out.m02 = lhs.m00*rhs.m02 + lhs.m01*rhs.m12 + lhs.m02*rhs.m22 + lhs.m03*rhs.m32;
  out.m03 = lhs.m00*rhs.m03 + lhs.m01*rhs.m13 + lhs.m02*rhs.m23 + lhs.m03*rhs.m33;
  out.m10 = lhs.m10*rhs.m00 + lhs.m11*rhs.m10 + lhs.m12*rhs.m20 + lhs.m13*rhs.m30;
  out.m11 = lhs.m10*rhs.m01 + lhs.m11*rhs.m11 + lhs.m12*rhs.m21 + lhs.m13*rhs.m31;
  out.m12 = lhs.m10*rhs.m02 + lhs.m11*rhs.m12 + lhs.m12*rhs.m22 + lhs.m13*rhs.m32;
  out.m13 = lhs.m10*rhs.m03 + lhs.m11*rhs.m13 + lhs.m12*rhs.m23 + lhs.m13*rhs.m33;
  out.m20 = lhs.m20*rhs.m00 + lhs.m21*rhs.m10 + lhs.m22*rhs.m20 + lhs.m23*rhs.m30;
  out.m21 = lhs.m20*rhs.m01 + lhs.m21*rhs.m11 + lhs.m22*rhs.m21 + lhs.m23*rhs.m31;
  out.m22 = lhs.m20*rhs.m02 + lhs.m21*rhs.m12 + lhs.m22*rhs.m22 + lhs.m23*rhs.m32;
  out.m23 = lhs.m20*rhs.m03 + lhs.m21*rhs.m13 + lhs.m22*rhs.m23 + lhs.m23*rhs.m33;
  out.m30 = lhs.m30*rhs.m00 + lhs.m31*rhs.m10 + lhs.m32*rhs.m20 + lhs.m33*rhs.m30;
  out.m31 = lhs.m30*rhs.m01 + lhs.m31*rhs.m11 + lhs.m32*rhs.m21 + lhs.m33*rhs.m31;
  out.m32 = lhs.m30*rhs.m02 + lhs.m31*rhs.m12 + lhs.m32*rhs.m22 + lhs.m33*rhs.m32;
  out.m33 = lhs.m30*rhs.m03 + lhs.m31*rhs.m13 + lhs.m32*rhs.m23 + lhs.m33*rhs.m33;
  return out;
}

static Mat4
translation_matrix(Vec3 pos)
{
  Mat4 out = mat4_identity();
  out.m30 = pos.x;
  out.m31 = pos.y;
  out.m32 = pos.z;
  return out;
}

static Mat4
orthographic_matrix(float left, float right, float bottom, float top,
                    float z_near, float z_far)
{
  Mat4 out = { 0 };
  out.m00 = 2.0f / (right - left);
  out.m11 = -2.0f / (top - bottom);
  out.m22 = 1.0f / (z_far - z_near);
  out.m03 = -(right+left) / (right-left);
  out.m13 = -(top+bottom) / (top-bottom);
  out.m23 = z_far / (z_far - z_near);
  out.m33 = 1.0f;
  return out;
}

static Mat4
perspective_matrix(float fov_y, float aspect_ratio, float z_near)
{
  Mat4 out = { 0 };
  float f = 1.0f / tan(fov_y * 0.5f);
  out.m00 = f / aspect_ratio;
  out.m11 = -f;
  out.m32 = -1.0f;
  out.m23 = z_near;
  return out;
}

static Mat4
look_at_matrix(Vec3 eye, Vec3 target, Vec3 up)
{
  Vec3 dir = vec3_normalize(vec3_sub(eye, target));
  Vec3 s = vec3_cross(dir, up);
  Vec3 u = vec3_cross(s, dir);
  Vec3 t = { vec3_dot(eye, s), vec3_dot(eye, u), vec3_dot(eye, dir) };
  return (Mat4) {
    s.x,  u.x,  dir.x, 0.0f,
    s.y,  u.y,  dir.y, 0.0f,
    s.z,  u.z,  dir.z, 0.0f,
    -t.x, -t.y, -t.z,  1.0f
  };
}

static int log_enable_colors = 1;

/**
   Prints validation layer messages.
   */
static void
log_func(int sev, const char* fmt, ...)
{
  const char* const colors[] = {
    "\x1b[36m",
    "\x1b[32m",
    "\x1b[33m",
    "\x1b[31m",
  };
  if (log_enable_colors) {
    printf("%s", colors[sev]);
  }
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  if (log_enable_colors) {
    printf("\x1b[0m\n");
  } else {
    printf("\n");
  }
}

#ifdef __cplusplus
}
#endif
