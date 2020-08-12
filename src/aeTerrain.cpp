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
#include "aeCompactingAllocator.h"

// @TODO: SIMD GetIntersection() is currently causing nans on windows
#if _AE_APPLE_ //|| _AE_WINDOWS_
#define TERRAIN_SIMD 1
#include <intrin.h>
#endif

#if _AE_OSX_ && TERRAIN_SIMD
#include <x86intrin.h>
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

float aeTerrain::GetBaseHeight( aeFloat3 p ) const
{
  // aeFloat2 p2 = ToMapTexCoords( aeFloat2( p.x, p.y ) );
  // uint8_t val = m_mapWallTex->NearestClamp( p2 ).r;
  // float wall = val == 255 ? 1.0f : 0.0f;
//  wall = ( wall - 0.3f ) / 0.3f;
//  wall = wall > 1.0f ? 1.0f : wall;
//  wall = wall < 0.0f ? 0.0f : wall;
  // wall *= kMapWallHeight;
  float wall = 0.0f;
  
  float h = 0.0f;
//  float floor = m_mapWallTex->CosineClamp( p2 ).b / 256.0f;
//  floor *= kMapWallHeight * 1.5f;
  // floor += 32.0f * m_noise.Smoothed2D< Interpolation::Cosine >( p.x * 0.05f, p.y * 0.05f );
  // float floor = 0.0f;


  // float d = UMAT::Abs( 933.0f - p.x );
  // float c = (20.0f - d) / -20.0f;
  // h += d > 20.0f ? 8.0f + 8.0f * c : 0.0f;
  // h += 4.0f * m_noise.Smoothed2D< Interpolation::Cosine >( p.x * 0.1f, p.y * 0.1f );
  // if ( d < 5.0f ) { h = 0.0f; }
  
  return wall + h + 256.5f;
}


float Box( aeFloat3 p, aeFloat3 b )
{
  aeFloat3 d;
  d.x = aeMath::Abs( p.x ) - b.x;
  d.y = aeMath::Abs( p.y ) - b.y;
  d.z = aeMath::Abs( p.z ) - b.z;
  aeFloat3 d0;
  d0.x = aeMath::Max( d.x, 0.0f );
  d0.y = aeMath::Max( d.y, 0.0f );
  d0.z = aeMath::Max( d.z, 0.0f );
  float f = aeFloat3( d0 ).Length();
  return f + fmin( fmax( d.x, fmax( d.y, d.z ) ), 0.0f );
}

float Cylinder( aeFloat3 p, aeFloat2 h )
{
  aeFloat2 d;
  d.x = aeFloat2( p.x, p.y ).Length();
  d.y = p.z;
  d.x = fabs( d.x );
  d.y = fabs( d.y );
  d -= h;
  aeFloat2 d0( fmax( d.x, 0.0f ), fmax( d.y, 0.0f ) );
  return fmin( fmax( d.x,d.y ), 0.0f ) + d0.Length();
}

float Sphere( aeFloat3 center, float radius, aeFloat3 p )
{
  return ( p - center ).Length() - radius;
}

float Ground( float height, aeFloat3 p )
{
  return p.z - height;
}

float Union( float d1, float d2 )
{
  return aeMath::Min( d1, d2 );
}

float Subtraction( float d1, float d2 )
{
  return aeMath::Max( -d1, d2 );
}

float Intersection( float d1, float d2 )
{
  return aeMath::Max( d1, d2 );
}

float SmoothUnion( float d1, float d2, float k )
{
  float h = aeMath::Clip01( 0.5f + 0.5f * ( d2 - d1 ) / k );
  return aeMath::Lerp( d2, d1, h ) - k * h * ( 1.0f - h );
}

float aeTerrain::TerrainValue( aeFloat3 p ) const
{
  float result = Ground( 6, p );
  result = Subtraction( Sphere( aeFloat3( 5, 5, 5 ), 3.5f, p ), result );
  //result = aeMath::Max( result, Sphere( aeFloat3( 4.5f ), s_test, p ) );
  //result = aeMath::Max( result, Sphere( aeFloat3( 7.5f ), s_test, p ) );
  //result = aeMath::Max( result, Sphere( aeFloat3( 4.5f ), 4.0f, p ) );
  //result = aeMath::Max( result, Sphere( aeFloat3( 2.0f + s_test ), 1.5f, p ) );
  return result;

  //return Sphere( aeFloat3( 4.0f ), 2.0f, p );
}

