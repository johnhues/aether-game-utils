//------------------------------------------------------------------------------
// 10_Splines.cpp
//------------------------------------------------------------------------------
// Copyright (c) 2025 John Hughes
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
#include "aether.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";

const char* kVertexShader = R"(
  AE_UNIFORM_HIGHP mat4 u_worldToProj;

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

const char* kFragmentShader = R"(
  AE_UNIFORM sampler2D u_tex;

  AE_IN_HIGHP vec4 v_color;
  AE_IN_HIGHP vec2 v_uv;

  void main()
  {
    AE_COLOR = v_color * AE_TEXTURE2D( u_tex, v_uv );
  }
)";

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  ae::FileSystem fileSystem;
  ae::Spline spline( TAG_EXAMPLE );
  ae::SpriteRenderer spriteRender = TAG_EXAMPLE;
  ae::Shader spriteShader;
  
  window.Initialize( 800, 600, false, true, true );
  window.SetTitle( "splines" );
  render.Initialize( &window );
  input.Initialize( &window );
  fileSystem.Initialize( "data", "ae", "splines" );
  spriteRender.Initialize( 1, 128 );
  spriteShader.Initialize( kVertexShader, kFragmentShader );
  spriteShader.SetBlending( true );
  
  spline.AppendControlPoint( ae::Vec3( -0.4f, -2.0f, 0.0f ) );
  spline.AppendControlPoint( ae::Vec3( -2.0f, 2.0f, 0.0f ) );
  spline.AppendControlPoint( ae::Vec3( 0.0f, 1.0f, 0.0f ) );
  spline.AppendControlPoint( ae::Vec3( 2.0f, 2.0f, 0.0f ) );
  spline.AppendControlPoint( ae::Vec3( 0.4f, -2.0f, 0.0f ) );
  spline.SetLooping( true );

  ae::TimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / 60.0f );

  ae::Texture2D tex;
  {
    const char* fileName = "circle.tga";
    uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, fileName );
    AE_ASSERT_MSG( fileSize, "Could not load #", fileName );
    ae::Scratch< uint8_t > fileBuffer( fileSize );
    fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileSize );
    ae::TargaFile targa = TAG_EXAMPLE;
    targa.Load( fileBuffer.Data(), fileBuffer.Length() );
    tex.Initialize( targa.textureParams );
  }

  float t = 0.0f;
  float angle = 0.0f;

  while( !input.quit )
  {
    input.Pump();
    render.Activate();
    render.Clear( ae::Color::Black() );
    spriteRender.Clear();

    ae::Matrix4 transform;

    float splineLen = spline.GetLength();
    for( float d = 0.0f; d < splineLen; d += 0.25f )
    {
      transform = ae::Matrix4::Translation( spline.GetPoint( d ) );
      transform *= ae::Matrix4::Scaling( ae::Vec3( 0.1f ) );
      spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::Blue() );
    }
    
    for( uint32_t i = 0; i < spline.GetControlPointCount(); i++ )
    {
      transform = ae::Matrix4::Translation( spline.GetControlPoint( i ) - ae::Vec3( 0.0f, 0.0f, 0.1f ) );
      transform *= ae::Matrix4::Scaling( ae::Vec3( 0.2f ) );
      spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::Red() );
    }

    ae::Vec3 p = spline.GetPoint( t );

    float t1;
    spline.GetMinDistance( p, nullptr, &t1 );
    transform = ae::Matrix4::Translation( spline.GetPoint( t1 ) - ae::Vec3( 0.0f, 0.0f, 0.2f ) );
    transform *= ae::Matrix4::Scaling( ae::Vec3( 0.4f ) );
    spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::White() );

    t += timeStep.GetTimeStep();
    if( t > splineLen )
    {
      t -= splineLen;
    }
    transform = ae::Matrix4::Translation( p - ae::Vec3( 0.0f, 0.0f, 0.2f ) );
    transform *= ae::Matrix4::Scaling( ae::Vec3( 0.3f ) );
    spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::Green() );

    angle += timeStep.GetTimeStep();
    // Orbit dot
    ae::Vec3 rotPos = p + ae::Vec3( cosf(angle), sinf(angle), 0.0f ) * 0.5f;
    transform = ae::Matrix4::Translation( rotPos - ae::Vec3( 0.0f, 0.0f, 0.2f ) );
    transform *= ae::Matrix4::Scaling( ae::Vec3( 0.2f ) );
    spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::PicoDarkPurple() );
    // Closest dot (check)
    spline.GetMinDistance( rotPos, &rotPos, &t1 );
    transform = ae::Matrix4::Translation( spline.GetPoint( t1 ) - ae::Vec3( 0.0f, 0.0f, 0.2f ) );
    transform *= ae::Matrix4::Scaling( ae::Vec3( 0.3f ) );
    spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::White() );
    // Closest dot
    transform = ae::Matrix4::Translation( rotPos - ae::Vec3( 0.0f, 0.0f, 0.2f ) );
    transform *= ae::Matrix4::Scaling( ae::Vec3( 0.2f ) );
    spriteRender.AddSprite( 0, transform, ae::Rect::FromPoints( ae::Vec2( 0.0f ), ae::Vec2( 1.0f ) ), ae::Color::PicoDarkPurple() );

    ae::UniformList uniforms;
    uniforms.Set( "u_worldToProj", ae::Matrix4::Scaling( ae::Vec3( 0.2f / render.GetAspectRatio(), 0.2f, 1.0f ) ) );
    uniforms.Set( "u_tex", &tex );
    spriteRender.SetParams( 0, &spriteShader, uniforms );
    spriteRender.Render();
    render.Present();
    timeStep.Tick();
  }

  AE_LOG( "Terminate" );

  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
