//------------------------------------------------------------------------------
// aeMesh.h
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
#ifndef AEMESH_H
#define AEMESH_H

//------------------------------------------------------------------------------
// Headers
//------------------------------------------------------------------------------
#include "aeArray.h"
#include "aeRender.h" // aeColor

//------------------------------------------------------------------------------
// aeMesh types
//------------------------------------------------------------------------------
struct aeMeshVertex
{
  aeFloat4 position;
  aeFloat4 normal;
  aeFloat2 tex[ 4 ];
  aeColor color[ 4 ];
};

typedef uint16_t aeMeshIndex;

struct aeMeshParams
{
  uint32_t vertexCount = 0;
  const aeFloat3* positions = nullptr;
  const aeFloat3* normals = nullptr;
  uint32_t positionStride = sizeof(aeFloat3);
  uint32_t normalStride = sizeof(aeFloat3);
  
  uint32_t indexCount = 0;
  const uint16_t* indices16 = nullptr;
};

//------------------------------------------------------------------------------
// aeMesh class
//------------------------------------------------------------------------------
class aeMesh
{
public:
  // Initialization
  bool LoadFileData( const uint8_t* data, uint32_t length, const char* extension, bool skipMeshOptimization = false );
  void Load( aeMeshParams params );
  // @NOTE: Serializing aeMesh across library versions may not work. Stream will be invaldated on failure.
  void Serialize( class aeBinaryStream* stream );
  void Transform( aeFloat4x4 transform ); // Permanently pre-transform loaded verts
  void Clear();

  // Mesh data
  const aeMeshVertex& GetVertex( uint32_t idx ) const { return m_vertices[ idx ]; }
  aeMeshIndex GetIndex( uint32_t idx ) const { return m_indices[ idx ]; }
  const aeMeshVertex* GetVertices() const;
  const aeMeshIndex* GetIndices() const;
  uint32_t GetVertexCount() const;
  uint32_t GetIndexCount() const;
  aeAABB GetAABB() const { return m_aabb; }
  
  //------------------------------------------------------------------------------
  // aeMesh raycast
  //------------------------------------------------------------------------------
  struct RaycastParams
  {
    aeFloat4x4 transform = aeFloat4x4::Identity();
    
    aeFloat3 source = aeFloat3( 0.0f );
    aeFloat3 direction = aeFloat3::Down;
    float maxLength = 0.0f;
    
    uint32_t maxHits = 1;
    bool hitCounterclockwise = true;
    bool hitClockwise = false;
    
    class aeDebugRender* debug = nullptr; // Draw collision results
    aeColor debugColor = aeColor::Red();
  };

  struct RaycastResult
  {
    struct Hit
    {
      aeFloat3 position = aeFloat3( 0.0f );
      aeFloat3 normal = aeFloat3( 0.0f );
      float t = 0.0f;
    };
    
    uint32_t hitCount = 0;
    Hit hits[ 8 ];
    
    void Accumulate( const RaycastParams& params, const RaycastResult& result );
  };

  bool Raycast( const RaycastParams& params, RaycastResult* outResult ) const;
  
  //------------------------------------------------------------------------------
  // aeMesh sphere collision
  //------------------------------------------------------------------------------
  struct PushOutParams
  {
    aeFloat4x4 transform = aeFloat4x4::Identity();
    
    aeSphere sphere;
    aeFloat3 velocity = aeFloat3( 0.0f );
    
    class aeDebugRender* debug = nullptr; // Draw collision results
    aeColor debugColor = aeColor::Red();
  };

  struct PushOutResult
  {
    aeFloat3 position = aeFloat3( 0.0f );
    aeFloat3 velocity = aeFloat3( 0.0f );
    
    uint32_t hitCount = 0;
    aeFloat3 hitPos[ 8 ];
    aeFloat3 hitNorm[ 8 ];
    
    void Accumulate( const PushOutParams& params, const PushOutResult& result );
  };

  bool PushOut( const PushOutParams& params, PushOutResult* outResult ) const;

private:
  aeArray< aeMeshVertex > m_vertices;
  aeArray< aeMeshIndex > m_indices;
  aeAABB m_aabb;
};

#endif
