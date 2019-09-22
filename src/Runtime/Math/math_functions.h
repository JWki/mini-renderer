#pragma once
#include "math_types.h"

#include <math.h>
#include <memory.h>

namespace gt
{
    namespace math
    {
        // -----------------------------------------------------------
        // -----------------------------------------------------------

        inline vec2f_t operator + (vec2f_t const& lhs, vec2f_t const& rhs);
        inline vec2f_t operator - (vec2f_t const& lhs, vec2f_t const& rhs);
        inline vec2f_t operator / (vec2f_t const& lhs, vec2f_t const& rhs);
        inline vec2f_t operator * (vec2f_t const& lhs, vec2f_t const& rhs);

        inline vec3f_t operator + (vec3f_t const& lhs, vec3f_t const& rhs);
        inline vec3f_t operator - (vec3f_t const& lhs, vec3f_t const& rhs);
        inline vec3f_t operator / (vec3f_t const& lhs, vec3f_t const& rhs);
        inline vec3f_t operator * (vec3f_t const& lhs, vec3f_t const& rhs);

        inline vec4f_t operator + (vec4f_t const& lhs, vec4f_t const& rhs);
        inline vec4f_t operator - (vec4f_t const& lhs, vec4f_t const& rhs);
        inline vec4f_t operator / (vec4f_t const& lhs, vec4f_t const& rhs);
        inline vec4f_t operator * (vec4f_t const& lhs, vec4f_t const& rhs);

        inline vec2f_t operator + (vec2f_t const& lhs, float rhs);
        inline vec2f_t operator - (vec2f_t const& lhs, float rhs);
        inline vec2f_t operator * (vec2f_t const& lhs, float rhs);
        inline vec2f_t operator / (vec2f_t const& lhs, float rhs);

        inline vec3f_t operator + (vec3f_t const& lhs, float rhs);
        inline vec3f_t operator - (vec3f_t const& lhs, float rhs);
        inline vec3f_t operator * (vec3f_t const& lhs, float rhs);
        inline vec3f_t operator / (vec3f_t const& lhs, float rhs);

        inline vec4f_t operator + (vec4f_t const& lhs, float rhs);
        inline vec4f_t operator - (vec4f_t const& lhs, float rhs);
        inline vec4f_t operator * (vec4f_t const& lhs, float rhs);
        inline vec4f_t operator / (vec4f_t const& lhs, float rhs);

        inline vec2f_t operator - (vec2f_t const& vec);
        inline vec3f_t operator - (vec3f_t const& vec);
        inline vec4f_t operator - (vec4f_t const& vec);

        // -----------------------------------------------------------

        inline vec4f_t operator * (mat4x4f_t const& lhs, vec4f_t const& rhs);
        inline vec4f_t operator * (quatf_t const& lhs, vec4f_t const& rhs);

        inline quatf_t operator * (quatf_t const& lhs, quatf_t const& rhs);
        inline mat4x4f_t operator * (mat4x4f_t const& lhs, mat4x4f_t const& rhs);

        inline vec3f_t transform_pos(mat4x4f_t const& transform, vec3f_t const& position);
        inline vec3f_t transform_dir(mat4x4f_t const& transform, vec3f_t const& direction);

        inline mat4x4f_t    quat_to_mat(quatf_t const& quat);
        inline quatf_t      quat_from_mat(mat4x4f_t const& mat);

        inline mat4x4f_t make_translation(vec3f_t const& translation);
        inline mat4x4f_t make_scale(vec3f_t const& scale);
        inline mat4x4f_t make_rotation(vec3f_t const& axis, float rad);
        inline mat4x4f_t make_perspective_proj(float fovInRad, float aspectRatio, float zNear, float zFar);
        inline mat4x4f_t make_ortho_proj(float left, float right, float bottom, float top, float zNear, float zFar);
        inline mat4x4f_t make_lookat(vec3f_t const& from, vec3f_t const& to, vec3f_t const& up);
        // -----------------------------------------------------------
        // -----------------------------------------------------------

        inline float dot(vec2f_t const& lhs, vec2f_t const& rhs);
        inline float dot(vec3f_t const& lhs, vec3f_t const& rhs);
        inline float dot(vec4f_t const& lhs, vec4f_t const& rhs);

        inline float length(vec2f_t const& vec);
        inline float length(vec3f_t const& vec);
        inline float length(vec4f_t const& vec);

        inline float squared_length(vec2f_t const& vec);
        inline float squared_length(vec3f_t const& vec);
        inline float squared_length(vec4f_t const& vec);

