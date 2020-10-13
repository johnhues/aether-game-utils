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
#include "aeCompactingAllocator.h"
#include "aeMath.h"
#include "aeObjectPool.h"
#include "aeRender.h"
#include "aeList.h"
#include <atomic>
#include <map> // HACK

namespace ctpl
{
  class thread_pool;
}

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

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
typedef float float16_t;
typedef uint8_t aeTerrainMaterialId;
#define PACK( _ae_something ) _ae_something

const uint32_t kChunkSize = 64;
const int32_t kTempChunkSize = kChunkSize + 2; // Include a 1 voxel border
const int32_t kTempChunkSize3 = kTempChunkSize * kTempChunkSize * kTempChunkSize; // Temp voxel count
const uint32_t kChunkCountMax = kChunkSize * kChunkSize * kChunkSize;
const uint32_t kWorldChunksWidth = 128;
const uint32_t kWorldChunksHeight = 2;
const uint32_t kWorldMaxWidth = kWorldChunksWidth * kChunkSize;
const uint32_t kWorldMaxHeight = kWorldChunksHeight * kChunkSize;
const uint32_t kMaxActiveChunks = 512;
const uint32_t kMaxLoadedChunks = kMaxActiveChunks * 2;
const uint32_t kMaxChunkVerts = kChunkSize * kChunkSize * kChunkSize;
const uint32_t kMaxChunkIndices = kChunkSize * kChunkSize * kChunkSize * 6;
const uint32_t kMaxChunkAllocationsPerTick = 1;
const float16_t kSkyBrightness = float16_t( 5.0f );
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
// TerrainVertex
//------------------------------------------------------------------------------
PACK( struct TerrainVertex
{
  aeFloat3 position;
  aeFloat3 normal;
  uint8_t materials[ 4 ];
  uint8_t info[ 4 ];
});
typedef uint16_t TerrainIndex;

struct RaycastResult
{
  bool hit;
  Block::Type type;
  float distance;
  int32_t posi[ 3 ];
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

  aeAABB GetAABB() const { return m_aabb; }
  aeOBB GetOBB() const { return aeOBB( m_localToWorld ); }
  const aeFloat4x4& GetTransform() const { return m_localToWorld; }
  void SetTransform( const aeFloat4x4& transform );

  virtual float GetValue( aeFloat3 p ) const = 0;
  virtual aeAABB OnSetTransform( aeFloat3 scale ) = 0;

  enum class Type
  {
    Union,
    Subtraction,
    Material
  };

  Type type = Type::Union;
  aeTerrainMaterialId materialId = 0;

protected:
  const aeFloat4x4& GetWorldToScaled() const { return m_worldToScaled; }

private:
  aeAABB m_aabb;
  aeFloat4x4 m_localToWorld;
  aeFloat4x4 m_worldToScaled;
};

//------------------------------------------------------------------------------
// Box class
//------------------------------------------------------------------------------
class Box : public Shape
{
public:
  float GetValue( aeFloat3 p ) const override;
  aeAABB OnSetTransform( aeFloat3 scale ) override;

  void SetCornerSize( float cornerSize ) { m_r = cornerSize; }
  float GetCornerSize() const { return m_r; }

private:
  aeFloat3 m_halfSize = aeFloat3( 0.0f );
  float m_r = 0.0f;
};

//------------------------------------------------------------------------------
// Cone class
//------------------------------------------------------------------------------
class Cone : public Shape
{
public:
  float GetValue( aeFloat3 p ) const override;
  aeAABB OnSetTransform( aeFloat3 scale ) override;

  // Valid range is 0-1, multiplied by obb size
  float top = 0.5f;
	float bottom = 1.0f;
	
private:
	aeFloat3 m_halfSize = aeFloat3( 0.0f );
};

//------------------------------------------------------------------------------
// Heightmap class
//------------------------------------------------------------------------------
class Heightmap : public Shape
{
public:
  void SetImage( ae::Image* heightMap ) { m_heightMap = heightMap; }
  float GetValue( aeFloat3 p ) const override;
  aeAABB OnSetTransform( aeFloat3 scale ) override;

private:
  aeFloat3 m_halfSize = aeFloat3( 0.0f );
  ae::Image* m_heightMap = nullptr;
};

} } // ae::Sdf

