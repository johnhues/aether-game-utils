//------------------------------------------------------------------------------
// aeTerrain.h
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
#ifndef AETERRAIN_H
#define AETERRAIN_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include <atomic>
#include "aeCompactingAllocator.h"
#include "aeImage.h"
#include "aeList.h"
#include "aeMath.h"
#include "aeMesh.h"
#include "aeObjectPool.h"
#include "aeRender.h"
#include <map>

//------------------------------------------------------------------------------
// aeUnit
//------------------------------------------------------------------------------
template < typename T >
class aeUnit
{
public:
  aeUnit() : m_v() { AE_STATIC_ASSERT( sizeof(*this) == sizeof(T) ); }
  aeUnit( const aeUnit& o ) : m_v( o.m_v ) {}
  explicit aeUnit( const T& vertexCount ) : m_v( vertexCount ) {}

  template < typename U >
  explicit operator U () const { return (U)m_v; }

  bool operator == ( const aeUnit& o ) const { return m_v == o.m_v; }
  bool operator != ( const aeUnit& o ) const { return m_v != o.m_v; }
  bool operator < ( const aeUnit& o ) const { return m_v < o.m_v; }
  bool operator > ( const aeUnit& o ) const { return m_v > o.m_v; }
  bool operator <= ( const aeUnit& o ) const { return m_v <= o.m_v; }
  bool operator >= ( const aeUnit& o ) const { return m_v >= o.m_v; }
  
  aeUnit< T > operator + ( const aeUnit& v ) const { return aeUnit( m_v + v.m_v ); }
  aeUnit< T > operator - ( const aeUnit& v ) const { return aeUnit( m_v - v.m_v ); }
  aeUnit< T >& operator ++ () { m_v++; return *this; }
  aeUnit< T >& operator -- () { m_v--; return *this; }
  aeUnit< T > operator ++ ( int ) { aeUnit< T > temp = *this; ++*this; return temp; }
  aeUnit< T > operator -- ( int ) { aeUnit< T > temp = *this; --*this; return temp; }
  aeUnit< T >& operator += ( const T& v ) { m_v += v; return *this; }
  aeUnit< T >& operator -= ( const T& v ) { m_v -= v; return *this; }
  
  T& Get() { return m_v; }
  T Get() const { return m_v; }

private:
  operator bool () const = delete;
  T m_v;
};

template < typename T >
inline std::ostream& operator<<( std::ostream& os, const aeUnit< T >& u )
{
  return os << (T)u;
}

namespace ctpl
{
  class thread_pool;
}

typedef float aeFloat16;
class aeDebugRender;

