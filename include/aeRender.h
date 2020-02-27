//------------------------------------------------------------------------------
// aeRender.h
//------------------------------------------------------------------------------
// Copyright (c) 2020 John Hughes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------
#ifndef AERENDER_H
#define AERENDER_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeMap.h"
#include "aeMath.h"
#include "aeString.h"

//------------------------------------------------------------------------------
// aeRender constants
//------------------------------------------------------------------------------
const uint32_t kMaxShaderAttributeCount = 16;
const uint32_t kMaxShaderAttributeNameLength = 16;
const uint32_t kMaxShaderDefines = 4;

typedef uint32_t aeQuadIndex;
const uint32_t aeQuadVertCount = 4;
const uint32_t aeQuadIndexCount = 6;
extern const aeFloat3 aeQuadVertPos[ aeQuadVertCount ];
extern const aeFloat2 aeQuadVertUvs[ aeQuadVertCount ];
extern const aeQuadIndex aeQuadIndices[ aeQuadIndexCount ];

//------------------------------------------------------------------------------
// aeColor struct
//------------------------------------------------------------------------------
struct aeColor
{
  aeColor() {}
  aeColor( const aeColor& ) = default;
  aeColor( float rgb );
  aeColor( float r, float g, float b );
  aeColor( float r, float g, float b, float a );
  aeColor( aeColor c, float a );

  static aeColor SRGB( float r, float g, float b );
  static aeColor SRGBA( float r, float g, float b, float a );
  static aeColor PS( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 );

  // @TODO: Remove
  explicit operator aeFloat3() const { return aeFloat3( r, g, b ); }
  explicit operator aeFloat4() const { return aeFloat4( r, g, b, a ); }

  aeFloat3 GetLinearRGB() const { return aeFloat3( *this ); }
  aeFloat4 GetLinearRGBA() const { return aeFloat4( *this ); }
  aeFloat3 GetSRGB() const;
  aeFloat4 GetSRGBA() const;

  aeColor Lerp( const aeColor& end, float t ) const;
  aeColor ScaleRGB( float s ) const { return aeColor( r * s, g * s, b * s, a ); }
  aeColor ScaleA( float s ) const { return aeColor( r, g, b, a * s ); }
  aeColor SetA( float alpha ) const { return aeColor( r, g, b, alpha ); }

  static float SRGBToRGB( float x );
  static float RGBToSRGB( float x );

  static const aeColor White;
  static const aeColor Red;
  static const aeColor Green;
  static const aeColor Blue;
  static const aeColor Orange;
  static const aeColor Gray;
  static const aeColor Black;

  float r;
  float g;
  float b;
  float a;
};

inline std::ostream& operator<<( std::ostream& os, aeColor c )
{
  return os << "<" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ">";
}

//------------------------------------------------------------------------------
// aeVertex types
//------------------------------------------------------------------------------
struct aeVertexUsage
{
  enum Type
  {
    Dynamic,
    Static
  };
};

struct aeVertexDataType
{
  enum Type
  {
    UInt8,
    UInt16,
    UInt32,
    Float
  };
};

struct aeVertexPrimitive
{
  enum Type
  {
    Point,
    Line,
    Triangle
  };
};

struct aeVertexAttribute
{
  char name[ kMaxShaderAttributeNameLength ];
  uint32_t componentCount;
  uint32_t type; // GL_BYTE, GL_SHORT, GL_FLOAT...
  uint32_t offset;
};

//------------------------------------------------------------------------------
// aeUniformList class
//------------------------------------------------------------------------------
class aeUniformList
{
public:
  struct Value
  {
    uint32_t sampler = 0;
    int32_t size = 0;
    aeFloat4x4 value;
  };

  void Set( const char* name, float value );
  void Set( const char* name, aeFloat2 value );
  void Set( const char* name, aeFloat3 value );
  void Set( const char* name, aeFloat4 value );
  void Set( const char* name, const aeFloat4x4& value );
  void Set( const char* name, const class aeTexture* tex );

  const Value* Get( const char* name ) const;

private:
  aeMap< aeStr32, Value > m_uniforms;
};

//------------------------------------------------------------------------------
// aeVertexData class
//------------------------------------------------------------------------------
class aeVertexData
{
public:
  aeVertexData();

  void Initialize( uint32_t vertexSize, uint32_t indexSize, uint32_t maxVertexCount, uint32_t maxIndexCount, aeVertexPrimitive::Type primitive, aeVertexUsage::Type vertexUsage, aeVertexUsage::Type indexUsage );
  void AddAttribute( const char *name, uint32_t componentCount, aeVertexDataType::Type type, uint32_t offset );
  void Destroy();