aeFloat3 aeTerrain::GetSurfaceDerivative( aeFloat3 p ) const
{
  aeFloat3 normal;
  for ( int32_t i = 0; i < 3; i++ )
  {
    aeFloat3 nt = p;
    nt[ i ] += 0.001f;
    normal[ i ] = TerrainValue( nt );
  }

  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal -= aeFloat3( TerrainValue( p ) );
  normal.SafeNormalize();
  AE_ASSERT( normal != aeFloat3( 0.0f ) );

  return normal;
}

int32_t aeTerrain::TerrainType( aeFloat3 p ) const
{
  // float cx = p.x - 900.0f;
  // float cy = p.y - 829.0f;
  // float d = cx * cx + cy * cy;
  // if ( d < 7 * 7 ) { return 0; }

  // float d2 = UMAT::Abs( 933.0f - p.x );
  // if ( d2 < 5.0f ) { return 0; }

  return 255;
}

void aeTerrain::GetChunkVerts( Chunk* chunk, TerrainVertex *vertexOut, TerrainIndex *indexOut, uint32_t* vertexCount, uint32_t* indexCount )
{
  AE_ASSERT( chunk );
  aeArray< TerrainVertex > vertices;
  aeArray< TerrainIndex > indices;
  
  int32_t chunkOffsetX = chunk->pos[ 0 ] * kChunkSize;
  int32_t chunkOffsetY = chunk->pos[ 1 ] * kChunkSize;
  int32_t chunkOffsetZ = chunk->pos[ 2 ] * kChunkSize;
  
  struct TempEdges
  {
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t b;

    // 3 planes which whose intersections are used to position vertices within voxel
    // EDGE_TOP_FRONT_BIT
    // EDGE_TOP_RIGHT_BIT
    // EDGE_SIDE_FRONTRIGHT_BIT
    aeFloat3 p[ 3 ];
    aeFloat3 n[ 3 ];
  };

  const int32_t tempChunkSize = kChunkSize + 2;
  aeAlloc::Scratch< TempEdges > edgeInfo( tempChunkSize * tempChunkSize * tempChunkSize );
  memset( edgeInfo.Data(), 0, edgeInfo.Length() * sizeof(TempEdges) );
  
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
      float gx = chunkOffsetX + x + cornerOffsets[ i ][ j ].x;
      float gy = chunkOffsetY + y + cornerOffsets[ i ][ j ].y;
      float gz = chunkOffsetZ + z + cornerOffsets[ i ][ j ].z;
      // @TODO: Should pre-calculate, or at least only look up corner (1,1,1) once
      cornerValues[ i ][ j ] = TerrainValue( aeFloat3( gx, gy, gz) );
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
        if ( chunk->i[ x ][ y ][ z ] != (TerrainIndex)~0 ) { continue; }
        
        aeFloat3 g;
        g.x = chunkOffsetX + x + 0.5f;
        g.y = chunkOffsetY + y + 0.5f;
        g.z = chunkOffsetZ + z + 0.5f;
        chunk->t[ x ][ y ][ z ] = ( TerrainValue( g ) > 0.0f ) ? Block::Exterior : Block::Interior;
      }
      continue;
    }
    
    TempEdges* te = &edgeInfo[ x + 1 + tempChunkSize * ( y + 1 + ( z + 1 ) * tempChunkSize ) ];
    te->b = edgeBits;
    te->x = x;
    te->y = y;
    te->z = z;
    
    // Iterate over voxel edges (only 3 for TempEdges)
    for ( int32_t e = 0; e < 3; e++ )
    if ( edgeBits & mask[ e ] )
    {
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
          
          float v = TerrainValue( cw );
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
      
      AE_ASSERT( edgePos.x == edgePos.x && edgePos.y == edgePos.y && edgePos.z == edgePos.z );
      te->p[ e ] = edgePos;
      te->n[ e ] = edgeNormal;
      
      AE_ASSERT( edgeVoxelPos.x >= 0.0f && edgeVoxelPos.x <= 1.0f );
      AE_ASSERT( edgeVoxelPos.y >= 0.0f && edgeVoxelPos.y <= 1.0f );
      AE_ASSERT( edgeVoxelPos.z >= 0.0f && edgeVoxelPos.z <= 1.0f );
      
      if ( x < 0 || y < 0 || z < 0 || x >= kChunkSize || y >= kChunkSize || z >= kChunkSize )
      {
        continue;
      }
      
      TerrainIndex ind[ 4 ];
      int32_t offsets[ 4 ][ 3 ];
      GetOffsetsFromEdge( mask[ e ], offsets );
      
      // @NOTE: Add new vertices for each edge intersection (centered in voxels for now).
      // Edges are eventually expanded into quads, so each edge needs 4 vertices.
      // This does some of the work for adjacent voxels.
      for ( int32_t j = 0; j < 4; j++ )
      {
        int32_t ox = x + offsets[ j ][ 0 ];
        int32_t oy = y + offsets[ j ][ 1 ];
        int32_t oz = z + offsets[ j ][ 2 ];
        
        // This check allows coords to be one out of chunk high end
        if ( ox < 0 || oy < 0 || oz < 0 || ox > kChunkSize || oy > kChunkSize || oz > kChunkSize ) { continue; }
        
        bool inCurrentChunk = ox < kChunkSize && oy < kChunkSize && oz < kChunkSize;
        if ( !inCurrentChunk || chunk->i[ ox ][ oy ][ oz ] == (TerrainIndex)~0 )
        {
          TerrainVertex vertex;
          vertex.position.x = ox + 0.5f;
          vertex.position.y = oy + 0.5f;
          vertex.position.z = oz + 0.5f;
          
          AE_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );
          
          TerrainIndex index = (TerrainIndex)vertices.Length();
          vertices.Append( vertex );
          ind[ j ] = index;
          
          if ( inCurrentChunk )
          {
            chunk->i[ ox ][ oy ][ oz ] = index;
            chunk->t[ ox ][ oy ][ oz ] = Block::Surface;
          }
        }
        else
        {
          TerrainIndex index = chunk->i[ ox ][ oy ][ oz ];
          AE_ASSERT( index < vertices.Length() );
          AE_ASSERT( chunk->t[ ox ][ oy ][ oz ] == Block::Surface );
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
        indices.Append( ind[ 0 ] );
        indices.Append( ind[ 1 ] );
        indices.Append( ind[ 2 ] );
        // tri1
        indices.Append( ind[ 1 ] );
        indices.Append( ind[ 3 ] );
        indices.Append( ind[ 2 ] );
      }
      else
      {
        // tri2
        indices.Append( ind[ 0 ] );
        indices.Append( ind[ 2 ] );
        indices.Append( ind[ 1 ] );
        //tri3
        indices.Append( ind[ 1 ] );
        indices.Append( ind[ 2 ] );
        indices.Append( ind[ 3 ] );
      }
    }
  }
  
  if ( indices.Length() == 0 )
  {
    // @TODO: Should differentiate between empty chunk and full chunk. It's possible though that
    // Chunk::t's are good enough for this though.
    *vertexCount = 0;
    *indexCount = 0;
    return;
  }
  
  const int32_t vc = (int32_t)vertices.Length();
  for ( int32_t i = 0; i < vc; i++ )
  {
    TerrainVertex* vertex = &vertices[ i ];
    int32_t x = aeMath::Floor( vertex->position.x );
    int32_t y = aeMath::Floor( vertex->position.y );
    int32_t z = aeMath::Floor( vertex->position.z );
    
    int32_t ec = 0;
    aeFloat3 p[ 12 ];
    aeFloat3 n[ 12 ];
    
    if ( x < 0 || y < 0 || z < 0 ) { AE_FAIL(); }
    if ( x > kChunkSize || y > kChunkSize || z > kChunkSize ) { AE_FAIL(); }
    
    TempEdges te = edgeInfo[ x + 1 + tempChunkSize * ( y + 1 + ( z + 1 ) * tempChunkSize ) ];
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
    te = edgeInfo[ x + tempChunkSize * ( y + 1 + ( z + 1 ) * tempChunkSize ) ];
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
    te = edgeInfo[ x + 1 + tempChunkSize * ( y + ( z + 1 ) * tempChunkSize ) ];
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
      ec++; }
    te = edgeInfo[ x + tempChunkSize * ( y + ( z + 1 ) * tempChunkSize ) ];
    if ( te.b & EDGE_SIDE_FRONTRIGHT_BIT )
    {
      p[ ec ] = te.p[ 2 ];
      p[ ec ].x -= 1.0f;
      p[ ec ].y -= 1.0f;
      n[ ec ] = te.n[ 2 ];
      ec++;
    }
    te = edgeInfo[ x + tempChunkSize * ( y + 1 + z * tempChunkSize ) ];
    if ( te.b & EDGE_TOP_RIGHT_BIT )
    {
      p[ ec ] = te.p[ 1 ];
      p[ ec ].x -= 1.0f;
      p[ ec ].z -= 1.0f;
      n[ ec ] = te.n[ 1 ];
      ec++;
    }
    te = edgeInfo[ x + 1 + tempChunkSize * ( y + z * tempChunkSize ) ];
    if ( te.b & EDGE_TOP_FRONT_BIT )
    {
      p[ ec ] = te.p[ 0 ];
      p[ ec ].y -= 1.0f;
      p[ ec ].z -= 1.0f;
      n[ ec ] = te.n[ 0 ];
      ec++;
    }
    te = edgeInfo[ x + 1 + tempChunkSize * ( y + 1 + z * tempChunkSize ) ];
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
      AE_ASSERT( p[ j ].x == p[ j ].x && p[ j ].y == p[ j ].y && p[ j ].z == p[ j ].z );
      AE_ASSERT( p[ j ].x >= 0.0f && p[ j ].x <= 1.0f );
      AE_ASSERT( p[ j ].y >= 0.0f && p[ j ].y <= 1.0f );
      AE_ASSERT( p[ j ].z >= 0.0f && p[ j ].z <= 1.0f );
    }
    aeFloat3 position = GetIntersection( p, n, ec );
    AE_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
    position.x = chunkOffsetX + x + aeMath::Clip( position.x, 0.0f, 0.999f );
    position.y = chunkOffsetY + y + aeMath::Clip( position.y, 0.0f, 0.999f );
    position.z = chunkOffsetZ + z + aeMath::Clip( position.z, 0.0f, 0.999f );
    
    vertex->position = position;
    vertex->normal = GetSurfaceDerivative( position );
    vertex->info[ 0 ] = 0;
    vertex->info[ 1 ] = (uint8_t)( 1.5f ); // @HACK: Lighting values
    vertex->info[ 2 ] = TerrainType( position );
    vertex->info[ 3 ] = 0;
  }
  
