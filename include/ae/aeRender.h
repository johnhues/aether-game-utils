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

  static aeColor R( uint8_t r );
  static aeColor RG( uint8_t r, uint8_t g );
  static aeColor RGB( uint8_t r, uint8_t g, uint8_t b );
  static aeColor RGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
  static aeColor R( float r );
  static aeColor RG( float r, float g );
  static aeColor RGB( float r, float g, float b );
  static aeColor RGBA( float r, float g, float b, float a );
  static aeColor RGBA( const float* v );
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
  aeColor DtLerp( float snappiness, float dt, const aeColor& target ) const;
  aeColor ScaleRGB( float s ) const { return aeColor( r * s, g * s, b * s, a ); }
  aeColor ScaleA( float s ) const { return aeColor( r, g, b, a * s ); }
  aeColor SetA( float alpha ) const { return aeColor( r, g, b, alpha ); }

  static float SRGBToRGB( float x );
  static float RGBToSRGB( float x );

  // Grayscale
  static aeColor White();
  static aeColor Gray();
  static aeColor Black();
  // Rainbow
  static aeColor Red();
  static aeColor Orange();
  static aeColor Yellow();
  static aeColor Green();
  static aeColor Blue();
  static aeColor Indigo();
  static aeColor Violet();
  // Pico
  static aeColor PicoBlack();
  static aeColor PicoDarkBlue();
  static aeColor PicoDarkPurple();
  static aeColor PicoDarkGreen();
  static aeColor PicoBrown();
  static aeColor PicoDarkGray();
  static aeColor PicoLightGray();
  static aeColor PicoWhite();
  static aeColor PicoRed();
  static aeColor PicoOrange();
  static aeColor PicoYellow();
  static aeColor PicoGreen();
  static aeColor PicoBlue();
  static aeColor PicoIndigo();
  static aeColor PicoPink();
  static aeColor PicoPeach();

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
    NormalizedUInt8,
    NormalizedUInt16,
    NormalizedUInt32,
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
  bool normalized;
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
    uint32_t target = 0;
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
  aeVertexData() = default;
  ~aeVertexData();

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
  aeVertexData( const aeVertexData& ) = delete;
  aeVertexData( aeVertexData&& ) = delete;
  void operator=( const aeVertexData& ) = delete;
  void operator=( aeVertexData&& ) = delete;

  void m_SetVertices( const void* vertices, uint32_t count );
  void m_SetIndices( const void* indices, uint32_t count );
  const aeVertexAttribute* m_GetAttributeByName( const char* name ) const;
  
  uint32_t m_array = 0;
  uint32_t m_vertices = ~0;
  uint32_t m_indices = ~0;
  uint32_t m_vertexCount = 0;
  uint32_t m_indexCount = 0;
  
  uint32_t m_maxVertexCount = 0;
  uint32_t m_maxIndexCount = 0;
  
  aeVertexPrimitive::Type m_primitive = (aeVertexPrimitive::Type)-1;
  aeVertexUsage::Type m_vertexUsage = (aeVertexUsage::Type)-1;
  aeVertexUsage::Type m_indexUsage = (aeVertexUsage::Type)-1;
  
  aeVertexAttribute m_attributes[ kMaxShaderAttributeCount ];
  uint32_t m_attributeCount = 0;
  uint32_t m_vertexSize = 0;
  uint32_t m_indexSize = 0;
  
  void* m_vertexReadable = nullptr;
  void* m_indexReadable = nullptr;
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

struct aeShaderCulling
{
  enum Type
  {
    None,
    ClockwiseFront,
    CounterclockwiseFront,
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
// @NOTE: Some special built in functions and defines are automatically included
//        for portability reasons (e.g. for OpenGL ES). There are also some
//        convenient helper functions to convert between linear and srgb color
//        spaces. It's not necessary to use any of these helpers and basic valid
//        GLSL can be provided instead.
//------------------------------------------------------------------------------
// Example vertex shader
/*
AE_UNIFORM_HIGHP mat4 u_worldToScreen;

AE_IN_HIGHP vec3 a_position;
AE_IN_HIGHP vec2 a_uv;
AE_IN_HIGHP vec4 a_color;

AE_OUT_HIGHP vec2 v_uv;
AE_OUT_HIGHP vec4 v_color;

void main()
{
  v_uv = a_uv;
  v_color = a_color;
  gl_Position = u_worldToScreen * vec4( a_position, 1.0 );
}
*/
// Example fragment shader
/*
AE_UNIFORM sampler2D u_tex;

AE_IN_HIGHP vec2 v_uv;
AE_IN_HIGHP vec4 v_color;

void main()
{
  AE_COLOR = AE_RGBA_TO_SRGBA( AE_SRGBA_TO_RGBA( AE_TEXTURE2D( u_tex, v_uv ) ) * v_color );
}
*/
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
  void SetCulling( aeShaderCulling::Type culling ) { m_culling = culling; }
  void SetWireframe( bool enabled ) { m_wireframe = enabled; }
  void SetBlendingPremul( bool enabled ) { m_blendingPremul = enabled; }
	
private:
  int m_LoadShader( const char* shaderStr, aeShaderType::Type type, const char* const* defines, int32_t defineCount );
  
