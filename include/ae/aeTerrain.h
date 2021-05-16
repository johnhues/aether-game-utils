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

//------------------------------------------------------------------------------
// Terrain types
//------------------------------------------------------------------------------
typedef float aeFloat16;
typedef uint8_t aeTerrainMaterialId;

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

const uint32_t aeTerrainNoiseScale = 4;
const uint32_t aeTerrainNoiseSize = 64;
const uint32_t aeTerrainNoiseSmoothSize = aeTerrainNoiseSize * aeTerrainNoiseScale;
typedef aeStaticImage3D< float, aeTerrainNoiseSmoothSize, aeTerrainNoiseSmoothSize, aeTerrainNoiseSmoothSize > aeTerrainNoise;

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
struct RaycastResult
{
  bool hit;
  Block::Type type;
  float distance;
  aeInt3 posi; // @NOTE: It's possible for the intersection to end up outside the original voxel
  aeFloat3 posf;
  aeFloat3 normal;
  bool touchedUnloaded;
};

struct EdgeCompact
{
  uint8_t f;
  int8_t nx;
  int8_t ny;
  int8_t nz;
};

float aeUnion( float d1, float d2 );
float aeSubtraction( float d1, float d2 );
float aeIntersection( float d1, float d2 );
float aeSmoothUnion( float d1, float d2, float k );
float aeSmoothSubtraction( float d1, float d2, float k );

namespace ctpl
{
  class thread_pool;
}

//------------------------------------------------------------------------------
// ae::Sdf
//------------------------------------------------------------------------------
namespace ae { namespace Sdf {

//------------------------------------------------------------------------------
// Shape class
//------------------------------------------------------------------------------
class Shape
{
public:
  Shape();
  virtual ~Shape() {}
  
  float GetValue( aeFloat3 p ) const;

  aeAABB GetAABB() const { return m_aabb; }
  aeOBB GetOBB() const { return aeOBB( m_localToWorld ); }
  
  void SetTransform( const aeFloat4x4& transform );
  const aeFloat4x4& GetTransform() const { return m_localToWorld; }
  aeFloat3 GetHalfSize() const { return m_halfSize; }
  bool IsSolid() const { return type == Type::Union || type == Type::SmoothUnion; }

  void Dirty() { m_dirty = true; } // Must be be explicitly called if object is modified after creation

  virtual ae::Sdf::Shape* Clone() const = 0;
  virtual aeHash Hash( aeHash hash ) const = 0;
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
  aeTerrainMaterialId materialId = 0;
  float smoothing = 0.0f; // Works with SmoothUnion and SmoothSubtraction types
  int32_t order = 0; // Lower values processed first, ie. to subtract from a solid the subtraction order should be higher
  
  aeTerrainNoise* noise = nullptr;
  float noiseStrength = 0.0f;
  aeFloat3 noiseOffset = aeFloat3( 0.0f );
  aeFloat3 noiseScale = aeFloat3( 1.0f );

protected:
  aeHash GetBaseHash( aeHash hash ) const;
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
// Box class
//------------------------------------------------------------------------------
class Box : public Shape
{
public:
  ae::Sdf::Shape* Clone() const override;
  aeHash Hash( aeHash hash ) const override;
  float GetValue( aeFloat3 p, int ) const override;

  float cornerRadius = 0.0f;
};

//------------------------------------------------------------------------------
// Cylinder class
//------------------------------------------------------------------------------
class Cylinder : public Shape
{
public:
  ae::Sdf::Shape* Clone() const override;
  aeHash Hash( aeHash hash ) const override;
  float GetValue( aeFloat3 p, int ) const override;

  // Valid range is 0-1, are multiplied by obb size
  float top = 1.0f;
	float bottom = 1.0f;
};

//------------------------------------------------------------------------------
// Heightmap class
//------------------------------------------------------------------------------
class Heightmap : public Shape
{
public:
  void SetImage( ae::Image* heightMap ) { m_heightMap = heightMap; }
  ae::Sdf::Shape* Clone() const override;
  aeHash Hash( aeHash hash ) const override;
  float GetValue( aeFloat3 p, int ) const override;

private:
  ae::Image* m_heightMap = nullptr;
};

} } // ae::Sdf

//------------------------------------------------------------------------------
// aeTerrainSDF class
//------------------------------------------------------------------------------
class aeTerrainSDF
{
public:
  aeTerrainSDF( class aeTerrain* terrain );

  template < typename T >
  T* CreateSdf();
  void DestroySdf( ae::Sdf::Shape* sdf );

  void UpdatePending();
  bool HasPending() const;
  void RenderDebug( aeDebugRender* debug );
  
