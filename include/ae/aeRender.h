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
#include "aether.h"

//------------------------------------------------------------------------------
// aeRender constants
//------------------------------------------------------------------------------
typedef uint32_t aeQuadIndex;
const uint32_t aeQuadVertCount = 4;
const uint32_t aeQuadIndexCount = 6;
extern const ae::Vec3 aeQuadVertPos[ aeQuadVertCount ];
extern const ae::Vec2 aeQuadVertUvs[ aeQuadVertCount ];
extern const aeQuadIndex aeQuadIndices[ aeQuadIndexCount ];

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
  void Render( const ae::Matrix4& worldToScreen );

  void SetBlending( bool enabled );
  void SetDepthTest( bool enabled );
  void SetDepthWrite( bool enabled );
  void SetSorting( bool enabled );

  // @NOTE: Each sprite is also transformed by the Render( worldToScreen ) above
  void AddSprite( const ae::Texture2D* texture, ae::Matrix4 transform, ae::Vec2 uvMin, ae::Vec2 uvMax, ae::Color color );
  void Clear();

private:
  void m_Render( const ae::Matrix4& worldToScreen, ae::Shader* shader );
  void m_LoadShaderAll();
  void m_LoadShaderOpaque();
  void m_LoadShaderTransparent();
  
  struct Vertex
  {
    ae::Vec3 pos;
    ae::Vec4 color;
    ae::Vec2 uv;
  };

  struct Sprite
  {
    ae::Matrix4 transform;
    ae::Vec2 uvMin;
    ae::Vec2 uvMax;
    ae::Color color;
    uint32_t textureId;
    float sort;
  };

  uint32_t m_count;
  uint32_t m_maxCount;
  Sprite* m_sprites;
  ae::VertexData m_vertexData;
  ae::Shader* m_shaderAll;
  ae::Shader* m_shaderOpaque;
  ae::Shader* m_shaderTransparent;
  ae::Map< const ae::Texture2D*, uint32_t > m_textures = AE_ALLOC_TAG_RENDER;
  
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
  //        'fontSize' is the width and height of each character in the texture.
  void Initialize( const ae::Texture2D* texture, uint32_t fontSize );
  void Terminate();
  void Render( const ae::Matrix4& uiToScreen );

  uint32_t GetFontSize() const { return m_fontSize; }

  void Add( ae::Vec3 pos, ae::Vec2 size, const char* str, ae::Color color, uint32_t lineLength, uint32_t charLimit );
  uint32_t GetLineCount( const char* str, uint32_t lineLength, uint32_t charLimit ) const;

private:
  uint32_t m_ParseText( const char* str, uint32_t lineLength, uint32_t charLimit, ae::Str512* outText ) const;

  static const uint32_t kMaxTextRects = 32;

  struct Vertex
  {
    ae::Vec3 pos;
    ae::Vec2 uv;
    ae::Color color;
  };

  struct TextRect
  {
    ae::Str512 text;
    ae::Vec3 pos;
    ae::Vec2 size;
    ae::Color color;
  };

  uint32_t m_fontSize;

  ae::VertexData m_vertexData;
  ae::Shader m_shader;
  const ae::Texture2D* m_texture = nullptr;

  uint32_t m_rectCount;
  TextRect m_rects[ kMaxTextRects ];
};

#endif
