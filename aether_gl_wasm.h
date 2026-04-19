//------------------------------------------------------------------------------
// aether_gl_wasm.h
// GL type definitions, constants, and WASM import declarations for _AE_WASM_=1.
// Included by aether.h instead of platform GL headers when targeting WASI/WASM.
//------------------------------------------------------------------------------
#pragma once

#include <stdint.h>

//------------------------------------------------------------------------------
// GL types
//------------------------------------------------------------------------------
typedef uint32_t GLenum;
typedef uint32_t GLbitfield;
typedef uint32_t GLuint;
typedef int32_t  GLint;
typedef int32_t  GLsizei;
typedef int64_t  GLsizeiptr;
typedef int64_t  GLintptr;
typedef float    GLfloat;
typedef double   GLdouble;
typedef uint8_t  GLboolean;
typedef char     GLchar;
typedef uint8_t  GLubyte;
typedef void     GLvoid;

typedef void (*GLDEBUGPROC)( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam );

#define GL_TRUE  1
#define GL_FALSE 0

//------------------------------------------------------------------------------
// GL constants — primitives
//------------------------------------------------------------------------------
#define GL_POINTS                                   0x0000
#define GL_LINES                                    0x0001
#define GL_LINE_STRIP                               0x0003
#define GL_TRIANGLES                                0x0004
#define GL_TRIANGLE_STRIP                           0x0005

//------------------------------------------------------------------------------
// GL constants — data types
//------------------------------------------------------------------------------
#define GL_BYTE                                     0x1400
#define GL_UNSIGNED_BYTE                            0x1401
#define GL_SHORT                                    0x1402
#define GL_UNSIGNED_SHORT                           0x1403
#define GL_INT                                      0x1404
#define GL_UNSIGNED_INT                             0x1405
#define GL_FLOAT                                    0x1406
#define GL_HALF_FLOAT                               0x140B

//------------------------------------------------------------------------------
// GL constants — depth functions
//------------------------------------------------------------------------------
#define GL_LESS                                     0x0201
#define GL_EQUAL                                    0x0202
#define GL_LEQUAL                                   0x0203
#define GL_GREATER                                  0x0204
#define GL_GEQUAL                                   0x0206
#define GL_ALWAYS                                   0x0207

//------------------------------------------------------------------------------
// GL constants — blend factors
//------------------------------------------------------------------------------
#define GL_ZERO                                     0x0000
#define GL_ONE                                      0x0001
#define GL_SRC_ALPHA                                0x0302
#define GL_ONE_MINUS_SRC_ALPHA                      0x0303
#define GL_DST_ALPHA                                0x0304
#define GL_ONE_MINUS_DST_ALPHA                      0x0305

//------------------------------------------------------------------------------
// GL constants — state / caps
//------------------------------------------------------------------------------
#define GL_CULL_FACE                                0x0B44
#define GL_DEPTH_TEST                               0x0B71
#define GL_BLEND                                    0x0BE2
#define GL_SCISSOR_TEST                             0x0C11
#define GL_UNPACK_ALIGNMENT                         0x0CF5
#define GL_MAX_TEXTURE_SIZE                         0x0D33
#define GL_VIEWPORT                                 0x0BA2

//------------------------------------------------------------------------------
// GL constants — winding / face
//------------------------------------------------------------------------------
#define GL_CW                                       0x0900
#define GL_CCW                                      0x0901
#define GL_FRONT                                    0x0404
#define GL_BACK                                     0x0405
#define GL_FRONT_AND_BACK                           0x0408
#define GL_LINE                                     0x1B01
#define GL_FILL                                     0x1B02

//------------------------------------------------------------------------------
// GL constants — clear bits
//------------------------------------------------------------------------------
#define GL_DEPTH_BUFFER_BIT                         0x00000100
#define GL_STENCIL_BUFFER_BIT                       0x00000400
#define GL_COLOR_BUFFER_BIT                         0x00004000

//------------------------------------------------------------------------------
// GL constants — textures
//------------------------------------------------------------------------------
#define GL_TEXTURE_2D                               0x0DE1
#define GL_TEXTURE_3D                               0x806F
#define GL_TEXTURE_CUBE_MAP                         0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X              0x8515
#define GL_TEXTURE0                                 0x84C0

