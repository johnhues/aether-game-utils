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
/*
#include <limits>
#include <errno.h>
#include <algorithm>
#include <vector>

#include "ProgramInterface.h"
#include "Meta.h"
#include "Allocator.h"
#include "TerrainBoss.h"
#include "Shader.h"
#include "Game.h"
#include "RenderDirector.h"
#include "Avatar.h"
#include "Frustum.h"
#include "ShaderBoss.h"
#include "TextureBoss.h"
#include "CompactingAllocator.h"
#include "HudBoss.h"
#include "MapBoss.h"
#include "Interpolation.h"
*/

#include "aeTerrain.h"
#include "aeCompactingAllocator.h"
//#include "aePlatform.h"

#if _AE_APPLE_ //|| _AE_WINDOWS_
#define TERRAIN_SIMD 1
#include <x86intrin.h>
#endif

#if _AE_OSX_ && TERRAIN_SIMD
#include <x86intrin.h>
#endif

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
  aeFloat3 c;
  for ( uint32_t i = 0; i < ic; i++ ) { c += p[ i ]; }
  c /= ic;
  for ( uint32_t i = 0; i < 10; i++ )
  for ( uint32_t j = 0; j < ic; j++ )
  {
    float d = n[ j ].Dot( p[ j ] - c );
    c += n[ j ] * ( d * 0.5f );
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

float TerrainBoss::GetBaseHeight( aeFloat3 p ) const
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

float Sphere( aeFloat3 p, float s )
{
  return p.Length() - s;
}


float TerrainBoss::TerrainValue( aeFloat3 p ) const
{
  // float result = GetBaseHeight( p );
  // return p.z - result;

  aeFloat3 o( 933.0f, 846.0f, 258.0f );
  float d = ( p - o ).Length();
  return d - 15.0f;
}

int32_t TerrainBoss::TerrainType( aeFloat3 p ) const
{
  // float cx = p.x - 900.0f;
  // float cy = p.y - 829.0f;
  // float d = cx * cx + cy * cy;
  // if ( d < 7 * 7 ) { return 0; }

  // float d2 = UMAT::Abs( 933.0f - p.x );
  // if ( d2 < 5.0f ) { return 0; }

  return 255;
}

/*
void TerrainBoss::GetChunkVerts( Chunk* chunk, TerrainVertex *vertexOut, TerrainIndex *indexOut, uint32_t* vertexCount, uint32_t* indexCount )
{
  AE_ASSERT( chunk );
  std::vector< TerrainVertex > vertices;
  std::vector< TerrainIndex > indices;
  
  int32_t chunkOffsetX = chunk->pos[ 0 ] * kChunkSize;
  int32_t chunkOffsetY = chunk->pos[ 1 ] * kChunkSize;
  int32_t chunkOffsetZ = chunk->pos[ 2 ] * kChunkSize;
  
  struct TempEdges
  {
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t b;
    aeFloat3 p[ 3 ];
    aeFloat3 n[ 3 ];
  };
  const int32_t tempChunkSize = kChunkSize + 2;
  ScratchBuffer<TempEdges> edgeInfo( tempChunkSize * tempChunkSize * tempChunkSize );
  memset( edgeInfo.Get(), 0, edgeInfo.GetBytes() );
  
  uint16_t mask[ 3 ];
  mask[ 0 ] = EDGE_TOP_FRONT_BIT;
  mask[ 1 ] = EDGE_TOP_RIGHT_BIT;
  mask[ 2 ] = EDGE_SIDE_FRONTRIGHT_BIT;
  
  aeFloat3 corner[ 3 ][ 2 ];
  corner[ 0 ][ 0 ] = aeFloat3( 0, 1, 1 );
  corner[ 0 ][ 1 ] = aeFloat3( 1, 1, 1 );
  corner[ 1 ][ 0 ] = aeFloat3( 1, 0, 1 );
  corner[ 1 ][ 1 ] = aeFloat3( 1, 1, 1 );
  corner[ 2 ][ 0 ] = aeFloat3( 1, 1, 0 );
  corner[ 2 ][ 1 ] = aeFloat3( 1, 1, 1 );
  
  const int32_t chunkPlus = kChunkSize + 1;
  for( int32_t z = -1; z < chunkPlus; z++ )
  for( int32_t y = -1; y < chunkPlus; y++ )
  for( int32_t x = -1; x < chunkPlus; x++ )
  {
    float grid[ 3 ][ 2 ];
    for ( int32_t i = 0; i < 3; i++ )
    for ( int32_t j = 0; j < 2; j++ )
    {
      float gx = chunkOffsetX + x + corner[ i ][ j ].x;
      float gy = chunkOffsetY + y + corner[ i ][ j ].y;
      float gz = chunkOffsetZ + z + corner[ i ][ j ].z;
      // TODO Should be able to precalculate first
      grid[ i ][ j ] = TerrainValue( aeFloat3( gx, gy, gz) );
    }
    
    uint16_t edgeBits = 0;
    if ( grid[ 0 ][ 0 ] * grid[ 0 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_TOP_FRONT_BIT; }
    if ( grid[ 1 ][ 0 ] * grid[ 1 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_TOP_RIGHT_BIT; }
    if ( grid[ 2 ][ 0 ] * grid[ 2 ][ 1 ] <= 0.0f ) { edgeBits |= EDGE_SIDE_FRONTRIGHT_BIT; }
    if ( edgeBits == 0 )
    {
      if ( x >= 0 && y >= 0 && z >= 0 && x < kChunkSize && y < kChunkSize && z < kChunkSize )
      {
        if ( chunk->i[ x ][ y ][ z ] != (TerrainIndex)~0 ) { continue; }
        
        aeFloat3 g;
        g.x = chunkOffsetX + x + 0.5f;
        g.y = chunkOffsetY + y + 0.5f;
        g.z = chunkOffsetZ + z + 0.5f;
        if ( TerrainValue( g ) > 0.0f )
        {
          aeFloat2 p2 = ToMapTexCoords( aeFloat2( g.x, g.y ) );
          uint8_t val = m_mapWallTex->NearestClamp( p2 ).r;
          chunk->t[ x ][ y ][ z ] = ( val == 0 ) ? Block::Exterior : Block::Blocking;
        }
        else { chunk->t[ x ][ y ][ z ] = Block::Interior; }
      }
      continue;
    }
    
    TempEdges* te = &edgeInfo[ x + 1 + tempChunkSize * ( y + 1 + ( z + 1 ) * tempChunkSize ) ];
    te->b = edgeBits;
    te->x = x;
    te->y = y;
    te->z = z;
    
    for ( int32_t e = 0; e < 3; e++ )
    if ( edgeBits & mask[ e ] )
    {
      aeFloat3 edgePos;
      {
        aeFloat3 c0;
        aeFloat3 c1;
        if ( grid[ e ][ 0 ] < grid[ e ][ 1 ] )
        {
          c0 = corner[ e ][ 0 ];
          c1 = corner[ e ][ 1 ];
        }
        else
        {
          c1 = corner[ e ][ 0 ];
          c0 = corner[ e ][ 1 ];
        }
        aeFloat3 ch( chunkOffsetX + x, chunkOffsetY + y, chunkOffsetZ + z );
        for ( int32_t i = 0; i < 7; i++ )
        {
          edgePos = ( c0 + c1 ) * 0.5f;
          aeFloat3 cw = ch + edgePos;
          float v = TerrainValue( cw );
          if ( v < 0.0f ) { c0 = edgePos; }
          if ( v > 0.0f ) { c1 = edgePos; }
        }
      }
      
      aeFloat3 edgeNormal;
      {
        for ( int32_t i = 0; i < 3; i++ )
        {
          aeFloat3 nt;
          nt.x = chunkOffsetX + x;
          nt.y = chunkOffsetY + y;
          nt.z = chunkOffsetZ + z;
          nt[ i ] += 0.0001f;
          edgeNormal[ i ] = TerrainValue( nt );
        }
        aeFloat3 ep( chunkOffsetX + x, chunkOffsetY + y, chunkOffsetZ + z );
        ep += edgePos;
        edgeNormal -= TerrainValue( ep );
        edgeNormal.SafeNormalize();
      }
      
      AE_ASSERT( edgePos.x == edgePos.x && edgePos.y == edgePos.y && edgePos.z == edgePos.z );
      te->p[ e ] = edgePos;
      te->n[ e ] = edgeNormal;
      
      AE_ASSERT( edgePos.x >= 0.0f && edgePos.x <= 1.0f );
      AE_ASSERT( edgePos.y >= 0.0f && edgePos.y <= 1.0f );
      AE_ASSERT( edgePos.z >= 0.0f && edgePos.z <= 1.0f );
      
      if ( x < 0 || y < 0 || z < 0 || x >= kChunkSize || y >= kChunkSize || z >= kChunkSize )
      {
        continue;
      }
      
      TerrainIndex ind[ 4 ];
      int32_t offsets[ 4 ][ 3 ];
      GetOffsetsFromEdge( mask[ e ], offsets );
      
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
          
          TerrainIndex index = (TerrainIndex)vertices.size();
          vertices.push_back( vertex );
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
          AE_ASSERT( index < vertices.size() );
          AE_ASSERT( chunk->t[ ox ][ oy ][ oz ] == Block::Surface );
          ind[ j ] = index;
        }
      }
      
      bool flip;
      if ( e == 0 ) { flip = !( grid[ 2 ][ 1 ] < 0.0f ); }
      else if ( e == 1 ) { flip = ( grid[ 2 ][ 1 ] < 0.0f ); }
      else { flip = ( grid[ 2 ][ 1 ] < 0.0f ); }
      
      if ( flip )
      {
        indices.push_back( ind[ 0 ] );
        indices.push_back( ind[ 1 ] );
        indices.push_back( ind[ 2 ] );
        indices.push_back( ind[ 1 ] );
        indices.push_back( ind[ 3 ] );
        indices.push_back( ind[ 2 ] );
      }
      else
      {
        indices.push_back( ind[ 0 ] );
        indices.push_back( ind[ 2 ] );
        indices.push_back( ind[ 1 ] );
        indices.push_back( ind[ 1 ] );
        indices.push_back( ind[ 2 ] );
        indices.push_back( ind[ 3 ] );
      }
    }
  }
  
  if ( indices.size() == 0 )
  {
    // TODO Should differentiate between empty chunk and full chunk
    *vertexCount = 0;
    *indexCount = 0;
    return;
  }
  
  const int32_t vc = (int32_t)vertices.size();
  for ( int32_t i = 0; i < vc; i++ )
  {
    TerrainVertex* vertex = &vertices[ i ];
    int32_t x = UMAT::Floor( vertex->position.x );
    int32_t y = UMAT::Floor( vertex->position.y );
    int32_t z = UMAT::Floor( vertex->position.z );
    
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
    
    for ( int32_t i = 0; i < ec; i++ )
    {
      AE_ASSERT( p[ i ].x == p[ i ].x && p[ i ].y == p[ i ].y && p[ i ].z == p[ i ].z );
      AE_ASSERT( p[ i ].x >= 0.0f && p[ i ].x <= 1.0f );
      AE_ASSERT( p[ i ].y >= 0.0f && p[ i ].y <= 1.0f );
      AE_ASSERT( p[ i ].z >= 0.0f && p[ i ].z <= 1.0f );
      AE_ASSERT( p[ i ][ 3 ] == 0.0f );
      AE_ASSERT( n[ i ][ 3 ] == 0.0f );
    }
    aeFloat3 position = GetIntersection( p, n, ec );
    AE_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
    position.x = UMAT::Max( 0.0f, position.x );
    position.y = UMAT::Max( 0.0f, position.y );
    position.z = UMAT::Max( 0.0f, position.z );
    position.x = UMAT::Min( 0.999f, position.x );
    position.y = UMAT::Min( 0.999f, position.y );
    position.z = UMAT::Min( 0.999f, position.z );
    position.x += chunkOffsetX + x;
    position.y += chunkOffsetY + y;
    position.z += chunkOffsetZ + z;
    
    // Vertex Normal
    aeFloat3 normal;
    for ( int32_t i = 0; i < 3; i++ )
    {
      aeFloat3 nt = position;
      nt[ i ] += 0.25f;
      normal[ i ] = TerrainValue( nt );
    }
    aeFloat3 pw = position;
    normal -= TerrainValue( pw );
    normal.SafeNormalize();
    
    float16_t light( 1.0f ); // HACK!!!
    
    AE_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
    vertex->position = position;
    vertex->normal = normal;
    vertex->info[ 0 ] = 0;
    vertex->info[ 1 ] = (uint8_t)( light + 0.5f );
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
  
  *vertexCount = (uint32_t)vertices.size();
  *indexCount = (uint32_t)indices.size();

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
  
  memcpy( vertexOut, vertices.data(), *vertexCount * sizeof(vertices[ 0 ]) );
  memcpy( indexOut, indices.data(), *indexCount * sizeof(indices[ 0 ]) );
}
*/

void TerrainBoss::UpdateChunkLighting( Chunk* chunk )
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

/*
Chunk* TerrainBoss::AllocChunk( int32_t cx, int32_t cy, int32_t cz )
{
  if ( m_totalChunks == kMaxLoadedChunks )
  {
    const aeFloat3 avatarPos = m_program->game->avatar->GetPosition();
    Chunk* farChunk = nullptr;
    float maxDist = 0.0f;
    
    Chunk* current = m_headChunk;
    while( current )
    {
      int32_t cx = current->pos[ 0 ];
      int32_t cy = current->pos[ 1 ];
      int32_t cz = current->pos[ 2 ];
      float d = ( avatarPos - aeFloat3( cx + 0.5f, cy + 0.5f, cz + 0.5f ) * kChunkSize ).LengthSquared();
      
      if ( maxDist < d )
      {
        maxDist = d;
        farChunk = current;
      }
      
      current = current->next;
    }
    
    AE_ASSERT( !farChunk->active );
    FreeChunk( farChunk );
  }
  
  uint32_t chunkIndex = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
  AE_ASSERT( m_chunks[ chunkIndex ] == nullptr );
  
  Chunk* chunk = m_chunkPool.Allocate();
  AE_ASSERT( chunk );
  memset( chunk, 0, sizeof( Chunk ) );
  memset( chunk->i, ~(uint8_t)0, sizeofarray(chunk->i) );
  
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

void TerrainBoss::FreeChunk( Chunk* chunk )
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

void TerrainBoss::GetOffsetsFromEdge( uint32_t edgeBit, int32_t (&offsets)[ 4 ][ 3 ] )
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
*/
Chunk* TerrainBoss::GetChunk( int32_t cx, int32_t cy, int32_t cz )
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


/*
void TerrainBoss::Initialize( ProgramInterface* program )
{
  memset( this, 0, sizeof(*this) );
  m_program = program;
  
  //m_chunkPool.Initialize(); @TODO: Reset pool
  
  uint32_t worldChunkBytes = sizeof(m_chunks[0]) * kWorldChunksWidth * kWorldChunksWidth * kWorldChunksHeight;
  uint32_t voxelCountBytes = sizeof(m_voxelCounts[0]) * kWorldChunksWidth * kWorldChunksWidth * kWorldChunksHeight;
  uint32_t allocSize = worldChunkBytes + voxelCountBytes;

  m_chunkRawAlloc = aeAlloc::AllocateRaw( 1, 16, worldChunkBytes + voxelCountBytes );
  
  memset( m_chunkRawAlloc, 0, worldChunkBytes );
  m_chunks = (Chunk**)m_chunkRawAlloc;
  m_voxelCounts = (int16_t*)( (int8_t*)m_chunkRawAlloc + worldChunkBytes );
  memset( m_voxelCounts, ~0, voxelCountBytes );
  
  memset( m_activeChunks, 0, countof(m_activeChunks) );
  m_headChunk = nullptr;
  m_tailChunk = nullptr;
  m_totalChunks = 0;
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockCollision[ i ] = true; }
  m_blockCollision[ Block::Exterior ] = false;
  m_blockCollision[ Block::Unloaded ] = false;
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockDensity[ i ] = 1.0f; }
  
  m_noise.Initialize( 0 );
  
  m_loaded = false;
}


void TerrainBoss::RenderInitialize( ProgramInterface* program )
{
  m_shader = program->game->shaderBoss->GetShader( "Terrain" );
  m_leavesShader = program->game->shaderBoss->GetShader( "Leaves" );
  
  grassTexture = program->game->textureBoss->GetTexture( "Textures/GrassDist00.png" );
  rockTexture = program->game->textureBoss->GetTexture( "Textures/Rock07.png" );
  dirtTexture = program->game->textureBoss->GetTexture( "Textures/Dirt03.png" );
  treeTexture = program->game->textureBoss->GetTexture( "Textures/Tree00.png" );
  leavesTexture = program->game->textureBoss->GetTexture( "Textures/Leaves00.png" );
  spiralTexture = program->game->textureBoss->GetTexture( "Textures/Spiral00.png" );
  
  m_mapWallTex = program->game->textureBoss->GetTextureReadable( "Textures/MapWalls00.png" );
  m_mapFloorHeightTex = program->game->textureBoss->GetTextureReadable( "Textures/MapFloorHeight00.png" );
}

void TerrainBoss::Update( class ProgramInterface* program )
{
  char str[ 128 ];
  sprintf( str, "Loaded Chunks: %d / %d", m_totalChunks, kMaxLoadedChunks );
  program->game->hudBoss->AddDebugString( 330, 155, 0.5f, aeFloat3(0.0f), str );
  sprintf( str, "Active Chunks: %d / %d", m_activeChunkCount, kMaxActiveChunks );
  program->game->hudBoss->AddDebugString( 330, 135, 0.5f, aeFloat3(0.0f), str );
  m_compactAlloc->Compact();
}

void TerrainBoss::BuildScene( ProgramInterface* program )
{
  const int32_t viewRadius = 5;
  const int32_t worldViewRadius2 = viewRadius * viewRadius * kChunkSize * kChunkSize;
  const int32_t viewDiam = viewRadius + viewRadius;
  
  RenderDirector* renderDirector = program->renderDirector;
  Frustum2 frustum( program );
  
  const aeFloat3 avatarPos = program->game->avatar->GetPosition();
  int32_t ci = int32_t(avatarPos.x) / kChunkSize;
  int32_t cj = int32_t(avatarPos.y) / kChunkSize;
  int32_t ck = int32_t(avatarPos.z) / kChunkSize;
  
  struct ChunkSort
  {
    Chunk *c;
    uint32_t pos[ 3 ];
    float32_t d;
  };
  
  for( uint32_t i = 0; i < m_activeChunkCount; i++ )
  {
    m_activeChunks[ i ]->active = false;
  }
  m_activeChunkCount = 0;
  
  ChunkSort chunkSort[ viewDiam * viewDiam * viewDiam ];
  uint32_t sortCount = 0;
  for( uint32_t cz = UMAT::Max( 0, ck - viewRadius ); cz < UMAT::Min( int32_t(kWorldChunksHeight), ck + viewRadius ); cz++ )
  for( uint32_t cy = UMAT::Max( 0, cj - viewRadius ); cy < UMAT::Min( int32_t(kWorldChunksWidth), cj + viewRadius ); cy++ )
  for( uint32_t cx = UMAT::Max( 0, ci - viewRadius ); cx < UMAT::Min( int32_t(kWorldChunksWidth), ci + viewRadius ); cx++ )
  {
    int32_t ci = cx + kWorldChunksWidth * ( cy + kWorldChunksWidth * cz );
    int16_t vc = m_voxelCounts[ ci ];
    if ( vc == 0 || vc == kChunkCountMax ) { continue; }
    
    float d = ( avatarPos - aeFloat3( cx + 0.5f, cy + 0.5f, cz + 0.5f ) * kChunkSize ).LengthSquared();
    if ( d >= worldViewRadius2 ) { continue; }
    
    Chunk* c = m_chunks[ ci ];
    if( !c )
    {
      c = AllocChunk( cx, cy, cz );
      
      uint32_t vertexCount;
      uint32_t indexCount;
      ScratchBuffer< TerrainVertex > vertexScratch( kMaxChunkVerts );
      ScratchBuffer< TerrainIndex > indexScratch( kMaxChunkVerts );
      
      GetChunkVerts( c, vertexScratch.Get(), indexScratch.Get(), &vertexCount, &indexCount );
      m_voxelCounts[ ci ] = vertexCount;
      if ( vertexCount == 0 || vertexCount == kChunkCountMax )
      {
        // TODO: It's super expensive to finally just throw away the uneeded chunk...
        FreeChunk( c );
        continue;
      }
      
      c->data.Initialize( vertexCount, indexCount, Primitive::Triangle, Usage::Dynamic );
      c->data.AddAttribute( "a_position", 3, GL_FLOAT, sizeof(aeFloat3) );
      c->data.AddAttribute( "a_normal", 3, GL_FLOAT, sizeof(aeFloat3) );
      c->data.AddAttribute( "a_info", 4, GL_UNSIGNED_BYTE, 4*sizeof(uint8_t) );
      c->data.SetVertexSize( sizeof(TerrainVertex) );
      c->data.SetIndexSize( sizeof(TerrainIndex) );
      
      uint32_t vertexBytes = vertexCount * sizeof(TerrainVertex);
      uint32_t indexBytes = indexCount * sizeof(TerrainIndex);
      c->data.SetVertices( vertexScratch.Get(), vertexBytes );
      c->data.SetIndices( indexScratch.Get(), indexBytes );
      
      m_compactAlloc->Allocate( (void**)(&c->vertices), vertexBytes );
      memcpy( c->vertices, vertexScratch.Get(), vertexBytes );
      
      c->lightDirty = true;
    }
    
    chunkSort[ sortCount ].c = c;
    chunkSort[ sortCount ].pos[ 0 ] = cx;
    chunkSort[ sortCount ].pos[ 1 ] = cy;
    chunkSort[ sortCount ].pos[ 2 ] = cz;
    chunkSort[ sortCount ].d = d;
    sortCount++;
  }
  std::sort( (ChunkSort*)chunkSort, (ChunkSort*)(chunkSort + sortCount),
    []( const ChunkSort &a, const ChunkSort &b ) -> bool
  {
      return a.d < b.d;
  });
  for ( int32_t i = 0; i < sortCount && m_activeChunkCount < kMaxActiveChunks; i++ )
  {
    Chunk* c = chunkSort[ i ].c;
    AE_ASSERT( c );
    m_activeChunks[ m_activeChunkCount++ ] = c;
    c->active = true;
  }

  for( uint32_t i = 0; i < m_activeChunkCount; i++ )
  {
    Chunk* chunk = m_activeChunks[ i ];
    AE_ASSERT( chunk->check == 0xCDCDCDCD );
    AE_ASSERT( chunk->active );
    if( frustum.TestChunk( chunk ) )
    {
      Renderable renderable;
      renderable.shader = m_shader;
      renderable.vertexData = &chunk->data;
      if ( renderable.vertexData->GetIndexCount() == 0 ) { continue; }
      AE_ASSERT( renderable.vertexData->GetIndexCount() != 0 );
      renderable.samplerNames[ 0 ] = "rockTex";
      renderable.samplerNames[ 1 ] = "dirtTex";
      renderable.samplerNames[ 2 ] = "spiralTex";
      renderable.samplerNames[ 3 ] = "treeTex";
      renderable.textures[ 0 ] = rockTexture->texture;
      renderable.textures[ 1 ] = spiralTexture->texture;
      renderable.textures[ 2 ] = dirtTexture->texture;
      renderable.textures[ 3 ] = treeTexture->texture;
      renderDirector->AddRenderable( RenderList::Terrain, renderable );
    }
    AE_ASSERT( chunk->active );
  }
}

bool TerrainBoss::GetCollision( uint32_t x, uint32_t y, uint32_t z )
{
  return m_blockCollision[ GetVoxel( x, y, z ) ];
}

bool TerrainBoss::GetCollision( aeFloat3 position )
{
  return m_blockCollision[ GetVoxel( position.x, position.y, position.z ) ];
}

uint8_t TerrainBoss::GetVoxel( aeFloat3 position )
{
  return GetVoxel( position.x, position.y, position.z );
}

uint8_t TerrainBoss::GetVoxel( uint32_t x, uint32_t y, uint32_t z )
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

float16_t TerrainBoss::GetLight( uint32_t x, uint32_t y, uint32_t z )
{
  uint32_t cix = x / kChunkSize;
  uint32_t ciy = y / kChunkSize;
  uint32_t ciz = z / kChunkSize;
  
  if( cix >= kWorldChunksWidth || ciy >= kWorldChunksWidth || ciz >= kWorldChunksHeight ) { return kSkyBrightness; }
  Chunk *chunk = GetChunk( cix, ciy, ciz );
  if( chunk == nullptr ) { return kSkyBrightness; }
  
  return chunk->l[ x - cix * kChunkSize ][ y - ciy * kChunkSize ][ z - ciz * kChunkSize ];
}

bool TerrainBoss::VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps )
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

RaycastResult TerrainBoss::RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision )
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

RaycastResult TerrainBoss::Raycast( aeFloat3 start, aeFloat3 ray )
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
