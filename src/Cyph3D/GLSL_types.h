#pragma once

#define GLSL_bool alignas(4) GLuint
#define GLSL_int alignas(4) GLint
#define GLSL_uint alignas(4) GLuint
#define GLSL_float alignas(4) GLfloat
#define GLSL_double alignas(8) GLdouble

#define GLSL_bvec2 alignas(8) glm::bvec2
#define GLSL_bvec3 alignas(16) glm::bvec3
#define GLSL_bvec4 alignas(16) glm::bvec4

#define GLSL_ivec2 alignas(8) glm::ivec2
#define GLSL_ivec3 alignas(16) glm::ivec3
#define GLSL_ivec4 alignas(16) glm::ivec4

#define GLSL_uvec2 alignas(8) glm::uvec2
#define GLSL_uvec3 alignas(16) glm::uvec3
#define GLSL_uvec4 alignas(16) glm::uvec4

#define GLSL_vec2 alignas(8) glm::vec2
#define GLSL_vec3 alignas(16) glm::vec3
#define GLSL_vec4 alignas(16) glm::vec4

#define GLSL_dvec2 alignas(16) glm::dvec2
#define GLSL_dvec3 alignas(32) glm::dvec3
#define GLSL_dvec4 alignas(32) glm::dvec4

#define GLSL_mat4 alignas(16) glm::mat4

#define GLSL_sampler2D alignas(8) GLuint64
#define GLSL_samplerCube alignas(8) GLuint64