#define GL_TEXTURE_MIN_FILTER                       0x2801
#define GL_TEXTURE_MAG_FILTER                       0x2800
#define GL_TEXTURE_WRAP_S                           0x2802
#define GL_TEXTURE_WRAP_T                           0x2803

#define GL_NEAREST                                  0x2600
#define GL_LINEAR                                   0x2601
#define GL_NEAREST_MIPMAP_NEAREST                   0x2700
#define GL_LINEAR_MIPMAP_LINEAR                     0x2703
#define GL_CLAMP_TO_EDGE                            0x812F
#define GL_REPEAT                                   0x2901

#define GL_TEXTURE_STORAGE_HINT_APPLE               0x85BC

//------------------------------------------------------------------------------
// GL constants — texture internal formats
//------------------------------------------------------------------------------
#define GL_RED                                      0x1903
#define GL_RG                                       0x8227
#define GL_RGB                                      0x1907
#define GL_RGBA                                     0x1908
#define GL_R8                                       0x8229
#define GL_RG8                                      0x822B
#define GL_R16F                                     0x822D
#define GL_R32F                                     0x822E
#define GL_R16UI                                    0x8234
#define GL_RG16F                                    0x822F
#define GL_RG32F                                    0x8230
#define GL_RGB8                                     0x8051
#define GL_RGBA8                                    0x8058
#define GL_SRGB8                                    0x8C41
#define GL_SRGB8_ALPHA8                             0x8C43
#define GL_RGBA16F                                  0x881A
#define GL_RGB16F                                   0x881B
#define GL_RGBA32F                                  0x8814
#define GL_RGB32F                                   0x8815
#define GL_DEPTH_COMPONENT16                        0x81A5
#define GL_DEPTH_COMPONENT32F                       0x8CAC

//------------------------------------------------------------------------------
// GL constants — buffers / usage
//------------------------------------------------------------------------------
#define GL_ARRAY_BUFFER                             0x8892
#define GL_ELEMENT_ARRAY_BUFFER                     0x8893
#define GL_STREAM_DRAW                              0x88E0
#define GL_STATIC_DRAW                              0x88E4
#define GL_DYNAMIC_DRAW                             0x88E8

//------------------------------------------------------------------------------
// GL constants — framebuffer
//------------------------------------------------------------------------------
#define GL_FRAMEBUFFER_BINDING                      0x8CA6
#define GL_READ_FRAMEBUFFER                         0x8CA8
#define GL_DRAW_FRAMEBUFFER                         0x8CA9
#define GL_FRAMEBUFFER_COMPLETE                     0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT        0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER       0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER       0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED                  0x8CDD
#define GL_FRAMEBUFFER_UNDEFINED                    0x8219
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE       0x8D56
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS     0x8DA8
#define GL_FRAMEBUFFER_SRGB                         0x8DB9
#define GL_COLOR_ATTACHMENT0                        0x8CE0
#define GL_DEPTH_ATTACHMENT                         0x8D00
#define GL_FRAMEBUFFER                              0x8D40

//------------------------------------------------------------------------------
// GL constants — shaders / programs
//------------------------------------------------------------------------------
#define GL_FRAGMENT_SHADER                          0x8B30
#define GL_VERTEX_SHADER                            0x8B31
#define GL_COMPILE_STATUS                           0x8B81
#define GL_LINK_STATUS                              0x8B82
#define GL_INFO_LOG_LENGTH                          0x8B84
#define GL_ACTIVE_UNIFORMS                          0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH                0x8B87
#define GL_ACTIVE_ATTRIBUTES                        0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH              0x8B8A
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS         0x8B4D

//------------------------------------------------------------------------------
// GL constants — uniform types
//------------------------------------------------------------------------------
#define GL_FLOAT_VEC2                               0x8B50
#define GL_FLOAT_VEC3                               0x8B51
#define GL_FLOAT_VEC4                               0x8B52
#define GL_INT_VEC2                                 0x8B53
#define GL_INT_VEC3                                 0x8B54
#define GL_INT_VEC4                                 0x8B55
#define GL_FLOAT_MAT4                               0x8B5C
#define GL_SAMPLER_2D                               0x8B5E
#define GL_SAMPLER_3D                               0x8B5F
#define GL_VERTEX_PROGRAM_POINT_SIZE                0x8642