        inline float distance(vec2f_t const& a, vec2f_t const& b);
        inline float distance(vec3f_t const& a, vec3f_t const& b);
        inline float distance(vec4f_t const& a, vec4f_t const& b);

        inline vec3f_t cross(vec3f_t const& lhs, vec3f_t const& rhs);

        inline vec2f_t normalize(vec2f_t const& vec);
        inline vec3f_t normalize(vec3f_t const& vec);
        inline vec4f_t normalize(vec4f_t const& vec);

        inline vec2f_t normalize_safe(vec2f_t const& vec, float eps = 0.00001f);
        inline vec3f_t normalize_safe(vec3f_t const& vec, float eps = 0.00001f);
        inline vec4f_t normalize_safe(vec4f_t const& vec, float eps = 0.00001f);

        inline quatf_t normalize(quatf_t const& quat);
        inline quatf_t normalize_safe(quatf_t const& quat, float eps = 0.00001f);

        inline quatf_t angle_axis(vec3f_t const& axis, float rad);

        inline quatf_t inverse(quatf_t const& quat);
        
        // -----------------------------------------------------------
        // -----------------------------------------------------------

        inline mat4x4f_t inverse(mat4x4f_t const& mat);

        inline mat4x4f_t transpose(mat4x4f_t const& mat);

        inline vec4f_t column(mat4x4f_t const& mat, int index);
        inline vec4f_t row(mat4x4f_t const& mat, int index);
        
        // -----------------------------------------------------------
        // -----------------------------------------------------------

        inline float RadToDeg(float rad);
        inline float DegToRad(float deg);

        inline double DegToRad(double deg);
        inline double RadToDeg(double rad);

        inline float sqrt(float v);
        inline double sqrt(double v);

        inline float sin(float v);
        inline float cos(float v);
        inline float tan(float v);
        inline float acos(float v);
        inline float asin(float v);

        inline double sin(double v);
        inline double cos(double v);
        inline double tan(double v);
        inline double acos(double v);
        inline double asin(double v);

        // -----------------------------------------------------------
        // -----------------------------------------------------------

        inline bool intersect_planes(plane_t const& a, plane_t const& b, line_t* intersection_line);
        inline bool intersect_line_x_plane(line_t const& line, plane_t const& plane, vec3f_t* intersection_point);

        // -----------------------------------------------------------
        // -----------------------------------------------------------
        
        template <typename T>
        T lerp(T const& a, T const& b, float alpha)
        {
            return a * (1.0f - alpha) + b * alpha;
        }

        template <typename T>
        T min(T a, T b)
        {
            return a < b ? a : b;
        }

        template <typename T>
        T max(T a, T b)
        {
            return a > b ? a : b;
        }

        template <typename T>
        T clamp(T value, T min_value, T max_value)
        {
            return min(max_value, max(value, min_value));
        }

        template <typename T>
        T saturate(T value)
        {
            return clamp(value, static_cast<T>(0), static_cast<T>(1));
        }

        template <typename T>
        T abs(T value)
        {
            return value > static_cast<T>(0) ? value : -value;
        }

        template <typename T>
        T sign(T value)
        {
            return value >= 0.0f ? static_cast<T>(1) : static_cast<T>(-1);
        }
    }
}

// -----------------------------------------------------------
// -----------------------------------------------------------

gt::math::vec2f_t gt::math::operator + (gt::math::vec2f_t const& lhs, gt::math::vec2f_t const& rhs)
{
    return vec2f_t(lhs.x + rhs.x, lhs.y + rhs.y);
}

gt::math::vec2f_t gt::math::operator - (gt::math::vec2f_t const& lhs, gt::math::vec2f_t const& rhs)
{
    return vec2f_t(lhs.x - rhs.x, lhs.y - rhs.y);
}

gt::math::vec2f_t gt::math::operator * (gt::math::vec2f_t const& lhs, gt::math::vec2f_t const& rhs)
{
    return vec2f_t(lhs.x * rhs.x, lhs.y * rhs.y);
}

gt::math::vec2f_t gt::math::operator / (gt::math::vec2f_t const& lhs, gt::math::vec2f_t const& rhs)
{
    return vec2f_t(lhs.x / rhs.x, lhs.y / rhs.y);
}

// -- 

gt::math::vec2f_t gt::math::operator + (gt::math::vec2f_t const& lhs, float rhs)
{
    return vec2f_t(lhs.x + rhs, lhs.y + rhs);
}