//  TODO
//  for( int32_t z = 0; z < kChunkSize; z++ )
//  for( int32_t y = 0; y < kChunkSize; y++ )
//  for( int32_t x = 0; x < kChunkSize; x++ )
//  {
//    TerrainIndex i = chunk->i[ x ][ y ][ z ];
//    if ( i == (TerrainIndex)~0 )
//    {
//      AE_ASSERT( chunk->t[ x ][ y ][ z ] != Block::Surface );
//    }
//    else
//    {
//      int32_t wx = x + chunkOffsetX;
//      int32_t wy = y + chunkOffsetY;
//      int32_t wz = z + chunkOffsetZ;
//      aeFloat3 pos = vertices[ i ].position;
//      AE_ASSERT( wx <= pos.x && pos.x < wx + 1 );
//      AE_ASSERT( wy <= pos.y && pos.y < wy + 1 );
//      AE_ASSERT( wz <= pos.z && pos.z < wz + 1 );
//      uint8_t t = chunk->t[ x ][ y ][ z ];
//      AE_ASSERT( t == Block::Surface );
//    }
//  }
  AE_ASSERT( vertices.Length() <= kMaxChunkVerts );
  AE_ASSERT( indices.Length() <= kMaxChunkIndices );
  
  *vertexCount = (uint32_t)vertices.Length();
  *indexCount = (uint32_t)indices.Length();
