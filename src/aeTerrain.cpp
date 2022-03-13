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
#include <ctpl_stl.h>

//------------------------------------------------------------------------------
// Configuration
//------------------------------------------------------------------------------
#ifndef AE_TERRAIN_LOG
  #define AE_TERRAIN_LOG 0
#endif

#ifndef AE_TERRAIN_SIMD
  #if _AE_LINUX_ || _AE_EMSCRIPTEN_
    #define AE_TERRAIN_SIMD 0
  #else
    #define AE_TERRAIN_SIMD 1
  #endif
#endif

#ifndef AE_TERRAIN_FANCY_NORMALS
  #define AE_TERRAIN_FANCY_NORMALS 0
#endif

#ifndef AE_TERRAIN_TOUCH_UP_VERT
  #define AE_TERRAIN_TOUCH_UP_VERT 0
#endif

//------------------------------------------------------------------------------
// SIMD headers
//------------------------------------------------------------------------------
#if AE_TERRAIN_SIMD && __aarch64__ && _AE_APPLE_
  #include "sse2neon.h"
#endif

//------------------------------------------------------------------------------
// Constants / helpers
//------------------------------------------------------------------------------
namespace ae {

ae::Vec3 GetIntersection( const ae::Vec3* p, const ae::Vec3* n, uint32_t ic )
{
#if AE_TERRAIN_SIMD
  __m128 c128 = _mm_setzero_ps();
  for ( uint32_t i = 0; i < ic; i++ )
  {
    __m128 p128 = _mm_load_ps( (float*)( p + i ) );
    c128 = _mm_add_ps( c128, p128 );
  }
  __m128 div = _mm_set1_ps( 1.0f / ic );
  c128 = _mm_mul_ps( c128, div );
  
  for ( uint32_t i = 0; i < 10; i++ )
  for ( uint32_t j = 0; j < ic; j++ )
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
  ae::Vec3 v;
  _mm_store_ps( (float*)&v, c128 );
  return v;
#else
  ae::Vec3 c( 0.0f );
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

TerrainSdfCache::TerrainSdfCache()
{
  m_chunk = ae::Int3( 0 );
  m_values = ae::NewArray< aeFloat16 >( AE_ALLOC_TAG_TERRAIN, kDim * kDim * kDim );
}

TerrainSdfCache::~TerrainSdfCache()
{
  ae::Delete( m_values );
  m_values = nullptr;
}

void TerrainSdfCache::Generate( ae::Int3 chunk, const TerrainJob* job )
{
  m_chunk = chunk;

  m_offseti = ae::Int3( kOffset ) - ae::Int3( m_chunk * kChunkSize );
  m_offsetf = ae::Vec3( (float)kOffset ) - ae::Vec3( m_chunk * kChunkSize );
  m_p = job->GetTerrainParams();

  ae::Int3 offset = m_chunk * kChunkSize - ae::Int3( kOffset );
  for ( int32_t z = 0; z < kDim; z++ )
  for ( int32_t y = 0; y < kDim; y++ )
  for ( int32_t x = 0; x < kDim; x++ )
  {
    uint32_t index = x + kDim * ( y + kDim * z );
    ae::Vec3 pos( offset.x + x, offset.y + y, offset.z + z );
    m_values[ index ] = job->GetValue( pos );
  }
}

float TerrainSdfCache::GetValue( ae::Vec3 pos ) const
{
  pos += m_offsetf;

  ae::Int3 posi = pos.FloorCopy();
  pos.x -= posi.x;
  pos.y -= posi.y;
  pos.z -= posi.z;

  float values[ 8 ] =
  {
    m_GetValue( posi ),
    m_GetValue( posi + ae::Int3( 1, 0, 0 ) ),
    m_GetValue( posi + ae::Int3( 0, 1, 0 ) ),
    m_GetValue( posi + ae::Int3( 1, 1, 0 ) ),
    m_GetValue( posi + ae::Int3( 0, 0, 1 ) ),
    m_GetValue( posi + ae::Int3( 1, 0, 1 ) ),
    m_GetValue( posi + ae::Int3( 0, 1, 1 ) ),
    m_GetValue( posi + ae::Int3( 1, 1, 1 ) ),
  };

  float x0 = ae::Lerp( values[ 0 ], values[ 1 ], pos.x );
  float x1 = ae::Lerp( values[ 2 ], values[ 3 ], pos.x );
  float x2 = ae::Lerp( values[ 4 ], values[ 5 ], pos.x );
  float x3 = ae::Lerp( values[ 6 ], values[ 7 ], pos.x );
  float y0 = ae::Lerp( x0, x1, pos.y );
  float y1 = ae::Lerp( x2, x3, pos.y );
  return ae::Lerp( y0, y1, pos.z );
}

float TerrainSdfCache::GetValue( ae::Int3 pos ) const
{
  return m_GetValue( pos + m_offseti );
}

ae::Vec3 TerrainSdfCache::GetDerivative( ae::Vec3 p ) const
{
  ae::Vec3 pv( GetValue( p ) );
  
  ae::Vec3 normal0;
  for ( int32_t i = 0; i < 3; i++ )
  {
    ae::Vec3 nt = p;
    nt[ i ] += m_p.normalSampleOffset;
    normal0[ i ] = GetValue( nt );
  }
  AE_ASSERT( normal0 != ae::Vec3( 0.0f ) );
  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal0 -= pv;
  AE_ASSERT( normal0 != ae::Vec3( 0.0f ) );
  normal0 /= normal0.Length();
  AE_ASSERT( normal0 == normal0 );

  ae::Vec3 normal1;
  for ( int32_t i = 0; i < 3; i++ )
  {
    ae::Vec3 nt = p;
    nt[ i ] -= m_p.normalSampleOffset;
    normal1[ i ] = GetValue( nt );
  }
  AE_ASSERT( normal1 != ae::Vec3( 0.0f ) );
  // This should be really close to 0 because it's really
  // close to the surface but not close enough to ignore.
  normal1 = pv - normal1;
  AE_ASSERT( normal1 != ae::Vec3( 0.0f ) );
  normal1 /= normal1.Length();
  AE_ASSERT( normal1 == normal1 );

  return ( normal1 + normal0 ).SafeNormalizeCopy();
}

float TerrainSdfCache::m_GetValue( ae::Int3 pos ) const
{
#if _AE_DEBUG_
  AE_ASSERT( pos.x >= 0 && pos.y >= 0 && pos.z >= 0 );
  AE_ASSERT( pos.x < kDim && pos.y < kDim && pos.z < kDim );
#endif
  return m_values[ pos.x + kDim * ( pos.y + kDim * pos.z ) ];
}

TerrainChunk::TerrainChunk() :
  m_generatedList( this )
{
  m_check = 0xCDCDCDCD;

  m_pos = ae::Int3( 0 );

  m_geoDirty = false; // @NOTE: Start false. This flag is only for chunks that need to be regenerated
  m_lightDirty = false;

  memset( m_t, 0, sizeof( m_t ) );
  memset( m_l, 0, sizeof( m_l ) );
  memset( m_i, ~(uint8_t)0, sizeof( m_i ) );
}

TerrainChunk::~TerrainChunk()
{}

// https://stackoverflow.com/questions/919612/mapping-two-integers-to-one-in-a-unique-and-deterministic-way
// https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
uint32_t TerrainChunk::GetIndex( ae::Int3 pos )
{
  uint32_t x = ( pos.x >= 0 ) ? 2 * pos.x : -2 * pos.x - 1;
  uint32_t y = ( pos.y >= 0 ) ? 2 * pos.y : -2 * pos.y - 1;
  uint32_t z = ( pos.z >= 0 ) ? 2 * pos.z : -2 * pos.z - 1;

  uint32_t max = ae::Max( x, y, z );
  uint32_t hash = max * max * max + ( 2 * max * z ) + z;
  if ( max == z )
  {
    uint32_t xy = ae::Max( x, y );
    hash += xy * xy;
  }

  if ( y >= x )
  {
    hash += x + y;
  }
  else
  {
    hash += y;
  }

  return hash;
}

void TerrainChunk::GetPosFromWorld( ae::Int3 pos, ae::Int3* chunkPos, ae::Int3* localPos )
{
  ae::Vec3 p( pos );
  p /= kChunkSize;
  ae::Int3 c = p.FloorCopy();
  if ( chunkPos )
  {
    *chunkPos = c;
  }
  if ( localPos )
  {
    ae::Int3 l = ( pos - c * kChunkSize );
    *localPos = ae::Int3(
      l.x % kChunkSize,
      l.y % kChunkSize,
      l.z % kChunkSize );
  }
}

uint32_t TerrainChunk::GetIndex() const
{
  return GetIndex( m_pos );
}

TerrainJob::TerrainJob() :
  m_hasJob( false ),
  m_running( false ),
  m_vertexCount( kChunkCountEmpty ),
  m_indexCount( 0 ),
  m_vertices( ae::Array< TerrainVertex >( AE_ALLOC_TAG_TERRAIN, (uint32_t)kMaxChunkVerts, TerrainVertex() ) ),
  m_indices( ae::Array< TerrainIndex >( AE_ALLOC_TAG_TERRAIN, kMaxChunkIndices, TerrainIndex() ) ),
  m_chunk( nullptr )
{
  edgeInfo = ae::NewArray< TempEdges >( AE_ALLOC_TAG_TERRAIN, kTempChunkSize3 );
}

TerrainJob::~TerrainJob()
{
  ae::Delete( edgeInfo );
  edgeInfo = nullptr;
}

void TerrainJob::StartNew( const TerrainParams& params, const TerrainSdf* sdf, TerrainChunk* chunk )
{
  AE_ASSERT( chunk );
  AE_ASSERT_MSG( !m_chunk, "Previous job not finished" );
  AE_ASSERT_MSG( !m_shapes.Length(), "Previous job not finished" );

  m_p = params;

  m_hasJob = true;
  m_running = true;

  m_vertexCount = kChunkCountEmpty;
  m_indexCount = 0;
  m_chunk = chunk;
  
  // 1) Add shapes to job that intersect current chunk
  ae::AABB aabb = chunk->GetAABB();
  for ( uint32_t i = 0; i < sdf->GetShapeCount(); i++ )
  {
    ae::Sdf* shape = sdf->GetShapeAtIndex( i );
    if ( shape->GetAABB().Intersect( aabb ) )
    {
      m_shapes.Append( shape );
    }
  }
  
  // 2) Put shapes in order
  std::stable_sort( m_shapes.Begin(), m_shapes.End(), []( const ae::Sdf* s0, const ae::Sdf* s1 )
  {
    return s0->order < s1->order;
  } );
  
  // 3) Remove non-solid shapes from the front of the list as they should have no affect
  // @TODO: Should skip this job altogether when the chunk will have no vertices
  while ( m_shapes.Length() && !m_shapes[ 0 ]->IsSolid() )
  {
    m_shapes.Remove( 0 );
  }
  
  // 4) Replace shapes with copies so they are not modified mid-job
  for ( uint32_t i = 0; i < m_shapes.Length(); i++ )
  {
    m_shapes[ i ] = m_shapes[ i ]->Clone();
  }
}

void TerrainJob::Do()
{
  AE_ASSERT( m_chunk );
  
  // Hash
  m_parameterHash = ae::Hash();
  
  m_parameterHash = m_p.GetHash( m_parameterHash );
  
  ae::Int3 chunkPos = m_chunk->m_pos;
  m_parameterHash = m_parameterHash.HashBasicType( chunkPos.x );
  m_parameterHash = m_parameterHash.HashBasicType( chunkPos.y );
  m_parameterHash = m_parameterHash.HashBasicType( chunkPos.z );
  
  for ( ae::Sdf* shape : m_shapes )
  {
    m_parameterHash = shape->Hash( m_parameterHash );
  }
  
  // Check disk to see if job has been completed before
  ae::Str128 filePath = ae::Str128::Format( "terrain/#_#_#_#", chunkPos.x, chunkPos.y, chunkPos.z, m_parameterHash.Get() );
  uint32_t fileSize = m_p.vfs ? m_p.vfs->GetSize( ae::FileSystem::Root::Cache, filePath.c_str() ) : 0;
  if ( fileSize )
  {
    ae::Scratch< uint8_t > fileData( AE_ALLOC_TAG_TERRAIN, fileSize );
    m_p.vfs->Read( ae::FileSystem::Root::Cache, filePath.c_str(), fileData.Data(), fileSize );
    ae::BinaryStream rStream = ae::BinaryStream::Reader( fileData.Data(), fileSize );
    rStream.SerializeUint32( m_vertexCount.Get() );
    rStream.SerializeUint32( m_indexCount );
    rStream.SerializeRaw( &m_vertices[ 0 ], (uint32_t)m_vertexCount * sizeof(m_vertices[ 0 ]) );
    rStream.SerializeRaw( &m_indices[ 0 ], m_indexCount * sizeof(m_indices[ 0 ]) );
    rStream.SerializeObject( *m_chunk );
  }
  else
  {
    AE_ASSERT( m_chunk );
    m_sdfCache.Generate( m_chunk->m_pos, this );
    m_chunk->Generate( &m_sdfCache, this, edgeInfo, &m_vertices[ 0 ], &m_indices[ 0 ], &m_vertexCount, &m_indexCount );
    
    ae::CollisionMesh::Params meshParams;
    // Vertices
    meshParams.positions = (float*)&m_vertices[ 0 ].position;
    meshParams.positionCount = (uint32_t)m_vertexCount;
    meshParams.positionStride = sizeof(m_vertices[ 0 ]);
    // Indices
    meshParams.indices = &m_indices[ 0 ];
    meshParams.indexCount = m_indexCount;
    meshParams.indexSize = sizeof(m_indices[ 0 ]);
    // Load
    m_chunk->m_mesh.Load( meshParams );
    
    if ( m_p.vfs )
    {
      // Write result
      ae::Array< uint8_t > data = AE_ALLOC_TAG_TERRAIN;
      ae::BinaryStream wStream = ae::BinaryStream::Writer( &data );
      wStream.SerializeUint32( m_vertexCount.Get() );
      wStream.SerializeUint32( m_indexCount );
      wStream.SerializeRaw( &m_vertices[ 0 ], (uint32_t)m_vertexCount * sizeof(m_vertices[ 0 ]) );
      wStream.SerializeRaw( &m_indices[ 0 ], m_indexCount * sizeof(m_indices[ 0 ]) );
      wStream.SerializeObject( *m_chunk );
      if ( !m_p.vfs->Write( ae::FileSystem::Root::Cache, filePath.c_str(), wStream.GetData(), wStream.GetOffset(), true ) )
      {
        AE_WARN( "Failed writing terrain chunk '#'", filePath );
      }
    }
  }
  
  m_running = false;
}

void TerrainJob::Finish()
{
  AE_ASSERT( m_chunk );
  AE_ASSERT( !m_running );
  
  for ( ae::Sdf* shape : m_shapes )
  {
    ae::Delete( shape );
  }
  m_shapes.Clear();

  m_hasJob = false;
  m_vertexCount = kChunkCountEmpty;
  m_indexCount = 0;
  m_chunk = nullptr;
}

bool TerrainJob::HasChunk( ae::Int3 pos ) const
{
  return m_chunk && m_chunk->m_pos == pos;
}

void TerrainChunk::Generate( const TerrainSdfCache* sdf, const TerrainJob* job, TerrainJob::TempEdges* edgeInfo, TerrainVertex* verticesOut, TerrainIndex* indexOut, VertexCount* vertexCountOut, uint32_t* indexCountOut )
{
#if AE_TERRAIN_FANCY_NORMALS
  struct TempTri
  {
    uint16_t i[ 3 ]; // isosurface generation
    uint16_t i1[ 3 ]; // normal split
    ae::Vec3 n;
  };
  ae::Array< TempTri > tempTris = AE_ALLOC_TAG_TERRAIN;

  struct TempVert
  {
    ae::Int3 posi; // Vertex voxel (not necessarily Floor(vert.pos), see vertex positioning)
    TerrainVertex v;
};
  ae::Array< TempVert > tempVerts = AE_ALLOC_TAG_TERRAIN;
#else
  VertexCount vertexCount = VertexCount( 0 );
  uint32_t indexCount = 0;
#endif

#if AE_TERRAIN_LOG
  AE_LOG( "Generate chunk #", m_pos );
#endif

  int32_t chunkOffsetX = m_pos.x * kChunkSize;
  int32_t chunkOffsetY = m_pos.y * kChunkSize;
  int32_t chunkOffsetZ = m_pos.z * kChunkSize;
  
  memset( edgeInfo, 0, kTempChunkSize3 * sizeof( *edgeInfo ) );
  
  uint16_t mask[ 3 ];
  mask[ 0 ] = EDGE_TOP_FRONT_BIT;
  mask[ 1 ] = EDGE_TOP_RIGHT_BIT;
  mask[ 2 ] = EDGE_SIDE_FRONTRIGHT_BIT;
  
  // 3 new edges to test
  ae::Vec3 cornerOffsets[ 3 ][ 2 ];
  // EDGE_TOP_FRONT_BIT
  cornerOffsets[ 0 ][ 0 ] = ae::Vec3( 0, 1, 1 );
  cornerOffsets[ 0 ][ 1 ] = ae::Vec3( 1, 1, 1 );
  // EDGE_TOP_RIGHT_BIT
  cornerOffsets[ 1 ][ 0 ] = ae::Vec3( 1, 0, 1 );
  cornerOffsets[ 1 ][ 1 ] = ae::Vec3( 1, 1, 1 );
  // EDGE_SIDE_FRONTRIGHT_BIT
  cornerOffsets[ 2 ][ 0 ] = ae::Vec3( 1, 1, 0 );
  cornerOffsets[ 2 ][ 1 ] = ae::Vec3( 1, 1, 1 );
  
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
      cornerValues[ i ][ j ] = sdf->GetValue( ae::Int3( gx, gy, gz ) );
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
    
    // Store block type
    // @TODO: Remove this when raycasting uses triangle mesh
    if ( edgeBits == 0 )
    {
      if ( x >= 0 && y >= 0 && z >= 0 && x < kChunkSize && y < kChunkSize && z < kChunkSize )
      {
        if ( m_i[ x ][ y ][ z ] != kInvalidTerrainIndex )
        {
          continue;
        }
        
        ae::Vec3 g;
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
    TerrainJob::TempEdges* te = &edgeInfo[ edgeIndex ];
    te->b = edgeBits;
    te->x = x;
    te->y = y;
    te->z = z;
    
    // Iterate over voxel edges (only 3 for TempEdges)
    for ( int32_t e = 0; e < 3; e++ )
    if ( edgeBits & mask[ e ] )
    {
#if !AE_TERRAIN_FANCY_NORMALS
      if ( vertexCount + VertexCount( 4 ) > kMaxChunkVerts || indexCount + 6 > kMaxChunkIndices )
      {
        *vertexCountOut = VertexCount( 0 );
        *indexCountOut = 0;
        return;
      }
#endif

      // Get intersection of edge and implicit surface
      ae::Vec3 edgeVoxelPos;
      // Start edgeVoxelPos calculation
      {
        // Determine which end of edge is inside/outside
        ae::Vec3 c0, c1;
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
        ae::Vec3 ch( chunkOffsetX + x, chunkOffsetY + y, chunkOffsetZ + z );
        // @TODO: This should probably be adjustable
        for ( int32_t i = 0; i < 16; i++ )
        {
          // @TODO: This can be simplified by lerping and using the t value to do a binary search
          edgeVoxelPos = ( c0 + c1 ) * 0.5f;
          ae::Vec3 cw = ch + edgeVoxelPos;
          
          float v = sdf->GetValue( cw );
          if ( ae::Abs( v ) < 0.001f )
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
      // End edgeVoxelPos calculation
      
      ae::Vec3 edgeWorldPos( chunkOffsetX + x, chunkOffsetY + y, chunkOffsetZ + z );
      edgeWorldPos += edgeVoxelPos;

      te->p[ e ] = edgeVoxelPos;
      te->n[ e ] = sdf->GetDerivative( edgeWorldPos );
      
      if ( x < 0 || y < 0 || z < 0 || x >= kChunkSize || y >= kChunkSize || z >= kChunkSize )
      {
        continue;
      }
      
      TerrainIndex ind[ 4 ];
      int32_t offsets[ 4 ][ 3 ];
      m_GetQuadVertexOffsetsFromEdge( mask[ e ], offsets );
      
      // @NOTE: Expand edge into two triangles. Add new vertices for each edge
      // intersection (centered in voxels at this point). Edges are eventually expanded
      // into quads, so each edge needs 4 vertices. This does some of the work
      // for adjacent voxels.
      for ( int32_t j = 0; j < 4; j++ )
      {
        int32_t ox = x + offsets[ j ][ 0 ];
        int32_t oy = y + offsets[ j ][ 1 ];
        int32_t oz = z + offsets[ j ][ 2 ];
        
        // This check allows coordinates to be one out of chunk high end
        if ( ox < 0 || oy < 0 || oz < 0 || ox > kChunkSize || oy > kChunkSize || oz > kChunkSize )
        {
          continue;
        }
        
        bool inCurrentChunk = ox < kChunkSize && oy < kChunkSize && oz < kChunkSize;
        if ( !inCurrentChunk || m_i[ ox ][ oy ][ oz ] == kInvalidTerrainIndex )
        {
          TerrainVertex vertex;
          vertex.position.x = ox + 0.5f;
          vertex.position.y = oy + 0.5f;
          vertex.position.z = oz + 0.5f;
          
          AE_ASSERT( vertex.position.x == vertex.position.x && vertex.position.y == vertex.position.y && vertex.position.z == vertex.position.z );
          
#if AE_TERRAIN_FANCY_NORMALS
          TerrainIndex index = (TerrainIndex)tempVerts.Length();
          tempVerts.Append( { ae::Int3( ox, oy, oz ), vertex } );
#else
          TerrainIndex index = (TerrainIndex)vertexCount;
          verticesOut[ (uint32_t)vertexCount ] = vertex;
          vertexCount++;
#endif
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
#if !AE_TERRAIN_FANCY_NORMALS
          AE_ASSERT_MSG( index < (TerrainIndex)vertexCount, "# < # ox:# oy:# oz:#", index, vertexCount, ox, oy, oz );
#endif
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
#if AE_TERRAIN_FANCY_NORMALS
        tempTris.Append( { ind[ 0 ], ind[ 1 ], ind[ 2 ] } );
        tempTris.Append( { ind[ 1 ], ind[ 3 ], ind[ 2 ] } );
#else
        // tri0
        indexOut[ indexCount++ ] = ind[ 0 ];
        indexOut[ indexCount++ ] = ind[ 1 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
        // tri1
        indexOut[ indexCount++ ] = ind[ 1 ];
        indexOut[ indexCount++ ] = ind[ 3 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
#endif
      }
      else
      {
#if AE_TERRAIN_FANCY_NORMALS
        tempTris.Append( { ind[ 0 ], ind[ 2 ], ind[ 1 ] } );
        tempTris.Append( { ind[ 1 ], ind[ 2 ], ind[ 3 ] } );
#else
        // tri2
        indexOut[ indexCount++ ] = ind[ 0 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
        indexOut[ indexCount++ ] = ind[ 1 ];
        //tri3
        indexOut[ indexCount++ ] = ind[ 1 ];
        indexOut[ indexCount++ ] = ind[ 2 ];
        indexOut[ indexCount++ ] = ind[ 3 ];
#endif
      }
    }
  }
  
#if AE_TERRAIN_FANCY_NORMALS
  if ( !tempTris.Length() )
#else
  if ( indexCount == 0 )
#endif
  {
    // @TODO: Should differentiate between empty chunk and full chunk. It's possible though that
    // Chunk::t's are good enough for this though.
    *vertexCountOut = kChunkCountEmpty;
    *indexCountOut = 0;
    return;
  }
  
#if AE_TERRAIN_FANCY_NORMALS
  const uint32_t vc = (int32_t)tempVerts.Length();
#else
  const uint32_t vc = (int32_t)vertexCount;
#endif
  for ( uint32_t i = 0; i < vc; i++ )
  {
#if AE_TERRAIN_FANCY_NORMALS
    TerrainVertex* vertex = &tempVerts[ i ].v;
#else
    TerrainVertex* vertex = &verticesOut[ i ];
#endif
    int32_t x = ae::Floor( vertex->position.x );
    int32_t y = ae::Floor( vertex->position.y );
    int32_t z = ae::Floor( vertex->position.z );
    AE_ASSERT( x >= 0 && y >= 0 && z >= 0 );
    AE_ASSERT( x <= kChunkSize && y <= kChunkSize && z <= kChunkSize );
    
    int32_t ec = 0;
    ae::Vec3 p[ 12 ];
    ae::Vec3 n[ 12 ];
    
    uint32_t edgeIndex = x + 1 + kTempChunkSize * ( y + 1 + ( z + 1 ) * kTempChunkSize );
    AE_ASSERT( edgeIndex < kTempChunkSize3 );
    TerrainJob::TempEdges te = edgeInfo[ edgeIndex ];
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
    
    // Validation
    AE_ASSERT( ec != 0 );
    for ( int32_t j = 0; j < ec; j++ )
    {
      AE_ASSERT( p[ j ] == p[ j ] );
      AE_ASSERT( p[ j ].x >= 0.0f && p[ j ].x <= 1.0f );
      AE_ASSERT( p[ j ].y >= 0.0f && p[ j ].y <= 1.0f );
      AE_ASSERT( p[ j ].z >= 0.0f && p[ j ].z <= 1.0f );
      AE_ASSERT( n[ j ] == n[ j ] );
    }

    // Normal
    vertex->normal = ae::Vec3( 0.0f );
    for ( int32_t j = 0; j < ec; j++ )
    {
      vertex->normal += n[ j ];
    }
    vertex->normal.SafeNormalize();
    
    // Position (after normals for AE_TERRAIN_TOUCH_UP_VERT)
    ae::Vec3 position = GetIntersection( p, n, ec );
    {
      AE_ASSERT( position.x == position.x && position.y == position.y && position.z == position.z );
      // @NOTE: Bias towards average of intersection points. This solves some intersecting triangles on sharp edges.
      // Based on notes here: https://www.boristhebrave.com/2018/04/15/dual-contouring-tutorial/
      ae::Vec3 averagePos( 0.0f );
      for ( int32_t i = 0; i < ec; i++ )
      {
        averagePos += p[ i ];
      }
      averagePos /= (float)ec;
      position = ae::Lerp( position, averagePos, 0.75f );
    }
#if AE_TERRAIN_TOUCH_UP_VERT
    //{
    //  ae::Vec3 iv0 = IntersectRayAABB( start, ray, result.posi );
    //  ae::Vec3 iv1 = IntersectRayAABB( start + ray, -ray, result.posi );
    //  float fv0 = sdf.GetValue( iv0 );
    //  float fv1 = sdf.GetValue( iv1 );
    //  if ( fv0 * fv1 <= 0.0f )
    //  {
    //  }
    //}
#endif
    // @NOTE: Do not clamp position values to voxel boundary. It's valid for a vertex to be placed
    // outside of the voxel is was generated from. This happens when a voxel has all corners inside
    // or outside of the sdf boundary, while also still having intersections (normally two per edge)
    // on one or more edges of the voxel.
    position.x = chunkOffsetX + x + position.x;
    position.y = chunkOffsetY + y + position.y;
    position.z = chunkOffsetZ + z + position.z;
    vertex->position = position;

    // @TODO: Lighting?
    vertex->info[ 0 ] = 0;
    vertex->info[ 1 ] = (uint8_t)( 1.5f ); // @HACK: Lighting values
    vertex->info[ 2 ] = 255;// @HACK: TerrainType( position );
    vertex->info[ 3 ] = 0;

    // Material
    uint8_t material = job->GetMaterial( position, vertex->normal );
    vertex->materials[ 0 ] = ( material == 0 ) ? 255 : 0;
    vertex->materials[ 1 ] = ( material == 1 ) ? 255 : 0;
    vertex->materials[ 2 ] = ( material == 2 ) ? 255 : 0;
    vertex->materials[ 3 ] = ( material == 3 ) ? 255 : 0;
  }

#if AE_TERRAIN_FANCY_NORMALS
  // Generate triangle normals
  for ( uint32_t i = 0; i < tempTris.Length(); i++ )
  {
    TempTri& tri = tempTris[ i ];
    ae::Vec3 p0 = tempVerts[ tri.i[ 0 ] ].v.position;
    ae::Vec3 p1 = tempVerts[ tri.i[ 1 ] ].v.position;
    ae::Vec3 p2 = tempVerts[ tri.i[ 2 ] ].v.position;
    tri.n = ( ( p1 - p0 ) % ( p2 - p0 ) ).SafeNormalizeCopy();
  }
  
  // Split verts based on normal while writing to verticesOut
  VertexCount tempVertCount( 0 );
  {
    struct SplitTri
    {
      SplitTri() : node( this ) {}
      
      TempTri* t;
      uint16_t* i;
      aeListNode< SplitTri > node;
    };
    
    for ( uint32_t vi = 0; vi < tempVerts.Length(); vi++ )
    {
      uint32_t vertTriCount = 0;
      SplitTri vertTris[ 16 ]; // @TODO: What is actual max?
      
      uint32_t normalGroupCount = 0;
      aeList< SplitTri > normalGroups[ countof(vertTris) ];
      
      for ( uint32_t ti = 0; ti < tempTris.Length(); ti++ )
      {
        TempTri& tri = tempTris[ ti ];
        
        int32_t triIndex = -1;
        if ( tri.i[ 0 ] == vi ) { triIndex = 0; }
        else if ( tri.i[ 1 ] == vi ) { triIndex = 1; }
        else if ( tri.i[ 2 ] == vi ) { triIndex = 2; }
        if ( triIndex != -1 )
        {
          // Keep info about each triangle with vert
          AE_ASSERT( vertTriCount < countof(vertTris) );
          SplitTri* splitTri = &vertTris[ vertTriCount ];
          splitTri->t = &tri;
          splitTri->i = &tri.i1[ triIndex ];
          vertTriCount++;
          
          // Create or find a normal group for triangle
          ae::Vec3 n = tri.n;
          aeList< SplitTri >* normalGroup = std::find_if( normalGroups, normalGroups + normalGroupCount, [n]( const auto& normalGroup )
          {
            for ( const SplitTri* tri = normalGroup.GetFirst(); tri; tri = tri->node.GetNext() )
            {
              if ( ( acos( n.Dot( tri->t->n ) ) >= 1.3f ) )
              {
                return true;
              }
            }
            return false;
          } );
          if ( normalGroup == normalGroups + normalGroupCount ) // if end
          {
            normalGroup = &normalGroups[ normalGroupCount ];
            normalGroupCount++;
          }
          AE_ASSERT( normalGroup );
          
          // Add triangle to normal group
          normalGroup->Append( splitTri->node );
        }
      }
      
      for ( uint32_t ni = 0; ni < normalGroupCount; ni++ )
      {
        aeList< SplitTri >& normalGroup = normalGroups[ ni ];
        
        // Calculate average group normal
        ae::Vec3 n( 0.0f );
        for ( const SplitTri* tri = normalGroup.GetFirst(); tri; tri = tri->node.GetNext() )
        {
          n += tri->t->n;
        }
        n.SafeNormalize();
        
        // Store new vertex
        TerrainVertex* v = &verticesOut[ (uint32_t)tempVertCount ];
        *v = tempVerts[ vi ].v;
        v->normal = n;
        
        // Update triangle indices in group with new index
        for ( SplitTri* tri = normalGroup.GetFirst(); tri; tri = tri->node.GetNext() )
        {
          *tri->i = (uint32_t)tempVertCount;
        }
        
        tempVertCount++;
      }
    }
  }

  // Write out triangle indices
  uint32_t tempIndexCount = 0;
  for ( uint32_t ti = 0; ti < tempTris.Length(); ti++ )
  {
    TempTri& tri = tempTris[ ti ];
    indexOut[ tempIndexCount++ ] = tri.i1[ 0 ];
    indexOut[ tempIndexCount++ ] = tri.i1[ 1 ];
    indexOut[ tempIndexCount++ ] = tri.i1[ 2 ];
  }
  
  AE_ASSERT( tempVertCount <= kMaxChunkVerts );
  AE_ASSERT( tempIndexCount <= kMaxChunkIndices );

  *vertexCountOut = tempVertCount;
  *indexCountOut = tempIndexCount;
#else
  // @TODO: Support kChunkCountInterior for raycasting
  AE_ASSERT( vertexCount <= kMaxChunkVerts );
  AE_ASSERT( indexCount <= kMaxChunkIndices );
  *vertexCountOut = vertexCount;
  *indexCountOut = indexCount;
#endif
}

const uint32_t kChunkFormatVersion = 1;
void TerrainChunk::Serialize( ae::BinaryStream* stream )
{
  uint32_t version = kChunkFormatVersion;
  stream->SerializeUint32( version );
  if ( version != kChunkFormatVersion )
  {
    stream->Invalidate();
    return;
  }
  
  AE_FAIL();
//  ae::CollisionMesh::SerializationParams params;
//  params.position = true;
//  params.normal = true;
//  params.uvSets = 0;
//  params.colorSets = 0;
//  params.userDataCount = 0;
//  m_mesh.Serialize( params, stream );
  
  stream->SerializeRaw( m_t, sizeof(m_t) );
  stream->SerializeRaw( m_l, sizeof(m_l) );
  stream->SerializeRaw( m_i, sizeof(m_i) );
  
  //@TODO: m_lightDirty
  
  // @NOTE: Don't modify 'm_geoDirty' because the chunk contents
  // could have changed since the job calling Serialize() has started
  
  // @NOTE: Render data is not initialized here because serialize is called
  // in a separate thread. Render data must be set from main thread.
}

ae::AABB TerrainChunk::GetAABB() const
{
  return GetAABB( m_pos );
}

ae::AABB TerrainChunk::GetAABB( ae::Int3 chunkPos )
{
  ae::Vec3 min( chunkPos * kChunkSize );
  ae::Vec3 max = min + ae::Vec3( (float)kChunkSize );
  return ae::AABB( min, max );
}

void TerrainChunk::m_SetVertexData( const TerrainVertex* verts, const TerrainIndex* indices, VertexCount vertexCount, uint32_t indexCount )
{
  // (Re)Initialize ae::VertexData here only when needed
  if ( m_data.GetIndexCount() == 0 // Not initialized
    || VertexCount( m_data.GetMaxVertexCount() ) < vertexCount // Too little storage for verts
    || m_data.GetMaxIndexCount() < indexCount ) // Too little storage for t_chunkIndices
  {
    m_data.Initialize( sizeof( TerrainVertex ), sizeof( TerrainIndex ), (uint32_t)vertexCount, indexCount, ae::VertexData::Primitive::Triangle, ae::VertexData::Usage::Dynamic, ae::VertexData::Usage::Dynamic );
    m_data.AddAttribute( "a_position", 3, ae::VertexData::Type::Float, offsetof( TerrainVertex, position ) );
    m_data.AddAttribute( "a_normal", 3, ae::VertexData::Type::Float, offsetof( TerrainVertex, normal ) );
    m_data.AddAttribute( "a_info", 4, ae::VertexData::Type::UInt8, offsetof( TerrainVertex, info ) );
    m_data.AddAttribute( "a_materials", 4, ae::VertexData::Type::NormalizedUInt8, offsetof( TerrainVertex, materials ) );
  }

  // Set vertices
  m_data.SetVertices( verts, (uint32_t)vertexCount );
  m_data.SetIndices( indices, indexCount );
  m_data.Upload();
}

//------------------------------------------------------------------------------
// TerrainMember functions
//------------------------------------------------------------------------------
void Terrain::UpdateChunkLighting( TerrainChunk* chunk )
{
//  int32_t cx = chunk->pos[ 0 ] * kChunkSize;
//  int32_t cy = chunk->pos[ 1 ] * kChunkSize;
//  int32_t cz = chunk->pos[ 2 ] * kChunkSize;
//  
//  ae::Vec3 ray[ 9 ];
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
//    ae::Vec3 source( cx + x + 0.5f, cy + y + 0.5f, cz + z + 0.5f );
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

TerrainChunk* Terrain::AllocChunk( ae::Int3 pos )
{
  TerrainChunk* chunk = m_chunkPool.New();
  if ( !chunk )
  {
    return nullptr;
  }
  
  chunk->m_pos = pos;
  chunk->m_lightDirty = true;
  
  //AE_ASSERT( chunk->m_mesh.GetVertexCount() == 0 );
  
  return chunk;
}

void Terrain::FreeChunk( TerrainChunk* chunk )
{
  AE_ASSERT( chunk );
  AE_ASSERT( chunk->m_check == 0xCDCDCDCD );

  // Only clear chunk from world if set (may not be set in the case of a new chunk with zero verts)
  auto iter = m_chunks.find( chunk->GetIndex() );
  if ( iter != m_chunks.end() && iter->second == chunk )
  {
    m_chunks.erase( iter );
  }
  
  chunk->m_mesh.Clear();

  // @NOTE: This has to be done last because CompactingAllocator keeps a pointer to m_vertices
  m_chunkPool.Delete( chunk );
}

void Terrain::m_SetVertexCount( uint32_t chunkIndex, VertexCount count )
{
  AE_ASSERT( count == kChunkCountDirty
    || count == kChunkCountInterior
    || count < kMaxChunkVerts );
  if ( count == kChunkCountEmpty )
  {
    m_vertexCounts.erase( chunkIndex );
  }
  else
  {
    if ( AE_TERRAIN_LOG && count == kChunkCountDirty )
    {
      AE_LOG( "Dirty chunk #", chunkIndex );
    }
    m_vertexCounts[ chunkIndex ] = count;
  }
}

float Terrain::GetChunkScore( ae::Int3 pos ) const
{
  ae::Vec3 chunkCenter = TerrainChunk::GetAABB( pos ).GetCenter();
  float centerDistance = ( m_center - chunkCenter ).Length();

  bool hasNeighbor = false;
  hasNeighbor = hasNeighbor || GetVertexCount( pos + ae::Int3( 1, 0, 0 ) ) > kChunkCountEmpty;
  hasNeighbor = hasNeighbor || GetVertexCount( pos + ae::Int3( 0, 1, 0 ) ) > kChunkCountEmpty;
  hasNeighbor = hasNeighbor || GetVertexCount( pos + ae::Int3( 0, 0, 1 ) ) > kChunkCountEmpty;
  hasNeighbor = hasNeighbor || GetVertexCount( pos + ae::Int3( -1, 0, 0 ) ) > kChunkCountEmpty;
  hasNeighbor = hasNeighbor || GetVertexCount( pos + ae::Int3( 0, -1, 0 ) ) > kChunkCountEmpty;
  hasNeighbor = hasNeighbor || GetVertexCount( pos + ae::Int3( 0, 0, -1 ) ) > kChunkCountEmpty;
  
  // @NOTE: Non-empty chunks are found sooner when chunks with neighbors are prioritized
  if ( hasNeighbor )
  {
    return centerDistance;
  }
  else
  {
    return centerDistance * centerDistance;
  }
}

void TerrainChunk::m_GetQuadVertexOffsetsFromEdge( uint32_t edgeBit, int32_t (&offsets)[ 4 ][ 3 ] )
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

//------------------------------------------------------------------------------
// Terrain chunk member functions
//------------------------------------------------------------------------------
TerrainChunk* Terrain::GetChunk( uint32_t chunkIndex )
{
  return (TerrainChunk*)( (const Terrain*)this )->GetChunk( chunkIndex );
}

TerrainChunk* Terrain::GetChunk( ae::Int3 pos )
{
  return (TerrainChunk*)( (const Terrain*)this )->GetChunk( pos );
}

const TerrainChunk* Terrain::GetChunk( uint32_t chunkIndex ) const
{
  auto iter = m_chunks.find( chunkIndex );
  if ( iter == m_chunks.end() )
  {
    return nullptr;
  }
  
  TerrainChunk* chunk = iter->second;
  AE_ASSERT( chunk->m_check == 0xCDCDCDCD );
  return chunk;
}

const TerrainChunk* Terrain::GetChunk( ae::Int3 pos ) const
{
  return GetChunk( TerrainChunk::GetIndex( pos ) );
}

VertexCount Terrain::GetVertexCount( uint32_t chunkIndex ) const
{
  auto iter = m_vertexCounts.find( chunkIndex );
  return ( iter == m_vertexCounts.end() ) ? kChunkCountEmpty : iter->second;
}

VertexCount Terrain::GetVertexCount( ae::Int3 pos ) const
{
  return GetVertexCount( TerrainChunk::GetIndex( pos ) );
}

//------------------------------------------------------------------------------
// Terrain member functions
//------------------------------------------------------------------------------
Terrain::Terrain() :
  sdf( this )
{}

Terrain::~Terrain()
{
  Terminate();
}

void Terrain::Initialize( uint32_t maxThreads, bool render )
{
  // @NOTE: This doesn't handle the case where enough verts are split
  // that the count surpasses TerrainIndex max.
  AE_ASSERT( kMaxChunkVerts <= VertexCount( ae::MaxValue< TerrainIndex >() ) );

  m_render = render;

  //m_chunkPool.Initialize(); @TODO: Reset pool
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockCollision[ i ] = true; }
  m_blockCollision[ Block::Exterior ] = false;
  m_blockCollision[ Block::Unloaded ] = false;
  
  for ( uint32_t i = 0; i < Block::COUNT; i++) { m_blockDensity[ i ] = 1.0f; }

  m_threadPool = ae::New< ctpl::thread_pool >( AE_ALLOC_TAG_TERRAIN, maxThreads );
  maxThreads = ae::Max( 1u, maxThreads );
  for ( uint32_t i = 0; i < maxThreads; i++ )
  {
    m_terrainJobs.Append( ae::New< TerrainJob >( AE_ALLOC_TAG_TERRAIN ) );
  }
}

void Terrain::Terminate()
{
  if ( m_threadPool )
  {
    m_threadPool->stop( true );
    ae::Delete( m_threadPool );
    m_threadPool = nullptr;
  }

  for ( uint32_t i = 0; i < m_terrainJobs.Length(); i++ )
  {
    TerrainJob* job = m_terrainJobs[ i ];
    if ( job->IsPendingFinish() )
    {
      job->Finish();
    }
    ae::Delete( job );
  }
  m_terrainJobs.Clear();

  for ( TerrainChunk* chunk = m_chunkPool.GetFirst(); chunk; chunk = m_chunkPool.GetNext( chunk ) )
  {
    FreeChunk( chunk );
  }
  AE_ASSERT( m_chunkPool.Length() == 0 );
}

void Terrain::Update( ae::Vec3 center, float radius )
{
  int32_t chunkViewRadius = radius / kChunkSize;
  const int32_t kChunkViewDiam = chunkViewRadius + chunkViewRadius;

  double currentTime = ae::GetTime();

  m_center = center;
  m_radius = radius;

  //------------------------------------------------------------------------------
  // Dirty the chunks containing sdf shapes that have been modified
  //------------------------------------------------------------------------------
  // @TODO: This isn't entirely thread safe... The memory location is guaranteed to
  // be valid, but it's possible for values to change while another thread is reading
  // them. It's possible these shapes could be accessed from a job. They should
  // probably be duplicated and given to the job when it starts.
  for ( uint32_t i = 0; i < sdf.GetShapeCount(); i++ )
  {
    Sdf* shape = sdf.GetShapeAtIndex( i );
    if ( shape->m_dirty )
    {
      m_Dirty( shape->m_aabbPrev );
      m_Dirty( shape->GetAABB() );

      shape->m_dirty = false;
      shape->m_aabbPrev = shape->GetAABB();
    }
  }
  
  //------------------------------------------------------------------------------
  // Determine which chunks will be processed
  //------------------------------------------------------------------------------
  //t_chunkMap.Clear();
  //t_chunkMap.Reserve( kChunkViewDiam * kChunkViewDiam * kChunkViewDiam );
  t_chunkMap_hack.clear();
  ae::Int3 viewChunk = ( m_center / kChunkSize ).NearestCopy();
  for ( int32_t k = 0; k < kChunkViewDiam; k++ )
  {
    for ( int32_t j = 0; j < kChunkViewDiam; j++ )
    {
      for ( int32_t i = 0; i < kChunkViewDiam; i++ )
      {
        ae::Int3 chunkPos( i, j, k );
        chunkPos -= ae::Int3( chunkViewRadius );
        chunkPos += viewChunk;

        ae::Vec3 chunkCenter = TerrainChunk::GetAABB( chunkPos ).GetCenter();
        float centerDistance = ( m_center - chunkCenter ).Length();
        if ( centerDistance >= radius )
        {
          continue;
        }

        uint32_t ci = TerrainChunk::GetIndex( chunkPos );
        VertexCount vc = GetVertexCount( ci );
        if ( vc == kChunkCountEmpty || vc == kChunkCountInterior )
        {
          continue;
        }

        if ( AE_TERRAIN_LOG )
        {
          AE_LOG( "p:# ci:# vc:#", chunkPos, ci, vc );
        }

        TerrainChunk* c = GetChunk( ci ); // @TODO: Is this needed with step below?
        if ( c )
        {
          AE_ASSERT( vc <= kMaxChunkVerts );
          //AE_ASSERT( c->m_mesh.GetVertexCount() );
        }

        //ChunkSort& chunkSort = t_chunkMap.Set( chunkPos, ChunkSort() );
        ChunkSort chunkSort;
        chunkSort.c = c; // @TODO: Is this needed with step below?
        chunkSort.pos = chunkPos;
        chunkSort.score = GetChunkScore( chunkPos );
        t_chunkMap_hack[ TerrainChunk::GetIndex( chunkPos ) ] = chunkSort;
      }
    }
  }
  // Add all currently generated chunks
  for ( TerrainChunk* c = m_generatedList.GetFirst(); c; c = c->m_generatedList.GetNext() )
  {
    ae::Int3 pos = c->m_pos;
//#if _AE_DEBUG_
//    const ChunkSort* check = t_chunkMap.TryGet( pos );
//    AE_ASSERT( check->c == c );
//#endif

    //ChunkSort& chunkSort = t_chunkMap.Set( pos, ChunkSort() );
    ChunkSort chunkSort;
    chunkSort.c = c;
    chunkSort.pos = pos;
    chunkSort.score = GetChunkScore( pos );
    t_chunkMap_hack[ TerrainChunk::GetIndex( pos ) ] = chunkSort;
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
  for ( auto& element : t_chunkMap_hack )
  {
    t_chunkSorts.Append( element.second );

    if ( AE_TERRAIN_LOG )
    {
      AE_LOG( "p:# s:# c:#", element.second.pos, element.second.score, element.second.c );
    }
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

      ae::Int3 chunkPos = sort->pos;
      int32_t otherJobIndex = m_terrainJobs.FindFn( [chunkPos]( TerrainJob* j )
      {
        return j->HasChunk( chunkPos );
      } );

      const char* status;
      if ( sort->c )
      {
        if ( otherJobIndex >= 0 )
        {
          status = "refreshing";
        }
        else
        {
          status = "generated";
        }
      }
      else
      {
        status = "pending";
      }

      ae::Str128 str = ae::Str128::Format( "pos:#\nindex:#\nscore:#\nstatus:#",
        chunkPos,
        TerrainChunk::GetIndex( chunkPos ),
        sort->score,
        status
      );

      ae::Vec3 center = TerrainChunk::GetAABB( sort->pos ).GetCenter();
      m_debugTextFn( center, str.c_str() );
    }
  }
  
  //------------------------------------------------------------------------------
  // Finish terrain jobs
  // @NOTE: Do this as late as possible so jobs can run while sorting is happening
  //------------------------------------------------------------------------------
  for ( uint32_t i = 0; i < m_terrainJobs.Length(); i++ )
  {
    TerrainJob* job = m_terrainJobs[ i ];
    if ( !job->IsPendingFinish() )
    {
      continue;
    }
    
    TerrainChunk* newChunk = job->GetChunk();
    AE_ASSERT( newChunk );
    AE_ASSERT( newChunk->m_check == 0xCDCDCDCD );
    uint32_t chunkIndex = newChunk->GetIndex();
    if ( AE_TERRAIN_LOG )
    {
      AE_LOG( "Finish terrain job # #", newChunk->GetIndex(), newChunk->GetAABB() );
    }

    TerrainChunk* oldChunk = GetChunk( chunkIndex );

    VertexCount vertexCount = job->GetVertexCount();
    AE_ASSERT( vertexCount <= kMaxChunkVerts );
    if ( vertexCount == kChunkCountEmpty
      || vertexCount == kChunkCountInterior )
    {
      // @NOTE: It's super expensive to finally just throw away the unneeded chunk...
      FreeChunk( newChunk );
      newChunk = nullptr;
    }
    else
    {
      if ( m_render )
      {
        newChunk->m_SetVertexData( job->GetVertices(), job->GetIndices(), vertexCount, job->GetIndexCount() );
      }

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
    m_SetVertexCount( chunkIndex, vertexCount );
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
      m_chunks.erase( chunkIndex );
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
    ae::Int3 chunkPos = chunkSort->pos;
    TerrainChunk* chunk = chunkSort->c;
    uint32_t chunkIndex = TerrainChunk::GetIndex( chunkPos );
    TerrainChunk* indexPosChunk = GetChunk( chunkIndex );
    if ( chunk )
    {
      VertexCount voxelCounts = GetVertexCount( chunkIndex );
      AE_ASSERT_MSG( indexPosChunk == chunk, "# #", indexPosChunk, chunk );
      AE_ASSERT_MSG( voxelCounts != kChunkCountDirty, "Chunk already existed, but had an invalid value" );
      AE_ASSERT( voxelCounts <= kMaxChunkVerts );
      //AE_ASSERT( chunk->m_mesh.GetVertexCount() );
      if ( m_render )
      {
        AE_ASSERT( chunk->m_data.GetVertexCount() );
        AE_ASSERT( chunk->m_data.GetVertexSize() );
      }
    }
    else
    {
      // @TODO: Chunks are always allocated below. What is this line for?
      // @NOTE: Grab any chunks that were finished generating after sorting completed
      chunk = indexPosChunk;
    }

    if( !chunk || chunk->m_geoDirty )
    {
      if ( m_threadPool->n_idle() == 0 && m_threadPool->size() > 0 )
      {
        break;
      }

      int32_t jobIndex = m_terrainJobs.FindFn( []( TerrainJob* j ) { return !j->HasJob(); } );
      if ( jobIndex < 0 )
      {
        break;
      }

      int32_t otherJobIndex = m_terrainJobs.FindFn( [chunkPos]( TerrainJob* j ) { return j->HasChunk( chunkPos ); } );
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
      chunk = AllocChunk( chunkPos );
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

              chunk = AllocChunk( chunkPos );
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
        if ( AE_TERRAIN_LOG )
        {
          AE_LOG( "Chunk loading reached equilibrium" );
        }
        break;
      }
      if ( AE_TERRAIN_LOG )
      {
        AE_LOG( "Start terrain job # #", chunk->GetIndex(), chunk->GetAABB() );
      }

      TerrainJob* job = m_terrainJobs[ jobIndex ];
      AE_ASSERT( job );
      job->StartNew( m_params, &sdf, chunk );
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

  if ( m_params.debug )
  {
    for ( uint32_t i = 0; i < m_terrainJobs.Length(); i++ )
    {
      const TerrainJob* job = m_terrainJobs[ i ];
      if ( job->HasJob() )
      {
        ae::AABB chunkAABB = job->GetChunk()->GetAABB();
        m_params.debug->AddLine( m_center, chunkAABB.GetCenter(), ae::Color::Red() );
        m_params.debug->AddAABB( chunkAABB.GetCenter(), chunkAABB.GetHalfSize(), ae::Color::PicoRed() );
      }
    }

    sdf.RenderDebug( m_params.debug );
  }
}

void Terrain::Render( const ae::Shader* shader, const ae::UniformList& shaderParams )
{
  if ( !m_render )
  {
    return;
  }

  //aeFrustum frustum;
  uint32_t activeCount = 0;
  for( uint32_t i = 0; i < t_chunkSorts.Length() && activeCount < kMaxActiveChunks; i++ )
  {
    TerrainChunk* chunk = t_chunkSorts[ i ].c;
    if ( !chunk )
    {
      continue;
    }
    uint32_t index = chunk->GetIndex();
    VertexCount vertexCount = GetVertexCount( index );
    AE_ASSERT( chunk->m_check == 0xCDCDCDCD );
    AE_ASSERT_MSG( vertexCount > kChunkCountEmpty, "vertex count: # index: #", vertexCount, index );
    AE_ASSERT( chunk->m_data.GetVertexCount() );
    AE_ASSERT( chunk->m_data.GetVertexSize() );
    
    // Only render the visible chunks
    //if( frustum.TestChunk( chunk ) ) // @TODO: Should make sure chunk is visible
    {
      chunk->m_data.Render( shader, shaderParams );
      activeCount++;
    }
  }

  if ( AE_TERRAIN_LOG )
  {
    AE_LOG( "chunks active:# allocated:#", activeCount, m_chunkPool.Length() );
  }
}

void Terrain::SetParams( const TerrainParams& params )
{
  bool dirty = m_params.GetHash() != params.GetHash();
  m_params = params;
  
  if ( dirty )
  {
    for ( TerrainChunk* chunk = m_chunkPool.GetFirst(); chunk; chunk = m_chunkPool.GetNext( chunk ) )
    {
      chunk->m_geoDirty = true;
    }
  }
}

void Terrain::GetParams( TerrainParams* outParams )
{
  if ( outParams )
  {
    *outParams = m_params;
  }
}

void Terrain::m_Dirty( ae::AABB aabb )
{
  // @NOTE: Add a buffer region so voxels on the edge of the aabb are refreshed
  aabb.Expand( kSdfBoundary );

  ae::Int3 minChunk = ( aabb.GetMin() / kChunkSize ).FloorCopy();
  ae::Int3 maxChunk = ( aabb.GetMax() / kChunkSize ).CeilCopy();
  if ( AE_TERRAIN_LOG )
  {
    AE_LOG( "Dirty area # (min:# max:#)", aabb, minChunk, maxChunk );
  }

  for ( int32_t z = minChunk.z; z < maxChunk.z; z++ )
  for ( int32_t y = minChunk.y; y < maxChunk.y; y++ )
  for ( int32_t x = minChunk.x; x < maxChunk.x; x++ )
  {
    ae::Int3 pos( x, y, z );
    if ( TerrainChunk* chunk = GetChunk( pos ) )
    {
      chunk->m_geoDirty = true;
    }
    else
    {
      if ( AE_TERRAIN_LOG )
      {
        AE_LOG( "Dirty chunk #", pos );
      }
      m_SetVertexCount( TerrainChunk::GetIndex( pos ), kChunkCountDirty );
    }
  }
}

bool Terrain::GetCollision( int32_t x, int32_t y, int32_t z ) const
{
  return m_blockCollision[ GetVoxel( x, y, z ) ];
}

bool Terrain::GetCollision( ae::Vec3 position ) const
{
  Block::Type type = GetVoxel(
    ae::Floor( position.x ),
    ae::Floor( position.y ),
    ae::Floor( position.z )
  );
  return m_blockCollision[ type ];
}

Block::Type Terrain::GetVoxel( ae::Vec3 position ) const
{
  return GetVoxel(
    ae::Floor( position.x ),
    ae::Floor( position.y ),
    ae::Floor( position.z )
  );
}

Block::Type Terrain::GetVoxel( int32_t x, int32_t y, int32_t z ) const
{
  ae::Int3 chunkPos, localPos;
  TerrainChunk::GetPosFromWorld( ae::Int3( x, y, z ), &chunkPos, &localPos );
  uint32_t ci = TerrainChunk::GetIndex( chunkPos );

  VertexCount vc = GetVertexCount( ci );
  if ( vc == kChunkCountEmpty ) { return Block::Exterior; }
  if ( vc == kChunkCountDirty ) { return Block::Unloaded; }
  if ( vc == kChunkCountInterior ) { return Block::Interior; }

  const TerrainChunk* chunk = GetChunk( ci );
  if ( !chunk ) { return Block::Unloaded; }
  return chunk->m_t[ localPos.x ][ localPos.y ][ localPos.z ];
}

//bool Terrain::m_GetVertex( int32_t x, int32_t y, int32_t z, TerrainVertex* outVertex ) const
//{
//  ae::Int3 chunkPos, localPos;
//  TerrainChunk::GetPosFromWorld( ae::Int3( x, y, z ), &chunkPos, &localPos );
//
//  uint32_t ci = TerrainChunk::GetIndex( chunkPos );
//
//  VertexCount vc = GetVertexCount( ci );
//  if ( vc == kChunkCountEmpty ) { return false; }
//  if ( vc == kChunkCountDirty ) { return false; }
//  if ( vc == kChunkCountInterior ) { return false; }
//
//  const TerrainChunk* chunk = GetChunk( ci );
//  if ( !chunk )
//  {
//    return false;
//  }
//
//  TerrainIndex index = chunk->m_i[ localPos.x ][ localPos.y ][ localPos.z ];
//  if ( index == kInvalidTerrainIndex )
//  {
//    return false;
//  }
//
//  if ( outVertex )
//  {
//    ae::CollisionMesh::Vertex vert = chunk->m_mesh.GetVertex( index );
//    outVertex->position = vert.position.GetXYZ();
//    outVertex->normal = vert.normal.GetXYZ();
//  }
//  return true;
//}

aeFloat16 Terrain::GetLight( int32_t x, int32_t y, int32_t z ) const
{
  const TerrainChunk* chunk = GetChunk( ae::Int3( x, y, z ) );
  if ( chunk == nullptr )
  {
    return kSkyBrightness;
  }

  x = ae::Mod( x, (int32_t)kChunkSize );
  y = ae::Mod( y, (int32_t)kChunkSize );
  z = ae::Mod( z, (int32_t)kChunkSize );
  return chunk->m_l[ x ][ y ][ z ];
}

//------------------------------------------------------------------------------
// Terrain raycast helpers
//------------------------------------------------------------------------------
namespace
{
  class DebugRay
  {
  public:
    DebugRay( ae::Vec3 start, ae::Vec3 ray, ae::DebugLines* debug ) :
      start( start ),
      ray( ray ),
      debug( debug )
    {}

    ~DebugRay()
    {
      if ( debug )
      {
        debug->AddLine( start, start + ray, color );
      }
    }

    ae::Vec3 start, ray;
    ae::DebugLines* debug;
    ae::Color color = ae::Color::Red();
  };

  ae::Vec3 IntersectRayAABB( ae::Vec3 p, ae::Vec3 d, ae::Int3 v )
  {
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();
    for ( int i = 0; i < 3; i++ )
    {
      if ( fabs( d[ i ] ) < 0.001f )
      {
        continue;
      }

      float ood = 1.0f / d[ i ];
      float t1 = ( v[ i ] - p[ i ] ) * ood;
      float t2 = ( v[ i ] + 1 - p[ i ] ) * ood;

      if ( t1 > t2 )
      {
        std::swap( t1, t2 );
      }

      if ( t1 > tmin )
      {
        tmin = t1;
      }

      if ( t2 > tmax )
      {
        tmax = t2;
      }
    }
    return p + d * tmin;
  }
}

//------------------------------------------------------------------------------
// Terrain raycast functions
//------------------------------------------------------------------------------
bool Terrain::VoxelRaycast( ae::Vec3 start, ae::Vec3 ray, int32_t minSteps ) const
{
  DebugRay debugRay( start, ray, m_params.debug );

  int32_t x = ae::Floor( start.x );
  int32_t y = ae::Floor( start.y );
  int32_t z = ae::Floor( start.z );
  
  if ( ray.LengthSquared() < 0.001f )
  {
    return false;
  }
  ae::Vec3 dir = ray.SafeNormalizeCopy();
  
  ae::Vec3 curpos = start;
  ae::Vec3 cb, tmax, tdelta;
  int stepX, outX;
  int stepY, outY;
  int stepZ, outZ;
  if ( dir.x > 0 )
  {
    stepX = 1;
    outX = ceil( start.x + ray.x );
    //outX = ae::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX ); // @TODO: Use terrain bounds
    cb.x = x + 1;
  }
  else
  {
    stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    outX = ae::Max( -1, outX );
    cb.x = x;
  }

  if ( dir.y > 0.0f )
  {
    stepY = 1;
    outY = ceil( start.y + ray.y );
    //outY = ae::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY ); // @TODO: Use terrain bounds
    cb.y = y + 1;
  }
  else 
  {
    stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    outY = ae::Max( -1, outY );
    cb.y = y;
  }

  if ( dir.z > 0.0f )
  {
    stepZ = 1;
    outZ = ceil( start.z + ray.z );
    //outZ = ae::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ ); // @TODO: Use terrain bounds
    cb.z = z + 1;
  }
  else 
  {
    stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    //outZ = ae::Max( -1, outZ );
    cb.z = z;
  }

  if ( dir.x != 0 )
  {
    float rxr = 1.0f / dir.x;
    tmax.x = (cb.x - curpos.x) * rxr; 
    tdelta.x = stepX * rxr;
  }
  else
  {
    tmax.x = 1000000;
  }

  if ( dir.y != 0 )
  {
    float ryr = 1.0f / dir.y;
    tmax.y = (cb.y - curpos.y) * ryr; 
    tdelta.y = stepY * ryr;
  }
  else
  {
    tmax.y = 1000000;
  }

  if ( dir.z != 0 )
  {
    float rzr = 1.0f / dir.z;
    tmax.z = (cb.z - curpos.z) * rzr; 
    tdelta.z = stepZ * rzr;
  }
  else
  {
    tmax.z = 1000000;
  }
  
  int32_t steps = 0;
  while ( !GetCollision( x, y, z ) || steps < minSteps )
  {
    if ( m_params.debug )
    {
      ae::Vec3 v = ae::Vec3( x, y, z ) + ae::Vec3( 0.5f );
      m_params.debug->AddOBB( ae::Matrix4::Translation( v ), ae::Color::Blue() );
    }

    steps++;
    
    if ( tmax.x < tmax.y )
    {
      if ( tmax.x < tmax.z )
      {
        x = x + stepX;
        if ( x == outX )
        {
          return false;
        }
        tmax.x += tdelta.x;
      }
      else
      {
        z = z + stepZ;
        if ( z == outZ )
        {
          return false;
        }
        tmax.z += tdelta.z;
      }
    }
    else
    {
      if ( tmax.y < tmax.z )
      {
        y = y + stepY;
        if ( y == outY )
        {
          return false;
        }
        tmax.y += tdelta.y;
      }
      else
      {
        z = z + stepZ;
        if ( z == outZ )
        {
          return false;
        }
        tmax.z += tdelta.z;
      }
    }
  }

  if ( m_params.debug )
  {
    ae::Vec3 v = ae::Vec3( x, y, z ) + ae::Vec3( 0.5f );
    m_params.debug->AddOBB( ae::Matrix4::Translation( v ), ae::Color::Green() );
    debugRay.color = ae::Color::Green();
  }

  return true;
}

//TerrainRaycastResult Terrain::RaycastFast( ae::Vec3 start, ae::Vec3 ray, bool allowSourceCollision ) const
//{
//  DebugRay debugRay( start, ray, m_params.debug );
//
//  TerrainRaycastResult result;
//  result.hit = false;
//  result.type = Block::Exterior;
//  result.distance = std::numeric_limits<float>::infinity();
//  result.posi = ae::Int3( 0 );
//  result.posf = ae::Vec3( std::numeric_limits<float>::infinity() );
//  result.normal = ae::Vec3( std::numeric_limits<float>::infinity() );
//  result.touchedUnloaded = false;
//  
//  int32_t x = ae::Floor( start.x );
//  int32_t y = ae::Floor( start.y );
//  int32_t z = ae::Floor( start.z );
//  
//  if ( ray.LengthSquared() < 0.001f )
//  {
//    return result;
//  }
//  ae::Vec3 dir = ray.SafeNormalizeCopy();
//  
//  ae::Vec3 curpos = start;
//  ae::Vec3 cb, tmax, tdelta;
//  int32_t stepX, outX;
//  int32_t stepY, outY;
//  int32_t stepZ, outZ;
//  if ( dir.x > 0 )
//  {
//    stepX = 1;
//    outX = ceil( start.x + ray.x );
//    //outX = ae::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX ); // @TODO: Use terrain bounds
//    cb.x = x + 1;
//  }
//  else 
//  {
//    stepX = -1;
//    outX = (int32_t)( start.x + ray.x ) - 1;
//    //outX = ae::Max( -1, outX );
//    cb.x = x;
//  }
//  if ( dir.y > 0.0f )
//  {
//    stepY = 1;
//    outY = ceil( start.y + ray.y );
//    //outY = ae::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY ); // @TODO: Use terrain bounds
//    cb.y = y + 1;
//  }
//  else 
//  {
//    stepY = -1;
//    outY = (int32_t)( start.y + ray.y ) - 1;
//    //outY = ae::Max( -1, outY );
//    cb.y = y;
//  }
//  if ( dir.z > 0.0f )
//  {
//    stepZ = 1;
//    outZ = ceil( start.z + ray.z );
//    //outZ = ae::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ ); // @TODO: Use terrain bounds
//    cb.z = z + 1;
//  }
//  else 
//  {
//    stepZ = -1;
//    outZ = (int32_t)( start.z + ray.z ) - 1;
//    //outZ = ae::Max( -1, outZ );
//    cb.z = z;
//  }
//  float rxr, ryr, rzr;
//  if ( dir.x != 0 )
//  {
//    rxr = 1.0f / dir.x;
//    tmax.x = (cb.x - curpos.x) * rxr; 
//    tdelta.x = stepX * rxr;
//  }
//  else tmax.x = 1000000;
//  if ( dir.y != 0 )
//  {
//    ryr = 1.0f / dir.y;
//    tmax.y = (cb.y - curpos.y) * ryr; 
//    tdelta.y = stepY * ryr;
//  }
//  else tmax.y = 1000000;
//  if ( dir.z != 0 )
//  {
//    rzr = 1.0f / dir.z;
//    tmax.z = (cb.z - curpos.z) * rzr; 
//    tdelta.z = stepZ * rzr;
//  }
//  else tmax.z = 1000000;
//  
//  while ( ( result.type = GetVoxel( x, y, z ) ) != Block::Surface || !allowSourceCollision )
//  {
//    if( result.type == Block::Unloaded )
//    {
//      result.touchedUnloaded = true;
//    }
//      
//    allowSourceCollision = true;
//    
//    if (tmax.x < tmax.y)
//    {
//      if (tmax.x < tmax.z)
//      {
//        x = x + stepX;
//        if ( x == outX )
//        {
//          return result;
//        }
//        tmax.x += tdelta.x;
//      }
//      else
//      {
//        z = z + stepZ;
//        if ( z == outZ )
//        {
//          return result;
//        }
//        tmax.z += tdelta.z;
//      }
//    }
//    else
//    {
//      if (tmax.y < tmax.z)
//      {
//        y = y + stepY;
//        if ( y == outY )
//        {
//          return result;
//        }
//        tmax.y += tdelta.y;
//      }
//      else
//      {
//        z = z + stepZ;
//        if ( z == outZ )
//        {
//          return result;
//        }
//        tmax.z += tdelta.z;
//      }
//    }
//  }
//  
//  AE_ASSERT( result.type != Block::Exterior && result.type != Block::Unloaded );
//  
//  result.hit = true;
//  result.posi = ae::Int3( x, y, z );
//  
//  ae::Int3 chunkPos, localPos;
//  TerrainChunk::GetPosFromWorld( result.posi, &chunkPos, &localPos );
//  const TerrainChunk* chunk = GetChunk( chunkPos );
//  AE_ASSERT( chunk );
//
//  TerrainIndex index = chunk->m_i[ localPos.x ][ localPos.y ][ localPos.z ];
//  // TODO Can somehow skip surface and hit interior cell
//  AE_ASSERT( index != kInvalidTerrainIndex );
//  ae::CollisionMesh::Vertex vert = chunk->m_mesh.GetVertex( index );
//  ae::Vec3 p = vert.position.GetXYZ();
//  ae::Vec3 n = vert.normal.GetXYZ().SafeNormalizeCopy(); // @TODO: Use sdf gradient, since verts can have multiple
//  ae::Vec3 r = ray.SafeNormalizeCopy();
//  float t = n.Dot( p - start ) / n.Dot( r );
//  result.distance = t;
//  result.posf = start + r * t;
//  result.normal = n;
//
//  // Debug
//  if ( m_params.debug )
//  {
//    m_params.debug->AddCircle( result.posf, result.normal, 0.25f, ae::Color::Green(), 16 );
//    m_params.debug->AddLine( result.posf, result.posf + result.normal, ae::Color::Green() );
//
//    ae::Vec3 v = ae::Vec3( x, y, z ) + ae::Vec3( 0.5f );
//    m_params.debug->AddOBB( ae::Matrix4::Translation( v ), ae::Color::Green() );
//
//    m_params.debug->AddSphere( p, 0.05f, ae::Color::Green(), 8 );
//    m_params.debug->AddLine( p, p + n, ae::Color::Green() );
//    if ( m_debugTextFn )
//    {
//      ae::Str64 str = ae::Str64::Format( "#: # (#)", index, vert.position, localPos );
//      m_debugTextFn( p, str.c_str() );
//    }
//
//    debugRay.color = ae::Color::Green();
//  }
//  
//  return result;
//}

bool Terrain::Raycast( const ae::CollisionMesh::RaycastParams& _params, ae::CollisionMesh::RaycastResult* outResult ) const
{
  ae::Vec3 start = _params.source;
  ae::Vec3 ray = _params.ray;
  ae::Vec3 dir = ray;
  float length = dir.SafeNormalize();
  DebugRay debugRay( start, ray, m_params.debug );
  
  if ( length < 0.001f )
  {
    return false;
  }
  
  ae::CollisionMesh::RaycastParams params = _params;
  if ( !params.debug )
  {
    params.debug = m_params.debug;
  }
  
  start /= kChunkSize;
  ray /= kChunkSize;
  
  int32_t x = ae::Floor( start.x );
  int32_t y = ae::Floor( start.y );
  int32_t z = ae::Floor( start.z );
  
  ae::Vec3 curpos = start;
  ae::Vec3 cb, tmax, tdelta;
  int32_t stepX, outX;
  int32_t stepY, outY;
  int32_t stepZ, outZ;
  if ( dir.x > 0 )
  {
    stepX = 1;
    outX = ceil( start.x + ray.x );
//    outX = ae::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outX ); // @TODO: Use terrain bounds
    cb.x = x + 1;
  }
  else
  {
    stepX = -1;
    outX = (int32_t)( start.x + ray.x ) - 1;
    //outX = ae::Max( -1, outX ); // @TODO: Use terrain bounds
    cb.x = x;
  }

  if ( dir.y > 0.0f )
  {
    stepY = 1;
    outY = ceil( start.y + ray.y );
//    outY = ae::Min( (int32_t)( kWorldChunksWidth * kChunkSize - 1 ), outY ); // @TODO: Use terrain bounds
    cb.y = y + 1;
  }
  else
  {
    stepY = -1;
    outY = (int32_t)( start.y + ray.y ) - 1;
    //outY = ae::Max( -1, outY ); // @TODO: Use terrain bounds
    cb.y = y;
  }

  if ( dir.z > 0.0f )
  {
    stepZ = 1;
    outZ = ceil( start.z + ray.z );
//    outZ = ae::Min( (int32_t)( kWorldChunksHeight * kChunkSize - 1 ), outZ ); // @TODO: Use terrain bounds
    cb.z = z + 1;
  }
  else
  {
    stepZ = -1;
    outZ = (int32_t)( start.z + ray.z ) - 1;
    //outZ = ae::Max( -1, outZ ); // @TODO: Use terrain bounds
    cb.z = z;
  }

  if ( dir.x != 0 )
  {
    float rxr = 1.0f / dir.x;
    tmax.x = (cb.x - curpos.x) * rxr;
    tdelta.x = stepX * rxr;
  }
  else
  {
    tmax.x = 1000000;
  }

  if ( dir.y != 0 )
  {
    float ryr = 1.0f / dir.y;
    tmax.y = (cb.y - curpos.y) * ryr;
    tdelta.y = stepY * ryr;
  }
  else
  {
    tmax.y = 1000000;
  }

  if ( dir.z != 0 )
  {
    float rzr = 1.0f / dir.z;
    tmax.z = (cb.z - curpos.z) * rzr;
    tdelta.z = stepZ * rzr;
  }
  else
  {
    tmax.z = 1000000;
  }

  ae::CollisionMesh::RaycastResult resultsAccum;
  while ( true )
  {
    const TerrainChunk* chunk = GetChunk( ae::Int3( x, y, z ) );
    if ( chunk )
    {
      resultsAccum = chunk->m_mesh.Raycast( params, resultsAccum );
    }
    
    if ( tmax.x < tmax.y )
    {
      if ( tmax.x < tmax.z )
      {
        x = x + stepX;
        if ( x == outX )
        {
          break;
        }
        tmax.x += tdelta.x;
      }
      else
      {
        z = z + stepZ;
        if ( z == outZ )
        {
          break;
        }
        tmax.z += tdelta.z;
      }
    }
    else
    {
      if ( tmax.y < tmax.z )
      {
        y = y + stepY;
        if ( y == outY )
        {
          break;
        }
        tmax.y += tdelta.y;
      }
      else
      {
        z = z + stepZ;
        if ( z == outZ )
        {
          break;
        }
        tmax.z += tdelta.z;
      }
    }
  }

  if ( outResult )
  {
    *outResult = resultsAccum;
  }
  return resultsAccum.hits.Length();
}

ae::CollisionMesh::PushOutInfo Terrain::PushOutSphere( const ae::CollisionMesh::PushOutParams& _params, const ae::CollisionMesh::PushOutInfo& info ) const
{
  ae::CollisionMesh::PushOutParams params = _params;
  params.transform = ae::Matrix4::Identity();
  
  ae::Int3 sphereMin, sphereMax;
  ae::AABB sphereAABB( info.sphere );
  TerrainChunk::GetPosFromWorld( sphereAABB.GetMin().FloorCopy(), &sphereMin, nullptr );
  TerrainChunk::GetPosFromWorld( sphereAABB.GetMax().CeilCopy(), &sphereMax, nullptr );
  
  ae::CollisionMesh::PushOutInfo results = info;
  for ( int32_t z = sphereMin.z; z <= sphereMax.z; z++ )
  for ( int32_t y = sphereMin.y; y <= sphereMax.y; y++ )
  for ( int32_t x = sphereMin.x; x <= sphereMax.x; x++ )
  {
    if ( const TerrainChunk* chunk = GetChunk( ae::Int3( x, y, z ) ) )
    {
      results = chunk->m_mesh.PushOut( params, results );
    }
  }
  
  return results;
}

} // ae