  uint32_t GetShapeCount() const { return m_shapes.Length(); }
  ae::Sdf::Shape* GetShapeAtIndex( uint32_t index ) const { return m_shapes[ index ]; }
  
  aeTerrainNoise noise;

private:
  friend class aeTerrain;

  class aeTerrain* m_terrain;
  aeArray< ae::Sdf::Shape* > m_shapes;
  aeArray< ae::Sdf::Shape* > m_shapesPrev;
  aeArray< ae::Sdf::Shape* > m_pendingCreated;
  aeArray< ae::Sdf::Shape* > m_pendingDestroy;
};

template < typename T >
T* aeTerrainSDF::CreateSdf()
{
  ae::Sdf::Shape* sdf = aeAlloc::Allocate< T >();
  sdf->noise = &noise;
  m_pendingCreated.Append( sdf );
  return static_cast< T* >( sdf );
}

//------------------------------------------------------------------------------
// aeTerrainSDFCache class
//------------------------------------------------------------------------------
class aeTerrainSDFCache
{
public:
  aeTerrainSDFCache();
  ~aeTerrainSDFCache();

  void Generate( aeInt3 chunk, const class aeTerrainJob* job );
  float GetValue( aeFloat3 pos ) const;
  float GetValue( aeInt3 pos ) const;
  aeFloat3 GetDerivative( aeFloat3 p ) const;

private:
  float m_GetValue( aeInt3 pos ) const;

  const int32_t kDim = kChunkSize + 5; // TODO: What should this value actually be? Corresponds to chunkPlus
  static const int32_t kOffset = 2;
  
  aeInt3 m_chunk;
  aeInt3 m_offseti; // Pre-computed chunk integer offset
  aeFloat3 m_offsetf; // Pre-computed chunk float offset

  aeFloat16* m_values;
};

//------------------------------------------------------------------------------
// aeTerrainJob class
//------------------------------------------------------------------------------
class aeTerrainJob
{
public:
  aeTerrainJob();
  ~aeTerrainJob();
  void StartNew( const class aeVfs* vfs, const aeTerrainSDF* sdf, struct aeTerrainChunk* chunk );
  void Do();
  void Finish();

  bool HasJob() const { return m_hasJob; }
  bool HasChunk( aeInt3 pos ) const;
  bool IsPendingFinish() const { return m_hasJob && !m_running; }

  const aeTerrainChunk* GetChunk() const { return m_chunk; }
  aeTerrainChunk* GetChunk() { return m_chunk; }
  const TerrainVertex* GetVertices() const { return m_vertices.Begin(); }
  const TerrainIndex* GetIndices() const { return m_indices.Begin(); }
  VertexCount GetVertexCount() const { return m_vertexCount; }
  uint32_t GetIndexCount() const { return m_indexCount; }
  
  float GetValue( aeFloat3 pos ) const;
  aeFloat3 GetDerivative( aeFloat3 pos ) const;
  aeTerrainMaterialId GetMaterial( aeFloat3 pos, aeFloat3 normal ) const;

private:
  // Management
  bool m_hasJob;
  std::atomic_bool m_running;

  // Input
  aeHash m_parameterHash;
  aeArray< ae::Sdf::Shape* > m_shapes;
  struct aeTerrainChunk* m_chunk;

  // Pre-computed sdf
  aeTerrainSDFCache m_sdfCache;

  // Output
  const aeVfs* m_vfs;
  VertexCount m_vertexCount;
  uint32_t m_indexCount;
  aeArray< TerrainVertex > m_vertices;
  aeArray< TerrainIndex > m_indices;

public:
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
// Terrain Chunk class
//------------------------------------------------------------------------------
struct aeTerrainChunk
{
  aeTerrainChunk();
  ~aeTerrainChunk();

  static uint32_t GetIndex( aeInt3 pos );
  static void GetPosFromWorld( aeInt3 pos, aeInt3* chunkPos, aeInt3* localPos );

  uint32_t GetIndex() const;
  void Generate( const aeTerrainSDFCache* sdf, const aeTerrainJob* job, aeTerrainJob::TempEdges* edgeBuffer, TerrainVertex* verticesOut, TerrainIndex* indexOut, VertexCount* vertexCountOut, uint32_t* indexCountOut );
  
  void Serialize( class aeBinaryStream* stream );

  aeAABB GetAABB() const;
  static aeAABB GetAABB( aeInt3 chunkPos );

  void m_SetVertexData( const TerrainVertex* verts, const TerrainIndex* indices, VertexCount vertexCount, uint32_t indexCount );
  