//------------------------------------------------------------------------------
// ae
//------------------------------------------------------------------------------
namespace ae {

//------------------------------------------------------------------------------
// Terrain types
//------------------------------------------------------------------------------
typedef uint8_t TerrainMaterialId;

struct TerrainVertex
{
  aeFloat3 position;
  aeFloat3 normal;
  uint8_t materials[ 4 ];
  uint8_t info[ 4 ];
};

typedef aeUnit< uint32_t > VertexCount;
const VertexCount kChunkCountInterior = VertexCount( ~0 - 1 ); // Whole chunk is solid. Used for raycasting
const VertexCount kChunkCountDirty = VertexCount( ~0 ); // Flags chunks that should be (re)generated
const VertexCount kChunkCountEmpty = VertexCount( 0 );

typedef uint16_t TerrainIndex;
const TerrainIndex kInvalidTerrainIndex = ~0;

const uint32_t TerrainNoiseScale = 4;
const uint32_t TerrainNoiseSize = 64;
const uint32_t TerrainNoiseSmoothSize = TerrainNoiseSize * TerrainNoiseScale;
typedef aeStaticImage3D< float, TerrainNoiseSmoothSize, TerrainNoiseSmoothSize, TerrainNoiseSmoothSize > TerrainNoise;

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
const uint32_t kChunkSize = 24; // @NOTE: This can't be too high or kMaxChunkVerts will be hit
const int32_t kTempChunkSize = kChunkSize + 2; // Include a 1 voxel border
const int32_t kTempChunkSize3 = kTempChunkSize * kTempChunkSize * kTempChunkSize; // Temp voxel count
const uint32_t kMaxActiveChunks = 1024 + 512;
const uint32_t kMaxLoadedChunks = kMaxActiveChunks * 2;
const VertexCount kMaxChunkVerts = VertexCount( aeMath::MaxValue< uint16_t >() );
// https://math.stackexchange.com/questions/1879255/average-valence-of-vertex-in-tetrahedral-mesh
const uint32_t kMaxChunkIndices = uint32_t( kMaxChunkVerts ) * 6; // Average vertex valence
const uint32_t kMaxChunkAllocationsPerTick = 1;
const aeFloat16 kSkyBrightness = aeFloat16( 5.0f );
const float kSdfBoundary = 2.0f;

struct Block
{
  enum Type : uint8_t
  {
    Exterior,
    Interior,
    Surface,
    Blocking,
    Unloaded,
    COUNT
  };
};

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
struct TerrainRaycastResult
{
  bool hit;
  Block::Type type;
  float distance;
  aeInt3 posi; // @NOTE: It's possible for the intersection to end up outside the original voxel
  aeFloat3 posf;
  aeFloat3 normal;
  bool touchedUnloaded;
};

float SdfUnion( float d1, float d2 );
float SdfSubtraction( float d1, float d2 );
float SdfIntersection( float d1, float d2 );
float SdfSmoothUnion( float d1, float d2, float k );
float SdfSmoothSubtraction( float d1, float d2, float k );

//------------------------------------------------------------------------------
// Sdf class
//------------------------------------------------------------------------------
class Sdf
{
public:
  Sdf();
  virtual ~Sdf() {}
  
  float GetValue( aeFloat3 p ) const;

  aeAABB GetAABB() const { return m_aabb; }
  aeOBB GetOBB() const { return aeOBB( m_localToWorld ); }
  
  void SetTransform( const aeFloat4x4& transform );
  const aeFloat4x4& GetTransform() const { return m_localToWorld; }
  aeFloat3 GetHalfSize() const { return m_halfSize; }
  bool IsSolid() const { return type == Type::Union || type == Type::SmoothUnion; }

  void Dirty() { m_dirty = true; } // Must be be explicitly called if object is modified after creation

  virtual Sdf* Clone() const = 0;
  virtual ae::Hash Hash( ae::Hash hash ) const = 0;
  virtual float GetValue( aeFloat3 p, int ) const = 0;

  enum class Type
  {
    Union,
    Subtraction,
    SmoothUnion,
    SmoothSubtraction,
    Material
  };
  Type type = Type::Union;
  TerrainMaterialId materialId = 0;
  float smoothing = 0.0f; // Works with SmoothUnion and SmoothSubtraction types
  int32_t order = 0; // Lower values processed first, ie. to subtract from a solid the subtraction order should be higher
  
  TerrainNoise* noise = nullptr;
  float topNoiseStrength = 0.0f;
  ae::Vec3 topNoiseOffset = ae::Vec3( 0.0f );
  ae::Vec3 topNoiseScale = ae::Vec3( 1.0f );
  float noiseStrength = 0.0f;
  ae::Vec3 noiseOffset = ae::Vec3( 0.0f );
  ae::Vec3 noiseScale = ae::Vec3( 1.0f );

protected:
  ae::Hash GetBaseHash( ae::Hash hash ) const;
  const aeFloat4x4& GetRemoveTRMatrix() const { return m_removeTR; }

private:
  aeAABB m_aabb;
  aeFloat3 m_halfSize;
  aeFloat4x4 m_localToWorld;
  aeFloat4x4 m_removeTR;

public:
  // Internal
  bool m_dirty = false;
  aeAABB m_aabbPrev;
};

//------------------------------------------------------------------------------
// SdfBox class
//------------------------------------------------------------------------------
class SdfBox : public Sdf
{
public:
  Sdf* Clone() const override;
  ae::Hash Hash( ae::Hash hash ) const override;
  float GetValue( aeFloat3 p, int ) const override;

