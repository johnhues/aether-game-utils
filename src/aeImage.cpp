//------------------------------------------------------------------------------
// aeImage.cpp
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
#include "aeImage.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void ae::Image::LoadRaw( const uint8_t* data, uint32_t width, uint32_t height, Format format, Format storage )
{
  AE_STATIC_ASSERT( (uint32_t)Format::R == 1 );
  AE_STATIC_ASSERT( (uint32_t)Format::RG == 2 );
  AE_STATIC_ASSERT( (uint32_t)Format::RGB == 3 );
  AE_STATIC_ASSERT( (uint32_t)Format::RGBA == 4 );

  m_width = width;
  m_height = height;
  m_channels = ( storage == Format::Auto ) ? (uint32_t)format : ( uint32_t )storage; // @NOTE: See static assert above

  uint32_t length = m_width * m_height;
  m_data.Reserve( length * m_channels );

  uint32_t formatChannels = (uint32_t)format; // @NOTE: See static assert above
  if ( formatChannels == m_channels )
  {
    // @NOTE: Direct copy
    m_data.Append( data, length * m_channels );
  }
  else if ( formatChannels > m_channels )
  {
    // @NOTE: More channels provided than needed
    for ( uint32_t i = 0; i < length; i++ )
    {
      m_data.Append( &data[ i * formatChannels ], m_channels );
    }
  }
  else
  {
    // @NOTE: Fewer channels provided than needed
    // Copy last color value into remaining channels, and set alpha to opaque
    uint8_t p[ 4 ];
    p[ 3 ] = 255;
    for ( uint32_t i = 0; i < length; i++ )
    {
      uint32_t index = i * formatChannels;
      memcpy( p, &data[ index ], formatChannels );
      memset( p + formatChannels, data[ index + formatChannels - 1 ], 3 - formatChannels );

      m_data.Append( p, m_channels );
    }
  }
}

bool ae::Image::LoadFile( const void* file, uint32_t length, Extension extension, Format storage )
{
  if ( !file || !length )
  {
    return false;
  }

  AE_ASSERT( extension == Extension::PNG );

  int32_t width = 0;
  int32_t height = 0;
  int32_t channels = 0;
  stbi_set_flip_vertically_on_load( 1 );
#if _AE_IOS_
  stbi_convert_iphone_png_to_rgb( 1 );
#endif
  uint8_t* image = stbi_load_from_memory( (const uint8_t*)file, length, &width, &height, &channels, STBI_default );
  if ( !image )
  {
    return false;
  }

  Format format = (Format)channels; // @NOTE: See static assert above
  LoadRaw( image, width, height, format, storage );
  stbi_image_free( image );

  return true;
}

aeColor ae::Image::Get( aeInt2 pixel ) const
{
  if ( pixel.x < 0 || pixel.y < 0 || pixel.x >= m_width || pixel.y >= m_height )
  {
    return aeColor::Black();
  }

  uint32_t index = ( pixel.y * m_width + pixel.x ) * m_channels;
  switch ( m_channels )
  {
    case 1:
    {
      return aeColor::R( m_data[ index ] );
    }
    case 2:
    {
      return aeColor::RG( m_data[ index ], m_data[ index + 1 ] );
    }
    case 3:
    {
      return aeColor::RGB( m_data[ index ], m_data[ index + 1 ], m_data[ index + 2 ] );
    }
    case 4:
    {
      return aeColor::RGBA( m_data[ index ], m_data[ index + 1 ], m_data[ index + 2 ], m_data[ index + 3 ] );
    }
  }

  return aeColor::Black();
}

aeColor ae::Image::Get( aeFloat2 pixel, Interpolation interpolation ) const
{
  aeInt2 pi = pixel.FloorCopy();

  switch ( interpolation )
  {
    case Interpolation::Nearest:
    {
      return Get( pi );
    }
    case Interpolation::Linear:
    {
      float x = pixel.x - pi.x;
      float y = pixel.y - pi.y;

      aeColor c00 = Get( pi );
      aeColor c10 = Get( pi + aeInt2( 1, 0 ) );
      aeColor c01 = Get( pi + aeInt2( 0, 1 ) );
      aeColor c11 = Get( pi + aeInt2( 1, 1 ) );

      aeColor c0 = c00.Lerp( c10, x );
      aeColor c1 = c01.Lerp( c11, x );

      return c0.Lerp( c1, y );
    }
    case Interpolation::Cosine:
    {
      float x = pixel.x - pi.x;
      float y = pixel.y - pi.y;

      aeColor c00 = Get( pi );
      aeColor c10 = Get( pi + aeInt2( 1, 0 ) );
      aeColor c01 = Get( pi + aeInt2( 0, 1 ) );
      aeColor c11 = Get( pi + aeInt2( 1, 1 ) );

      aeColor c0 = aeMath::Interpolation::Cosine( c00, c10, x );
      aeColor c1 = aeMath::Interpolation::Cosine( c01, c11, x );

      return aeMath::Interpolation::Cosine( c0, c1, y );
    }
  }

  return aeColor::Black();
}