//  ScratchBuffer<int8_t> vertexTest( *vertexCount );
//  memset( vertexTest.Get(), 0, vertexTest.GetBytes() );
//  for ( int32_t i = 0; i < *indexCount; i++ )
//  {
//    vertexTest[ indices[ i ] ] = 1;
//  }
//  for ( int32_t i = 0; i < *vertexCount; i++ )
//  {
//    AE_ASSERT( vertexTest[ i ] == 1 );
//  }
  memcpy( vertexOut, &vertices[ 0 ], *vertexCount * sizeof(vertices[ 0 ]) );
  memcpy( indexOut, &indices[ 0 ], *indexCount * sizeof(indices[ 0 ]) );
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
    chunk->l[ x ][ y ][ z ] = kSkyBrightness * mod * 0.85f;
  }
  
  chunk->lightDirty = false;
}

Chunk* aeTerrain::AllocChunk( aeFloat3 center, int32_t cx, int32_t cy, int32_t cz )
{
  if ( m_totalChunks == kMaxLoadedChunks )
  {
    Chunk* farChunk = nullptr;
    float maxDist = 0.0f;
    
    Chunk* current = m_headChunk;
    while( current )
    {
      int32_t cx = current->pos[ 0 ];
      int32_t cy = current->pos[ 1 ];
      int32_t cz = current->pos[ 2 ];
      float d = ( center - aeFloat3( cx + 0.5f, cy + 0.5f, cz + 0.5f ) * kChunkSize ).LengthSquared();
      
      if ( maxDist < d )
      {
        maxDist = d;
        farChunk = current;
      }
      
      current = current->next;
    }
    
    AE_ASSERT( farChunk );
    AE_ASSERT( !farChunk->active );
    FreeChunk( farChunk );
  }
  
  uint32_t chunkIndex = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
  AE_ASSERT( m_chunks[ chunkIndex ] == nullptr );
  
  Chunk* chunk = m_chunkPool.Allocate();
  AE_ASSERT( chunk );
  memset( chunk, 0, sizeof( Chunk ) );
  memset( chunk->i, ~(uint8_t)0, sizeof(chunk->i) );
  
  memset( &chunk->check, 0xCD, sizeof(chunk->check) );
  AE_ASSERT( chunk->check == 0xCDCDCDCD );
  chunk->pos[ 0 ] = cx;
  chunk->pos[ 1 ] = cy;
  chunk->pos[ 2 ] = cz;
  chunk->lightDirty = true;
  
  if ( m_headChunk == nullptr ) { m_headChunk = chunk; }
  if ( m_tailChunk != nullptr ) { m_tailChunk->next = chunk; }
  chunk->prev = m_tailChunk;
  m_tailChunk = chunk;
  m_tailChunk->next = nullptr;
  
  m_chunks[ chunkIndex ] = chunk;
  m_totalChunks++;
  
  return chunk;
}