//------------------------------------------------------------------------------
// aeTerrainSDF class
//------------------------------------------------------------------------------
class aeTerrainSDF
{
public:
  template < typename T >
  T* CreateSdf();
  void DestroySdf( ae::Sdf::Shape* sdf );

  void UpdatePending();
  bool HasPending() const;
  void RenderDebug( aeDebugRender* debug );

  // @NOTE: This will be called from multiple threads simultaneously and so must be const
  float GetValue( aeFloat3 pos ) const;
  aeFloat3 GetDerivative( aeFloat3 pos ) const;
  aeTerrainMaterialId GetMaterial( aeFloat3 pos ) const;

  bool TestAABB( aeAABB aabb ) const;

  void* m_userdata = nullptr;
  float ( *m_fn1 )( aeFloat3 ) = nullptr;
  float ( *m_fn2 )( void* userdata, aeFloat3 ) = nullptr;

private:
  aeArray< ae::Sdf::Shape* > m_shapes;
  aeArray< ae::Sdf::Shape* > m_pendingCreated;
  aeArray< ae::Sdf::Shape* > m_pendingDestroy;
};

template < typename T >
T* aeTerrainSDF::CreateSdf()
{
  ae::Sdf::Shape* sdf = aeAlloc::Allocate< T >();
  m_pendingCreated.Append( sdf );
  return static_cast< T* >( sdf );
}

//------------------------------------------------------------------------------
// SDFCache class
//------------------------------------------------------------------------------
class SDFCache
{
public:
  SDFCache();
  ~SDFCache();

  void Generate( aeInt3 chunk, const aeTerrainSDF* sdf );
  float GetValue( aeFloat3 pos ) const;
  float GetValue( aeInt3 pos ) const;
  aeFloat3 GetDerivative( aeFloat3 p ) const;
  uint8_t GetMaterial( aeInt3 pos ) const;

private:
  float m_GetValue( aeInt3 pos ) const;

  const int32_t kDim = kChunkSize + 5; // TODO: What should this value actually be? Corresponds to chunkPlus
  static const int32_t kOffset = 2;
  
  aeInt3 m_chunk;
  aeInt3 m_offseti; // Pre-computed chunk integer offset
  aeFloat3 m_offsetf; // Pre-computed chunk float offset

  float16_t* m_sdf;
  aeTerrainMaterialId* m_material;
};

//------------------------------------------------------------------------------
// aeTerrainJob class
//------------------------------------------------------------------------------
class aeTerrainJob
{
public:
  aeTerrainJob();
  ~aeTerrainJob();
  void StartNew( const aeTerrainSDF* sdf, struct Chunk* chunk );
  void Do();
  void Finish();

  bool HasJob() const { return m_hasJob; }
  bool HasChunk( aeInt3 pos ) const;
  bool IsPendingFinish() const { return m_hasJob && !m_running; }

  Chunk* GetChunk() { return m_chunk; }
  const TerrainVertex* GetVertices() const { return m_vertexCount ? &m_vertices[ 0 ] : nullptr; }
  const TerrainIndex* GetIndices() const { return m_indexCount ? &m_indices[ 0 ] : nullptr; }
  uint32_t GetVertexCount() const { return m_vertexCount; }
  uint32_t GetIndexCount() const { return m_indexCount; }

private:
  // Management
  bool m_hasJob;
  std::atomic_bool m_running;

  // Input
  const aeTerrainSDF* m_sdf;
  struct Chunk* m_chunk;

  // Pre-computed sdf
  SDFCache m_sdfCache;

  // Output
  uint32_t m_vertexCount;
  uint32_t m_indexCount;
  aeArray< TerrainVertex > m_vertices;
  aeArray< TerrainIndex > m_indices;

public:
  // Temp edges (pre-allocate edges for all future jobs)
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

  TempEdges* edgeInfo;
};

//------------------------------------------------------------------------------
// Terrain Chunk class
//------------------------------------------------------------------------------
struct Chunk // @TODO: aeTerrainChunk
{
  Chunk();
  ~Chunk();

  static uint32_t GetIndex( aeInt3 pos );
  uint32_t GetIndex() const;
  void Generate( const SDFCache* sdf, aeTerrainJob::TempEdges* edgeBuffer, TerrainVertex* verticesOut, TerrainIndex* indexOut, uint32_t* vertexCountOut, uint32_t* indexCountOut );
  
