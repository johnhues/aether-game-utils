//------------------------------------------------------------------------------
// aeTerrain.cpp
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
#include "aeTerrain.h"
#include "aeClock.h"
#include "aeCompactingAllocator.h"
#include <ctpl_stl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void ae::Image::LoadRaw( const uint8_t* data, uint32_t width, uint32_t height, Format format, Format storage, uint32_t channelSize )
{
  AE_STATIC_ASSERT( (uint32_t)Format::R == 1 );
  AE_STATIC_ASSERT( (uint32_t)Format::RG == 2 );
  AE_STATIC_ASSERT( (uint32_t)Format::RGB == 3 );
  AE_STATIC_ASSERT( (uint32_t)Format::RGBA == 4 );

  m_width = width;
  m_height = height;
  m_channels = ( storage == Format::Auto ) ? (uint32_t)format : ( uint32_t )storage; // @NOTE: See static assert above
  m_channelSize = channelSize;
	
	
  uint32_t length = m_width * m_height;
  m_data.Reserve( length * m_channels );

  uint32_t formatChannels = (uint32_t)format; // @NOTE: See static assert above
  if ( formatChannels == m_channels )
  {
    // @NOTE: Direct copy
    m_data.Append( data, length * m_channels * m_channelSize );
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
  bool is16BitImage = stbi_is_16_bit_from_memory( (const uint8_t*)file, length );
  uint32_t channelSize = 1;
	
  uint8_t* image;
  if (is16BitImage)
  {
     image = (uint8_t*)stbi_load_16_from_memory( (const uint8_t*)file, length, &width, &height, &channels, STBI_default );
	 channelSize = 2;
	  
	 if (channels != 1)
	 {
		 return false;
	 }
  }

  else
  {
	image = stbi_load_from_memory( (const uint8_t*)file, length, &width, &height, &channels, STBI_default );
  }
	
  if ( !image )
  {
    return false;
  }

  Format format = (Format)channels; // @NOTE: See static assert above
  LoadRaw( image, width, height, format, storage, channelSize );
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
	  if ( m_channelSize == 2 )
	  {
		 const uint16_t* data16 = (const uint16_t*)&m_data[0];
	     return aeColor(data16[ index ] / 65535.0f, 0.0f, 0.0f, 1.0f);
	  }
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

// @TODO: SIMD GetIntersection() is currently causing nans on windows
#if _AE_APPLE_ //|| _AE_WINDOWS_
  #define TERRAIN_SIMD 1
  #ifdef _AE_WINDOWS
    #include <intrin.h>
  #else
    #include <x86intrin.h>
  #endif
#endif

static float s_test = 0.0f;

namespace
{
aeFloat3 GetIntersection( const aeFloat3* p, const aeFloat3* n, uint32_t ic )
{
#if TERRAIN_SIMD
  __m128 c128 = _mm_setzero_ps();
  for ( uint32_t i = 0; i < ic; i++ )
  {
    __m128 p128 = _mm_load_ps( (float*)( p + i ) );
    c128 = _mm_add_ps( c128, p128 );
  }
  __m128 div = _mm_set1_ps( 1.0f / ic );
  c128 = _mm_mul_ps( c128, div );
  
  for ( int32_t i = 0; i < 10; i++ )
  for ( int32_t j = 0; j < ic; j++ )
  {
    __m128 p128 = _mm_load_ps( (float*)( p + j ) );
    p128 = _mm_sub_ps( p128, c128 );
    __m128 n128 = _mm_load_ps( (float*)( n + j ) );
    
    __m128 d = _mm_mul_ps( p128, n128 );
    d = _mm_hadd_ps( d, d );
    d = _mm_hadd_ps( d, d );
    
    __m128 s = _mm_set1_ps( 0.5f );
    s = _mm_mul_ps( s, n128 );
    s = _mm_mul_ps( s, d );
    c128 = _mm_add_ps( c128, s );
  }
  aeFloat3 v;
  _mm_store_ps( (float*)&v, c128 );
  return v;
#else
  aeFloat3 c( 0.0f );
  for ( uint32_t i = 0; i < ic; i++ )
  {
    c += p[ i ];
  }
  c /= ic;

  for ( uint32_t i = 0; i < 10; i++ )
  {
    for ( uint32_t j = 0; j < ic; j++ )
    {
      float d = n[ j ].Dot( p[ j ] - c );
      c += n[ j ] * ( d * 0.5f );
    }
  }

  return c;
#endif
}

const uint16_t EDGE_TOP_FRONT_INDEX = 0;
const uint16_t EDGE_TOP_RIGHT_INDEX = 1;
const uint16_t EDGE_TOP_BACK_INDEX = 2;
const uint16_t EDGE_TOP_LEFT_INDEX = 3;
const uint16_t EDGE_SIDE_FRONTLEFT_INDEX = 4;
const uint16_t EDGE_SIDE_FRONTRIGHT_INDEX = 5;
const uint16_t EDGE_SIDE_BACKRIGHT_INDEX = 6;
const uint16_t EDGE_SIDE_BACKLEFT_INDEX = 7;
const uint16_t EDGE_BOTTOM_FRONT_INDEX = 8;
const uint16_t EDGE_BOTTOM_RIGHT_INDEX = 9;
const uint16_t EDGE_BOTTOM_BACK_INDEX = 10;
const uint16_t EDGE_BOTTOM_LEFT_INDEX = 11;
const uint16_t EDGE_TOP_FRONT_BIT = 1 << EDGE_TOP_FRONT_INDEX;
const uint16_t EDGE_TOP_RIGHT_BIT = 1 << EDGE_TOP_RIGHT_INDEX;
const uint16_t EDGE_TOP_BACK_BIT = 1 << EDGE_TOP_BACK_INDEX;
const uint16_t EDGE_TOP_LEFT_BIT = 1 << EDGE_TOP_LEFT_INDEX;
const uint16_t EDGE_SIDE_FRONTLEFT_BIT = 1 << EDGE_SIDE_FRONTLEFT_INDEX;
const uint16_t EDGE_SIDE_FRONTRIGHT_BIT = 1 << EDGE_SIDE_FRONTRIGHT_INDEX;
const uint16_t EDGE_SIDE_BACKRIGHT_BIT = 1 << EDGE_SIDE_BACKRIGHT_INDEX;
const uint16_t EDGE_SIDE_BACKLEFT_BIT = 1 << EDGE_SIDE_BACKLEFT_INDEX;
const uint16_t EDGE_BOTTOM_FRONT_BIT = 1 << EDGE_BOTTOM_FRONT_INDEX;
const uint16_t EDGE_BOTTOM_RIGHT_BIT = 1 << EDGE_BOTTOM_RIGHT_INDEX;
const uint16_t EDGE_BOTTOM_BACK_BIT = 1 << EDGE_BOTTOM_BACK_INDEX;
const uint16_t EDGE_BOTTOM_LEFT_BIT = 1 << EDGE_BOTTOM_LEFT_INDEX;
}

SDFCache::SDFCache()
{
  m_chunk = aeInt3( 0 );
  m_sdf = aeAlloc::AllocateArray< float16_t >( kDim * kDim * kDim );
  m_material = aeAlloc::AllocateArray< aeTerrainMaterialId >( kDim * kDim * kDim );
}

SDFCache::~SDFCache()
{
  aeAlloc::Release( m_sdf );
  aeAlloc::Release( m_material );
  m_sdf = nullptr;
  m_material = nullptr;
}

void SDFCache::Generate( aeInt3 chunk, const aeTerrainSDF* sdf )
{
  m_chunk = chunk;

  m_offseti = aeInt3( kOffset ) - aeInt3( m_chunk * kChunkSize );
  m_offsetf = aeFloat3( (float)kOffset ) - aeFloat3( m_chunk * kChunkSize );

  aeInt3 offset = m_chunk * kChunkSize - aeInt3( kOffset );
  for ( int32_t z = 0; z < kDim; z++ )
  for ( int32_t y = 0; y < kDim; y++ )
  for ( int32_t x = 0; x < kDim; x++ )
  {
    uint32_t index = x + kDim * ( y + kDim * z );
    aeFloat3 pos( offset.x + x, offset.y + y, offset.z + z );
    m_sdf[ index ] = sdf->GetValue( pos );
    m_material[ index ] = sdf->GetMaterial( pos );
  }
}

float SDFCache::GetValue( aeFloat3 pos ) const
{
  pos += m_offsetf;

  aeInt3 posi = pos.FloorCopy();
  pos.x -= posi.x;
  pos.y -= posi.y;
  pos.z -= posi.z;

  float values[ 8 ] =
  {
    m_GetValue( posi ),
    m_GetValue( posi + aeInt3( 1, 0, 0 ) ),
    m_GetValue( posi + aeInt3( 0, 1, 0 ) ),
    m_GetValue( posi + aeInt3( 1, 1, 0 ) ),
    m_GetValue( posi + aeInt3( 0, 0, 1 ) ),
    m_GetValue( posi + aeInt3( 1, 0, 1 ) ),
    m_GetValue( posi + aeInt3( 0, 1, 1 ) ),
    m_GetValue( posi + aeInt3( 1, 1, 1 ) ),
  };

  float x0 = aeMath::Lerp( values[ 0 ], values[ 1 ], pos.x );
  float x1 = aeMath::Lerp( values[ 2 ], values[ 3 ], pos.x );
  float x2 = aeMath::Lerp( values[ 4 ], values[ 5 ], pos.x );
  float x3 = aeMath::Lerp( values[ 6 ], values[ 7 ], pos.x );
  float y0 = aeMath::Lerp( x0, x1, pos.y );
  float y1 = aeMath::Lerp( x2, x3, pos.y );
  return aeMath::Lerp( y0, y1, pos.z );
}

float SDFCache::GetValue( aeInt3 pos ) const
{
  return m_GetValue( pos + m_offseti );
}

aeFloat3 SDFCache::GetDerivative( aeFloat3 p ) const
{
  aeFloat3 normal0;
  for ( int32_t i = 0; i < 3; i++ )
  {
    aeFloat3 nt = p;
    nt[ i ] += 0.2f;
    normal0[ i ] = GetValue( nt );
  }
  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal0 -= aeFloat3( GetValue( p ) );
  normal0.SafeNormalize();
  AE_ASSERT( normal0 != aeFloat3( 0.0f ) );
  AE_ASSERT( normal0 == normal0 );

  aeFloat3 normal1;
  for ( int32_t i = 0; i < 3; i++ )
  {
    aeFloat3 nt = p;
    nt[ i ] -= 0.2f;
    normal1[ i ] = GetValue( nt );
  }
  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal1 = aeFloat3( GetValue( p ) ) - normal1;
  normal1.SafeNormalize();
  AE_ASSERT( normal1 != aeFloat3( 0.0f ) );
  AE_ASSERT( normal1 == normal1 );

  return ( normal1 + normal0 ).SafeNormalizeCopy();
}

uint8_t SDFCache::GetMaterial( aeInt3 pos ) const
{
  pos += m_offseti;
#if _AE_DEBUG_
  AE_ASSERT( pos.x >= 0 && pos.y >= 0 && pos.z >= 0 );
  AE_ASSERT( pos.x < kDim&& pos.y < kDim&& pos.z < kDim );
#endif
  return m_material[ pos.x + kDim * ( pos.y + kDim * pos.z ) ];
}

float SDFCache::m_GetValue( aeInt3 pos ) const
{
#if _AE_DEBUG_
  AE_ASSERT( pos.x >= 0 && pos.y >= 0 && pos.z >= 0 );
  AE_ASSERT( pos.x < kDim && pos.y < kDim && pos.z < kDim );
#endif
  return m_sdf[ pos.x + kDim * ( pos.y + kDim * pos.z ) ];
}

Chunk::Chunk() :
  m_generatedList( this )
{
  m_check = 0xCDCDCDCD;

  m_pos = aeInt3( 0 );

  m_geoDirty = false; // @NOTE: Start false. This flag is only for chunks that need to be regenerated
  m_lightDirty = false;

  m_vertices = nullptr;

  memset( m_t, 0, sizeof( m_t ) );
  memset( m_l, 0, sizeof( m_l ) );
  memset( m_i, ~(uint8_t)0, sizeof( m_i ) );
}

Chunk::~Chunk()
{
  AE_ASSERT( !m_vertices );
}

uint32_t Chunk::GetIndex( aeInt3 pos )
{
  return pos.x + kWorldChunksWidth * ( pos.y + kWorldChunksWidth * pos.z );
}

uint32_t Chunk::GetIndex() const
{
  return GetIndex( m_pos );
}

aeTerrainJob::aeTerrainJob() :
  m_hasJob( false ),
  m_running( false ),
  m_sdf( nullptr ),
  m_vertexCount( 0 ),
  m_indexCount( 0 ),
  m_vertices( kMaxChunkVerts, TerrainVertex() ),
  m_indices( kMaxChunkIndices, TerrainIndex() ),
  m_chunk( nullptr )
{
  edgeInfo = aeAlloc::AllocateArray< TempEdges >( kTempChunkSize3 );
}

aeTerrainJob::~aeTerrainJob()
{
  aeAlloc::Release( edgeInfo );
  edgeInfo = nullptr;
}

void aeTerrainJob::StartNew( const aeTerrainSDF* sdf, Chunk* chunk )
{
  AE_ASSERT( chunk );
  AE_ASSERT_MSG( !m_chunk, "Previous job not finished" );

  m_hasJob = true;
  m_running = true;

  m_sdf = sdf;
  m_vertexCount = 0;
  m_indexCount = 0;
  m_chunk = chunk;
}

void aeTerrainJob::Do()
{
  m_sdfCache.Generate( m_chunk->m_pos, m_sdf );
  m_chunk->Generate( &m_sdfCache, edgeInfo, &m_vertices[ 0 ], &m_indices[ 0 ], &m_vertexCount, &m_indexCount );
  m_running = false;
}

void aeTerrainJob::Finish()
{
  AE_ASSERT( m_chunk );
  AE_ASSERT( !m_running );

  m_hasJob = false;
  m_sdf = nullptr;
  m_vertexCount = 0;
  m_indexCount = 0;
  m_chunk = nullptr;
}

bool aeTerrainJob::HasChunk( aeInt3 pos ) const
{
  return m_chunk && m_chunk->m_pos == pos;
}

void Chunk::Generate( const SDFCache* sdf, aeTerrainJob::TempEdges* edgeInfo, TerrainVertex* verticesOut, TerrainIndex* indexOut, uint32_t* vertexCountOut, uint32_t* indexCountOut )
{
  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;

  // AE_LOG( "Generate chunk #", m_pos );

  int32_t chunkOffsetX = m_pos.x * kChunkSize;
  int32_t chunkOffsetY = m_pos.y * kChunkSize;
  int32_t chunkOffsetZ = m_pos.z * kChunkSize;
  
  memset( edgeInfo, 0, kTempChunkSize3 * sizeof( *edgeInfo ) );
  
  uint16_t mask[ 3 ];
  mask[ 0 ] = EDGE_TOP_FRONT_BIT;
  mask[ 1 ] = EDGE_TOP_RIGHT_BIT;
  mask[ 2 ] = EDGE_SIDE_FRONTRIGHT_BIT;
  
  // 3 new edges to test
  aeFloat3 cornerOffsets[ 3 ][ 2 ];
  // EDGE_TOP_FRONT_BIT
  cornerOffsets[ 0 ][ 0 ] = aeFloat3( 0, 1, 1 );
  cornerOffsets[ 0 ][ 1 ] = aeFloat3( 1, 1, 1 );
  // EDGE_TOP_RIGHT_BIT
  cornerOffsets[ 1 ][ 0 ] = aeFloat3( 1, 0, 1 );
  cornerOffsets[ 1 ][ 1 ] = aeFloat3( 1, 1, 1 );
  // EDGE_SIDE_FRONTRIGHT_BIT
  cornerOffsets[ 2 ][ 0 ] = aeFloat3( 1, 1, 0 );
  cornerOffsets[ 2 ][ 1 ] = aeFloat3( 1, 1, 1 );
  
  // @NOTE: This phase generates the surface mesh for the current chunk. The vertex
  // positions will be centered at the end of this phase, and will be nudged later
  // to the correct position within the voxel.
  const int32_t chunkPlus = kChunkSize + 1;
  for( int32_t z = -1; z < chunkPlus; z++ )
  for( int32_t y = -1; y < chunkPlus; y++ )
  for( int32_t x = -1; x < chunkPlus; x++ )
  {
    float cornerValues[ 3 ][ 2 ];
    for ( int32_t i = 0; i < 3; i++ )
    for ( int32_t j = 0; j < 2; j++ )
    {
      int32_t gx = chunkOffsetX + x + cornerOffsets[ i ][ j ].x;
      int32_t gy = chunkOffsetY + y + cornerOffsets[ i ][ j ].y;
      int32_t gz = chunkOffsetZ + z + cornerOffsets[ i ][ j ].z;
      // @TODO: Should pre-calculate, or at least only look up corner (1,1,1) once
      cornerValues[ i ][ j ] = sdf->GetValue( aeInt3( gx, gy, gz ) );
      if ( cornerValues[ i ][ j ] == 0.0f )
      {
        // @NOTE: Never let a terrain value be exactly 0, or else surface will end up with multiple vertices for the same point in the sdf
        cornerValues[ i ][ j ] = 0.0001f;
      }
    }
    
    // Detect if any of the 3 new edges being tested intersect the implicit surface
    uint16_t edgeBits = 0;
    if ( cornerValues[ 0 ][ 0 ] * cornerValues[ 0 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_TOP_FRONT_BIT; }
    if ( cornerValues[ 1 ][ 0 ] * cornerValues[ 1 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_TOP_RIGHT_BIT; }
    if ( cornerValues[ 2 ][ 0 ] * cornerValues[ 2 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_SIDE_FRONTRIGHT_BIT; }
    if ( edgeBits == 0 )
    {
      if ( x >= 0 && y >= 0 && z >= 0 && x < kChunkSize && y < kChunkSize && z < kChunkSize )
      {
        if ( m_i[ x ][ y ][ z ] != (TerrainIndex)~0 ) { continue; }
        
        aeFloat3 g;
        g.x = chunkOffsetX + x + 0.5f;
        g.y = chunkOffsetY + y + 0.5f;
        g.z = chunkOffsetZ + z + 0.5f;
        // @TODO: This is really expensive and might not be needed. Investigate removing 'Block' type altogether
        m_t[ x ][ y ][ z ] = ( sdf->GetValue( g ) > 0.0f ) ? Block::Exterior : Block::Interior;
      }
      continue;
    }
    
    uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    aeTerrainJob::TempEdges* te = &edgeInfo[ edgeIndex ];
    te->b = edgeBits;
    te->x = x;
    te->y = y;
    te->z = z;
    
    // Iterate over voxel edges (only 3 for TempEdges)
    for ( int32_t e = 0; e < 3; e++ )
    if ( edgeBits & mask[ e ] )
    {
      if ( vertexCount + 4 > kMaxChunkVerts || indexCount + 6 > kMaxChunkIndices )
      {
        *vertexCountOut = 0;
        *indexCountOut = 0;
        return;
      }

      aeFloat3 edgeVoxelPos;
      {
        // Determine which end of edge is inside/outside
        aeFloat3 c0, c1;
        if ( cornerValues[ e ][ 0 ] < cornerValues[ e ][ 1 ] )
        {
          c0 = cornerOffsets[ e ][ 0 ]; // Inside surface
          c1 = cornerOffsets[ e ][ 1 ]; // Outside surface
        }
        else
        {
          c0 = cornerOffsets[ e ][ 1 ]; // Inside surface
          c1 = cornerOffsets[ e ][ 0 ]; // Outside surface
        }

        // Find actual surface intersection point
        aeFloat3 ch( chunkOffsetX + x, chunkOffsetY + y, chunkOffsetZ + z );
        // @TODO: This should probably be adjustable
        for ( int32_t i = 0; i < 16; i++ )
        {
          // @TODO: This can be simplified by lerping and using the t value to do a binary search
          edgeVoxelPos = ( c0 + c1 ) * 0.5f;
          aeFloat3 cw = ch + edgeVoxelPos;
          
          float v = sdf->GetValue( cw );
          if ( aeMath::Abs( v ) < 0.001f )
          {
            break;
          }
          else if ( v < 0.0f )
          {
            c0 = edgeVoxelPos;
          }
          else
          {
            c1 = edgeVoxelPos;
          }
        }
      }
      AE_ASSERT( edgeVoxelPos.x == edgeVoxelPos.x && edgeVoxelPos.y == edgeVoxelPos.y && edgeVoxelPos.z == edgeVoxelPos.z );
      AE_ASSERT( edgeVoxelPos.x >= 0.0f && edgeVoxelPos.x <= 1.0f );
      AE_ASSERT( edgeVoxelPos.y >= 0.0f && edgeVoxelPos.y <= 1.0f );
      AE_ASSERT( edgeVoxelPos.z >= 0.0f && edgeVoxelPos.z <= 1.0f );
      
      aeFloat3 edgeWorldPos( chunkOffsetX + x, chunkOffsetY + y, chunkOffsetZ + z );
      edgeWorldPos += edgeVoxelPos;

      te->p[ e ] = edgeVoxelPos;
      te->n[ e ] = sdf->GetDerivative( edgeWorldPos );
      
      if ( x < 0 || y < 0 || z < 0 || x >= kChunkSize || y >= kChunkSize || z >= kChunkSize )
      {
        continue;
      }
      
      TerrainIndex ind[ 4 ];
      int32_t offsets[ 4 ][ 3 ];
      m_GetOffsetsFromEdge( mask[ e ], offsets );
      
      // @NOTE: Add new vertices for each edge intersection (centered in voxels for now).
      // Edges are eventually expanded into quads, so each edge needs 4 vertices.
      // This does some of the work for adjacent voxels.
      for ( int32_t j = 0; j < 4; j++ )
      {
        int32_t ox = x + offsets[ j ][ 0 ];
        int32_t oy = y + offsets[ j ][ 1 ];
        int32_t oz = z + offsets[ j ][ 2 ];
        
        // This check allows coordinates to be one out of chunk high end
        if ( ox < 0 || oy < 0 || oz < 0 || ox > kChunkSize || oy > kChunkSize || oz > kChunkSize ) { continue; }
        
        bool inCurrentChunk = ox < kChunkSize && oy < kChunkSize && oz < kChunkSize;
        if ( !inCurrentChunk || m_i[ ox ][ oy ][ oz ] == (TerrainIndex)~0 )
        {
          TerrainVertex vertex;
          vertex.position.x = ox + 0.5f;
          vertex.position.y = oy + 0.5f;
          vertex.position.z = oz + 0.5f;
          
          AE_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );
          
          TerrainIndex index = (TerrainIndex)vertexCount;
          verticesOut[ vertexCount++ ] = vertex;
          ind[ j ] = index;
          
          if ( inCurrentChunk )
          {
            m_i[ ox ][ oy ][ oz ] = index;
            m_t[ ox ][ oy ][ oz ] = Block::Surface;
          }
        }
        else
        {
          TerrainIndex index = m_i[ ox ][ oy ][ oz ];
          AE_ASSERT_MSG( index < vertexCount, "# < # ox:# oy:# oz:#", index, vertexCount, ox, oy, oz );
          AE_ASSERT( ox < kChunkSize );
          AE_ASSERT( oy < kChunkSize );
          AE_ASSERT( oz < kChunkSize );
          AE_ASSERT( m_t[ ox ][ oy ][ oz ] == Block::Surface );
          ind[ j ] = index;
        }
      }
      
      bool flip = false;
      // 0 - EDGE_TOP_FRONT_BIT
      // 1 - EDGE_TOP_RIGHT_BIT
      // 2 - EDGE_SIDE_FRONTRIGHT_BIT
      if ( e == 0 ) { flip = ( cornerValues[ 2 ][ 1 ] > 0.0f ); }
      else if ( e == 1 ) { flip = ( cornerValues[ 2 ][ 1 ] < 0.0f ); }
      else { flip = ( cornerValues[ 2 ][ 1 ] < 0.0f ); }

      // @TODO: This assumes counter clockwise culling
      if ( flip )
      {
        // tri0
        indexOut[ indexCount++ ] = ind[ 0 ];
        indexOut[ indexCount++ ] = ind[ 1 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
        // tri1
        indexOut[ indexCount++ ] = ind[ 1 ];
        indexOut[ indexCount++ ] = ind[ 3 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
      }
      else
      {
        // tri2
        indexOut[ indexCount++ ] = ind[ 0 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
        indexOut[ indexCount++ ] = ind[ 1 ];
        //tri3
        indexOut[ indexCount++ ] = ind[ 1 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
        indexOut[ indexCount++ ] = ind[ 3 ];
      }
    }
  }
  
  if ( indexCount == 0 )
  {
    // @TODO: Should differentiate between empty chunk and full chunk. It's possible though that
    // Chunk::t's are good enough for this though.
    *vertexCountOut = 0;
    *indexCountOut = 0;
    return;
  }
  
  const int32_t vc = (int32_t)vertexCount;
  for ( int32_t i = 0; i < vc; i++ )
  {
    TerrainVertex* vertex = &verticesOut[ i ];
    int32_t x = aeMath::Floor( vertex->position.x );
    int32_t y = aeMath::Floor( vertex->position.y );
    int32_t z = aeMath::Floor( vertex->position.z );
    
    int32_t ec = 0;
    aeFloat3 p[ 12 ];
    aeFloat3 n[ 12 ];
    
    if ( x < 0 || y < 0 || z < 0 ) { AE_FAIL(); }
    if ( x > kChunkSize || y > kChunkSize || z > kChunkSize ) { AE_FAIL(); }
    
    uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    aeTerrainJob::TempEdges te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_TOP_FRONT_BIT )
    {
      p[ ec ] = te.p[ 0 ];
      n[ ec ] = te.n[ 0 ];
      ec++;
    }
    if ( te.b & EDGE_TOP_RIGHT_BIT )
    {
      p[ ec ] = te.p[ 1 ];
      n[ ec ] = te.n[ 1 ];
      ec++;
    }
    if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
    {
      p[ ec ] = te.p[ 2 ];
      n[ ec ] = te.n[ 2 ];
      ec++;
    }
    edgeIndex = x + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_TOP_RIGHT_BIT )
    {
      p[ ec ] = te.p[ 1 ];
      p[ ec ].x -= 1.0f;
      n[ ec ] = te.n[ 1 ];
      ec++;
    }
    if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
    {
      p[ ec ] = te.p[ 2 ];
      p[ ec ].x -= 1.0f;
      n[ ec ] = te.n[ 2 ];
      ec++;
    }
    edgeIndex = x + 1 + kTempChunkSize * ( y + ( z + 1 ) * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_TOP_FRONT_BIT )
    {
      p[ ec ] = te.p[ 0 ];
      p[ ec ].y -= 1.0f;
      n[ ec ] = te.n[ 0 ];
      ec++;
    }
    if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
    {
      p[ ec ] = te.p[ 2 ];
      p[ ec ].y -= 1.0f;
      n[ ec ] = te.n[ 2 ];
      ec++;
    }
    edgeIndex = x + kTempChunkSize * ( y + ( z + 1 ) * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
    {
      p[ ec ] = te.p[ 2 ];
      p[ ec ].x -= 1.0f;
      p[ ec ].y -= 1.0f;
      n[ ec ] = te.n[ 2 ];
      ec++;
    }
    edgeIndex = x + kTempChunkSize * ( y + 1 + z * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_TOP_RIGHT_BIT )
    {
      p[ ec ] = te.p[ 1 ];
      p[ ec ].x -= 1.0f;
      p[ ec ].z -= 1.0f;
      n[ ec ] = te.n[ 1 ];
      ec++;
    }
    edgeIndex = x + 1 + kTempChunkSize * ( y + z * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_TOP_FRONT_BIT )
    {
      p[ ec ] = te.p[ 0 ];
      p[ ec ].y -= 1.0f;
      p[ ec ].z -= 1.0f;
      n[ ec ] = te.n[ 0 ];
      ec++;
    }
    edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + z * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    te = edgeInfo[ edgeIndex ];
    if ( te.b & EDGE_TOP_FRONT_BIT )
    {
      p[ ec ] = te.p[ 0 ];
      p[ ec ].z -= 1.0f;
      n[ ec ] = te.n[ 0 ];
      ec++;
    }
    if ( te.b & EDGE_TOP_RIGHT_BIT )
    {
      p[ ec ] = te.p[ 1 ];
      p[ ec ].z -= 1.0f;
      n[ ec ] = te.n[ 1 ];
      ec++;
    }
    
    AE_ASSERT( ec != 0 );
    
    for ( int32_t j = 0; j < ec; j++ )
    {
      AE_ASSERT( p[ j ] == p[ j ] );
      AE_ASSERT( p[ j ].x >= 0.0f && p[ j ].x <= 1.0f );
      AE_ASSERT( p[ j ].y >= 0.0f && p[ j ].y <= 1.0f );
      AE_ASSERT( p[ j ].z >= 0.0f && p[ j ].z <= 1.0f );
      AE_ASSERT( n[ j ] == n[ j ] );
    }
    aeFloat3 position = GetIntersection( p, n, ec );
    AE_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
    position.x = chunkOffsetX + x + aeMath::Clip( position.x, 0.0f, 0.999f );
    position.y = chunkOffsetY + y + aeMath::Clip( position.y, 0.0f, 0.999f );
    position.z = chunkOffsetZ + z + aeMath::Clip( position.z, 0.0f, 0.999f );
    
    vertex->position = position;
    vertex->normal = aeFloat3( 0.0f );
    for ( int32_t j = 0; j < ec; j++ )
    {
      vertex->normal += n[ j ];
    }
    vertex->normal.SafeNormalize();
    vertex->info[ 0 ] = 0;
    vertex->info[ 1 ] = (uint8_t)( 1.5f ); // @HACK: Lighting values
    vertex->info[ 2 ] = 255;// @HACK: TerrainType( position );
    vertex->info[ 3 ] = 0;

    uint8_t material = sdf->GetMaterial( position.FloorCopy() );
    vertex->materials[ 0 ] = ( material == 0 ) ? 255 : 0;
    vertex->materials[ 1 ] = ( material == 1 ) ? 255 : 0;
    vertex->materials[ 2 ] = ( material == 2 ) ? 255 : 0;
    vertex->materials[ 3 ] = ( material == 3 ) ? 255 : 0;
  }
  
  AE_ASSERT( vertexCount <= kMaxChunkVerts );
  AE_ASSERT( indexCount <= kMaxChunkIndices );
  *vertexCountOut = vertexCount;
  *indexCountOut = indexCount;
}

void aeTerrain::UpdateChunkLighting( Chunk* chunk )
{
//  int32_t cx = chunk->pos[ 0 ] * kChunkSize;
//  int32_t cy = chunk->pos[ 1 ] * kChunkSize;
//  int32_t cz = chunk->pos[ 2 ] * kChunkSize;
//  
//  aeFloat3 ray[ 9 ];
//  for ( int32_t sy = 0; sy < 3; sy++ )
//  for ( int32_t sx = 0; sx < 3; sx++ )
//  {
//    int32_t i = sy * 3 + sx;
//    ray[ i ].x = sx - 1;
//    ray[ i ].y = sy - 1;
//    ray[ i ].z = 1;
//    ray[ i ].Normalize();
//    ray[ i ] *= 40.0f;
//  }
  
  for( uint32_t z = 0; z < kChunkSize; z++ )
  for( uint32_t y = 0; y < kChunkSize; y++ )
  for( uint32_t x = 0; x < kChunkSize; x++ )
  {
//    int32_t hits = 0;
//    aeFloat3 source( cx + x + 0.5f, cy + y + 0.5f, cz + z + 0.5f );
//    for ( int32_t r = 0; r < 9; r++ )
//    {
//      if ( VoxelRaycast( source, ray[ r ], 9   ) != Block::Exterior ) { hits++; }
//    }
//    float mod = 1.0f - ( hits / 9.0f );
    float mod = 0.7125f;
    chunk->m_l[ x ][ y ][ z ] = kSkyBrightness * mod * 0.85f;
  }
  
  chunk->m_lightDirty = false;
}

Chunk* aeTerrain::AllocChunk( aeFloat3 center, aeInt3 pos )
{
  AE_ASSERT( pos.x >= 0 );
  AE_ASSERT( pos.y >= 0 );
  AE_ASSERT( pos.z >= 0 );
  AE_ASSERT( pos.x < kWorldChunksWidth );
  AE_ASSERT( pos.y < kWorldChunksWidth );
  AE_ASSERT( pos.z < kWorldChunksHeight );
  
  Chunk* chunk = m_chunkPool.Allocate();
  if ( !chunk )
  {
    return nullptr;
  }
  
  chunk->m_pos = pos;
  chunk->m_lightDirty = true;
  
  AE_ASSERT( !chunk->m_vertices );
  
  return chunk;
}

void aeTerrain::FreeChunk( Chunk* chunk )
{
  AE_ASSERT( chunk );
  AE_ASSERT( chunk->m_check == 0xCDCDCDCD );

  // Only clear chunk from world if set (may not be set in the case of a new chunk with zero verts)
  uint32_t chunkIndex = chunk->GetIndex();
  if ( m_chunks[ chunkIndex ] == chunk )
  {
    m_chunks[ chunkIndex ] = nullptr;
  }
  
  // @TODO: Cleanup how chunk resources are released
  //m_compactAlloc.Free( &chunk->m_vertices );
  //AE_ASSERT( !chunk->m_vertices );
  aeAlloc::Release( chunk->m_vertices );
  chunk->m_vertices = nullptr;

  // @NOTE: This has to be done last because CompactingAllocator keeps a pointer to m_vertices
  m_chunkPool.Free( chunk );
}

float aeTerrain::GetChunkScore( aeInt3 pos ) const
{
  aeFloat3 chunkCenter = ( aeFloat3( pos ) + aeFloat3( 0.5f ) ) * kChunkSize;
  float centerDistance = ( m_center - chunkCenter ).Length();

  bool hasNeighbor = false;
  hasNeighbor = hasNeighbor || 0 < GetVoxelCount( pos + aeInt3( 1, 0, 0 ) );
  hasNeighbor = hasNeighbor || 0 < GetVoxelCount( pos + aeInt3( 0, 1, 0 ) );
  hasNeighbor = hasNeighbor || 0 < GetVoxelCount( pos + aeInt3( 0, 0, 1 ) );
  hasNeighbor = hasNeighbor || 0 < GetVoxelCount( pos + aeInt3( -1, 0, 0 ) );
  hasNeighbor = hasNeighbor || 0 < GetVoxelCount( pos + aeInt3( 0, -1, 0 ) );
  hasNeighbor = hasNeighbor || 0 < GetVoxelCount( pos + aeInt3( 0, 0, -1 ) );
  
  if ( hasNeighbor )
  {
    return centerDistance;
  }
  else
  {
    // @NOTE: Non-empty chunks are found faster when chunks with empty neighbors are deprioritized
    return centerDistance * centerDistance;
  }
}

void Chunk::m_GetOffsetsFromEdge( uint32_t edgeBit, int32_t (&offsets)[ 4 ][ 3 ] )
{
  if ( edgeBit == EDGE_TOP_FRONT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
    offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = 1;
  }
  else if ( edgeBit == EDGE_TOP_RIGHT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
    offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = 1;
  }
  else if ( edgeBit == EDGE_TOP_BACK_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
    offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = 1;
  }
  else if ( edgeBit == EDGE_TOP_LEFT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = -1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 1;
    offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = 1;
  }
  else if ( edgeBit == EDGE_SIDE_FRONTLEFT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = -1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
    offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = 0;
  }
  else if ( edgeBit == EDGE_SIDE_FRONTRIGHT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
    offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = 0;
  }
  else if ( edgeBit == EDGE_SIDE_BACKRIGHT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
    offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = 0;
  }
  else if ( edgeBit == EDGE_SIDE_BACKLEFT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = -1; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = 0;
    offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = 0;
  }
  else if ( edgeBit == EDGE_BOTTOM_FRONT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = 1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
    offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = 1; offsets[ 3 ][ 2 ] = -1;
  }
  else if ( edgeBit == EDGE_BOTTOM_RIGHT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
    offsets[ 3 ][ 0 ] = 1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = -1;
  }
  else if ( edgeBit == EDGE_BOTTOM_BACK_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = 0; offsets[ 1 ][ 1 ] = -1; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
    offsets[ 3 ][ 0 ] = 0; offsets[ 3 ][ 1 ] = -1; offsets[ 3 ][ 2 ] = -1;
  }
  else if ( edgeBit == EDGE_BOTTOM_LEFT_BIT )
  {
    offsets[ 0 ][ 0 ] = 0; offsets[ 0 ][ 1 ] = 0; offsets[ 0 ][ 2 ] = 0;
    offsets[ 1 ][ 0 ] = -1; offsets[ 1 ][ 1 ] = 0; offsets[ 1 ][ 2 ] = 0;
    offsets[ 2 ][ 0 ] = 0; offsets[ 2 ][ 1 ] = 0; offsets[ 2 ][ 2 ] = -1;
    offsets[ 3 ][ 0 ] = -1; offsets[ 3 ][ 1 ] = 0; offsets[ 3 ][ 2 ] = -1;
  }
  else
  {
    AE_FAIL();
  }
}

const Chunk* aeTerrain::GetChunk( aeInt3 pos ) const
{
  if ( pos.x >= kWorldChunksWidth || pos.y >= kWorldChunksWidth || pos.z >= kWorldChunksHeight )
  {
    return nullptr;
  }

  uint32_t c = pos.x + kWorldChunksWidth * ( pos.y + kWorldChunksWidth * pos.z );
  
  Chunk* chunk = m_chunks[ c ];
  if ( chunk ) { AE_ASSERT( chunk->m_check == 0xCDCDCDCD ); }
  return chunk;
}

Chunk* aeTerrain::GetChunk( aeInt3 pos )
{
  return (Chunk*)( (const aeTerrain*)this )->GetChunk( pos );
}

int32_t aeTerrain::GetVoxelCount( aeInt3 pos ) const
{
  if ( pos.x >= kWorldChunksWidth || pos.y >= kWorldChunksWidth || pos.z >= kWorldChunksHeight )
  {
    return -1;
  }

  return m_voxelCounts[ Chunk::GetIndex( pos ) ];
}

aeTerrain::~aeTerrain()
{
  Terminate();
}

void aeTerrain::Initialize( uint32_t maxThreads, bool render )
{
  m_render = render;

  //m_compactAlloc.Expand( 128 * ( 1 << 20 ) );

  //m_chunkPool.Initialize(); @TODO: Reset pool
  
  uint32_t worldChunkBytes = sizeof(m_chunks[0]) * kWorldChunksWidth * kWorldChunksWidth * kWorldChunksHeight;
  uint32_t voxelCountBytes = sizeof(m_voxelCounts[0]) * kWorldChunksWidth * kWorldChunksWidth * kWorldChunksHeight;
  uint32_t allocSize = worldChunkBytes + voxelCountBytes;

  // @TODO: An alignment of 1 here is probably not okay. Maybe world chunks and voxels should
  //        be separately allocated as arrays with the correct type.
  m_chunkRawAlloc = aeAlloc::AllocateRaw( 1, 1, worldChunkBytes + voxelCountBytes );
  
  memset( m_chunkRawAlloc, 0, worldChunkBytes );
  m_chunks = (Chunk**)m_chunkRawAlloc;
  m_voxelCounts = (int16_t*)( (int8_t*)m_chunkRawAlloc + worldChunkBytes );
  // @TODO: This is a little too sneaky, it sets all voxel counts to -1 which is used to
  //        initialize them later on. Chunks should be flagged for initialization more explicitly.
  memset( m_voxelCounts, ~0, voxelCountBytes );

  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockCollision[ i ] = true; }
  m_blockCollision[ Block::Exterior ] = false;
  m_blockCollision[ Block::Unloaded ] = false;
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockDensity[ i ] = 1.0f; }

  m_threadPool = aeAlloc::Allocate< ctpl::thread_pool >( maxThreads );
  maxThreads = aeMath::Max( 1u, maxThreads );
  for ( uint32_t i = 0; i < maxThreads; i++ )
  {
    m_terrainJobs.Append( aeAlloc::Allocate< aeTerrainJob >() );
  }
}

void aeTerrain::Terminate()
{
  if ( m_threadPool )
  {
    m_threadPool->stop( true );
    aeAlloc::Release( m_threadPool );
    m_threadPool = nullptr;
  }

  for ( uint32_t i = 0; i < m_terrainJobs.Length(); i++ )
  {
    aeTerrainJob* job = m_terrainJobs[ i ];
    if ( job->IsPendingFinish() )
    {
      job->Finish();
    }
    aeAlloc::Release( job );
  }
  m_terrainJobs.Clear();

  for ( Chunk* chunk = m_chunkPool.GetFirst(); chunk; chunk = m_chunkPool.GetNext( chunk ) )
  {
    FreeChunk( chunk );
  }
  AE_ASSERT( m_chunkPool.Length() == 0 );

  if ( m_chunkRawAlloc )
  {
    aeAlloc::Release( m_chunkRawAlloc );
    m_chunkRawAlloc = nullptr;
  }
}

void aeTerrain::Update( aeFloat3 center, float radius )
{
  int32_t chunkViewRadius = radius / kChunkSize;
  const int32_t kChunkViewDiam = chunkViewRadius + chunkViewRadius;

  double currentTime = aeClock::GetTime();

  m_center = center;
  m_radius = radius;
  
  //------------------------------------------------------------------------------
  // Determine which chunks will be processed
  //------------------------------------------------------------------------------
  //t_chunkMap.Clear();
  //t_chunkMap.Reserve( kChunkViewDiam * kChunkViewDiam * kChunkViewDiam );
  t_chunkMap_hack.clear();
  aeInt3 viewChunk = ( m_center / kChunkSize ).NearestCopy();
  for ( int32_t k = 0; k < kChunkViewDiam; k++ )
  {
    for ( int32_t j = 0; j < kChunkViewDiam; j++ )
    {
      for ( int32_t i = 0; i < kChunkViewDiam; i++ )
      {
        aeInt3 chunkPos( i, j, k );
        chunkPos -= aeInt3( chunkViewRadius );
        chunkPos += viewChunk;

        if ( chunkPos.x < 0 || chunkPos.x >= kWorldChunksWidth
          || chunkPos.y < 0 || chunkPos.y >= kWorldChunksWidth
          || chunkPos.z < 0 || chunkPos.z >= kWorldChunksHeight )
        {
          continue;
        }

        int32_t ci = chunkPos.x + kWorldChunksWidth * ( chunkPos.y + kWorldChunksWidth * chunkPos.z );
        int16_t vc = m_voxelCounts[ ci ];
        if ( vc == 0 || vc == kChunkCountMax ) // @TODO: What does it mean to skip on chunk max?
        {
          continue;
        }

        aeFloat3 chunkCenter = ( aeFloat3( chunkPos ) + aeFloat3( 0.5f ) ) * kChunkSize;
        float centerDistance = ( m_center - chunkCenter ).Length();
        if ( centerDistance >= radius )
        {
          continue;
        }

        Chunk* c = m_chunks[ ci ];
        if ( c )
        {
          AE_ASSERT( m_voxelCounts[ ci ] > 0 );
          AE_ASSERT( c->m_vertices );
        }

        //ChunkSort& chunkSort = t_chunkMap.Set( chunkPos, ChunkSort() );
        ChunkSort chunkSort;
        chunkSort.c = c;
        chunkSort.pos = chunkPos;
        chunkSort.score = GetChunkScore( chunkPos );
        t_chunkMap_hack[ Chunk::GetIndex( chunkPos ) ] = chunkSort;
      }
    }
  }
  // Add all currently generated chunks
  for ( Chunk* c = m_generatedList.GetFirst(); c; c = c->m_generatedList.GetNext() )
  {
    aeInt3 pos = c->m_pos;
//#if _AE_DEBUG_
//    const ChunkSort* check = t_chunkMap.TryGet( pos );
//    AE_ASSERT( check->c == c );
//#endif

    //ChunkSort& chunkSort = t_chunkMap.Set( pos, ChunkSort() );
    ChunkSort chunkSort;
    chunkSort.c = c;
    chunkSort.pos = pos;
    chunkSort.score = GetChunkScore( pos );
    t_chunkMap_hack[ Chunk::GetIndex( pos ) ] = chunkSort;
  }

  //------------------------------------------------------------------------------
  // Sort chunks based on priority
  //------------------------------------------------------------------------------
  t_chunkSorts.Clear();
  t_chunkSorts.Reserve( kChunkViewDiam * kChunkViewDiam * kChunkViewDiam );
  //for ( uint32_t i = 0; i < t_chunkMap.Length(); i++ )
  //{
  //  t_chunkSorts.Append( t_chunkMap.GetValue( i ) );
  //}
  for ( auto element : t_chunkMap_hack )
  {
    t_chunkSorts.Append( element.second );
  }
  // Sort chunks by score, low score is best
  if ( t_chunkSorts.Length() )
  {
    std::sort( &t_chunkSorts[ 0 ], ( &t_chunkSorts[ 0 ] + t_chunkSorts.Length() ),
      []( const ChunkSort& a, const ChunkSort& b ) -> bool
    {
      return a.score < b.score;
    } );
  }
  if ( m_debugTextFn )
  {
    for ( uint32_t i = 0; i < t_chunkSorts.Length(); i++ )
    {
      const ChunkSort* sort = &t_chunkSorts[ i ];

      aeFloat3 p = aeFloat3( sort->pos );
      p += aeFloat3( 0.5f );
      p *= kChunkSize;

      const char* status = sort->c ? "generated" : "pending";
      aeStr64 str = aeStr64::Format( "score:#\nstatus:#", sort->score, status );
      m_debugTextFn( p, str.c_str() );
    }
  }
  
  //------------------------------------------------------------------------------
  // Finish terrain jobs
  // @NOTE: Do this as late as possible so jobs can run while sorting is happening
  //------------------------------------------------------------------------------
  for ( uint32_t i = 0; i < m_terrainJobs.Length(); i++ )
  {
    aeTerrainJob* job = m_terrainJobs[ i ];
    if ( !job->IsPendingFinish() )
    {
      continue;
    }

    Chunk* newChunk = job->GetChunk();
    AE_ASSERT( newChunk );
    AE_ASSERT( newChunk->m_check == 0xCDCDCDCD );

    uint32_t chunkIndex = newChunk->GetIndex();
    Chunk* oldChunk = m_chunks[ chunkIndex ];

    uint32_t vertexCount = job->GetVertexCount();
    uint32_t indexCount = job->GetIndexCount();

    AE_ASSERT( vertexCount <= kChunkCountMax );
    if ( vertexCount == 0 || vertexCount == kChunkCountMax )
    {
      // @TODO: It's super expensive to finally just throw away the unneeded chunk...
      FreeChunk( newChunk );
      newChunk = nullptr;
    }
    else
    {
      if ( m_render )
      {
        // (Re)initialize aeVertexData here only when needed
        if ( newChunk->m_data.GetIndexCount() == 0 // Not initialized
          || newChunk->m_data.GetMaxVertexCount() < vertexCount // Too little storage for verts
          || newChunk->m_data.GetMaxIndexCount() < indexCount ) // Too little storage for t_chunkIndices
        {
          newChunk->m_data.Initialize( sizeof( TerrainVertex ), sizeof( TerrainIndex ), vertexCount, indexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Dynamic, aeVertexUsage::Dynamic );
          newChunk->m_data.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( TerrainVertex, position ) );
          newChunk->m_data.AddAttribute( "a_normal", 3, aeVertexDataType::Float, offsetof( TerrainVertex, normal ) );
          newChunk->m_data.AddAttribute( "a_info", 4, aeVertexDataType::UInt8, offsetof( TerrainVertex, info ) );
          newChunk->m_data.AddAttribute( "a_materials", 4, aeVertexDataType::NormalizedUInt8, offsetof( TerrainVertex, materials ) );
        }

        // Set vertices
        newChunk->m_data.SetVertices( job->GetVertices(), vertexCount );
        newChunk->m_data.SetIndices( job->GetIndices(), indexCount );
      }

      // Copy chunk verts from job
      // @TODO: This should be handled by the Chunk
      uint32_t vertexBytes = vertexCount * sizeof( TerrainVertex );
      //m_compactAlloc.Allocate( &chunk->m_vertices, vertexBytes );
      newChunk->m_vertices = aeAlloc::AllocateArray< TerrainVertex >( vertexCount );
      memcpy( newChunk->m_vertices, job->GetVertices(), vertexBytes );

      // Ready for lighting
      newChunk->m_lightDirty = true;
      if ( oldChunk )
      {
        // @NOTE: Copy dirty flag to new chunk in case it's been modified since the job started.
        newChunk->m_geoDirty = oldChunk->m_geoDirty;
      }
    }

    if ( oldChunk )
    {
      // @NOTE: Replace old chunk in sorted list with the job chunk
      int32_t sortIndex = t_chunkSorts.FindFn( [oldChunk]( ChunkSort& cs ) { return cs.c == oldChunk; } );
      if ( sortIndex >= 0 )
      {
        t_chunkSorts[ sortIndex ].c = newChunk;
      }
      FreeChunk( oldChunk );
    }

    // Record that the chunk has been generated
    m_voxelCounts[ chunkIndex ] = vertexCount;
    // Set world grid chunk
    if ( newChunk )
    {
      m_chunks[ chunkIndex ] = newChunk;
      // Add new chunk to list of finished chunks
      m_generatedList.Append( newChunk->m_generatedList );
    }
    else
    {
      // Clear old chunk
      m_chunks[ chunkIndex ] = nullptr;
    }

    job->Finish();
  }

  if ( m_threadPool->size() == 0 || m_threadPool->n_idle() == m_threadPool->size() )
  {
    // "Commit" changes to sdf safely while no jobs are running
    sdf.UpdatePending();
  }
  else if ( sdf.HasPending() )
  {
    // Don't start new terrain jobs if sdf has changed
    return;
  }

  //------------------------------------------------------------------------------
  // Start new terrain jobs
  //------------------------------------------------------------------------------
  for ( uint32_t i = 0; i < t_chunkSorts.Length(); i++ )
  {
    const ChunkSort* chunkSort = &t_chunkSorts[ i ];
    aeInt3 chunkPos = chunkSort->pos;
    Chunk* chunk = chunkSort->c;
    uint32_t chunkIndex = Chunk::GetIndex( chunkPos );
    Chunk* indexPosChunk = m_chunks[ chunkIndex ];
    if ( chunk )
    {
      AE_ASSERT_MSG( indexPosChunk == chunk, "# #", indexPosChunk, chunk );
      AE_ASSERT_MSG( m_voxelCounts[ chunkIndex ] > 0, "count #", m_voxelCounts[ chunkIndex ] );
      AE_ASSERT( chunk->m_vertices );
      if ( m_render )
      {
        AE_ASSERT( chunk->m_data.GetVertexCount() );
        AE_ASSERT( chunk->m_data.GetVertexSize() );
      }
    }
    else
    {
      // @NOTE: Grab any chunks that were finished generating after sorting completed
      chunk = indexPosChunk;
    }

    if( !chunk || chunk->m_geoDirty )
    {
      if ( m_threadPool->n_idle() == 0 && m_threadPool->size() > 0 )
      {
        break;
      }

      int32_t jobIndex = m_terrainJobs.FindFn( []( aeTerrainJob* j ) { return !j->HasJob(); } );
      if ( jobIndex < 0 )
      {
        break;
      }

      int32_t otherJobIndex = m_terrainJobs.FindFn( [chunkPos]( aeTerrainJob* j ) { return j->HasChunk( chunkPos ); } );
      if ( otherJobIndex >= 0 )
      {
        // @NOTE: Already queued
        continue;
      }

      bool chunkDirty = false;
      if ( chunk && chunk->m_geoDirty )
      {
        // @NOTE: Clear dirty flag here (and not when the job is finished) so any changes
        // made while the job is running will not be lost and will cause the chunk to be
        // regenerated again.
        chunk->m_geoDirty = false;
        chunkDirty = true;
      }

      // Always allocate a new chunk (even for dirty chunks)
      chunk = AllocChunk( center, chunkPos );
      if ( !chunk )
      {
        for ( int32_t i = t_chunkSorts.Length() - 1; i >= 0; i-- )
        {
          ChunkSort* other = &t_chunkSorts[ i ];
          if ( other->c )
          {
            // @NOTE: Always steal the lowest priority chunk to regenerate dirty chunks
            if ( chunkDirty || other->score > chunkSort->score )
            {
              FreeChunk( other->c );
              t_chunkSorts.Remove( i );

              chunk = AllocChunk( center, chunkPos );
              AE_ASSERT( chunk );
            }
            break;
          }
          else
          {
            t_chunkSorts.Remove( i );
          }
        }
      }
      if ( !chunk )
      {
        // Loaded chunks at equilibrium. The highest priority chunks are already loaded.
        // AE_LOG( "Chunk loading reached equilibrium" );
        break;
      }

      aeTerrainJob* job = m_terrainJobs[ jobIndex ];
      AE_ASSERT( job );
      job->StartNew( &sdf, chunk );
      // @NOTE: replaceDirty_CHECK doesn't do anything, but is used to assert when the job
      // is done that another chunk is being replaced.
      if ( m_threadPool->size() )
      {
        m_threadPool->push( [ job ]( int id )
        {
          AE_ASSERT( job );
          AE_ASSERT( job->GetChunk() );
          job->Do();
        } );
      }
      else
      {
        job->Do();
        break;
      }
    }
  }
}

void aeTerrain::Render( const aeShader* shader, const aeUniformList& shaderParams )
{
  if ( !m_render )
  {
    return;
  }

  //aeFrustum frustum;
  uint32_t activeCount = 0;
  for( uint32_t i = 0; i < t_chunkSorts.Length() && activeCount < kMaxActiveChunks; i++ )
  {
    Chunk* chunk = t_chunkSorts[ i ].c;
    if ( !chunk )
    {
      continue;
    }
    AE_ASSERT( chunk->m_check == 0xCDCDCDCD );
    uint32_t chunkIndex = chunk->GetIndex();
    AE_ASSERT( m_voxelCounts[ chunkIndex ] > 0 );
    AE_ASSERT( chunk->m_data.GetVertexCount() );
    AE_ASSERT( chunk->m_data.GetVertexSize() );
    
    // Only render the visible chunks
    //if( frustum.TestChunk( chunk ) ) // @TODO: Should make sure chunk is visible
    {
      chunk->m_data.Render( shader, shaderParams );
      activeCount++;
    }
  }

  //AE_LOG( "chunks active:# allocated:#", activeCount, m_chunkPool.Length() );
}

void aeTerrain::RenderDebug( aeDebugRender* debug )
{
  sdf.RenderDebug( debug );
}

void aeTerrain::SetCallback( void* userdata, float ( *fn )( void*, aeFloat3 ) )
{
  sdf.m_userdata = userdata;
  sdf.m_fn1 = nullptr;
  sdf.m_fn2 = fn;
}

void aeTerrain::SetCallback( float ( *fn )( aeFloat3 ) )
{
  sdf.m_userdata = nullptr;
  sdf.m_fn1 = fn;
  sdf.m_fn2 = nullptr;
}

void aeTerrain::Dirty( aeAABB aabb )
{
  // @NOTE: Add a buffer region so voxels on the edge of the aabb are refreshed
  aabb.Expand( kSdfBoundary );

  aeInt3 minChunk = ( aabb.GetMin() / kChunkSize ).FloorCopy();
  aeInt3 maxChunk = ( aabb.GetMax() / kChunkSize ).CeilCopy();
  minChunk = aeMath::Max( minChunk, aeInt3( 0 ) );
  maxChunk = aeMath::Min( maxChunk, aeInt3( kWorldChunksWidth, kWorldChunksWidth, kWorldChunksHeight ) );

  for ( int32_t z = minChunk.z; z < maxChunk.z; z++ )
  for ( int32_t y = minChunk.y; y < maxChunk.y; y++ )
  for ( int32_t x = minChunk.x; x < maxChunk.x; x++ )
  {
    aeInt3 pos( x, y, z );
    if ( Chunk* chunk = GetChunk( pos ) )
    {
      chunk->m_geoDirty = true;
    }
    else
    {
      m_voxelCounts[ Chunk::GetIndex( pos ) ] = ~0;
    }
  }
}

bool aeTerrain::GetCollision( uint32_t x, uint32_t y, uint32_t z ) const
{
  return m_blockCollision[ GetVoxel( x, y, z ) ];
}

bool aeTerrain::GetCollision( aeFloat3 position ) const
{
  return m_blockCollision[ GetVoxel( position.x, position.y, position.z ) ];
}

Block::Type aeTerrain::GetVoxel( aeFloat3 position ) const
{
  return GetVoxel( position.x, position.y, position.z );
}

Block::Type aeTerrain::GetVoxel( uint32_t x, uint32_t y, uint32_t z ) const
{
  int32_t cx = x / kChunkSize;
  int32_t cy = y / kChunkSize;
  int32_t cz = z / kChunkSize;
  
  if( cx < 0 || cy < 0 || cz < 0
    || cx >= kWorldChunksWidth
    || cy >= kWorldChunksWidth
    || cz >= kWorldChunksHeight )
  {
    return Block::Exterior;
  }
  
  uint32_t ci = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
  int16_t vc = m_voxelCounts[ ci ];
  if ( vc == 0 ) { return Block::Exterior; }
  // TODO AE_ASSERT( vc != -1 && vc != kChunkCountMax );
  if ( vc == -1 ) { return Block::Unloaded; }
  if ( vc == kChunkCountMax ) { return Block::Interior; }
  Chunk* chunk = m_chunks[ ci ];
  if ( !chunk ) { return Block::Unloaded; }
  return chunk->m_t[ x % kChunkSize ][ y % kChunkSize ][ z % kChunkSize ];
}

const TerrainVertex* aeTerrain::m_GetVertex( int32_t x, int32_t y, int32_t z ) const
{
  int32_t cx = x / kChunkSize;
  int32_t cy = y / kChunkSize;
  int32_t cz = z / kChunkSize;

  if ( cx < 0 || cy < 0 || cz < 0
    || cx >= kWorldChunksWidth
    || cy >= kWorldChunksWidth
    || cz >= kWorldChunksHeight )
  {
    return nullptr;
  }

  uint32_t ci = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
  int16_t vc = m_voxelCounts[ ci ];
  if ( vc == 0 )
  {
    return nullptr;
  }
  // TODO AE_ASSERT( vc != -1 && vc != kChunkCountMax );
  if ( vc == -1 )
  {
    return nullptr;
  }
  if ( vc == kChunkCountMax )
  {
    return nullptr;
  }
  Chunk* chunk = m_chunks[ ci ];
  if ( !chunk )
  {
    return nullptr;
  }

  TerrainIndex index = chunk->m_i[ x % kChunkSize ][ y % kChunkSize ][ z % kChunkSize ];
  if ( index == (TerrainIndex)~0 )
  {
    return nullptr;
  }

  return &chunk->m_vertices[ index ];
}

float16_t aeTerrain::GetLight( uint32_t x, uint32_t y, uint32_t z ) const
{
  uint32_t cix = x / kChunkSize;
  uint32_t ciy = y / kChunkSize;
  uint32_t ciz = z / kChunkSize;
  
  if( cix >= kWorldChunksWidth || ciy >= kWorldChunksWidth || ciz >= kWorldChunksHeight ) { return kSkyBrightness; }
  const Chunk *chunk = GetChunk( aeInt3( cix, ciy, ciz ) );
  if( chunk == nullptr ) { return kSkyBrightness; }
  
  return chunk->m_l[ x - cix * kChunkSize ][ y - ciy * kChunkSize ][ z - ciz * kChunkSize ];
}

bool aeTerrain::VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps ) const
{
  int32_t x = aeMath::Floor( start.x );
  int32_t y = aeMath::Floor( start.y );
  int32_t z = aeMath::Floor( start.z );
  
  if ( ray.LengthSquared() < 0.001f ){ return Block::Exterior; }
  aeFloat3 dir = ray.SafeNormalizeCopy();
  
  aeFloat3 curpos = start;
  aeFloat3 cb, tmax, tdelta;
  int stepX, outX;
	int stepY, outY;
	int stepZ, outZ;
	if (dir.x > 0)
	{
		stepX = 1;
    outX = ceil( start.x + ray.x );
    outX = aeMath::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX );
		cb.x = x + 1;
	}
	else 
	{
		stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = aeMath::Max( -1, outX );
		cb.x = x;
	}
	if (dir.y > 0.0f)
	{
		stepY = 1;
    outY = ceil( start.y + ray.y );
    outY = aeMath::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY );
		cb.y = y + 1;
	}
	else 
	{
		stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = aeMath::Max( -1, outY );
		cb.y = y;
	}
	if (dir.z > 0.0f)
	{
		stepZ = 1;
    outZ = ceil( start.z + ray.z );
    outZ = aeMath::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ );
		cb.z = z + 1;
	}
	else 
	{
		stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    outZ = aeMath::Max( -1, outZ );
		cb.z = z;
	}
	float rxr, ryr, rzr;
	if (dir.x != 0)
	{
		rxr = 1.0f / dir.x;
		tmax.x = (cb.x - curpos.x) * rxr; 
		tdelta.x = stepX * rxr;
	}
	else tmax.x = 1000000;
	if (dir.y != 0)
	{
		ryr = 1.0f / dir.y;
		tmax.y = (cb.y - curpos.y) * ryr; 
		tdelta.y = stepY * ryr;
	}
	else tmax.y = 1000000;
	if (dir.z != 0)
	{
		rzr = 1.0f / dir.z;
		tmax.z = (cb.z - curpos.z) * rzr; 
		tdelta.z = stepZ * rzr;
	}
	else tmax.z = 1000000;
  
  int32_t steps = 0;
  while ( !GetCollision( x, y, z ) || steps < minSteps )
	{
    steps++;
    
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				x = x + stepX;
				if ( x == outX ) return false;
				tmax.x += tdelta.x;
			}
			else
			{
				z = z + stepZ;
				if ( z == outZ ) return false;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				y = y + stepY;
				if ( y == outY ) return false;
				tmax.y += tdelta.y;
			}
			else
			{
				z = z + stepZ;
				if ( z == outZ ) return false;
				tmax.z += tdelta.z;
			}
		}
	}
  return true;
}