void aeTerrain::FreeChunk( Chunk* chunk )
{
  AE_ASSERT( chunk );
  AE_ASSERT( !chunk->active );
  
  int32_t cx = chunk->pos[ 0 ];
  int32_t cy = chunk->pos[ 1 ];
  int32_t cz = chunk->pos[ 2 ];
  uint32_t c = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
  m_chunks[ c ] = nullptr;
  m_totalChunks--;
  
  Chunk* prev = chunk->prev;
  Chunk* next = chunk->next;
  if ( prev ) { prev->next = next; }
  else { m_headChunk = next; }
  if ( next) { next->prev = prev; }
  else { m_tailChunk = prev; }
  
  m_chunkPool.Free( chunk );
}

void aeTerrain::GetOffsetsFromEdge( uint32_t edgeBit, int32_t (&offsets)[ 4 ][ 3 ] )
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

Chunk* aeTerrain::GetChunk( int32_t cx, int32_t cy, int32_t cz )
{
  if ( cx >= kWorldChunksWidth || cy >= kWorldChunksWidth || cz >= kWorldChunksHeight )
  {
    AE_FAIL_MSG( "Failed to GetChunk at: # # #", cx, cy, cz );
  }
  
  uint32_t c = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
  
  Chunk* chunk = m_chunks[ c ];
  if ( chunk ) { AE_ASSERT( chunk->check == 0xCDCDCDCD ); }
  return chunk;
}

void aeTerrain::Initialize()
{
  m_compactAlloc.Expand( 84 * ( 1 << 20 ) );

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
  
  memset( m_activeChunks, 0, sizeof(m_activeChunks) );
  m_headChunk = nullptr;
  m_tailChunk = nullptr;
  m_totalChunks = 0;
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockCollision[ i ] = true; }
  m_blockCollision[ Block::Exterior ] = false;
  m_blockCollision[ Block::Unloaded ] = false;
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockDensity[ i ] = 1.0f; }
  
  //m_noise.Initialize( 0 ); // @TODO: Add noise module!
}

void aeTerrain::Update()
{
  //s_test += 0.016666f * 0.1f;
  //if ( s_test > 2.0f )
  //{
  //  s_test -= 2.0f;
  //}
  //AE_LOG( "s_test #", s_test );

  // @TODO: Provide better interface for debug info
  //char str[ 128 ];
  //sprintf( str, "Loaded Chunks: # / #", m_totalChunks, kMaxLoadedChunks );
  //program->game->hudBoss->AddDebugString( 330, 155, 0.5f, aeFloat3(0.0f), str );
  //sprintf( str, "Active Chunks: # / #", m_activeChunkCount, kMaxActiveChunks );
  //program->game->hudBoss->AddDebugString( 330, 135, 0.5f, aeFloat3(0.0f), str );
}