  uint32_t m_fragmentShader;
  uint32_t m_vertexShader;
  uint32_t m_program;

  bool m_blending;
  bool m_blendingPremul;
  bool m_depthTest;
  bool m_depthWrite;
  aeShaderCulling::Type m_culling;
  bool m_wireframe;
  
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

struct aeTextureFormat
{
  enum Type
  {
	  Depth32F,
	  
	  R8, // unorm
	  R16_UNORM, // for height fields
	  R16F,
	  R32F,
	  
	  RG8, // unorm
	  RG16F,
	  RG32F,
	  
	  RGB8, // unorm
	  RGB8_SRGB,
	  RGB16F,
	  RGB32F,
	  
	  RGBA8, // unorm
	  RGBA8_SRGB,
	  RGBA16F,
	  RGBA32F,
	
	// non-specific formats, prefer specific types above
    R = RGBA8,
	RG = RG8,
    RGB = RGB8,
    RGBA = RGBA8,
	  
	Depth = Depth32F,
	  
    // TODO: these formats are implemented for OSX only for now
	SRGB = RGB8_SRGB,
    SRGBA = RGBA8_SRGB,
  };
};

struct aeTextureType
{
  enum Type
  {
    Uint8,
	Uint16,
    HalfFloat,
    Float
  };
};

class aeTexture
{
public:
  aeTexture() = default;
  virtual ~aeTexture();

  void Initialize( uint32_t target );
  virtual void Destroy();

  uint32_t GetTexture() const { return m_texture; }
  uint32_t GetTarget() const { return m_target; }

private:
  aeTexture( const aeTexture& ) = delete;
  aeTexture( aeTexture&& ) = delete;
  void operator=( const aeTexture& ) = delete;
  void operator=( aeTexture&& ) = delete;

  uint32_t m_texture = 0;
  uint32_t m_target = 0;
};

class aeTexture2D : public aeTexture
{
public:
  void Initialize( const void* data, uint32_t width, uint32_t height, aeTextureFormat::Type format, aeTextureType::Type type, aeTextureFilter::Type filter, aeTextureWrap::Type wrap, bool autoGenerateMipmaps = false );
  void Initialize( const char* file, aeTextureFilter::Type filter, aeTextureWrap::Type wrap, bool autoGenerateMipmaps = false,
	  bool isSRGB = false,
	  bool is16BitImage = false );
  void Destroy() override;

  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }

private:
  uint32_t m_width = 0;
  uint32_t m_height = 0;
  bool m_hasAlpha = false;
};

class aeRenderTarget
{
public:
  ~aeRenderTarget();
  void Initialize( uint32_t width, uint32_t height );
  void AddTexture( aeTextureFilter::Type filter, aeTextureWrap::Type wrap );
  void AddDepth( aeTextureFilter::Type filter, aeTextureWrap::Type wrap );
  void Destroy();

  void Activate();
  void Clear( aeColor color );
  void Render( const aeShader* shader, const aeUniformList& uniforms );
  void Render2D( uint32_t textureIndex, aeRect ndc, float z );

  const aeTexture2D* GetTexture( uint32_t index ) const;
  const aeTexture2D* GetDepth() const;
  uint32_t GetWidth() const;
  uint32_t GetHeight() const;

  static aeFloat4x4 GetQuadToNDCTransform( aeRect ndc, float z );

private:
  struct Vertex
  {
    aeFloat3 pos;
    aeFloat2 uv;
  };

  uint32_t m_fbo = 0;

  aeArray< aeTexture2D* > m_targets;
  aeTexture2D m_depth;

  uint32_t m_width = 0;
  uint32_t m_height = 0;

  aeVertexData m_quad;
  aeShader m_shader;
};

//------------------------------------------------------------------------------
// aeSpriteRender class
//------------------------------------------------------------------------------
class aeSpriteRender
{
public:
  aeSpriteRender();
  void Initialize( uint32_t maxCount );
  void Destroy();
  // @TODO: change name to ndc space or remove and just use AddSprite transform
  void Render( const aeFloat4x4& worldToScreen );

  void SetBlending( bool enabled );
  void SetDepthTest( bool enabled );
  void SetDepthWrite( bool enabled );
  void SetSorting( bool enabled );