RaycastResult aeTerrain::RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision ) const
{
  RaycastResult result;
  result.hit = false;
  result.type = Block::Exterior;
  result.distance = std::numeric_limits<float>::infinity();
  result.posi[ 0 ] = ~0;
  result.posi[ 1 ] = ~0;
  result.posi[ 2 ] = ~0;
  result.posf = aeFloat3( std::numeric_limits<float>::infinity() );
  result.normal = aeFloat3( std::numeric_limits<float>::infinity() );
  result.touchedUnloaded = false;
  
  int32_t x = aeMath::Floor( start.x );
  int32_t y = aeMath::Floor( start.y );
  int32_t z = aeMath::Floor( start.z );
  
  if ( ray.LengthSquared() < 0.001f ){ return result; }
  aeFloat3 dir = ray.SafeNormalizeCopy();
  
  aeFloat3 curpos = start;
  aeFloat3 cb, tmax, tdelta;
  int32_t stepX, outX;
	int32_t stepY, outY;
	int32_t stepZ, outZ;
	if (dir.x > 0)
	{
		stepX = 1;
    outX = ceil( start.x + ray.x );
    outX = aeMath::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX );
		cb.x = x + 1;
	}
	else 
	{
		stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = aeMath::Max( -1, outX );
		cb.x = x;
	}
	if (dir.y > 0.0f)
	{
		stepY = 1;
    outY = ceil( start.y + ray.y );
    outY = aeMath::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY );
		cb.y = y + 1;
	}
	else 
	{
		stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = aeMath::Max( -1, outY );
		cb.y = y;
	}
	if (dir.z > 0.0f)
	{
		stepZ = 1;
    outZ = ceil( start.z + ray.z );
    outZ = aeMath::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ );
		cb.z = z + 1;
	}
	else 
	{
		stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    outZ = aeMath::Max( -1, outZ );
		cb.z = z;
	}
	float rxr, ryr, rzr;
	if (dir.x != 0)
	{
		rxr = 1.0f / dir.x;
		tmax.x = (cb.x - curpos.x) * rxr; 
		tdelta.x = stepX * rxr;
	}
	else tmax.x = 1000000;
	if (dir.y != 0)
	{
		ryr = 1.0f / dir.y;
		tmax.y = (cb.y - curpos.y) * ryr; 
		tdelta.y = stepY * ryr;
	}
	else tmax.y = 1000000;
	if (dir.z != 0)
	{
		rzr = 1.0f / dir.z;
		tmax.z = (cb.z - curpos.z) * rzr; 
		tdelta.z = stepZ * rzr;
	}
	else tmax.z = 1000000;
  
  while ( ( result.type = GetVoxel( x, y, z ) ) != Block::Surface || !allowSourceCollision )
	{
    if( result.type == Block::Unloaded ) { result.touchedUnloaded = true; }
      
    allowSourceCollision = true;
    
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				x = x + stepX;
				if ( x == outX ) return result;
				tmax.x += tdelta.x;
			}
			else
			{
				z = z + stepZ;
				if ( z == outZ ) return result;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				y = y + stepY;
				if ( y == outY ) return result;
				tmax.y += tdelta.y;
			}
			else
			{
				z = z + stepZ;
				if ( z == outZ ) return result;
				tmax.z += tdelta.z;
			}
		}
	}
  
  AE_ASSERT( result.type != Block::Exterior && result.type != Block::Unloaded );
  
  result.hit = true;
  result.posi[ 0 ] = x;
  result.posi[ 1 ] = y;
  result.posi[ 2 ] = z;
  
  int32_t cx = x / kChunkSize;
  int32_t cy = y / kChunkSize;
  int32_t cz = z / kChunkSize;
  const Chunk* c = GetChunk( aeInt3( cx, cy, cz ) );
  AE_ASSERT( c );
  int32_t lx = x % kChunkSize;
  int32_t ly = y % kChunkSize;
  int32_t lz = z % kChunkSize;
  TerrainIndex index = c->m_i[ lx ][ ly ][ lz ];
  // TODO Can somehow skip surface and hit interior cell
  AE_ASSERT( index != (TerrainIndex)~0 );
  aeFloat3 p = c->m_vertices[ index ].position;
  aeFloat3 n = c->m_vertices[ index ].normal.SafeNormalizeCopy();
  aeFloat3 r = ray.SafeNormalizeCopy();
  float t = n.Dot( p - start ) / n.Dot( r );
  result.distance = t;
  result.posf = start + r * t;
  result.normal = n;
  
  return result;
}

