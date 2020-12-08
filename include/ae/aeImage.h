//------------------------------------------------------------------------------
// aeImage.h
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
#ifndef AEIMAGE_H
#define AEIMAGE_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeRender.h"

//------------------------------------------------------------------------------
// aeImage
//------------------------------------------------------------------------------
namespace ae
{
  class Image
  {
  public:
    enum class Extension
    {
      PNG
    };

    enum class Interpolation
    {
      Nearest,
      Linear,
      Cosine
    };

    enum class Format
    {
      Auto,
      R,
      RG,
      RGB,
      RGBA
    };

    void LoadRaw( const uint8_t* data, uint32_t width, uint32_t height, Format format, Format storage = Format::Auto );
    bool LoadFile( const void* file, uint32_t length, Extension extension, Format storage = Format::Auto );

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetChannels() const { return m_channels; }

    aeColor Get( aeInt2 pixel ) const;
    aeColor Get( aeFloat2 pixel, Interpolation interpolation ) const;

  private:
    aeArray< uint8_t > m_data;
    int32_t m_width = 0;
    int32_t m_height = 0;
    uint32_t m_channels = 0;
  };
}

#endif