  float cornerRadius = 0.0f;
};

//------------------------------------------------------------------------------
// SdfCylinder class
//------------------------------------------------------------------------------
class SdfCylinder : public Sdf
{
public:
  Sdf* Clone() const override;
  ae::Hash Hash( ae::Hash hash ) const override;
  float GetValue( aeFloat3 p, int ) const override;

  // Valid range is 0-1, are multiplied by obb size
  float top = 1.0f;
	float bottom = 1.0f;
};

//------------------------------------------------------------------------------
// SdfHeightmap class
//------------------------------------------------------------------------------
class SdfHeightmap : public Sdf
{
public:
  void SetImage( ae::Image* heightMap ) { m_heightMap = heightMap; }
  Sdf* Clone() const override;
  ae::Hash Hash( ae::Hash hash ) const override;
  float GetValue( aeFloat3 p, int ) const override;

private:
  ae::Image* m_heightMap = nullptr;
};

//------------------------------------------------------------------------------
// TerrainParams struct
// @NOTE: This struct provides parameters which globally affects how Terrain
//        generates vertex data.
//------------------------------------------------------------------------------
struct TerrainParams
{
  ae::FileSystem* vfs = nullptr;
  aeDebugRender* debug = nullptr;
  float normalSampleOffset = 0.25f;
  float smoothingAmount = 0.05f;
  
  ae::Hash GetHash( ae::Hash hash = ae::Hash() ) const
  {
    // @NOTE: Only hash parameters that affect final terrain output
    hash = hash.HashBasicType( normalSampleOffset );
    hash = hash.HashBasicType( smoothingAmount );
    return hash;
  }
};

//------------------------------------------------------------------------------
// TerrainSdf class
//------------------------------------------------------------------------------
class TerrainSdf
{
public:
  TerrainSdf( class Terrain* terrain );

  template < typename T >
  T* CreateSdf();
  void DestroySdf( Sdf* sdf );

  void UpdatePending();
  bool HasPending() const;
  void RenderDebug( aeDebugRender* debug );
  
  uint32_t GetShapeCount() const { return m_shapes.Length(); }
  Sdf* GetShapeAtIndex( uint32_t index ) const { return m_shapes[ index ]; }
  
  TerrainNoise noise;

private:
  friend class Terrain;

  class Terrain* m_terrain;
  ae::Array< Sdf* > m_shapes = AE_ALLOC_TAG_TERRAIN;
  ae::Array< Sdf* > m_shapesPrev = AE_ALLOC_TAG_TERRAIN;
  ae::Array< Sdf* > m_pendingCreated = AE_ALLOC_TAG_TERRAIN;
  ae::Array< Sdf* > m_pendingDestroy = AE_ALLOC_TAG_TERRAIN;
};

template < typename T >
T* TerrainSdf::CreateSdf()
{
  Sdf* sdf = ae::New< T >( AE_ALLOC_TAG_TERRAIN );
  sdf->noise = &noise;
  m_pendingCreated.Append( sdf );
  return static_cast< T* >( sdf );
}

//------------------------------------------------------------------------------
// TerrainSdfCache class
//------------------------------------------------------------------------------
class TerrainSdfCache
{
public:
  TerrainSdfCache();
  ~TerrainSdfCache();

  void Generate( aeInt3 chunk, const class TerrainJob* job );
  float GetValue( aeFloat3 pos ) const;
  float GetValue( aeInt3 pos ) const;
  aeFloat3 GetDerivative( aeFloat3 p ) const;

private:
  float m_GetValue( aeInt3 pos ) const;

  const int32_t kDim = kChunkSize + 5; // TODO: What should this value actually be? Corresponds to 'chunkPlus'
  static const int32_t kOffset = 2;
  
  aeInt3 m_chunk;
  aeInt3 m_offseti; // Pre-computed chunk integer offset
  aeFloat3 m_offsetf; // Pre-computed chunk float offset
  TerrainParams m_p;