  // @NOTE: Each sprite is also transformed by the Render( worldToScreen ) above
  void AddSprite( const aeTexture2D* texture, aeFloat4x4 transform, aeFloat2 uvMin, aeFloat2 uvMax, aeColor color );
  void Clear();

private:
  void m_Render( const aeFloat4x4& worldToScreen, aeShader* shader );
  void m_LoadShaderAll();
  void m_LoadShaderOpaque();
  void m_LoadShaderTransparent();
  
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
    float sort;
  };

  uint32_t m_count;
  uint32_t m_maxCount;
  Sprite* m_sprites;
  aeVertexData m_vertexData;
  aeShader* m_shaderAll;
  aeShader* m_shaderOpaque;
  aeShader* m_shaderTransparent;
  aeMap< const aeTexture2D*, uint32_t > m_textures;
  
  bool m_blending;
  bool m_depthTest;
  bool m_depthWrite;
  bool m_sorting;
};

//------------------------------------------------------------------------------
// aeTextRender class
//------------------------------------------------------------------------------
class aeTextRender
{
public:
  // @NOTE: 'imagePath' should be the path to a square texture with ascii
  //        characters evenly spaced from top left to bottom right. The
  //        texture can be a single channel without transparency. Luminance
  //        of the red channel is used for transparency.
  //        'charSize' is the width and height of each character in the texture.
  void Initialize( const char* imagePath, aeTextureFilter::Type filterType, uint32_t fontSize ); // @TODO: Text render should take a texture and the user should handle file loading
  void Terminate();
  void Render( const aeFloat4x4& uiToScreen );

  uint32_t GetFontSize() const { return m_fontSize; }

  void Add( aeFloat3 pos, aeFloat2 size, const char* str, aeColor color, uint32_t lineLength, uint32_t charLimit );
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
    aeFloat3 pos;
    aeFloat2 size;
    aeColor color;
  };

  uint32_t m_fontSize;

  aeVertexData m_vertexData;
  aeShader m_shader;
  aeTexture2D m_texture;

  uint32_t m_rectCount;
  TextRect m_rects[ kMaxTextRects ];
};

//------------------------------------------------------------------------------
// aeDebugRender class
//------------------------------------------------------------------------------
class aeDebugRender
{
public:
  void Initialize();
  void Destroy();
  void Render( const aeFloat4x4& worldToScreen );
  void Clear();

  void AddLine( aeFloat3 p0, aeFloat3 p1, aeColor color );
  void AddDistanceCheck( aeFloat3 p0, aeFloat3 p1, float distance );

  void AddRect( aeFloat3 pos, aeFloat3 up, aeFloat3 normal, aeFloat2 size, aeColor color );
  void AddCircle( aeFloat3 pos, aeFloat3 normal, float radius, aeColor color, uint32_t pointCount );

  void AddSphere( aeFloat3 pos, float radius, aeColor color, uint32_t pointCount );
  void AddAABB( aeFloat3 pos, aeFloat3 halfSize, aeColor color );
  void AddCube( aeFloat4x4 transform, aeColor color );

private:
  static const uint32_t kMaxDebugObjects = 128;

  struct DebugVertex
  {
    aeFloat3 pos;
    aeColor color;
  };
  aeArray< DebugVertex > m_verts;
  aeVertexData m_vertexData;
  aeShader m_shader;

  enum class DebugType
  {
    Line,
    Rect,
    Circle,
    Sphere,
    AABB,
    Cube,
  };

  struct DebugObject
  {
    DebugType type;
    aeFloat3 pos;
    aeFloat3 end;
    aeQuat rotation;
    aeFloat3 size;
    float radius;
    aeColor color;
    uint32_t pointCount; // circle only
    aeFloat4x4 transform;
  };
  uint32_t m_objCount;
  DebugObject m_objs[ kMaxDebugObjects ];
};

//------------------------------------------------------------------------------
// aeRender class
//------------------------------------------------------------------------------
class aeRender
{
public:
  aeRender();
  ~aeRender();

  void InitializeOpenGL( class aeWindow* window );
  void Terminate();

  void Activate();
  void Clear( aeColor color );
  void Present();

  class aeWindow* GetWindow() { return m_window; }
  aeRenderTarget* GetCanvas() { return &m_canvas; }

  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }
  float GetAspectRatio() const;

  aeFloat4x4 GetWindowToCanvasTransform() const; // Mouse to canvas
  aeFloat4x4 GetWindowToWorld( const aeFloat4x4& worldToNdc ) const; // Mouse to world
  aeRect GetNDCRect() const;

  // this is so imgui and the main render copy can enable srgb writes in (GL only)
  void EnableSRGBWrites( bool enable );
	
  // have to inject a barrier to readback from active render target (GL only)
  void AddTextureBarrier();

private:
  class aeRenderInternal* m_renderInternal;

  class aeWindow* m_window;
  uint32_t m_width;
  uint32_t m_height;

  aeRenderTarget m_canvas;
};

#endif
