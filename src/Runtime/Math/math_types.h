#pragma once
#include <memory.h>

namespace gt 
{
    namespace math
    {
        // -----------------------------------------------------------
        struct vec2f_t
        {
            union {
                struct {
                    float x;
                    float y;
                };
                struct {
                    float u;
                    float v;
                };
                float elements[2];
            };

            vec2f_t(float v) : x(v), y(v) {}
            vec2f_t() : vec2f_t(0.0f) {}
            vec2f_t(float a, float b) : x(a), y(b) {} 

            explicit operator float* () { return elements; }

            float const& operator [] (int index) const { return elements[index]; }
            float& operator [] (int index) { return elements[index]; }
        };

        // -----------------------------------------------------------
        struct vec3f_t
        {
            union {
                struct {
                    float x;
                    float y;
                    float z;
                };
                vec2f_t xy;
                float elements[3];
            };

            vec3f_t(float v) : x(v), y(v), z(v) {}
            vec3f_t() : vec3f_t(0.0f) {}
            vec3f_t(float a, float b, float c) : x(a), y(b), z(c) {}
        
            explicit operator float* () { return elements; }

            float const& operator [] (int index) const { return elements[index]; }
            float& operator [] (int index) { return elements[index]; }
        };

        // -----------------------------------------------------------
        struct vec4f_t
        {
            union {
                struct {
                    float x;
                    float y;
                    float z;
                    float w;
                };
                vec2f_t xy;
                vec3f_t xyz;
                float elements[4];
            };

            vec4f_t(float v) : x(v), y(v), z(v), w(z) {}
            vec4f_t() : vec4f_t(0.0f) {}
            vec4f_t(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}
            explicit vec4f_t(vec3f_t const& abc, float d) : xyz(abc), w(d) {}

            explicit operator float* () { return elements; }

            float const&    operator [] (int index) const    { return elements[index]; }
            float&            operator [] (int index)            { return elements[index]; }
        };

        // -----------------------------------------------------------
        struct quatf_t
        {
            union {
                struct {
                    float w;
                    float x;
                    float y;
                    float z;
                };
                struct {
                    float r;
                    float i;
                    float j;
                    float k;
                };
                float elements[4];
            };

            quatf_t() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
            quatf_t(float d, float a, float b, float c) : w(d), x(a), y(b), z(c) {}
        
            explicit operator float* () { return elements; }
        };

        // -----------------------------------------------------------
        struct mat4x4f_t
        {
            union {
                struct {
                    vec4f_t columns[4];
                };
                float elements[16];
            };

            mat4x4f_t()
            {
                memset(elements, 0x0, sizeof(float) * 16);
                elements[0] = 1.0f;
                elements[5] = 1.0f;
                elements[10] = 1.0f;
                elements[15] = 1.0f;
            }

            explicit operator float* () { return elements; }
            vec4f_t&          operator [] (int column)        { return columns[column]; }
            vec4f_t const&    operator [] (int column) const  { return columns[column]; }

            float& operator () (int elementIndex) { 
                return elements[elementIndex];
            }
            float const& operator () (int elementIndex) const {
                return elements[elementIndex];
            }
        };

        // -----------------------------------------------------------
        static constexpr float PI = 3.14159265359f;
   
        // -----------------------------------------------------------
        // -----------------------------------------------------------

        struct plane_t {
            vec3f_t normal;
            float d;
        };

        struct line_t {
            vec3f_t normal;
            vec3f_t p;
        };

     }

    
}