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

    void LoadRaw( const void* data, uint32_t width, uint32_t height, uint32_t channels );
    bool LoadFile( const void* file, uint32_t length, Extension extension );

    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetChannels() const { return m_channels; }

    aeColor Get( aeInt2 pixel ) const;
    aeColor Get( aeFloat2 pixel, Interpolation interpolation ) const;

  private:
    aeArray< uint8_t > m_data;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_channels = 0;
  };
}

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------
typedef float float16_t;
#define PACK( _ae_something ) _ae_something

static const uint32_t kChunkSize = 16;
static const uint32_t kChunkCountMax = kChunkSize * kChunkSize * kChunkSize;
static const uint32_t kWorldChunksWidth = 128;
static const uint32_t kWorldChunksHeight = 32;
static const uint32_t kWorldMaxWidth = kWorldChunksWidth * kChunkSize;
static const uint32_t kWorldMaxHeight = kWorldChunksHeight * kChunkSize;
static const uint32_t kMaxActiveChunks = 512;
static const uint32_t kMaxLoadedChunks = kMaxActiveChunks * 2;
static const uint32_t kMaxChunkVerts = kChunkSize * kChunkSize * kChunkSize;
static const uint32_t kMaxChunkIndices = kChunkSize * kChunkSize * kChunkSize * 6;
static const float16_t kSkyBrightness = float16_t( 5.0f );

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

struct Chunk
{
  uint32_t check;
  int32_t pos[ 3 ];
  bool active;
  bool geoDirty;
  bool lightDirty;
  aeVertexData data;
  TerrainVertex* vertices;
  Chunk* next;
  Chunk* prev;
  
  Block::Type t[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  float16_t l[ kChunkSize ][ kChunkSize ][ kChunkSize ];
  TerrainIndex i[ kChunkSize ][ kChunkSize ][ kChunkSize ];
};

class aeTerrain
{
public:
  void Initialize();
  void Update();
  void Render( aeFloat3 center, const class aeShader* shader, const aeUniformList& shaderParams );

  void SetCallback( void* userdata, float ( *fn )( void*, aeFloat3 ) );
  void SetCallback( float ( *fn )( aeFloat3 ) );
  
  Block::Type GetVoxel( uint32_t x, uint32_t y, uint32_t z );
  Block::Type GetVoxel( aeFloat3 position );
  bool GetCollision( uint32_t x, uint32_t y, uint32_t z );
  bool GetCollision( aeFloat3 position );
  float16_t GetLight( uint32_t x, uint32_t y, uint32_t z );
  Chunk* GetChunk( int32_t cx, int32_t cy, int32_t cz );
  //void VoxelizeAndAddMesh(
  //  aeFloat3* vertices, uint16_t* indices,
  //  uint32_t vertexCount, uint32_t indexCount,
  //  aeFloat4x4& modelToWorld, Block::Type type );
  
  bool VoxelRaycast( aeFloat3 start, aeFloat3 ray, int32_t minSteps );
  RaycastResult RaycastFast( aeFloat3 start, aeFloat3 ray, bool allowSourceCollision );
  RaycastResult Raycast( aeFloat3 start, aeFloat3 ray );
  
  float GetBaseHeight( aeFloat3 p ) const;
  float TerrainValue( aeFloat3 p ) const;
  aeFloat3 GetSurfaceDerivative( aeFloat3 p ) const;

private:
  inline int32_t TerrainType( aeFloat3 p ) const;
  void GetChunkVerts( Chunk* chunk, TerrainVertex *vertices, TerrainIndex *indices, uint32_t* vertexCount, uint32_t* indexCount );
  void UpdateChunkLighting( Chunk* chunk );
  void UpdateChunkLightingHelper( Chunk *chunk, uint32_t x, uint32_t y, uint32_t z, float16_t l );
  Chunk* AllocChunk( aeFloat3 center, int32_t cx, int32_t cy, int32_t cz );
  void FreeChunk( Chunk* chunk );
  static void GetOffsetsFromEdge( uint32_t edgeBit, int32_t (&offsets)[ 4 ][ 3 ] );
  
  aeCompactingAllocator m_compactAlloc;
  struct Chunk **m_chunks = nullptr;
  int16_t* m_voxelCounts = nullptr;
  struct Chunk *m_activeChunks[ kMaxActiveChunks ];
  aeObjectPool<struct Chunk, kMaxLoadedChunks> m_chunkPool;
  uint32_t m_activeChunkCount = 0;

  void* m_userdata = nullptr;
  float ( *m_fn1 )( aeFloat3 ) = nullptr;
  float ( *m_fn2 )( void* userdata, aeFloat3 ) = nullptr;
  
  struct Chunk* m_headChunk = nullptr;
  struct Chunk* m_tailChunk = nullptr;
  uint32_t m_totalChunks = 0;
  uint8_t* m_chunkRawAlloc = nullptr;
  
  bool m_blockCollision[ Block::COUNT ];
  float16_t m_blockDensity[ Block::COUNT ];
  //UMAT::Noise m_noise;
  
  aeTexture2D* grassTexture = nullptr;
  aeTexture2D* rockTexture = nullptr;
  aeTexture2D* dirtTexture = nullptr;
  aeTexture2D* treeTexture = nullptr;
  aeTexture2D* leavesTexture = nullptr;

  aeTexture2D* spiralTexture = nullptr;
  aeTexture2D* m_mapWallTex = nullptr;
  aeTexture2D* m_mapFloorHeightTex = nullptr;
};

#endif