  void SetVertices( const void* vertices, uint32_t count );
  void SetIndices( const void* indices, uint32_t count );
  
  const void* GetVertices() const;
  const void* GetIndices() const;
  uint32_t GetVertexSize() const { return m_vertexSize; }
  uint32_t GetIndexSize() const { return m_indexSize; }
  
  uint32_t GetVertexCount() const { return m_vertexCount; }
  uint32_t GetIndexCount() const { return m_indexCount; }
  uint32_t GetMaxVertexCount() const { return m_maxVertexCount; }
  uint32_t GetMaxIndexCount() const { return m_maxIndexCount; }
  uint32_t GetAttributeCount() const { return m_attributeCount; }
  aeVertexPrimitive::Type GetPrimitiveType() const { return m_primitive; }

  void Render( const class aeShader* shader, const aeUniformList& uniforms );
  void Render( const class aeShader* shader, uint32_t primitiveCount, const aeUniformList& uniforms );
  
private:
  void m_SetVertices( const void* vertices, uint32_t count );
  void m_SetIndices( const void* indices, uint32_t count );
  const aeVertexAttribute* m_GetAttributeByName( const char* name ) const;
  
  uint32_t m_array;
  uint32_t m_vertices;
  uint32_t m_indices;
  uint32_t m_vertexCount;
  uint32_t m_indexCount;
  
  uint32_t m_maxVertexCount;
  uint32_t m_maxIndexCount;
  
  aeVertexPrimitive::Type m_primitive;
  aeVertexUsage::Type m_vertexUsage;
  aeVertexUsage::Type m_indexUsage;
  
  aeVertexAttribute m_attributes[ kMaxShaderAttributeCount ];
  uint32_t m_attributeCount;
  uint32_t m_vertexSize;
  uint32_t m_indexSize;
  
  void* m_vertexReadable;
  void* m_indexReadable;
};

//------------------------------------------------------------------------------
// aeShader types
//------------------------------------------------------------------------------
struct aeShaderType
{
  enum Type
  {
    Vertex,
    Fragment
  };
};

struct aeShaderAttribute
{
  char name[ kMaxShaderAttributeNameLength ];
  uint32_t type; // GL_FLOAT, GL_FLOAT_VEC4, GL_FLOAT_MAT4...
  int32_t location;
};

struct aeShaderUniform
{
  aeStr32 name;
  uint32_t type;
  int32_t location;
};

//------------------------------------------------------------------------------
// aeShader class
//------------------------------------------------------------------------------
class aeShader
{
public:
  aeShader();
  ~aeShader();
  
  void Initialize( const char* vertexStr, const char* fragStr, const char* const* defines, int32_t defineCount );
  void Destroy();

  void SetBlending( bool enabled ) { m_blending = enabled; }
  void SetDepthTest( bool enabled ) { m_depthTest = enabled; }
  void SetDepthWrite( bool enabled ) { m_depthWrite = enabled; }
  
private:
  int m_LoadShader( const char* shaderStr, aeShaderType::Type type, const char* const* defines, int32_t defineCount );
  
  uint32_t m_fragmentShader;
  uint32_t m_vertexShader;
  uint32_t m_program;

  bool m_blending;
  bool m_depthTest;
  bool m_depthWrite;
  
  aeShaderAttribute m_attributes[ kMaxShaderAttributeCount ];
  uint32_t m_attributeCount;

  aeMap< aeStr32, aeShaderUniform > m_uniforms;

public:
  // Internal
  void Activate( const aeUniformList& uniforms ) const;
  const aeShaderAttribute* GetAttributeByIndex( uint32_t index ) const;
  uint32_t GetAttributeCount() const { return m_attributeCount; }
};

//------------------------------------------------------------------------------
// aeTexture
//------------------------------------------------------------------------------
bool get_png_info( const uint8_t* data, uint32_t length, uint32_t* widthOut, uint32_t* heightOut, uint32_t* depthOut );
bool load_png( const uint8_t* data, uint32_t length, uint8_t* dataOut, uint32_t maxOut );

struct aeTextureFilter
{
  enum Type
  {
    Linear,
    Nearest
  };
};

struct aeTextureWrap
{
  enum Type
  {
    Repeat,
    Clamp
  };
};

class aeTexture
{
public:
  aeTexture() : m_texture( 0 ) {}
  uint32_t GetTexture() const { return m_texture; }

protected:
  uint32_t m_texture;
};

class aeTexture2D : public aeTexture
{
public:
  aeTexture2D();
  // @TODO: Rename depth or change it to an alpha bool
  void Initialize( const uint8_t* data, uint32_t width, uint32_t height, uint32_t depth, aeTextureFilter::Type filter, aeTextureWrap::Type wrap );
  void Initialize( const char* file, aeTextureFilter::Type filter, aeTextureWrap::Type wrap );
  void Destroy();

  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }

private:
  uint32_t m_width;
  uint32_t m_height;
  bool m_hasAlpha;
};

class aeRenderTexture : public aeTexture
{
public:
  aeRenderTexture();
  void Initialize( uint32_t width, uint32_t height, aeTextureFilter::Type filter, aeTextureWrap::Type wrap );
  void Destroy();

  void Activate();
  void Render2D( aeRect ndc, float z );

  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }

  static aeFloat4x4 GetQuadToNDCTransform( aeRect ndc, float z );

private:
  struct Vertex
  {
    aeFloat3 pos;
    aeFloat2 uv;
  };

  uint32_t m_fbo;

  uint32_t m_width;
  uint32_t m_height;

  aeVertexData m_quad;
  aeShader m_shader;
};

//------------------------------------------------------------------------------
// aeSpriteRenderer class
//------------------------------------------------------------------------------
class aeSpriteRenderer
{
public:
  void Initialize( uint32_t maxCount );
  void Destroy();
  // @TODO: change name to ndc space or remove and just use AddSprite transform
  void Render( const aeFloat4x4& worldToScreen );

  // @NOTE: Each sprite is also transformed by the Render( worldToScreen ) above
  void AddSprite( const aeTexture2D* texture, aeFloat4x4 transform, aeFloat2 uvMin, aeFloat2 uvMax, aeColor color );
  void Clear();

private:
  struct Vertex
  {
    aeFloat3 pos;
    aeFloat4 color;
    aeFloat2 uv;
  };

  struct Sprite
  {
    aeFloat4x4 transform;
    aeFloat2 uvMin;
    aeFloat2 uvMax;
    aeColor color;
    uint32_t textureId;
  };

  uint32_t m_count;
  uint32_t m_maxCount;
  Sprite* m_sprites;
  aeVertexData m_vertexData;
  aeShader m_shader;
  aeMap< const aeTexture2D*, uint32_t > m_textures;
};

//------------------------------------------------------------------------------
// aeTextRenderer class
//------------------------------------------------------------------------------
class aeTextRenderer
{
public:
  // @NOTE: 'imagePath' should be the path to a square texture with ascii
  //        characters evenly spaced from top left to bottom right. The
  //        texture can be a single channel without transparency. Luminance
  //        of the red channel is used for transparency.
  //        'charSize' is the width and height of each character in the texture.
  void Initialize( const char* imagePath, aeTextureFilter::Type filterType, uint32_t charSize );
  void Terminate();
  void Render( const aeFloat4x4& uiToScreen );

  void Add( aeFloat2 pos, aeFloat2 size, const char* str, aeColor color, uint32_t lineLength, uint32_t charLimit );
  uint32_t GetLineCount( const char* str, uint32_t lineLength, uint32_t charLimit ) const;

private:
  uint32_t m_ParseText( const char* str, uint32_t lineLength, uint32_t charLimit, aeStr512* outText ) const;

  static const uint32_t kMaxTextRects = 32;

  struct Vertex
  {
    aeFloat3 pos;
    aeFloat2 uv;
    aeColor color;
  };

  struct TextRect
  {
    aeStr512 text;
    aeFloat2 pos;
    aeFloat2 size;
    aeColor color;
  };

  uint32_t m_charSize;

  aeVertexData m_vertexData;
  aeShader m_shader;
  aeTexture2D m_texture;

  uint32_t m_rectCount;
  TextRect m_rects[ kMaxTextRects ];
};

//------------------------------------------------------------------------------
// aeRenderer class
//------------------------------------------------------------------------------
class aeRenderer
{
public:
  aeRenderer();
  void Initialize( class aeWindow* window, uint32_t width, uint32_t height );
  void Terminate();
  void StartFrame();
  void EndFrame();

  void Resize( uint32_t width, uint32_t height );

  void SetClearColor( aeColor color );
  aeColor GetClearColor() const;

  uint32_t GetWidth() const { return m_targetWidth; }
  uint32_t GetHeight() const { return m_targetHeight; }
  float GetAspectRatio() const { return m_targetWidth / (float)m_targetHeight; }

  aeFloat4x4 GetWindowToRenderTransform();

private:
  aeRect m_GetNDCRect() const;

  class aeWindow* m_window;
  void* m_context;

  uint32_t m_targetWidth;
  uint32_t m_targetHeight;

  aeRenderTexture m_canvas;

  aeColor m_clearColor;

  int32_t m_defaultFbo;
};

#endif