void aeTerrain::Render( aeFloat3 center, const aeShader* shader, const aeUniformList& shaderParams )
{
  const int32_t viewRadius = 5;
  const int32_t worldViewRadius2 = viewRadius * viewRadius * kChunkSize * kChunkSize;
  const int32_t viewDiam = viewRadius + viewRadius;
  
  int32_t ci = int32_t( center.x ) / kChunkSize;
  int32_t cj = int32_t( center.y ) / kChunkSize;
  int32_t ck = int32_t( center.z ) / kChunkSize;
  
  struct ChunkSort
  {
    Chunk *c;
    uint32_t pos[ 3 ];
    float centerDistance;
  };
  
  for( uint32_t i = 0; i < m_activeChunkCount; i++ )
  {
    m_activeChunks[ i ]->active = false;
  }
  m_activeChunkCount = 0;
  
  //------------------------------------------------------------------------------
  // Manage chunks based on new 'center' value
  //------------------------------------------------------------------------------
  aeAlloc::Scratch< ChunkSort > chunkSort( viewDiam * viewDiam * viewDiam );
  uint32_t sortCount = 0;
  for( uint32_t cz = aeMath::Max( 0, ck - viewRadius ); cz < aeMath::Min( int32_t(kWorldChunksHeight), ck + viewRadius ); cz++ )
  for( uint32_t cy = aeMath::Max( 0, cj - viewRadius ); cy < aeMath::Min( int32_t(kWorldChunksWidth), cj + viewRadius ); cy++ )
  for( uint32_t cx = aeMath::Max( 0, ci - viewRadius ); cx < aeMath::Min( int32_t(kWorldChunksWidth), ci + viewRadius ); cx++ )
  {
    int32_t ci = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
    int16_t vc = m_voxelCounts[ ci ];
    if ( vc == 0 || vc == kChunkCountMax ) { continue; }
    
    float centerDistance = ( center - aeFloat3( cx + 0.5f, cy + 0.5f, cz + 0.5f ) * kChunkSize ).LengthSquared();
    if ( centerDistance >= worldViewRadius2 ) { continue; }
    
    Chunk* c = m_chunks[ ci ];
    //if ( c ) { c->geoDirty = true; } // @HACK
    if( !c || c->geoDirty )
    {
      if ( c )
      {
        // @HACK: Should not need to free chunk, it should be possible to update existing
        FreeChunk( c );
        c = nullptr;
      }

      // Allocate a new chunk when needed
      if ( !c )
      {
        c = AllocChunk( center, cx, cy, cz );
      }
      
      // Generate vertex positions from current chunk
      uint32_t vertexCount, indexCount;
      aeAlloc::Scratch< TerrainVertex > vertexScratch( kMaxChunkVerts );
      aeAlloc::Scratch< TerrainIndex > indexScratch( kMaxChunkIndices );
      GetChunkVerts( c, vertexScratch.Data(), indexScratch.Data(), &vertexCount, &indexCount );
      m_voxelCounts[ ci ] = vertexCount;
      AE_ASSERT( vertexCount <= kChunkCountMax );
      if ( vertexCount == 0 || vertexCount == kChunkCountMax )
      {
        // @TODO: It's super expensive to finally just throw away the unneeded chunk...
        FreeChunk( c );
        continue;
      }

      // Initialize aeVertexData here only once
      if ( c->data.GetIndexCount() == 0
        || c->data.GetMaxVertexCount() < vertexCount
        || c->data.GetMaxIndexCount() < indexCount )
      {
        c->data.Initialize( sizeof( TerrainVertex ), sizeof( TerrainIndex ), vertexCount, indexCount, aeVertexPrimitive::Triangle, aeVertexUsage::Dynamic, aeVertexUsage::Dynamic );
        c->data.AddAttribute( "a_position", 3, aeVertexDataType::Float, offsetof( TerrainVertex, position ) );
        c->data.AddAttribute( "a_normal", 3, aeVertexDataType::Float, offsetof( TerrainVertex, normal ) );
        c->data.AddAttribute( "a_info", 4, aeVertexDataType::UInt8, offsetof( TerrainVertex, info ) );
      }
      
      // Set vertices
      c->data.SetVertices( vertexScratch.Data(), vertexCount );
      c->data.SetIndices( indexScratch.Data(), indexCount );
      
      uint32_t vertexBytes = vertexCount * sizeof(TerrainVertex);
      m_compactAlloc.Allocate( &c->vertices, vertexBytes );
      memcpy( c->vertices, vertexScratch.Data(), vertexBytes );
      
      // Dirty flags
      c->geoDirty = false;
      c->lightDirty = true;
    }
    
    chunkSort[ sortCount ].c = c;
    chunkSort[ sortCount ].pos[ 0 ] = cx;
    chunkSort[ sortCount ].pos[ 1 ] = cy;
    chunkSort[ sortCount ].pos[ 2 ] = cz;
    chunkSort[ sortCount ].centerDistance = centerDistance;
    sortCount++;
  }

  //------------------------------------------------------------------------------
  // Sort existing chunks for priority
  //------------------------------------------------------------------------------
  // Sort chunks by distance from center, closest to farthest
  std::sort( chunkSort.Data(), (chunkSort.Data() + sortCount),
    []( const ChunkSort &a, const ChunkSort &b ) -> bool
  {
      return a.centerDistance < b.centerDistance;
  });
  // Reactivate the closest chunks
  for ( int32_t i = 0; i < sortCount && m_activeChunkCount < kMaxActiveChunks; i++ )
  {
    Chunk* c = chunkSort[ i ].c;
    AE_ASSERT( c );
    m_activeChunks[ m_activeChunkCount++ ] = c;
    c->active = true;
  }

  //------------------------------------------------------------------------------
  // Render
  //------------------------------------------------------------------------------
  //aeFrustum frustum;
  for( uint32_t i = 0; i < m_activeChunkCount; i++ )
  {
    Chunk* chunk = m_activeChunks[ i ];
    AE_ASSERT( chunk->check == 0xCDCDCDCD );
    AE_ASSERT( chunk->active );
    // Only render the visible chunks
    //if( frustum.TestChunk( chunk ) ) // @TODO: Should make sure chunk is visible
    {
      //params.Set( "rockTex", rockTexture );
      //params.Set( "dirtTex", dirtTexture );
      //params.Set( "spiralTex", spiralTexture );
      //params.Set( "treeTex", treeTexture );
      chunk->data.Render( shader, shaderParams );
    }
    AE_ASSERT( chunk->active );
  }
}

