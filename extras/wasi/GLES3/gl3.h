//------------------------------------------------------------------------------
// GLES3/gl3.h
//------------------------------------------------------------------------------
// The part of OpenGL ES 3.0 used by imgui's OpenGL 3 backend, imported from the
// 'gl' module that aether.js provides. Only for wasi-sdk builds, which have no
// GL headers. glGetString() returns null.
//------------------------------------------------------------------------------
#ifndef AE_WASI_GLES3_GL3_H
#define AE_WASI_GLES3_GL3_H

#include <stdint.h>

typedef uint32_t GLenum;
typedef uint8_t GLboolean;
typedef uint32_t GLbitfield;
typedef int32_t GLint;
typedef int32_t GLsizei;
typedef uint32_t GLuint;
typedef float GLfloat;
typedef char GLchar;
typedef uint8_t GLubyte;
typedef void GLvoid;
typedef intptr_t GLsizeiptr;
typedef intptr_t GLintptr;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_NO_ERROR 0
#define GL_ONE 1
#define GL_TRIANGLES 0x0004
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_STENCIL_TEST 0x0B90
#define GL_VIEWPORT 0x0BA2
#define GL_BLEND 0x0BE2
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_TEXTURE_2D 0x0DE1
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_RGBA 0x1908
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FUNC_ADD 0x8006
#define GL_BLEND_EQUATION_RGB 0x8009
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_TEXTURE0 0x84C0
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_STREAM_DRAW 0x88E0
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_SAMPLER_BINDING 0x8919
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_CURRENT_PROGRAM 0x8B8D

