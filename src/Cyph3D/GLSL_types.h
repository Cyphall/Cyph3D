#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#define GLSL_bool alignas(4) uint32_t
#define GLSL_int alignas(4) int32_t
#define GLSL_uint alignas(4) uint32_t
#define GLSL_float alignas(4) float

#define GLSL_bvec2 alignas(8) glm::uvec2
#define GLSL_bvec3 alignas(16) glm::uvec3
#define GLSL_bvec4 alignas(16) glm::uvec4

#define GLSL_ivec2 alignas(8) glm::ivec2
#define GLSL_ivec3 alignas(16) glm::ivec3
#define GLSL_ivec4 alignas(16) glm::ivec4

#define GLSL_uvec2 alignas(8) glm::uvec2
#define GLSL_uvec3 alignas(16) glm::uvec3
#define GLSL_uvec4 alignas(16) glm::uvec4

#define GLSL_vec2 alignas(8) glm::vec2
#define GLSL_vec3 alignas(16) glm::vec3
#define GLSL_vec4 alignas(16) glm::vec4

#define GLSL_mat4 alignas(16) glm::mat4