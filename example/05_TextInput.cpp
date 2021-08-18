//------------------------------------------------------------------------------
// 05_TextInput.cpp
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
#include "Common.h"

//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main()
{
  AE_LOG( "Initialize" );

  ae::Window window;
  ae::GraphicsDevice render;
  ae::Input input;
  aeTextRender textRender;
  
  window.Initialize( 1280, 720, false, true );
  window.SetTitle( "example" );
  render.Initialize( &window );
  input.Initialize( &window );
//  input.SetTextMode( true );
//  input.SetText( "Try typing. Copy and paste should also work." );
  
  ae::Texture2D tex;
  LoadPng( &tex, "font.png", ae::Texture::Filter::Linear, ae::Texture::Wrap::Repeat, false, true );
  textRender.Initialize( &tex, 8 );
  
  ae::TimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / 60.0f );

  while ( !input.quit )
  {
    input.Pump();
    render.Activate();
    render.Clear( aeColor::Green().ScaleRGB( 0.01f ) );
    //render.StartFrame( window.GetWidth() / 4, window.GetHeight() / 4 );

    // UI units in pixels, origin in bottom left
    ae::Matrix4 textToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
    textToNdc *= ae::Matrix4::Translation( ae::Vec3( render.GetWidth() / -2.0f, render.GetHeight() / -2.0f, 0.0f ) );

    // Format input text buffer
    aeStr512 displayText( ">" );
//    displayText.Append( input.GetText() );
    
    static int s_blink = 0;
    static int s_prevLen = 0;
    if ( s_prevLen != displayText.Length() )
    {
      s_prevLen = displayText.Length();
      s_blink = 0;
    }
    else if ( s_blink >= 60 )
    {
      s_blink -= 60;
    }
    displayText.Append( s_blink < 30 ? "_" : " " );
    s_blink++;

    // Render text in top left corner
    int maxLineLength = render.GetWidth() / textRender.GetFontSize() - 2;
    ae::Vec3 textPos( textRender.GetFontSize(), render.GetHeight() - textRender.GetFontSize(), 0.0f );
    textRender.Add( textPos, ae::Vec2( (float)textRender.GetFontSize() ), displayText.c_str(), aeColor::Green(), maxLineLength, 0 );
    textRender.Render( textToNdc );

    render.Present();
    timeStep.Wait();
  }

  AE_LOG( "Terminate" );

  textRender.Terminate();
  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