#ifdef __cplusplus
extern "C" {
#endif
#define _AE_GLES3_IMPORT( name ) __attribute__(( import_module( "gl" ), import_name( #name ) ))
_AE_GLES3_IMPORT( glActiveTexture ) void glActiveTexture( GLenum texture );
_AE_GLES3_IMPORT( glAttachShader ) void glAttachShader( GLuint program, GLuint shader );
_AE_GLES3_IMPORT( glBindBuffer ) void glBindBuffer( GLenum target, GLuint buffer );
_AE_GLES3_IMPORT( glBindSampler ) void glBindSampler( GLuint unit, GLuint sampler );
_AE_GLES3_IMPORT( glBindTexture ) void glBindTexture( GLenum target, GLuint texture );
_AE_GLES3_IMPORT( glBindVertexArray ) void glBindVertexArray( GLuint array );
_AE_GLES3_IMPORT( glBlendEquation ) void glBlendEquation( GLenum mode );
_AE_GLES3_IMPORT( glBlendEquationSeparate ) void glBlendEquationSeparate( GLenum modeRGB, GLenum modeAlpha );
_AE_GLES3_IMPORT( glBlendFuncSeparate ) void glBlendFuncSeparate( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha );
_AE_GLES3_IMPORT( glBufferData ) void glBufferData( GLenum target, GLsizeiptr size, const void* data, GLenum usage );
_AE_GLES3_IMPORT( glBufferSubData ) void glBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void* data );
_AE_GLES3_IMPORT( glClear ) void glClear( GLbitfield mask );
_AE_GLES3_IMPORT( glClearColor ) void glClearColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
_AE_GLES3_IMPORT( glCompileShader ) void glCompileShader( GLuint shader );
_AE_GLES3_IMPORT( glCreateProgram ) GLuint glCreateProgram();
_AE_GLES3_IMPORT( glCreateShader ) GLuint glCreateShader( GLenum type );
_AE_GLES3_IMPORT( glDeleteBuffers ) void glDeleteBuffers( GLsizei n, const GLuint* buffers );
_AE_GLES3_IMPORT( glDeleteProgram ) void glDeleteProgram( GLuint program );
_AE_GLES3_IMPORT( glDeleteSamplers ) void glDeleteSamplers( GLsizei count, const GLuint* samplers );
_AE_GLES3_IMPORT( glDeleteShader ) void glDeleteShader( GLuint shader );
_AE_GLES3_IMPORT( glDeleteTextures ) void glDeleteTextures( GLsizei n, const GLuint* textures );
_AE_GLES3_IMPORT( glDeleteVertexArrays ) void glDeleteVertexArrays( GLsizei n, const GLuint* arrays );
_AE_GLES3_IMPORT( glDetachShader ) void glDetachShader( GLuint program, GLuint shader );
_AE_GLES3_IMPORT( glDisable ) void glDisable( GLenum cap );
_AE_GLES3_IMPORT( glDisableVertexAttribArray ) void glDisableVertexAttribArray( GLuint index );
_AE_GLES3_IMPORT( glDrawElements ) void glDrawElements( GLenum mode, GLsizei count, GLenum type, const void* indices );
_AE_GLES3_IMPORT( glEnable ) void glEnable( GLenum cap );
_AE_GLES3_IMPORT( glEnableVertexAttribArray ) void glEnableVertexAttribArray( GLuint index );
_AE_GLES3_IMPORT( glGenBuffers ) void glGenBuffers( GLsizei n, GLuint* buffers );
_AE_GLES3_IMPORT( glGenSamplers ) void glGenSamplers( GLsizei count, GLuint* samplers );
_AE_GLES3_IMPORT( glGenTextures ) void glGenTextures( GLsizei n, GLuint* textures );
_AE_GLES3_IMPORT( glGenVertexArrays ) void glGenVertexArrays( GLsizei n, GLuint* arrays );
_AE_GLES3_IMPORT( glGetAttribLocation ) GLint glGetAttribLocation( GLuint program, const GLchar* name );
_AE_GLES3_IMPORT( glGetError ) GLenum glGetError();
_AE_GLES3_IMPORT( glGetIntegerv ) void glGetIntegerv( GLenum pname, GLint* data );
_AE_GLES3_IMPORT( glGetProgramInfoLog ) void glGetProgramInfoLog( GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog );
_AE_GLES3_IMPORT( glGetProgramiv ) void glGetProgramiv( GLuint program, GLenum pname, GLint* params );
_AE_GLES3_IMPORT( glGetShaderInfoLog ) void glGetShaderInfoLog( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog );
_AE_GLES3_IMPORT( glGetShaderiv ) void glGetShaderiv( GLuint shader, GLenum pname, GLint* params );
_AE_GLES3_IMPORT( glGetString ) const GLubyte* glGetString( GLenum name );
_AE_GLES3_IMPORT( glGetUniformLocation ) GLint glGetUniformLocation( GLuint program, const GLchar* name );
_AE_GLES3_IMPORT( glGetVertexAttribiv ) void glGetVertexAttribiv( GLuint index, GLenum pname, GLint* params );
_AE_GLES3_IMPORT( glGetVertexAttribPointerv ) void glGetVertexAttribPointerv( GLuint index, GLenum pname, void** pointer );
_AE_GLES3_IMPORT( glIsEnabled ) GLboolean glIsEnabled( GLenum cap );
_AE_GLES3_IMPORT( glIsProgram ) GLboolean glIsProgram( GLuint program );
_AE_GLES3_IMPORT( glLinkProgram ) void glLinkProgram( GLuint program );
_AE_GLES3_IMPORT( glPixelStorei ) void glPixelStorei( GLenum pname, GLint param );
_AE_GLES3_IMPORT( glSamplerParameteri ) void glSamplerParameteri( GLuint sampler, GLenum pname, GLint param );
_AE_GLES3_IMPORT( glScissor ) void glScissor( GLint x, GLint y, GLsizei width, GLsizei height );
_AE_GLES3_IMPORT( glShaderSource ) void glShaderSource( GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length );
_AE_GLES3_IMPORT( glTexImage2D ) void glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels );
_AE_GLES3_IMPORT( glTexParameteri ) void glTexParameteri( GLenum target, GLenum pname, GLint param );
_AE_GLES3_IMPORT( glTexSubImage2D ) void glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels );
_AE_GLES3_IMPORT( glUniform1i ) void glUniform1i( GLint location, GLint v0 );
_AE_GLES3_IMPORT( glUniformMatrix4fv ) void glUniformMatrix4fv( GLint location, GLsizei count, GLboolean transpose, const GLfloat* value );
_AE_GLES3_IMPORT( glUseProgram ) void glUseProgram( GLuint program );
_AE_GLES3_IMPORT( glVertexAttribPointer ) void glVertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer );
_AE_GLES3_IMPORT( glViewport ) void glViewport( GLint x, GLint y, GLsizei width, GLsizei height );
#undef _AE_GLES3_IMPORT
#ifdef __cplusplus
}
#endif

#endif