gt::math::vec2f_t gt::math::operator - (gt::math::vec2f_t const& lhs, float rhs)
{
    return vec2f_t(lhs.x - rhs, lhs.y - rhs);
}

gt::math::vec2f_t gt::math::operator * (gt::math::vec2f_t const& lhs, float rhs)
{
    return vec2f_t(lhs.x * rhs, lhs.y * rhs);
}

gt::math::vec2f_t gt::math::operator / (gt::math::vec2f_t const& lhs, float rhs)
{
    return vec2f_t(lhs.x / rhs, lhs.y / rhs);
}

// ---------------------


gt::math::vec3f_t gt::math::operator + (gt::math::vec3f_t const& lhs, gt::math::vec3f_t const& rhs)
{
    return vec3f_t(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

gt::math::vec3f_t gt::math::operator - (gt::math::vec3f_t const& lhs, gt::math::vec3f_t const& rhs)
{
    return vec3f_t(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

gt::math::vec3f_t gt::math::operator * (gt::math::vec3f_t const& lhs, gt::math::vec3f_t const& rhs)
{
    return vec3f_t(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

gt::math::vec3f_t gt::math::operator / (gt::math::vec3f_t const& lhs, gt::math::vec3f_t const& rhs)
{
    return vec3f_t(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
}

// -- 

gt::math::vec3f_t gt::math::operator + (gt::math::vec3f_t const& lhs, float rhs)
{
    return vec3f_t(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs);
}

gt::math::vec3f_t gt::math::operator - (gt::math::vec3f_t const& lhs, float rhs)
{
    return vec3f_t(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs);
}

gt::math::vec3f_t gt::math::operator * (gt::math::vec3f_t const& lhs, float rhs)
{
    return vec3f_t(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

gt::math::vec3f_t gt::math::operator / (gt::math::vec3f_t const& lhs, float rhs)
{
    return vec3f_t(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

// ---------------------

gt::math::vec4f_t gt::math::operator + (gt::math::vec4f_t const& lhs, gt::math::vec4f_t const& rhs)
{
    return vec4f_t(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
}

gt::math::vec4f_t gt::math::operator - (gt::math::vec4f_t const& lhs, gt::math::vec4f_t const& rhs)
{
    return vec4f_t(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
}

gt::math::vec4f_t gt::math::operator * (gt::math::vec4f_t const& lhs, gt::math::vec4f_t const& rhs)
{
    return vec4f_t(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w);
}

gt::math::vec4f_t gt::math::operator / (gt::math::vec4f_t const& lhs, gt::math::vec4f_t const& rhs)
{
    return vec4f_t(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}

// -- 

gt::math::vec4f_t gt::math::operator + (gt::math::vec4f_t const& lhs, float rhs)
{
    return vec4f_t(lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs);
}

gt::math::vec4f_t gt::math::operator - (gt::math::vec4f_t const& lhs, float rhs)
{
    return vec4f_t(lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs);
}

gt::math::vec4f_t gt::math::operator * (gt::math::vec4f_t const& lhs, float rhs)
{
    return vec4f_t(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs);
}

gt::math::vec4f_t gt::math::operator / (gt::math::vec4f_t const& lhs, float rhs)
{
    return vec4f_t(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs);
}

// --

gt::math::vec2f_t gt::math::operator - (gt::math::vec2f_t const& vec)
{
    return vec2f_t(-vec.x, -vec.y);
}

gt::math::vec3f_t gt::math::operator - (gt::math::vec3f_t const& vec)
{
    return vec3f_t(-vec.x, -vec.y, -vec.z);
}

gt::math::vec4f_t gt::math::operator - (gt::math::vec4f_t const& vec)
{
    return vec4f_t(-vec.x, -vec.y, -vec.z, -vec.w);
}

// -----------------------------------------------------------
// -----------------------------------------------------------

gt::math::vec4f_t gt::math::operator * (gt::math::mat4x4f_t const& lhs, gt::math::vec4f_t const& rhs)
{
    return vec4f_t(
        dot(row(lhs, 0), rhs),
        dot(row(lhs, 1), rhs),
        dot(row(lhs, 2), rhs),
        dot(row(lhs, 3), rhs)
    );
}

gt::math::quatf_t gt::math::operator * (gt::math::quatf_t const& lhs, gt::math::quatf_t const& rhs)
{
    return quatf_t(
        lhs.r * rhs.r - lhs.i * rhs.i - lhs.j * rhs.j - lhs.k * rhs.k,
        lhs.r * rhs.i + lhs.i * rhs.r + lhs.j * rhs.k - lhs.k * rhs.j,
        lhs.r * rhs.j + lhs.j * rhs.r + lhs.k * rhs.i - lhs.i * rhs.k,
        lhs.r * rhs.k + lhs.k * rhs.r + lhs.i * rhs.j - lhs.j * rhs.i
    );
}

gt::math::mat4x4f_t gt::math::operator * (gt::math::mat4x4f_t const& lhs, gt::math::mat4x4f_t const& rhs)
{
    mat4x4f_t result;

    // first row
    result[0][0] = lhs[0][0] * rhs[0][0] + lhs[1][0] * rhs[0][1] + lhs[2][0] * rhs[0][2] + lhs[3][0] * rhs[0][3];
    result[1][0] = lhs[0][0] * rhs[1][0] + lhs[1][0] * rhs[1][1] + lhs[2][0] * rhs[1][2] + lhs[3][0] * rhs[1][3];
    result[2][0] = lhs[0][0] * rhs[2][0] + lhs[1][0] * rhs[2][1] + lhs[2][0] * rhs[2][2] + lhs[3][0] * rhs[2][3];
    result[3][0] = lhs[0][0] * rhs[3][0] + lhs[1][0] * rhs[3][1] + lhs[2][0] * rhs[3][2] + lhs[3][0] * rhs[3][3];

    // second row
    result[0][1] = lhs[0][1] * rhs[0][0] + lhs[1][1] * rhs[0][1] + lhs[2][1] * rhs[0][2] + lhs[3][1] * rhs[0][3];
    result[1][1] = lhs[0][1] * rhs[1][0] + lhs[1][1] * rhs[1][1] + lhs[2][1] * rhs[1][2] + lhs[3][1] * rhs[1][3];
    result[2][1] = lhs[0][1] * rhs[2][0] + lhs[1][1] * rhs[2][1] + lhs[2][1] * rhs[2][2] + lhs[3][1] * rhs[2][3];
    result[3][1] = lhs[0][1] * rhs[3][0] + lhs[1][1] * rhs[3][1] + lhs[2][1] * rhs[3][2] + lhs[3][1] * rhs[3][3];

    // third row
    result[0][2] = lhs[0][2] * rhs[0][0] + lhs[1][2] * rhs[0][1] + lhs[2][2] * rhs[0][2] + lhs[3][2] * rhs[0][3];
    result[1][2] = lhs[0][2] * rhs[1][0] + lhs[1][2] * rhs[1][1] + lhs[2][2] * rhs[1][2] + lhs[3][2] * rhs[1][3];
    result[2][2] = lhs[0][2] * rhs[2][0] + lhs[1][2] * rhs[2][1] + lhs[2][2] * rhs[2][2] + lhs[3][2] * rhs[2][3];
    result[3][2] = lhs[0][2] * rhs[3][0] + lhs[1][2] * rhs[3][1] + lhs[2][2] * rhs[3][2] + lhs[3][2] * rhs[3][3];

    // fourth row
    result[0][3] = lhs[0][3] * rhs[0][0] + lhs[1][3] * rhs[0][1] + lhs[2][3] * rhs[0][2] + lhs[3][3] * rhs[0][3];
    result[1][3] = lhs[0][3] * rhs[1][0] + lhs[1][3] * rhs[1][1] + lhs[2][3] * rhs[1][2] + lhs[3][3] * rhs[1][3];
    result[2][3] = lhs[0][3] * rhs[2][0] + lhs[1][3] * rhs[2][1] + lhs[2][3] * rhs[2][2] + lhs[3][3] * rhs[2][3];
    result[3][3] = lhs[0][3] * rhs[3][0] + lhs[1][3] * rhs[3][1] + lhs[2][3] * rhs[3][2] + lhs[3][3] * rhs[3][3];

    return result;
}

gt::math::vec3f_t gt::math::transform_pos(gt::math::mat4x4f_t const& transform, gt::math::vec3f_t const& position)
{
    return (transform * vec4f_t(position, 1.0f)).xyz;
}

gt::math::vec3f_t gt::math::transform_dir(gt::math::mat4x4f_t const& transform, gt::math::vec3f_t const& direction)
{
    return (transform * vec4f_t(direction, 0.0f)).xyz;

}

gt::math::mat4x4f_t gt::math::transpose(gt::math::mat4x4f_t const& mat)
{
    gt::math::mat4x4f_t res;
    for (auto i = 0; i < 4; ++i) {
        for (auto j = 0; j < 4; ++j) {
            res[i][j] = mat[j][i];
        }
    }
    return res;
}

gt::math::mat4x4f_t gt::math::quat_to_mat(gt::math::quatf_t const& quat)
{
    mat4x4f_t result;

    /*result[0][0] = 1.0f - 2.0f * quat.y * quat.y - 2.0f * quat.z * quat.z;
    result[1][0] = 2.0f * quat.x * quat.y - 2.0f * quat.z * quat.w;
    result[2][0] = 2.0f * quat.x * quat.z - 2.0f * quat.y * quat.w;

    result[0][1] = 2.0f * quat.x * quat.y + 2.0f * quat.z * quat.w;
    result[1][1] = 1.0f - 2.0f * quat.x * quat.x - 2.0f * quat.z * quat.z;
    result[2][1] = 2.0f * quat.y * quat.z - 2.0f * quat.x * quat.w;

    result[0][2] = 2.0f * quat.x * quat.z - 2.0f * quat.y * quat.w;
    result[1][2] = 2.0f * quat.y * quat.z + 2.0f * quat.x * quat.w;
    result[2][2] = 1.0f - 2.0f * quat.x * quat.x - 2.0f * quat.y * quat.y;*/


    result(0) = 1.0f - 2.0f * quat.y * quat.y - 2.0f * quat.z * quat.z;
    result(1) = 2.0f * quat.x * quat.y - 2.0f * quat.z * quat.w;
    result(2) = 2.0f * quat.x * quat.z + 2.0f * quat.y * quat.w;

    result(4) = 2.0f * quat.x * quat.y + 2.0f * quat.z * quat.w;
    result(5) = 1.0f - 2.0f * quat.x * quat.x - 2.0f * quat.z * quat.z;
    result(6) = 2.0f * quat.y * quat.z - 2.0f * quat.x * quat.w;

    result(8) = 2.0f * quat.x * quat.z - 2.0f * quat.y * quat.w;
    result(9) = 2.0f * quat.y * quat.z + 2.0f * quat.x * quat.w;
    result(10) = 1.0f - 2.0f * quat.x * quat.x - 2.0f * quat.y * quat.y;

    return transpose(result);
}

gt::math::quatf_t gt::math::quat_from_mat(mat4x4f_t const& mat)
{
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    // @note should assert that mat is orthonormal
    float trace = mat[0][0] + mat[1][1] + mat[2][2]; 
    quatf_t res;
    if (trace > 0) {
        float s = 0.5f / sqrtf(trace + 1.0f);
        res.w = 0.25f / s;
        res.x = (mat[1][2] - mat[2][1]) * s;
        res.y = (mat[2][0] - mat[0][2]) * s;
        res.z = (mat[0][1] - mat[1][0]) * s;
    }
    else {
        if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2]) {
            float s = 2.0f * sqrtf(1.0f + mat[0][0] - mat[1][1] - mat[2][2]);
            res.w = (mat[1][2] - mat[2][1]) / s;
            res.x = 0.25f * s;
            res.y = (mat[1][0] + mat[0][1]) / s;
            res.z = (mat[2][0] + mat[0][2]) / s;
        }
        else if (mat[1][1] > mat[2][2]) {
            float s = 2.0f * sqrtf(1.0f + mat[1][1] - mat[0][0] - mat[2][2]);
            res.w = (mat[2][0] - mat[0][2]) / s;
            res.x = (mat[1][0] + mat[0][1]) / s;
            res.y = 0.25f * s;
            res.z = (mat[2][1] + mat[1][2]) / s;
        }
        else {
            float s = 2.0f * sqrtf(1.0f + mat[2][2] - mat[0][0] - mat[1][1]);
            res.w = (mat[0][1] - mat[1][0]) / s;
            res.x = (mat[2][0] + mat[0][2]) / s;
            res.y = (mat[2][1] + mat[1][2]) / s;
            res.z = 0.25f * s;
        }
    }
    return normalize(res);
}

// ---- 


gt::math::mat4x4f_t gt::math::make_translation(vec3f_t const& translation)
{
    mat4x4f_t result;
    result.columns[3] = vec4f_t(translation, 1.0f);
    return result;
}

gt::math::mat4x4f_t gt::math::make_scale(vec3f_t const& scale)
{
    mat4x4f_t result;
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    return result;
}


gt::math::mat4x4f_t gt::math::make_rotation(vec3f_t const& axis, float rad)
{
    mat4x4f_t rotate;
    mat4x4f_t base;

    float a = rad;
    float c = cos(a);
    float s = sin(a);

    auto normalizedAxis = normalize(axis);
    auto temp = normalizedAxis * (1.0f - c);

    rotate[0][0] = c + temp[0] * axis[0];
    rotate[0][1] = temp[0] * axis[1] + s * axis[2];
    rotate[0][2] = temp[0] * axis[2] - s * axis[1];

    rotate[1][0] = temp[1] * axis[0] - s * axis[2];
    rotate[1][1] = c + temp[1] * axis[1];
    rotate[1][2] = temp[1] * axis[2] + s * axis[0];

    rotate[2][0] = temp[2] * axis[0] + s * axis[1];
    rotate[2][1] = temp[2] * axis[1] - s * axis[0];
    rotate[2][2] = c + temp[2] * axis[2];

    auto m0 = column(base, 0);
    auto m1 = column(base, 1);
    auto m2 = column(base, 2);
    auto m3 = column(base, 3);

    mat4x4f_t result;
    for (int i = 0; i < 4; ++i) {
        result[i][0] = m0[i] * rotate[0][0] + m1[i] * rotate[0][1] + m2[i] * rotate[0][2];
        result[i][1] = m0[i] * rotate[1][0] + m1[i] * rotate[1][1] + m2[i] * rotate[1][2];
        result[i][2] = m0[i] * rotate[2][0] + m1[i] * rotate[2][1] + m2[i] * rotate[2][2];
        result[i][3] = m3[i];
    }
    /*result[0] = normalize(result[0]);
    result[1] = normalize(result[1]);
    result[2] = normalize(result[2]);*/
    return result;
}


gt::math::mat4x4f_t gt::math::make_perspective_proj(float fovInRad, float aspectRatio, float zNear, float zFar)
{
    mat4x4f_t result;

    float yScale = 1.0f / tan(fovInRad * 0.5f);
    float xScale = yScale / (aspectRatio);
    result[0][0] = xScale;
    result[1][1] = yScale;
    result[2][2] = zFar / (zFar - zNear);
    result[3][2] = -zNear * zFar / (zFar - zNear);
    result[2][3] = 1.0f;
    result[3][3] = 0.0f;
    //return mat4x4f();
    return result;
}

gt::math::mat4x4f_t gt::math::make_ortho_proj(float left, float right, float bottom, float top, float zNear, float zFar)
{
    mat4x4f_t result;

    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = 1.0f / (zFar - zNear);
    result[0][3] = (left + right) / (left - right);
    result[1][3] = (top + bottom) / (bottom - top);
    result[2][3] = zNear / (zNear - zFar);

    return result;
}

gt::math::mat4x4f_t gt::math::make_lookat(gt::math::vec3f_t const& from, gt::math::vec3f_t const& to, gt::math::vec3f_t const& up)
{
    auto direction = normalize(to - from);
    auto x = normalize(cross(up, direction));
    auto y = normalize(cross(direction, x));

    mat4x4f_t result;
    result[0] = vec4f_t(x, 0.0f);
    result[1] = vec4f_t(y, 0.0f);
    result[2] = vec4f_t(direction, 0.0f);
    result[3] = vec4f_t(from, 1.0f);
    return result;
}

// -----------------------------------------------------------
// -----------------------------------------------------------

float gt::math::dot(gt::math::vec2f_t const& lhs, gt::math::vec2f_t const& rhs)
{
    return lhs.x* rhs.x + lhs.y * rhs.y;
}

float gt::math::dot(gt::math::vec3f_t const& lhs, gt::math::vec3f_t const& rhs)
{
    return lhs.x* rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

float gt::math::dot(gt::math::vec4f_t const& lhs, gt::math::vec4f_t const& rhs)
{
    return lhs.x* rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}

gt::math::vec3f_t gt::math::cross(gt::math::vec3f_t const& lhs, gt::math::vec3f_t const& rhs)
{
    return vec3f_t(
        lhs.y * rhs.z - rhs.y * lhs.z,
        lhs.z * rhs.x - rhs.z * lhs.x,
        lhs.x * rhs.y - rhs.x * lhs.y
    );
}

gt::math::vec2f_t gt::math::normalize(gt::math::vec2f_t const& vec)
{
    return vec / length(vec);
}

gt::math::vec2f_t gt::math::normalize_safe(gt::math::vec2f_t const& vec, float eps)
{
    auto sqrLen = dot(vec, vec);
    if (sqrLen <= eps * eps) { return vec2f_t(); }
    return normalize(vec);
}

gt::math::vec3f_t gt::math::normalize(gt::math::vec3f_t const& vec)
{
    return vec / length(vec);
}

gt::math::vec3f_t gt::math::normalize_safe(gt::math::vec3f_t const& vec, float eps)
{
    auto sqrLen = dot(vec, vec);
    if (sqrLen <= eps * eps) { return vec3f_t(); }
    return normalize(vec);
}

gt::math::vec4f_t gt::math::normalize(gt::math::vec4f_t const& vec)
{
    return vec / length(vec);
}

gt::math::vec4f_t gt::math::normalize_safe(gt::math::vec4f_t const& vec, float eps)
{
    auto sqrLen = squared_length(vec);
    if (sqrLen <= eps * eps) { return vec4f_t(); }
    return normalize(vec);
}


float gt::math::length(gt::math::vec2f_t const& vec)
{
    return sqrt(dot(vec, vec));
}

float gt::math::squared_length(gt::math::vec2f_t const& vec)
{
    return dot(vec, vec);
}

float gt::math::length(gt::math::vec3f_t const& vec)
{
    return sqrt(dot(vec, vec));
}

float gt::math::squared_length(gt::math::vec3f_t const& vec)
{
    return dot(vec, vec);
}


float gt::math::length(gt::math::vec4f_t const& vec)
{
    return sqrt(dot(vec, vec));
}

float gt::math::squared_length(gt::math::vec4f_t const& vec)
{
    return dot(vec, vec);
}

float gt::math::distance(gt::math::vec2f_t const& a, gt::math::vec2f_t const& b)
{
    return length(a - b);
}

float gt::math::distance(gt::math::vec3f_t const& a, gt::math::vec3f_t const& b)
{
    return length(a - b);
}

float gt::math::distance(gt::math::vec4f_t const& a, gt::math::vec4f_t const& b)
{
    return length(a - b);
}

// ---

gt::math::quatf_t gt::math::normalize(gt::math::quatf_t const& quat)
{
    const auto invLen = 1.0f / sqrt(quat.w * quat.w + quat.x * quat.x + quat.y * quat.y + quat.z * quat.z);

    return quatf_t(
        quat.w * invLen,
        quat.x * invLen,
        quat.y * invLen,
        quat.z * invLen
    );
}

gt::math::quatf_t gt::math::normalize_safe(gt::math::quatf_t const& quat, float eps)
{
    const auto d = quat.w * quat.w + quat.x * quat.x + quat.y * quat.y + quat.z * quat.z;
    if (d < eps * eps) {
        return quatf_t();
    }
    const auto invLen = 1.0f / sqrt(d);

    return quatf_t(
        quat.w * invLen,
        quat.x * invLen,
        quat.y * invLen,
        quat.z * invLen
    );
}

gt::math::quatf_t gt::math::inverse(gt::math::quatf_t const& quat)
{
    const auto d = quat.w * quat.w + quat.x * quat.x + quat.y * quat.y + quat.z * quat.z;
    const auto invD = 1.0f / d;
    return quatf_t(
        quat.w * invD,
        -quat.x * invD,
        -quat.y * invD,
        -quat.z * invD
    );
}

gt::math::quatf_t gt::math::angle_axis(gt::math::vec3f_t const& axis, float rad)
{
    auto normalized_axis = normalize(axis); // @NOTE just assume this is already normalized?
    quatf_t res;
    auto s = math::sin(rad * 0.5f);
    res.x = axis.x * s;
    res.y = axis.y * s;
    res.z = axis.z * s;
    res.w = math::cos(rad * 0.5f);
    return res;
}

// -----------------------------------------------------------
// -----------------------------------------------------------

gt::math::mat4x4f_t gt::math::inverse(gt::math::mat4x4f_t const& mat)
{
    mat4x4f_t result;

    float det = 0.0f;
    float const* m = mat.elements;
    float* inv = result.elements;

    inv[0] = m[5] * m[10] * m[15] -
        m[5] * m[11] * m[14] -
        m[9] * m[6] * m[15] +
        m[9] * m[7] * m[14] +
        m[13] * m[6] * m[11] -
        m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
        m[4] * m[11] * m[14] +
        m[8] * m[6] * m[15] -
        m[8] * m[7] * m[14] -
        m[12] * m[6] * m[11] +
        m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
        m[4] * m[11] * m[13] -
        m[8] * m[5] * m[15] +
        m[8] * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
        m[4] * m[10] * m[13] +
        m[8] * m[5] * m[14] -
        m[8] * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
        m[1] * m[11] * m[14] +
        m[9] * m[2] * m[15] -
        m[9] * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
        m[0] * m[11] * m[14] -
        m[8] * m[2] * m[15] +
        m[8] * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
        m[0] * m[11] * m[13] +
        m[8] * m[1] * m[15] -
        m[8] * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
        m[0] * m[10] * m[13] -
        m[8] * m[1] * m[14] +
        m[8] * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
        m[1] * m[7] * m[14] -
        m[5] * m[2] * m[15] +
        m[5] * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
        m[0] * m[7] * m[14] +
        m[4] * m[2] * m[15] -
        m[4] * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
        m[0] * m[7] * m[13] -
        m[4] * m[1] * m[15] +
        m[4] * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
        m[0] * m[6] * m[13] +
        m[4] * m[1] * m[14] -
        m[4] * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
        m[1] * m[7] * m[10] +
        m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] -
        m[9] * m[2] * m[7] +
        m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
        m[0] * m[7] * m[10] -
        m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] +
        m[8] * m[2] * m[7] -
        m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
        m[0] * m[7] * m[9] +
        m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -
        m[8] * m[1] * m[7] +
        m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
        m[0] * m[6] * m[9] -
        m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] +
        m[8] * m[1] * m[6] -
        m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0) {
        return mat4x4f_t();
    }

    det = 1.0f / det;

    for (auto i = 0; i < 16; i++) {
        inv[i] = inv[i] * det;
    }

    return result;
}

// --

gt::math::vec4f_t gt::math::column(gt::math::mat4x4f_t const& mat, int index)
{
    return mat.columns[index];
}

gt::math::vec4f_t gt::math::row(gt::math::mat4x4f_t const& mat, int index)
{
    return vec4f_t(
        mat[0][index],
        mat[1][index],
        mat[2][index],
        mat[3][index]
    );
}

// -----------------------------------------------------------
// -----------------------------------------------------------


float gt::math::RadToDeg(float rad)
{
    return rad * (180.0f / PI);
}

float gt::math::DegToRad(float deg)
{
    return deg * (PI / 180.0f);
}

double gt::math::DegToRad(double deg)
{
    return deg * (static_cast<double>(PI) / 180.0);
}

double gt::math::RadToDeg(double rad)
{
    return rad * (180.0 / static_cast<double>(PI));
}

float gt::math::sqrt(float v)
{
    return ::sqrtf(v);
}
double gt::math::sqrt(double v)
{
    return ::sqrt(v);
}

float gt::math::sin(float v)
{
    return ::sinf(v);
}

float gt::math::cos(float v)
{
    return ::cosf(v);
}

float gt::math::tan(float v)
{
    return ::tanf(v);
}

float gt::math::acos(float v)
{
    return ::acosf(v);
}

float gt::math::asin(float v)
{
    return ::asinf(v);
}

double gt::math::sin(double v)
{
    return ::sin(v);
}
double gt::math::cos(double v)
{
    return ::cos(v);
}
double gt::math::tan(double v)
{
    return ::tan(v);
}
double gt::math::acos(double v)
{
    return ::acos(v);
}
double gt::math::asin(double v)
{
    return ::asin(v);
}

//

bool gt::math::intersect_planes(plane_t const& a, plane_t const& b, line_t* intersection_line)
{
    auto p3_normal = cross(a.normal, b.normal);
    auto det = squared_length(p3_normal);
    if (det < 0.000001f) {
        if (intersection_line != nullptr) {
            intersection_line->p = (cross(p3_normal, b.normal) * a.d + cross(a.normal, p3_normal) * b.d) / det;
            intersection_line->normal = p3_normal;
        }
        return true;
    }
    return false;
}

bool gt::math::intersect_line_x_plane(line_t const& line, plane_t const& plane, vec3f_t* intersection_point)
{
    auto plane_p = plane.normal * plane.d;
    if (dot(line.normal, plane.normal) < 0.000001f) { return false; }
    if (!intersection_point) { return true; }
    auto diff = line.p - plane_p;
    auto prod1 = dot(diff, plane.normal);
    auto prod2 = dot(line.normal, plane.normal);
    auto prod3 = prod1 / prod2;
    *intersection_point = line.p - line.normal * prod3;
    return true;
}