aeFloat3 IntersectRayAABB( aeFloat3 p, aeFloat3 d, const int32_t v[ 3 ] )
{
  float tmin = 0.0f;
  float tmax = std::numeric_limits<float>::max();
  for ( int i = 0; i < 3; i++ )
  {
    if ( fabs( d[ i ] ) < 0.001f ) { continue; }
    float ood = 1.0f / d[ i ];
    float t1 = ( v[ i ] - p[ i ] ) * ood;
    float t2 = ( v[ i ] + 1 - p[ i ] ) * ood;
    if ( t1 > t2 ) std::swap( t1, t2 );
    if ( t1 > tmin ) tmin = t1;
    if ( t2 > tmax ) tmax = t2;
  }
  return p + d * tmin;
}

RaycastResult aeTerrain::Raycast( aeFloat3 start, aeFloat3 ray ) const
{
  RaycastResult result;
  result.hit = false;
  result.type = Block::Exterior;
  result.distance = std::numeric_limits<float>::infinity();
  result.posi[ 0 ] = ~0;
  result.posi[ 1 ] = ~0;
  result.posi[ 2 ] = ~0;
  result.posf = aeFloat3( std::numeric_limits<float>::infinity() );
  result.normal = aeFloat3( std::numeric_limits<float>::infinity() );
  result.touchedUnloaded = false;
  
  int32_t x = aeMath::Floor( start.x );
  int32_t y = aeMath::Floor( start.y );
  int32_t z = aeMath::Floor( start.z );
  
  if ( ray.LengthSquared() < 0.001f ){ return result; }
  aeFloat3 dir = ray.SafeNormalizeCopy();
  
  aeFloat3 curpos = start;
  aeFloat3 cb, tmax, tdelta;
  int32_t stepX, outX;
	int32_t stepY, outY;
	int32_t stepZ, outZ;
	if (dir.x > 0)
	{
		stepX = 1;
    outX = ceil( start.x + ray.x );
    outX = aeMath::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX );
		cb.x = x + 1;
	}
	else 
	{
		stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = aeMath::Max( -1, outX );
		cb.x = x;
	}
	if (dir.y > 0.0f)
	{
		stepY = 1;
    outY = ceil( start.y + ray.y );
    outY = aeMath::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY );
		cb.y = y + 1;
	}
	else 
	{
		stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = aeMath::Max( -1, outY );
		cb.y = y;
	}
	if (dir.z > 0.0f)
	{
		stepZ = 1;
    outZ = ceil( start.z + ray.z );
    outZ = aeMath::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ );
		cb.z = z + 1;
	}
	else 
	{
		stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    outZ = aeMath::Max( -1, outZ );
		cb.z = z;
	}
	float rxr, ryr, rzr;
	if (dir.x != 0)
	{
		rxr = 1.0f / dir.x;
		tmax.x = (cb.x - curpos.x) * rxr; 
		tdelta.x = stepX * rxr;
	}
	else tmax.x = 1000000;
	if (dir.y != 0)
	{
		ryr = 1.0f / dir.y;
		tmax.y = (cb.y - curpos.y) * ryr; 
		tdelta.y = stepY * ryr;
	}
	else tmax.y = 1000000;
	if (dir.z != 0)
	{
		rzr = 1.0f / dir.z;
		tmax.z = (cb.z - curpos.z) * rzr; 
		tdelta.z = stepZ * rzr;
	}
	else tmax.z = 1000000;
  
  while ( true )
	{
    result.type = GetVoxel( x, y, z );
    if ( result.type == Block::Surface )
    {
      result.posi[ 0 ] = x;
      result.posi[ 1 ] = y;
      result.posi[ 2 ] = z;
      aeFloat3 iv0 = IntersectRayAABB( start, ray, result.posi );
      aeFloat3 iv1 = IntersectRayAABB( start + ray, -ray, result.posi );
      float fv0 = sdf.GetValue( iv0 );
      float fv1 = sdf.GetValue( iv1 );
      if( fv0 * fv1 <= 0.0f )
      {
        if ( fv0 > fv1 )
        {
          std::swap( fv0, fv1 );
          std::swap( iv0, iv1 );
        }
        aeFloat3 p;
        float fp = 0.0f;
        for ( int32_t ic = 0; ic < 10; ic++ )
        {
          p = iv0 * 0.5f + iv1 * 0.5f;
          fp = sdf.GetValue( p );
          if ( fp < 0.0f ) { iv0 = p; }
          else { iv1 = p; }
        }
        
        result.distance = ( p - start ).Length();
        result.posf = p;
        result.normal = sdf.GetDerivative( p );
        result.hit = true;
        
        return result;
      }
    }
    else if( result.type == Block::Unloaded ) { result.touchedUnloaded = true; }
    
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				x = x + stepX;
				if ( x == outX ) return result;
				tmax.x += tdelta.x;
			}
			else
			{
				z = z + stepZ;
				if ( z == outZ ) return result;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				y = y + stepY;
				if ( y == outY ) return result;
				tmax.y += tdelta.y;
			}
			else
			{
				z = z + stepZ;
				if ( z == outZ ) return result;
				tmax.z += tdelta.z;
			}
		}
	}
  
  return result;
}