/*
bool aeTerrain::GetCollision( uint32_t x, uint32_t y, uint32_t z )
{
  return m_blockCollision[ GetVoxel( x, y, z ) ];
}

bool aeTerrain::GetCollision( aeFloat3 position )
{
  return m_blockCollision[ GetVoxel( position.x, position.y, position.z ) ];
}

uint8_t aeTerrain::GetVoxel( aeFloat3 position )
{
  return GetVoxel( position.x, position.y, position.z );
}

uint8_t aeTerrain::GetVoxel( uint32_t x, uint32_t y, uint32_t z )
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
  return chunk->t[ x % kChunkSize ][ y % kChunkSize ][ z % kChunkSize ];
}

float16_t aeTerrain::GetLight( uint32_t x, uint32_t y, uint32_t z )
{
  uint32_t cix = x / kChunkSize;
  uint32_t ciy = y / kChunkSize;
  uint32_t ciz = z / kChunkSize;
  
  if( cix >= kWorldChunksWidth || ciy >= kWorldChunksWidth || ciz >= kWorldChunksHeight ) { return kSkyBrightness; }
  Chunk *chunk = GetChunk( cix, ciy, ciz );
  if( chunk == nullptr ) { return kSkyBrightness; }
  
  return chunk->l[ x - cix * kChunkSize ][ y - ciy * kChunkSize ][ z - ciz * kChunkSize ];
}

bool aeTerrain::VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps )
{
  int32_t x = UMAT::Floor( start.x );
  int32_t y = UMAT::Floor( start.y );
  int32_t z = UMAT::Floor( start.z );
  
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
    outX = UMAT::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX );
		cb.x = x + 1;
	}
	else 
	{
		stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = UMAT::Max( -1, outX );
		cb.x = x;
	}
	if (dir.y > 0.0f)
	{
		stepY = 1;
    outY = ceil( start.y + ray.y );
    outY = UMAT::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY );
		cb.y = y + 1;
	}
	else 
	{
		stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = UMAT::Max( -1, outY );
		cb.y = y;
	}
	if (dir.z > 0.0f)
	{
		stepZ = 1;
    outZ = ceil( start.z + ray.z );
    outZ = UMAT::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ );
		cb.z = z + 1;
	}
	else 
	{
		stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    outZ = UMAT::Max( -1, outZ );
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

RaycastResult aeTerrain::RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision )
{
  RaycastResult result;
  result.hit = false;
  result.type = ~0;
  result.distance = std::numeric_limits<float>::infinity();
  result.posi[ 0 ] = ~0;
  result.posi[ 1 ] = ~0;
  result.posi[ 2 ] = ~0;
  result.posf = aeFloat3( std::numeric_limits<float>::infinity() );
  result.normal = aeFloat3( std::numeric_limits<float>::infinity() );
  result.touchedUnloaded = false;
  
  int32_t x = UMAT::Floor( start.x );
  int32_t y = UMAT::Floor( start.y );
  int32_t z = UMAT::Floor( start.z );
  
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
    outX = UMAT::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX );
		cb.x = x + 1;
	}
	else 
	{
		stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = UMAT::Max( -1, outX );
		cb.x = x;
	}
	if (dir.y > 0.0f)
	{
		stepY = 1;
    outY = ceil( start.y + ray.y );
    outY = UMAT::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY );
		cb.y = y + 1;
	}
	else 
	{
		stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = UMAT::Max( -1, outY );
		cb.y = y;
	}
	if (dir.z > 0.0f)
	{
		stepZ = 1;
    outZ = ceil( start.z + ray.z );
    outZ = UMAT::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ );
		cb.z = z + 1;
	}
	else 
	{
		stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    outZ = UMAT::Max( -1, outZ );
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
  Chunk* c = GetChunk( cx, cy, cz );
  AE_ASSERT( c );
  int32_t lx = x % kChunkSize;
  int32_t ly = y % kChunkSize;
  int32_t lz = z % kChunkSize;
  TerrainIndex index = c->i[ lx ][ ly ][ lz ];
  // TODO Can somehow skip surface and hit interior cell
  AE_ASSERT( index != (TerrainIndex)~0 );
  aeFloat3 p = c->vertices[ index ].position;
  aeFloat3 n = c->vertices[ index ].normal.SafeNormalizeCopy();
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

RaycastResult aeTerrain::Raycast( aeFloat3 start, aeFloat3 ray )
{
  RaycastResult result;
  result.hit = false;
  result.type = ~0;
  result.distance = std::numeric_limits<float>::infinity();
  result.posi[ 0 ] = ~0;
  result.posi[ 1 ] = ~0;
  result.posi[ 2 ] = ~0;
  result.posf = aeFloat3( std::numeric_limits<float>::infinity() );
  result.normal = aeFloat3( std::numeric_limits<float>::infinity() );
  result.touchedUnloaded = false;
  
  int32_t x = UMAT::Floor( start.x );
  int32_t y = UMAT::Floor( start.y );
  int32_t z = UMAT::Floor( start.z );
  
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
    outX = UMAT::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX );
		cb.x = x + 1;
	}
	else 
	{
		stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = UMAT::Max( -1, outX );
		cb.x = x;
	}
	if (dir.y > 0.0f)
	{
		stepY = 1;
    outY = ceil( start.y + ray.y );
    outY = UMAT::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY );
		cb.y = y + 1;
	}
	else 
	{
		stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = UMAT::Max( -1, outY );
		cb.y = y;
	}
	if (dir.z > 0.0f)
	{
		stepZ = 1;
    outZ = ceil( start.z + ray.z );
    outZ = UMAT::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ );
		cb.z = z + 1;
	}
	else 
	{
		stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    outZ = UMAT::Max( -1, outZ );
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
      float fv0 = TerrainValue( iv0 );
      float fv1 = TerrainValue( iv1 );
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
          fp = TerrainValue( p );
          if ( fp < 0.0f ) { iv0 = p; }
          else { iv1 = p; }
        }
        
        aeFloat3 n;
        for ( int32_t i = 0; i < 3; i++ )
        {
          aeFloat3 nt = p;
          nt[ i ] += 0.0001f;
          n[ i ] = TerrainValue( nt );
        }
        n -= aeFloat3( fp );
        n.SafeNormalize();
        
        result.distance = ( p - start ).Length();
        result.posf = p;
        result.normal = n;
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
*/