  // @TODO: Replace this with aeStaticImage3D
  aeFloat16* m_values;
};

//------------------------------------------------------------------------------
// TerrainJob class
// @NOTE: This class takes aeSdfShape's as input, and outputs renderable vertex
//        data. It typically does work in another thread.
//------------------------------------------------------------------------------
class TerrainJob
{
public:
  TerrainJob();
  ~TerrainJob();
  void StartNew( const TerrainParams& params, const TerrainSdf* sdf, struct TerrainChunk* chunk );
  void Do();
  void Finish();

  bool HasJob() const { return m_hasJob; }
  bool HasChunk( aeInt3 pos ) const;
  bool IsPendingFinish() const { return m_hasJob && !m_running; }

  const TerrainChunk* GetChunk() const { return m_chunk; }
  TerrainChunk* GetChunk() { return m_chunk; }
  const TerrainVertex* GetVertices() const { return m_vertices.Begin(); }
  const TerrainIndex* GetIndices() const { return m_indices.Begin(); }
  VertexCount GetVertexCount() const { return m_vertexCount; }
  uint32_t GetIndexCount() const { return m_indexCount; }
  
  float GetValue( aeFloat3 pos ) const;
  aeFloat3 GetDerivative( aeFloat3 pos ) const;
  TerrainMaterialId GetMaterial( aeFloat3 pos, aeFloat3 normal ) const;

private:
  // Management
  bool m_hasJob;
  std::atomic_bool m_running;

  // Input
  ae::Hash m_parameterHash;
  ae::Array< Sdf* > m_shapes = AE_ALLOC_TAG_TERRAIN;
  struct TerrainChunk* m_chunk;
  TerrainParams m_p;

  // Pre-computed sdf
  TerrainSdfCache m_sdfCache;

  // Output
  VertexCount m_vertexCount;
  uint32_t m_indexCount;
  ae::Array< TerrainVertex > m_vertices;
  ae::Array< TerrainIndex > m_indices;

public:
  const TerrainParams& GetTerrainParams() const { return m_p; }
  
  // Temp edges (pre-allocated for all future jobs)
  struct TempEdges
  {
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t b;

    // 3 planes whose intersections are used to position vertices within voxel
    // EDGE_TOP_FRONT_BIT
    // EDGE_TOP_RIGHT_BIT
    // EDGE_SIDE_FRONTRIGHT_BIT
    aeFloat3 p[ 3 ];
    aeFloat3 n[ 3 ];
  };

  TempEdges* edgeInfo;
};

//------------------------------------------------------------------------------
// TerrainChunk class
// @NOTE: Stores vertex data of fully generated chunks. Also provides information
//        for collision and if the section of terrain it represents has changed
//        and should be regenerated.
//------------------------------------------------------------------------------
struct TerrainChunk
{
  TerrainChunk();
  ~TerrainChunk();

  static uint32_t GetIndex( aeInt3 pos );
  static void GetPosFromWorld( aeInt3 pos, aeInt3* chunkPos, aeInt3* localPos );

  uint32_t GetIndex() const;
  void Generate( const TerrainSdfCache* sdf, const TerrainJob* job, TerrainJob::TempEdges* edgeBuffer, TerrainVertex* verticesOut, TerrainIndex* indexOut, VertexCount* vertexCountOut, uint32_t* indexCountOut );
  
  void Serialize( ae::BinaryStream* stream );

  aeAABB GetAABB() const;
  static aeAABB GetAABB( aeInt3 chunkPos );

  void m_SetVertexData( const TerrainVertex* verts, const TerrainIndex* indices, VertexCount vertexCount, uint32_t indexCount );
  
  uint32_t m_check;
  aeInt3 m_pos;
  bool m_geoDirty;
  bool m_lightDirty;
  aeVertexData m_data;
  ae::Mesh m_mesh;
  aeListNode< TerrainChunk > m_generatedList;
  
