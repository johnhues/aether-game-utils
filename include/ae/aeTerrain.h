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
#define PACK( _ae_something ) _ae_something

const uint32_t kChunkSize = 64;
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

PACK( struct TerrainVertex
{
  aeFloat3 position;
  aeFloat3 normal;
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

struct DCInfo
{
  uint16_t edgeFlags;
  EdgeCompact edges[ 12 ];
};

class aeTerrainSDF
{
public:
  float TerrainValue( aeFloat3 p ) const;
  aeFloat3 GetSurfaceDerivative( aeFloat3 p ) const;

  void* m_userdata = nullptr;
  float ( *m_fn1 )( aeFloat3 ) = nullptr;
  float ( *m_fn2 )( void* userdata, aeFloat3 ) = nullptr;
};

class aeTerrainJob
{
public:
  aeTerrainJob();
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

  bool replaceDirty_CHECK;

private:
  bool m_hasJob;
  std::atomic_bool m_running;

  const aeTerrainSDF* m_sdf;
  uint32_t m_vertexCount;
  uint32_t m_indexCount;
  aeArray< TerrainVertex > m_vertices;
  aeArray< TerrainIndex > m_indices;
  struct Chunk* m_chunk;
};

struct Chunk // @TODO: aeTerrainChunk
{
  Chunk();
  ~Chunk();

  static uint32_t GetIndex( aeInt3 pos );
  uint32_t GetIndex() const;
  void Generate( const aeTerrainSDF* sdf, TerrainVertex* verticesOut, TerrainIndex* indexOut, uint32_t* vertexCountOut, uint32_t* indexCountOut );
  
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

class aeTerrain
{
public:
  ~aeTerrain();

  void Initialize( uint32_t maxThreads, bool render );
  void Terminate();
  void Update( aeFloat3 center, float radius );
  void Render( const class aeShader* shader, const aeUniformList& shaderParams );

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
  //void VoxelizeAndAddMesh(
  //  aeFloat3* vertices, uint16_t* indices,
  //  uint32_t vertexCount, uint32_t indexCount,
  //  aeFloat4x4& modelToWorld, Block::Type type );
  
  bool VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps ) const;
  RaycastResult RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision ) const;
  RaycastResult Raycast( aeFloat3 start, aeFloat3 ray ) const;
  
private:
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

  aeTerrainSDF m_sdf;

  aeList< Chunk > m_generatedList;
  uint8_t* m_chunkRawAlloc = nullptr;
  
  bool m_blockCollision[ Block::COUNT ];
  float16_t m_blockDensity[ Block::COUNT ];
  
  ctpl::thread_pool* m_threadPool = nullptr;
  aeArray< aeTerrainJob* > m_terrainJobs;

  std::function< void( aeFloat3, const char* ) > m_debugTextFn;
};

#endif
