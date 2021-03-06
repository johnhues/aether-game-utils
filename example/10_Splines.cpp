//------------------------------------------------------------------------------
// 10_Splines.cpp
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
// Headers
//------------------------------------------------------------------------------
#include "ae/aetherEXT.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  aeWindow window;
  aeRender render;
  aeInput input;
  aeSpline spline( ae::Tag( "example" ) );
  aeSpriteRender spriteRender;
  
  window.Initialize( 800, 600, false, true );
  window.SetTitle( "splines" );
  render.InitializeOpenGL( &window );
  input.Initialize( &window );
  spriteRender.Initialize( 64 );
  spriteRender.SetBlending( true );
  spriteRender.SetDepthTest( true );
  spriteRender.SetDepthWrite( true );
  spriteRender.SetSorting( true );
  
  spline.AppendControlPoint( aeFloat3( -0.4f, -2.0f, 0.0f ) );
  spline.AppendControlPoint( aeFloat3( -2.0f, 2.0f, 0.0f ) );
  spline.AppendControlPoint( aeFloat3( 0.0f, 1.0f, 0.0f ) );
  spline.AppendControlPoint( aeFloat3( 2.0f, 2.0f, 0.0f ) );
  spline.AppendControlPoint( aeFloat3( 0.4f, -2.0f, 0.0f ) );
  spline.SetLooping( true );

  ae::TimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / 60.0f );

  aeTexture2D tex;
  tex.Initialize( "circle.png", aeTextureFilter::Linear, aeTextureWrap::Repeat );

  float t = 0.0f;

  while ( !input.GetState()->exit )
  {
    input.Pump();
    render.Activate();
    render.Clear( aeColor::Black() );
    spriteRender.Clear();

    aeFloat4x4 transform;

    for ( uint32_t i = 0; i < spline.GetControlPointCount(); i++ )
    {
      transform = aeFloat4x4::Translation( spline.GetControlPoint( i ) - aeFloat3( 0.0f, 0.0f, 0.1f ) );
      transform.Scale( aeFloat3( 0.2f ) );
      spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Red() );
    }

    float splineLen = spline.GetLength();
    for ( float d = 0.0f; d < splineLen; d += 0.25f )
    {
      transform = aeFloat4x4::Translation( spline.GetPoint( d ) );
      transform.Scale( aeFloat3( 0.1f ) );
      spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Blue() );
    }

    t += timeStep.GetTimeStep();
    if ( t > splineLen )
    {
      t -= splineLen;
    }
    transform = aeFloat4x4::Translation( spline.GetPoint( t ) - aeFloat3( 0.0f, 0.0f, 0.2f ) );
    transform.Scale( aeFloat3( 0.3f ) );
    spriteRender.AddSprite( &tex, transform, aeFloat2( 0.0f ), aeFloat2( 1.0f ), aeColor::Green() );

    aeFloat4x4 screenTransform = aeFloat4x4::Scaling( aeFloat3( 1.0f / 5.0f, render.GetAspectRatio() / 5.0f, 1.0f ) );
    spriteRender.Render( screenTransform );
    render.Present();
    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