bool aeTerrain::SweepSphere( aeSphere sphere, aeFloat3 ray, float* distanceOut, aeFloat3* normalOut, aeFloat3* posOut ) const
{
  aeSphere sphereEnd = sphere;
  sphereEnd.center += ray;

  aeAABB bounds( sphere );
  bounds.Expand( aeAABB( sphereEnd ) );
  const aeInt3 min = bounds.GetMin().FloorCopy();
  const aeInt3 max = bounds.GetMax().CeilCopy();

  const aeLineSegment travelSeg( sphere.center, sphere.center + ray );

  bool anyHit = false;
  float tMin = ray.Length();
  aeFloat3 posResult;
  aeFloat3 normalResult;
  for ( int32_t z = min.z; z < max.z; z++ )
  for ( int32_t y = min.y; y < max.y; y++ )
  for ( int32_t x = min.x; x < max.x; x++ )
  {
    const TerrainVertex* v = m_GetVertex( x, y, z );
    if ( !v )
    {
      continue;
    }

    aeFloat3 vertex = v->position;
    if ( travelSeg.GetMinDistance( vertex ) > sphere.radius )
    {
      continue;
    }

    if ( ray.Dot( vertex - sphere.center ) <= 0.0f )
    {
      continue;
    }

    float t = 0.0f;
    if ( sphere.Raycast( vertex, -ray, &t ) && t <= tMin )
    {
      anyHit = true;
      tMin = t;
      posResult = vertex;
      normalResult = v->normal.SafeNormalizeCopy();
    }
  }

  if ( anyHit )
  {
    if ( distanceOut )
    {
      *distanceOut = tMin;
    }

    if ( normalOut )
    {
      *normalOut = normalResult;
    }

    if ( posOut )
    {
      *posOut = posResult;
    }

    return true;
  }
  else
  {
    return false;
  }
}

