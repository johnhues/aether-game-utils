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
  ~aeSpriteRender();
  void Initialize( uint32_t maxCount );
  void Destroy();
  void Render( const ae::Matrix4& localToProjection );

  void SetBlending( bool enabled );
  void SetDepthTest( bool enabled );
  void SetDepthWrite( bool enabled );
  void SetSorting( bool enabled );

  // @NOTE: Each sprite is also transformed by the Render( localToProjection ) transform above
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
    const ae::Texture2D* texture;
    float sort;
  };

  uint32_t m_count;
  uint32_t m_maxCount;
  Sprite* m_sprites;
  Vertex* m_vertices;
  ae::VertexArray m_vertexData;
  ae::Shader* m_shaderAll;
  ae::Shader* m_shaderOpaque;
  ae::Shader* m_shaderTransparent;
  
  bool m_blending;
  bool m_depthTest;
  bool m_depthWrite;
  bool m_sorting;
};

#endif