  uint32_t m_check;
  aeInt3 m_pos;
  bool m_geoDirty;
  bool m_lightDirty;
  aeVertexData m_data;
  TerrainVertex* m_vertices;
  aeListNode< Chunk > m_generatedList;
  
  Block::Type m_t[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  float16_t m_l[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  TerrainIndex m_i[ kChunkSize ][ kChunkSize ][ kChunkSize ];

private:
  static void m_GetOffsetsFromEdge( uint32_t edgeBit, int32_t( &offsets )[ 4 ][ 3 ] );
};

//------------------------------------------------------------------------------
// aeTerrain class
//------------------------------------------------------------------------------
class aeTerrain
{
public:
  ~aeTerrain();

  void Initialize( uint32_t maxThreads, bool render );
  void Terminate();
  void Update( aeFloat3 center, float radius );
  void Render( const class aeShader* shader, const aeUniformList& shaderParams );
  void RenderDebug( class aeDebugRender* debug );

  void SetCallback( void* userdata, float ( *fn )( void*, aeFloat3 ) );
  void SetCallback( float ( *fn )( aeFloat3 ) );
  void SetDebugTextCallback( std::function< void( aeFloat3, const char* ) > fn ) { m_debugTextFn = fn; }
  void Dirty( aeAABB aabb );
  
  Block::Type GetVoxel( uint32_t x, uint32_t y, uint32_t z ) const;
  Block::Type GetVoxel( aeFloat3 position ) const;
  bool GetCollision( uint32_t x, uint32_t y, uint32_t z ) const;
  bool GetCollision( aeFloat3 position ) const;
  float16_t GetLight( uint32_t x, uint32_t y, uint32_t z ) const;
  const Chunk* GetChunk( aeInt3 pos ) const;
  Chunk* GetChunk( aeInt3 pos );
  int32_t GetVoxelCount( aeInt3 pos ) const;

  bool VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps ) const;
  RaycastResult RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision ) const;
  RaycastResult Raycast( aeFloat3 start, aeFloat3 ray ) const;
  // @NOTE: SweepSphere returns true on collision and writes out the following:
  // distanceOut is distance traveled before collision
  // normalOut is ground normal
  // posOut is ground collision point
  bool SweepSphere( aeSphere sphere, aeFloat3 ray, float* distanceOut = nullptr, aeFloat3* normalOut = nullptr, aeFloat3* posOut = nullptr ) const;
  bool PushOutSphere( aeSphere sphere, aeFloat3* offsetOut = nullptr, class aeDebugRender* debug = nullptr ) const;
  
  aeTerrainSDF sdf;

private:
  const TerrainVertex* m_GetVertex( int32_t x, int32_t y, int32_t z ) const;
  void UpdateChunkLighting( Chunk* chunk );
  //void UpdateChunkLightingHelper( Chunk *chunk, uint32_t x, uint32_t y, uint32_t z, float16_t l );
  Chunk* AllocChunk( aeFloat3 center, aeInt3 pos );
  void FreeChunk( Chunk* chunk );
  float GetChunkScore( aeInt3 pos ) const;

  bool m_render = false;
  aeFloat3 m_center = aeFloat3( 0.0f );
  float m_radius = 0.0f;
  
  //aeCompactingAllocator m_compactAlloc;
  struct Chunk **m_chunks = nullptr;
  int16_t* m_voxelCounts = nullptr; // Kept even when chunks are freed so they are not regenerated again if they are empty
  aeObjectPool< Chunk, kMaxLoadedChunks > m_chunkPool;

  // Keep these across frames instead of allocating temporary space for each generated chunk
  struct ChunkSort
  {
    Chunk* c;
    aeInt3 pos;
    float score;
  };
  //aeMap< aeInt3, ChunkSort > t_chunkMap;
  std::map< uint32_t, ChunkSort > t_chunkMap_hack;
  aeArray< ChunkSort > t_chunkSorts;

  aeList< Chunk > m_generatedList;
  uint8_t* m_chunkRawAlloc = nullptr;
  
  bool m_blockCollision[ Block::COUNT ];
  float16_t m_blockDensity[ Block::COUNT ];
  
  ctpl::thread_pool* m_threadPool = nullptr;
  aeArray< aeTerrainJob* > m_terrainJobs;

  std::function< void( aeFloat3, const char* ) > m_debugTextFn;
};

#endif