//------------------------------------------------------------------------------
// GL constants — debug
//------------------------------------------------------------------------------
#define GL_DEBUG_OUTPUT                             0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS                 0x8242
#define GL_DEBUG_TYPE_ERROR                         0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR           0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR            0x824E
#define GL_DEBUG_TYPE_PORTABILITY                   0x824F
#define GL_DEBUG_TYPE_PERFORMANCE                   0x8250
#define GL_DEBUG_TYPE_OTHER                         0x8251
#define GL_DEBUG_SEVERITY_HIGH                      0x9146
#define GL_DEBUG_SEVERITY_MEDIUM                    0x9147
#define GL_DEBUG_SEVERITY_LOW                       0x9148

//------------------------------------------------------------------------------
// GL constants — misc
//------------------------------------------------------------------------------
#define GL_NO_ERROR                                 0

//------------------------------------------------------------------------------
// GL function declarations — WASM imports from "gl" module
//------------------------------------------------------------------------------
#define _AE_GL_IMPORT( ret, name, params ) \
	__attribute__((import_module("gl"), import_name(#name))) extern ret name params

_AE_GL_IMPORT( void,   glClear,                      ( GLbitfield mask ) );
_AE_GL_IMPORT( void,   glClearColor,                 ( GLfloat r, GLfloat g, GLfloat b, GLfloat a ) );
_AE_GL_IMPORT( void,   glClearDepthf,                ( GLfloat depth ) );
_AE_GL_IMPORT( void,   glViewport,                   ( GLint x, GLint y, GLsizei width, GLsizei height ) );
_AE_GL_IMPORT( void,   glEnable,                     ( GLenum cap ) );
_AE_GL_IMPORT( void,   glDisable,                    ( GLenum cap ) );
_AE_GL_IMPORT( void,   glDepthMask,                  ( GLboolean flag ) );
_AE_GL_IMPORT( void,   glDepthFunc,                  ( GLenum func ) );
_AE_GL_IMPORT( void,   glBlendFunc,                  ( GLenum sfactor, GLenum dfactor ) );
_AE_GL_IMPORT( void,   glFrontFace,                  ( GLenum mode ) );
_AE_GL_IMPORT( void,   glGetIntegerv,                ( GLenum pname, GLint* data ) );
_AE_GL_IMPORT( void,   glPixelStorei,                ( GLenum pname, GLint param ) );
_AE_GL_IMPORT( GLenum, glGetError,                   () );
_AE_GL_IMPORT( void,   glBindTexture,                ( GLenum target, GLuint texture ) );
_AE_GL_IMPORT( void,   glGenTextures,                ( GLsizei n, GLuint* textures ) );
_AE_GL_IMPORT( void,   glDeleteTextures,             ( GLsizei n, const GLuint* textures ) );
_AE_GL_IMPORT( void,   glTexParameteri,              ( GLenum target, GLenum pname, GLint param ) );
_AE_GL_IMPORT( void,   glDrawArrays,                 ( GLenum mode, GLint first, GLsizei count ) );
_AE_GL_IMPORT( void,   glDrawElements,               ( GLenum mode, GLsizei count, GLenum type, const void* indices ) );
_AE_GL_IMPORT( GLuint, glCreateProgram,              () );
_AE_GL_IMPORT( void,   glAttachShader,               ( GLuint program, GLuint shader ) );
_AE_GL_IMPORT( void,   glLinkProgram,                ( GLuint program ) );
_AE_GL_IMPORT( void,   glGetProgramiv,               ( GLuint program, GLenum pname, GLint* params ) );
_AE_GL_IMPORT( void,   glGetProgramInfoLog,          ( GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog ) );
_AE_GL_IMPORT( void,   glGetActiveAttrib,            ( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name ) );
_AE_GL_IMPORT( GLint,  glGetAttribLocation,          ( GLuint program, const GLchar* name ) );
_AE_GL_IMPORT( void,   glGetActiveUniform,           ( GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name ) );
_AE_GL_IMPORT( GLint,  glGetUniformLocation,         ( GLuint program, const GLchar* name ) );
_AE_GL_IMPORT( void,   glDeleteShader,               ( GLuint shader ) );
_AE_GL_IMPORT( void,   glDeleteProgram,              ( GLuint program ) );
_AE_GL_IMPORT( void,   glUseProgram,                 ( GLuint program ) );
_AE_GL_IMPORT( void,   glBlendFuncSeparate,          ( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha ) );
_AE_GL_IMPORT( GLuint, glCreateShader,               ( GLenum type ) );
_AE_GL_IMPORT( void,   glShaderSource,               ( GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length ) );
_AE_GL_IMPORT( void,   glCompileShader,              ( GLuint shader ) );
_AE_GL_IMPORT( void,   glGetShaderiv,                ( GLuint shader, GLenum pname, GLint* params ) );
_AE_GL_IMPORT( void,   glGetShaderInfoLog,           ( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog ) );
_AE_GL_IMPORT( void,   glActiveTexture,              ( GLenum texture ) );
_AE_GL_IMPORT( void,   glUniform1i,                  ( GLint location, GLint v0 ) );
_AE_GL_IMPORT( void,   glUniform1fv,                 ( GLint location, GLsizei count, const GLfloat* value ) );
_AE_GL_IMPORT( void,   glUniform2fv,                 ( GLint location, GLsizei count, const GLfloat* value ) );
_AE_GL_IMPORT( void,   glUniform3fv,                 ( GLint location, GLsizei count, const GLfloat* value ) );
_AE_GL_IMPORT( void,   glUniform4fv,                 ( GLint location, GLsizei count, const GLfloat* value ) );
_AE_GL_IMPORT( void,   glUniformMatrix4fv,           ( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value ) );
_AE_GL_IMPORT( void,   glGenerateMipmap,             ( GLenum target ) );
_AE_GL_IMPORT( void,   glBindFramebuffer,            ( GLenum target, GLuint framebuffer ) );
_AE_GL_IMPORT( void,   glFramebufferTexture2D,       ( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) );
_AE_GL_IMPORT( void,   glGenFramebuffers,            ( GLsizei n, GLuint* framebuffers ) );
_AE_GL_IMPORT( void,   glDeleteFramebuffers,         ( GLsizei n, const GLuint* framebuffers ) );
_AE_GL_IMPORT( GLenum, glCheckFramebufferStatus,     ( GLenum target ) );
_AE_GL_IMPORT( void,   glDrawBuffers,                ( GLsizei n, const GLenum* bufs ) );
_AE_GL_IMPORT( void,   glGenVertexArrays,            ( GLsizei n, GLuint* arrays ) );
_AE_GL_IMPORT( void,   glBindVertexArray,            ( GLuint array ) );
_AE_GL_IMPORT( void,   glDeleteVertexArrays,         ( GLsizei n, const GLuint* arrays ) );
_AE_GL_IMPORT( void,   glDeleteBuffers,              ( GLsizei n, const GLuint* buffers ) );
_AE_GL_IMPORT( void,   glBindBuffer,                 ( GLenum target, GLuint buffer ) );
_AE_GL_IMPORT( void,   glGenBuffers,                 ( GLsizei n, GLuint* buffers ) );
_AE_GL_IMPORT( void,   glBufferData,                 ( GLenum target, GLsizeiptr size, const void* data, GLenum usage ) );
_AE_GL_IMPORT( void,   glBufferSubData,              ( GLenum target, GLintptr offset, GLsizeiptr size, const void* data ) );
_AE_GL_IMPORT( void,   glEnableVertexAttribArray,    ( GLuint index ) );
_AE_GL_IMPORT( void,   glVertexAttribPointer,        ( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer ) );
_AE_GL_IMPORT( void,   glVertexAttribDivisor,        ( GLuint index, GLuint divisor ) );
_AE_GL_IMPORT( void,   glDrawElementsInstanced,      ( GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount ) );
_AE_GL_IMPORT( void,   glDrawArraysInstanced,        ( GLenum mode, GLint first, GLsizei count, GLsizei instancecount ) );
_AE_GL_IMPORT( void,   glTexImage2D,                 ( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels ) );
_AE_GL_IMPORT( void,   glTexSubImage2D,              ( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels ) );
_AE_GL_IMPORT( void,   glTexStorage2D,               ( GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height ) );

#ifndef GL_NV_texture_barrier
_AE_GL_IMPORT( void,   glTextureBarrierNV,           () );
#endif

// Additional formats needed by aether texture code
#define GL_DEPTH_COMPONENT                          0x1902
#define GL_BGR                                      0x80E0
#define GL_BGRA                                     0x80E1

#undef _AE_GL_IMPORT