  uint32_t m_check;
  aeInt3 m_pos;
  bool m_geoDirty;
  bool m_lightDirty;
  aeVertexData m_data;
  aeMesh m_mesh;
  aeListNode< aeTerrainChunk > m_generatedList;
  
  Block::Type m_t[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  aeFloat16 m_l[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  TerrainIndex m_i[ kChunkSize ][ kChunkSize ][ kChunkSize ];

private:
  static void m_GetQuadVertexOffsetsFromEdge( uint32_t edgeBit, int32_t( &offsets )[ 4 ][ 3 ] );
};

//------------------------------------------------------------------------------
// aeTerrain class
//------------------------------------------------------------------------------
class aeTerrain
{
public:
  aeTerrain();
  ~aeTerrain();

  void Initialize( uint32_t maxThreads, bool render );
  void Terminate();
  void Update( aeFloat3 center, float radius );
  void Render( const class aeShader* shader, const aeUniformList& shaderParams );

  void SetVfs( class aeVfs* vfs ); // Use aeVfs::Cache to save to cache terrain
  void SetDebug( class aeDebugRender* debug );

  void SetDebugTextCallback( std::function< void( aeFloat3, const char* ) > fn ) { m_debugTextFn = fn; }
  uint32_t GetMaxThreads() const { return m_terrainJobs.Length(); }
  
  Block::Type GetVoxel( int32_t x, int32_t y, int32_t z ) const;
  Block::Type GetVoxel( aeFloat3 position ) const;
  bool GetCollision( int32_t x, int32_t y, int32_t z ) const;
  bool GetCollision( aeFloat3 position ) const;
  aeFloat16 GetLight( int32_t x, int32_t y, int32_t z ) const;

  aeTerrainChunk* GetChunk( uint32_t chunkIndex );
  aeTerrainChunk* GetChunk( aeInt3 pos );
  const aeTerrainChunk* GetChunk( uint32_t chunkIndex ) const;
  const aeTerrainChunk* GetChunk( aeInt3 pos ) const;
  VertexCount GetVertexCount( uint32_t chunkIndex ) const;
  VertexCount GetVertexCount( aeInt3 pos ) const;

  // Simple voxel grid test
  bool VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps ) const;
  // Uses voxel grid and terrain normal so position result is slightly lumpy (non-continuous)
  RaycastResult RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision ) const;
  
  // Triangle raycast against terrain
  bool Raycast( const aeMesh::RaycastParams& params, aeMesh::RaycastResult* outResult ) const;
  // Triangle-sphere push out
  bool PushOutSphere( const aeMesh::PushOutParams& params, aeMesh::PushOutResult* outResult ) const;
  
  aeTerrainSDF sdf;

private:
  bool m_GetVertex( int32_t x, int32_t y, int32_t z, TerrainVertex* outVertex ) const;
  void UpdateChunkLighting( aeTerrainChunk* chunk );
  
  aeTerrainChunk* AllocChunk( aeInt3 pos );
  void FreeChunk( aeTerrainChunk* chunk );
  void m_SetVertexCount( uint32_t chunkIndex, VertexCount count );
  float GetChunkScore( aeInt3 pos ) const;

  aeVfs* m_vfs = nullptr;
  aeDebugRender* m_debug = nullptr;

  bool m_render = false;
  aeFloat3 m_center = aeFloat3( 0.0f );
  float m_radius = 0.0f;
  
  //aeCompactingAllocator m_compactAlloc;
  std::map< uint32_t, struct aeTerrainChunk* > m_chunks3;
  std::map< uint32_t, VertexCount > m_vertexCounts; // Kept even when chunks are freed so they are not regenerated again if they are empty
  //aeMap<>
  aeObjectPool< aeTerrainChunk, kMaxLoadedChunks > m_chunkPool;

  // Keep these across frames instead of allocating temporary space for each generated chunk
  struct ChunkSort
  {
    aeTerrainChunk* c;
    aeInt3 pos;
    float score;
  };
  //aeMap< aeInt3, ChunkSort > t_chunkMap;
  std::map< uint32_t, ChunkSort > t_chunkMap_hack;
  aeArray< ChunkSort > t_chunkSorts;

  aeList< aeTerrainChunk > m_generatedList;
  
  bool m_blockCollision[ Block::COUNT ];
  aeFloat16 m_blockDensity[ Block::COUNT ];
  
  ctpl::thread_pool* m_threadPool = nullptr;
  aeArray< aeTerrainJob* > m_terrainJobs;

  std::function< void( aeFloat3, const char* ) > m_debugTextFn;

public:
  // Internal
  void m_Dirty( aeAABB aabb );
};

#endif
