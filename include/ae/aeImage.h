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
    ae::Array< uint8_t > m_data = AE_ALLOC_TAG_RENDER;
    int32_t m_width = 0;
    int32_t m_height = 0;
    uint32_t m_channels = 0;
  };
}

template < typename Pixel, uint32_t Width, uint32_t Height, uint32_t Depth >
class aeStaticImage3D
{
public:
  aeStaticImage3D();
  void Set( aeInt3 p, Pixel v );
  Pixel Get( aeInt3 p ) const;
  template < Pixel (*BlendFn)(Pixel, Pixel, float) >
  Pixel Get( aeFloat3 p ) const;
  
  uint32_t GetWidth() const { return Width; }
  uint32_t GetHeight() const { return Height; }
  uint32_t GetDepth() const { return Depth; }
  
private:
  Pixel m_data[ Depth ][ Height ][ Width ];
};

inline int32_t aeModulo( int32_t a, int32_t b )
{
  return ( a % b + b ) % b;
}

template < typename Pixel, uint32_t Width, uint32_t Height, uint32_t Depth >
inline aeStaticImage3D< Pixel, Width, Height, Depth >::aeStaticImage3D()
{
  for ( int32_t z = 0; z < Depth; z++ )
  for ( int32_t y = 0; y < Height; y++ )
  for ( int32_t x = 0; x < Width; x++ )
  {
    m_data[ z ][ y ][ x ] = Pixel();
  }
}

template < typename Pixel, uint32_t Width, uint32_t Height, uint32_t Depth >
inline void aeStaticImage3D< Pixel, Width, Height, Depth >::Set( aeInt3 p, Pixel v )
{
  m_data[ aeModulo( p.z, Depth ) ][ aeModulo( p.y, Height ) ][ aeModulo( p.x, Width ) ] = v;
}

template < typename Pixel, uint32_t Width, uint32_t Height, uint32_t Depth >
inline Pixel aeStaticImage3D< Pixel, Width, Height, Depth >::Get( aeInt3 p ) const
{
  return m_data[ aeModulo( p.z, Depth ) ][ aeModulo( p.y, Height ) ][ aeModulo( p.x, Width ) ];
}

template < typename Pixel, uint32_t Width, uint32_t Height, uint32_t Depth >
template < Pixel (*BlendFn)(Pixel, Pixel, float) >
inline Pixel aeStaticImage3D< Pixel, Width, Height, Depth >::Get( aeFloat3 pf ) const
{
  aeInt3 pi = pf.FloorCopy();
  float xf = pf.x - pi.x;
  
  Pixel c011 = Get( aeInt3( pi.x, pi.y, pi.z ) );
  Pixel c111 = Get( aeInt3( pi.x + 1, pi.y, pi.z ) );
  Pixel c001 = Get( aeInt3( pi.x, pi.y + 1, pi.z ) );
  Pixel c101 = Get( aeInt3( pi.x + 1, pi.y + 1, pi.z ) );
  
  Pixel c01 = BlendFn( c001, c101, xf );
  Pixel c11 = BlendFn( c011, c111, xf );
  
  Pixel c1 = BlendFn( c11, c01, pf.y - pi.y );
  
  Pixel c010 = Get( aeInt3( pi.x, pi.y, pi.z + 1 ) );
  Pixel c110 = Get( aeInt3( pi.x + 1, pi.y, pi.z + 1 ) );
  Pixel c000 = Get( aeInt3( pi.x, pi.y + 1, pi.z + 1 ) );
  Pixel c100 = Get( aeInt3( pi.x + 1, pi.y + 1, pi.z + 1 ) );
  
  Pixel c00 = BlendFn( c000, c100, xf );
  Pixel c10 = BlendFn( c010, c110, xf );
  
  Pixel c0 = BlendFn( c10, c00, pf.y - pi.y );
  
  return BlendFn( c1, c0, pf.z - pi.z );
}

#endif