  Block::Type m_t[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  aeFloat16 m_l[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  TerrainIndex m_i[ kChunkSize ][ kChunkSize ][ kChunkSize ];

private:
  static void m_GetQuadVertexOffsetsFromEdge( uint32_t edgeBit, int32_t( &offsets )[ 4 ][ 3 ] );
};

//------------------------------------------------------------------------------
// Terrain class
//------------------------------------------------------------------------------
class Terrain
{
public:
  Terrain();
  ~Terrain();

  void Initialize( uint32_t maxThreads, bool render );
  void Terminate();
  void Update( aeFloat3 center, float radius );
  void Render( const class aeShader* shader, const aeUniformList& shaderParams );

  void SetParams( const TerrainParams& params );
  void GetParams( TerrainParams* outParams );

  void SetDebugTextCallback( std::function< void( aeFloat3, const char* ) > fn ) { m_debugTextFn = fn; }
  uint32_t GetMaxThreads() const { return m_terrainJobs.Length(); }
  
  Block::Type GetVoxel( int32_t x, int32_t y, int32_t z ) const;
  Block::Type GetVoxel( aeFloat3 position ) const;
  bool GetCollision( int32_t x, int32_t y, int32_t z ) const;
  bool GetCollision( aeFloat3 position ) const;
  aeFloat16 GetLight( int32_t x, int32_t y, int32_t z ) const;

  TerrainChunk* GetChunk( uint32_t chunkIndex );
  TerrainChunk* GetChunk( aeInt3 pos );
  const TerrainChunk* GetChunk( uint32_t chunkIndex ) const;
  const TerrainChunk* GetChunk( aeInt3 pos ) const;
  VertexCount GetVertexCount( uint32_t chunkIndex ) const;
  VertexCount GetVertexCount( aeInt3 pos ) const;

  // Simple voxel grid test
  bool VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps ) const;
  // Uses voxel grid and terrain normal so position result is slightly lumpy (non-continuous)
  TerrainRaycastResult RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision ) const;
  
  // Triangle raycast against terrain
  bool Raycast( const ae::Mesh::RaycastParams& params, ae::Mesh::RaycastResult* outResult ) const;
  // Triangle-sphere push out
  ae::Mesh::PushOutInfo PushOutSphere( const ae::Mesh::PushOutParams& params, const ae::Mesh::PushOutInfo& info ) const;
  
  TerrainSdf sdf;

private:
  bool m_GetVertex( int32_t x, int32_t y, int32_t z, TerrainVertex* outVertex ) const;
  void UpdateChunkLighting( TerrainChunk* chunk );
  
  TerrainChunk* AllocChunk( aeInt3 pos );
  void FreeChunk( TerrainChunk* chunk );
  void m_SetVertexCount( uint32_t chunkIndex, VertexCount count );
  float GetChunkScore( aeInt3 pos ) const;

  TerrainParams m_params;

  bool m_render = false;
  aeFloat3 m_center = aeFloat3( 0.0f );
  float m_radius = 0.0f;
  
  std::map< uint32_t, struct TerrainChunk* > m_chunks;
  std::map< uint32_t, VertexCount > m_vertexCounts; // Kept even when chunks are freed so they are not regenerated again if they are empty
  ae::ObjectPool< TerrainChunk, kMaxLoadedChunks > m_chunkPool;

  // Keep these across frames instead of allocating temporary space for each generated chunk
  struct ChunkSort
  {
    TerrainChunk* c;
    aeInt3 pos;
    float score;
  };
  //ae::Map< aeInt3, ChunkSort > t_chunkMap;
  std::map< uint32_t, ChunkSort > t_chunkMap_hack;
  ae::Array< ChunkSort > t_chunkSorts = AE_ALLOC_TAG_TERRAIN;

  aeList< TerrainChunk > m_generatedList;
  
  bool m_blockCollision[ Block::COUNT ];
  aeFloat16 m_blockDensity[ Block::COUNT ];
  
  ctpl::thread_pool* m_threadPool = nullptr;
  ae::Array< TerrainJob* > m_terrainJobs = AE_ALLOC_TAG_TERRAIN; // @TODO: Should be static, and shouldn't be a pointer

  std::function< void( aeFloat3, const char* ) > m_debugTextFn;

public:
  // Internal
  void m_Dirty( aeAABB aabb );
};

} // ae

#endif
