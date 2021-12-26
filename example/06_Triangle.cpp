//------------------------------------------------------------------------------
// 06_Triangle.cpp
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
#include "ae/aether.h"

//------------------------------------------------------------------------------
// Shaders
//------------------------------------------------------------------------------
const char* kVertShader = R"(
  AE_UNIFORM mat4 u_modelToNdc;
  AE_IN_HIGHP vec4 a_position;
  AE_IN_HIGHP vec4 a_color;
  AE_OUT_HIGHP vec4 v_color;
  void main()
  {
    v_color = a_color;
    gl_Position = u_modelToNdc * a_position;
  })";

const char* kFragShader = R"(
  AE_IN_HIGHP vec4 v_color;
  void main()
  {
    AE_COLOR = v_color;
  })";

//------------------------------------------------------------------------------
// Triangle
//------------------------------------------------------------------------------
struct Vertex
{
  ae::Vec4 pos;
  ae::Vec4 color;
};

Vertex kTriangleVerts[] =
{
  { ae::Vec4( -0.5f, -0.4f, 0.0f, 1.0f ), ae::Color::PicoRed().GetLinearRGBA() },
  { ae::Vec4( 0.5f, -0.4f, 0.0f, 1.0f ), ae::Color::PicoGreen().GetLinearRGBA() },
  { ae::Vec4( 0.0f, 0.4f, 0.0f, 1.0f ), ae::Color::PicoBlue().GetLinearRGBA() },
};

uint16_t kTriangleIndices[] =
{
  0, 1, 2
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
  ae::Shader shader;
  ae::VertexData vertexData;
  
  window.Initialize( 1280, 720, false, true );
  window.SetTitle( "triangle" );
  render.Initialize( &window );
  input.Initialize( &window );
  timeStep.SetTimeStep( 1.0f / 60.0f );

  shader.Initialize( kVertShader, kFragShader, nullptr, 0 );

  vertexData.Initialize( sizeof( *kTriangleVerts ), sizeof( *kTriangleIndices ), countof( kTriangleVerts ), countof( kTriangleIndices ), ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Static, ae::VertexData::Usage::Static );
  vertexData.AddAttribute( "a_position", 4, ae::VertexData::Type::Float, offsetof( Vertex, pos ) );
  vertexData.AddAttribute( "a_color", 4, ae::VertexData::Type::Float, offsetof( Vertex, color ) );
  vertexData.SetVertices( kTriangleVerts, countof( kTriangleVerts ) );
  vertexData.SetIndices( kTriangleIndices, countof( kTriangleIndices ) );
  vertexData.Upload();

  ae::Vec3 pos( 0.0f );
  float scale = 1.0f;
  float rotation = 0.0f;

  AE_LOG( "Run" );
  while ( !input.quit )
  {
    input.Pump();
    render.Activate();
    render.Clear( ae::Color::PicoDarkPurple() );

    rotation += timeStep.GetDt();
    
    if ( input.mouse.leftButton && !input.mousePrev.leftButton )
    {
      input.SetMouseCaptured( true );
    }
    if ( input.Get( ae::Key::Escape ) && !input.GetPrev( ae::Key::Escape ) )
    {
      input.SetMouseCaptured( false );
    }
    
    if ( input.GetMouseCaptured() )
    {
      pos.x += input.mouse.movement.x * 0.001f;
      pos.y += input.mouse.movement.y * 0.001f;
    }
    else if ( input.mouse.usingTouch )
    {
      pos.x += input.mouse.scroll.x * 0.01f;
      pos.y += input.mouse.scroll.y * -0.01f;
    }
    else
    {
      scale += input.mouse.scroll.y * 0.01f;
      scale = ae::Clip( scale, 0.1f, 2.0f );
    }
    
    ae::Matrix4 transform = ae::Matrix4::Translation( pos );
    transform *= ae::Matrix4::RotationY( rotation );
    transform *= ae::Matrix4::Scaling( ae::Vec3( scale / render.GetAspectRatio(), scale, scale ) );

    ae::UniformList uniformList;
    uniformList.Set( "u_modelToNdc", transform );
    vertexData.Render( &shader, uniformList );

    render.Present();
    timeStep.Wait();
  }

  AE_LOG( "Terminate" );
  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
