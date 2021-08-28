//------------------------------------------------------------------------------
// 02_Sprites.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2021 John Hughes
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
// Headers
//------------------------------------------------------------------------------
#include "ae/aetherEXT.h"
#include "Common.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const uint32_t kMaxSpriteCount = 32;

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
  AE_UNIFORM mat4 u_worldToProj;

  AE_IN_HIGHP vec4 a_position;
  AE_IN_HIGHP vec4 a_color;
  AE_IN_HIGHP vec2 a_uv;

  AE_OUT_HIGHP vec4 v_color;
  AE_OUT_HIGHP vec2 v_uv;

  void main()
  {
    v_color = a_color;
    v_uv = a_uv;
    gl_Position = u_worldToProj * a_position;
  }
)";

const char* kFragShader = R"(
  AE_UNIFORM sampler2D u_tex;

  AE_IN_HIGHP vec4 v_color;
  AE_IN_HIGHP vec2 v_uv;

  void main()
  {
    AE_COLOR = AE_TEXTURE2D( u_tex, v_uv ) * v_color;
  }
)";

//------------------------------------------------------------------------------
// Cube
//------------------------------------------------------------------------------
struct SpriteVertex
{
  ae::Vec4 pos;
  ae::Vec4 color;
  ae::Vec2 uv;
};

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );
  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::TimeStep timeStep;
  ae::Texture2D spriteTex;
  ae::Shader spriteShader;
  ae::VertexData vertexData;
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "sprites" );
  render.Initialize( &window );
  input.Initialize( &window );
  timeStep.SetTimeStep( 1.0f / 60.0f );
  
  // Sprites
  LoadPng( &spriteTex, "circle.png", ae::Texture::Filter::Linear, ae::Texture::Wrap::Repeat, false, true );
  spriteShader.Initialize( kVertShader, kFragShader, nullptr, 0 );
  spriteShader.SetBlending( true );
  vertexData.Initialize( sizeof(SpriteVertex), 2, 4 * kMaxSpriteCount, 6 * kMaxSpriteCount, ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Dynamic, ae::VertexData::Usage::Dynamic );
  vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof(SpriteVertex, pos) );
  vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof(SpriteVertex, color) );
  vertexData.AddAttribute( "a_uv", 2, ae::VertexData::Type::Float, offsetof(SpriteVertex, uv) );
  auto AddSprite = [ &vertexData ]( ae::Matrix4 localToWorld, ae::Rect uvs, ae::Color color )
  {
    const uint16_t spriteIndices[] = { 3, 0, 1, 3, 1, 2 };
    vertexData.AppendIndices( spriteIndices, countof(spriteIndices), vertexData.GetVertexCount() );

    ae::Vec2 min = uvs.GetMin();
    ae::Vec2 max = uvs.GetMax();
    SpriteVertex spriteVerts[] =
    {
      { .pos = localToWorld * ae::Vec4( -0.5f, -0.5f, 0.0f, 1.0f ), .color = color.GetLinearRGBA(), .uv = ae::Vec2( min.x, min.y ) },
      { .pos = localToWorld * ae::Vec4( 0.5f, -0.5f, 0.0f, 1.0f ), .color = color.GetLinearRGBA(), .uv = ae::Vec2( max.x, min.y ) },
      { .pos = localToWorld * ae::Vec4( 0.5f, 0.5f, 0.0f, 1.0f ), .color = color.GetLinearRGBA(), .uv = ae::Vec2( max.x, max.y ) },
      { .pos = localToWorld * ae::Vec4( -0.5f, 0.5f, 0.0f, 1.0f ), .color = color.GetLinearRGBA(), .uv = ae::Vec2( min.x, max.y ) }
    };
    vertexData.AppendVertices( spriteVerts, countof(spriteVerts) );
  };

  while ( !input.quit )
  {
    input.Pump();
    render.Activate();
    render.Clear( ae::Color::PicoDarkPurple() );
    vertexData.ClearVertices();
    vertexData.ClearIndices();

    // Back
    ae::Matrix4 localToWorld = ae::Matrix4::Translation( ae::Vec3( -0.5f, -0.5f, 0.5f ) );
    localToWorld *= ae::Matrix4::Scaling( ae::Vec3( 1.0f, 1.0f, 0.0f ) );
    AddSprite( localToWorld, ae::Rect( 0.0f, 0.0f, 1.0f, 1.0f ), ae::Color::PicoRed() );

    // Middle
    localToWorld = ae::Matrix4::Scaling(  ae::Vec3( 1.0f, 1.0f, 0.5f ) );
    AddSprite( localToWorld, ae::Rect( 0.0f, 0.0f, 1.0f, 1.0f ), ae::Color::PicoGreen() );

    // Front
    localToWorld = ae::Matrix4::Translation( ae::Vec3( 0.5f, 0.5f, -0.5f ) );
    localToWorld *= ae::Matrix4::Scaling( ae::Vec3( 1.0f, 1.0f, 0.0f ) );
    AddSprite( localToWorld, ae::Rect( 0.0f, 0.0f, 1.0f, 1.0f ), ae::Color::PicoBlue() );

    ae::UniformList uniforms;
    uniforms.Set( "u_worldToProj", ae::Matrix4::Scaling( ae::Vec3( 1.0f / 2.0f, render.GetAspectRatio() / 2.0f, 1.0f ) ) );
    uniforms.Set( "u_tex", &spriteTex );
    vertexData.Upload();
    vertexData.Render( &spriteShader, uniforms, 0, 6 );
    render.Present();
    timeStep.Wait();
  }

  AE_LOG( "Terminate" );
  vertexData.Terminate();
  spriteShader.Terminate();
  spriteTex.Terminate();
  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
