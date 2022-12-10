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
#include "aether.h"
#include "ae/loaders.h"
#include "ae/SpriteRenderer.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const ae::Tag TAG_EXAMPLE = "example";

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
  ae::TextRender textRender = TAG_EXAMPLE;
  
  window.Initialize( 1280, 720, false, true );
  window.SetTitle( "example" );
  render.Initialize( &window );
  input.Initialize( &window );
  input.SetTextMode( true );
  fileSystem.Initialize( "data", "ae", "text_input" );
  
  ae::Texture2D fontTexture;
  {
    const char* fileName = "font.png";
    uint32_t fileSize = fileSystem.GetSize( ae::FileSystem::Root::Data, fileName );
    AE_ASSERT_MSG( fileSize, "Could not load #", fileName );
    ae::Scratch< uint8_t > fileBuffer( fileSize );
    fileSystem.Read( ae::FileSystem::Root::Data, fileName, fileBuffer.Data(), fileSize );
    ae::stbLoadPng( &fontTexture, fileBuffer.Data(), fileSize, ae::Texture::Filter::Nearest, ae::Texture::Wrap::Repeat, false, true );
  }
  textRender.Initialize( 1, 512, &fontTexture, 8, 1.0f );
  
  ae::TimeStep timeStep;
  timeStep.SetTimeStep( 1.0f / 60.0f );
  
  float textZoom = 4.0f;
  const char* defaultText = "Try typing.\nCopy and paste should also work.\nYou can zoom by scrolling.\nPress 'ESC' to reset.";
  input.SetText( defaultText );

  while ( !input.quit )
  {
    input.Pump();
    render.Activate();
    render.Clear( ae::Color::Green().ScaleRGB( 0.01f ) );
    
    if ( input.Get( ae::Key::Escape ) && !input.GetPrev( ae::Key::Escape ) )
    {
      textZoom = 4.0f;
      input.SetText( defaultText );
    }
    textZoom += input.mouse.scroll.y * 0.01f;

    // UI units in pixels, origin in bottom left
    ae::Matrix4 textToNdc = ae::Matrix4::Scaling( ae::Vec3( 2.0f / render.GetWidth(), 2.0f / render.GetHeight(), 1.0f ) );
    textToNdc *= ae::Matrix4::Translation( ae::Vec3( render.GetWidth() / -2.0f, render.GetHeight() / 2.0f, 0.0f ) );
    textToNdc *= ae::Matrix4::Scaling( ae::Vec3( textZoom, textZoom, 1.0f ) );

    // Format input text buffer
    ae::Str512 displayText( ">" );
    displayText.Append( input.GetText() );
    
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
    ae::Vec3 textPos( textRender.GetFontSize() / 2.0f, textRender.GetFontSize() / -2.0f, 0.0f );
    textRender.Add( textPos, ae::Vec2( (float)textRender.GetFontSize() ), displayText.c_str(), ae::Color::Green(), maxLineLength, 0 );
    textRender.Render( textToNdc );

    render.Present();
    timeStep.Tick();
  }

  AE_LOG( "Terminate" );

  textRender.Terminate();
  fontTexture.Terminate();
  input.Terminate();
  render.Terminate();
  window.Terminate();

  return 0;
}