bool aeTerrain::PushOutSphere( aeSphere sphere, aeFloat3* offsetOut, aeDebugRender* debug ) const
{
  aeAABB sphereAABB( sphere );
  aeInt3 sphereMin = sphereAABB.GetMin().FloorCopy();
  aeInt3 sphereMax = sphereAABB.GetMax().CeilCopy();
  if ( debug )
  {
    debug->AddAABB( ( sphereAABB.GetMax() + sphereAABB.GetMin() ) * 0.5f, ( sphereAABB.GetMax() - sphereAABB.GetMin() ) * 0.5f, aeColor::PicoPink() );
  }

  aeFloat3 pushOutDir( 0.0f );

  for ( int32_t z = sphereMin.z; z < sphereMax.z; z++ )
  for ( int32_t y = sphereMin.y; y < sphereMax.y; y++ )
  for ( int32_t x = sphereMin.x; x < sphereMax.x; x++ )
  {
    const TerrainVertex* v = m_GetVertex( x, y, z );
    if ( !v )
    {
      continue;
    }

    aeFloat3 centerToVert = v->position - sphere.center;
    float c = centerToVert.Dot( centerToVert ) - sphere.radius * sphere.radius;
    if ( c > 0.0f )
    {
      // Vertex outside sphere
      continue;
    }

    pushOutDir += v->normal.SafeNormalizeCopy();
  }
  if ( pushOutDir == aeFloat3( 0.0f ) )
  {
    if ( debug )
    {
      debug->AddSphere( sphere.center, sphere.radius, aeColor::Green(), 16 );
    }
    return false;
  }
  pushOutDir.SafeNormalize();

  float pushOutLength = 0.0f;
  for ( int32_t z = sphereMin.z; z < sphereMax.z; z++ )
  for ( int32_t y = sphereMin.y; y < sphereMax.y; y++ )
  for ( int32_t x = sphereMin.x; x < sphereMax.x; x++ )
  {
    const TerrainVertex* v = m_GetVertex( x, y, z );
    if ( !v )
    {
      continue;
    }

    aeFloat3 centerToVert = v->position - sphere.center;
    float c = centerToVert.Dot( centerToVert ) - sphere.radius * sphere.radius;
    if ( c > 0.0f )
    {
      // Vertex outside sphere
      continue;
    }

    aeFloat3 normal = v->normal.SafeNormalizeCopy(); // Opposite of actual ray direction
    float b = centerToVert.Dot( normal );
    aeFloat3 surfaceToVert = normal * ( b + sqrtf( b * b - c ) );
    float t2 = pushOutDir.Dot( surfaceToVert ); // Projected length of surfaceToVert on to normalized pushOut
    if ( debug )
    {
      debug->AddLine( v->position, v->position - surfaceToVert, aeColor::Green() );
      debug->AddLine( sphere.center, sphere.center + surfaceToVert, aeColor::Green() );
    }
    pushOutLength = aeMath::Max( pushOutLength, t2 );
  }

  if ( debug )
  {
    debug->AddLine( sphere.center, sphere.center + pushOutDir * pushOutLength, aeColor::Red() );
    debug->AddLine( sphere.center, sphere.center + pushOutDir, aeColor::Blue() );

    debug->AddSphere( sphere.center, sphere.radius, aeColor::Blue(), 16 );
    debug->AddSphere( sphere.center + pushOutDir * pushOutLength, sphere.radius, aeColor::Red(), 16 );
  }

  if ( offsetOut )
  {
    *offsetOut = pushOutDir * pushOutLength;
  }
  return true;
}
